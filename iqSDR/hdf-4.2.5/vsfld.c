/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF.  The full HDF copyright notice, including       *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at      *
 * http://hdfgroup.org/products/hdf4/doc/Copyright.html.  If you do not have *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef RCSID
static char RcsId[] = "@(#)$Revision: 4932 $";
#endif

/* $Id: vsfld.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*****************************************************************************
* vsetf.c
* Part of the HDF VSet interface.
*

LOCAL ROUTINES

EXPORTED ROUTINES
 VSIZEOF      -- returns the machine size of a field type.
 VSsetfields  -- sets the fields in a vdata for reading or writing.
                 Truncates each field to max length of FIELDNAMELENMAX.
 VSfdefine    -- Defines a (one) new field within the vdata.
 VFnfields    -- Return the number of fields in this Vdata.
 VFfieldname  -- Return the name of the given field in this Vdata. 
 VFfieldtype  -- Return the type of the given field in this Vdata.
 VFfieldisize -- Return the internal (HDF) size of the given 
                  field in this Vdata. 
 VFfieldesize -- Return the external (local machine) size of the given 
                  field in this Vdata.
 VFfieldorder -- Return the order of the given field in this Vdata.
 VSfpack      -- pack into or unpack from a buf the values of fully
                  interlaced fields (of the entire record).

************************************************************************/

#define VSET_INTERFACE
#include "hdf.h"
#include <stdarg.h>

#ifdef _MSC_VER
#define strdup _strdup
#endif

/*
   ** ==================================================================
   ** PRIVATE data areas and routines
   ** ==================================================================
   * */

static const SYMDEF rstab[] =
{
    {"PX", DFNT_FLOAT32, SIZE_FLOAT32, 1},
    {"PY", DFNT_FLOAT32, SIZE_FLOAT32, 1},
    {"PZ", DFNT_FLOAT32, SIZE_FLOAT32, 1},

    {"IX", DFNT_INT32, SIZE_INT32, 1},
    {"IY", DFNT_INT32, SIZE_INT32, 1},
    {"IZ", DFNT_INT32, SIZE_INT32, 1},

    {"NX", DFNT_FLOAT32, SIZE_FLOAT32, 1},
    {"NY", DFNT_FLOAT32, SIZE_FLOAT32, 1},
    {"NZ", DFNT_FLOAT32, SIZE_FLOAT32, 1},

};

#define NRESERVED ( sizeof(rstab)/sizeof(SYMDEF) )

/* ------------------------------------------------------------------ */
/*
   ** sets the fields in a vdata for reading or writing
   ** RETURNS FAIL if error, and SUCCEED if ok.
   ** truncates each field to max length of  FIELDNAMELENMAX.
 */
intn 
VSsetfields(int32 vkey, const char *fields)
{
    char      **av;
    int32       ac, found;
    intn j, i;
    uint16       uj;
    uint16       order;
    int32       value;
    DYN_VREADLIST  *rlist;
    DYN_VWRITELIST *wlist;
    vsinstance_t *w;
    VDATA      *vs;
    intn       ret_value = FAIL;
    CONSTR(FUNC, "VSsetfields");

    /* check if a NULL field list is passed in, then return with
       error (bug #554) - BMR 4/30/01 */
    if (fields == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    if (HAatom_group(vkey)!=VSIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* locate vs's index in vstab */
    if (NULL == (w = (vsinstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    vs = w->vs;
    if (vs == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    if ((scanattrs(fields, &ac, &av) == FAIL) || (ac == 0))
        HGOTO_ERROR(DFE_BADFIELDS, FAIL);

     /* check number of fields limit  */
     if (ac > VSFIELDMAX)
         HGOTO_ERROR(DFE_SYMSIZE,FAIL);

    /*
     * write to an empty vdata : set the write list but do not set the
     *   read list cuz there is nothing there to read yet...
     */
    if (vs->access == 'w')
       {
        if (vs->nvertices == 0)
           {
              wlist=&(vs->wlist);
              if (wlist->n == 0) /* fields not set yet, Sept. 96. */
                /* do not re-set fields if they were already set. */
              {
                  wlist->ivsize = 0;
                  wlist->n = 0;

              /* allocate space for the internal WRITELIST structures */
                  /* Allocate buffer to hold all the int16/uint16 arrays */
                  if((wlist->bptr=HDmalloc(sizeof(uint16)*(size_t)(ac*5)))==NULL)
                      HGOTO_ERROR(DFE_NOSPACE,FAIL);

                  /* Use buffer to support the other arrays */
                  wlist->type=(int16 *)wlist->bptr;
                  wlist->off=(uint16 *)wlist->type+ac;
                  wlist->isize=wlist->off+ac;
                  wlist->order=wlist->isize+ac;
                  wlist->esize=wlist->order+ac;
                  if((wlist->name=HDmalloc(sizeof(char *)*(size_t)ac))==NULL)
                    {
                      HDfree(wlist->bptr);
                      HGOTO_ERROR(DFE_NOSPACE,FAIL);
                    } /* end if */

                  for (i = 0; i < ac; i++)
                    {
                        found = FALSE;
                    /* --- first look in the user's symbol table --- */
                        for (j = 0; j < vs->nusym; j++)
                            if (!HDstrcmp(av[i], vs->usym[j].name))
                              {
                                  found = TRUE;
                                
                                  if((wlist->name[wlist->n]=HDstrdup(vs->usym[j].name))==NULL)
                                   {
                                      HDfree(wlist->name);
                                      HDfree(wlist->bptr);
                                      HGOTO_ERROR(DFE_NOSPACE,FAIL);
                                    } /* end if */
                                  order = vs->usym[j].order;
                                  wlist->type[wlist->n] = vs->usym[j].type;
                                  wlist->order[wlist->n] = order;

                                  value = order * DFKNTsize(vs->usym[j].type | DFNT_NATIVE);
                                  if (value == FAIL)
                                      HGOTO_ERROR(DFE_BADFIELDS, FAIL);
                                  wlist->esize[wlist->n] = (uint16) value;

                                  value = order * vs->usym[j].isize;
                                  if (value > MAX_FIELD_SIZE)
                                      HGOTO_ERROR(DFE_BADFIELDS, FAIL);
                                  wlist->isize[wlist->n] = (uint16) value;

                                  value = (int32) wlist->ivsize + (int32) (wlist->isize[wlist->n]);
                                  if (value > MAX_FIELD_SIZE)
                                      HGOTO_ERROR(DFE_BADFIELDS, FAIL);
                                  wlist->ivsize = (uint16) value;

                                  wlist->n++;
                                  break;
                              }

                    /* --- now look in the reserved symbol table --- */
                        if (!found)
                          {
                              for (j = 0; j < (intn)NRESERVED; j++)
                                  if (!HDstrcmp(av[i], rstab[j].name))
                                    {
                                        found = TRUE;

                                        if((wlist->name[wlist->n]=HDstrdup(rstab[j].name))==NULL)
                                          {
                                            HDfree(wlist->name);
                                            HDfree(wlist->bptr);
                                            HGOTO_ERROR(DFE_NOSPACE,FAIL);
                                          } /* end if */
                                        order = rstab[j].order;
                                        wlist->type[wlist->n] = rstab[j].type;
                                        wlist->order[wlist->n] = order;
                                        value = order * DFKNTsize(rstab[j].type | DFNT_NATIVE);
                                        if (value == FAIL)
                                          HGOTO_ERROR(DFE_BADFIELDS, FAIL);
                                        wlist->esize[wlist->n] = (uint16) value;
                                        wlist->isize[wlist->n] = (uint16) (order * rstab[j].isize);
                                        wlist->ivsize += (uint16) (wlist->isize[wlist->n]);
                                        wlist->n++;
                                        break;
                                    }
                          }
                        if (!found)     /* field is not a defined field - error  */
                            HGOTO_ERROR(DFE_BADFIELDS, FAIL);
                    }

              /* *********************************************************** */
              /* compute and save the fields' offsets */
                   for (uj = 0, i = 0; i < wlist->n; i++)
                    {
                        wlist->off[i] = (uint16) uj;
                        uj += wlist->isize[i];
                    }

                  vs->marked = TRUE; /* mark vdata as being modified */
                  vs->new_h_sz = TRUE; /* mark vdata header size being changed */

                  HGOTO_DONE(SUCCEED); /* OK */
              } /* if wlist->n == 0 */
          } /* writing to empty vdata */
      } /* writing to vdata */

    /*
     *   No matter the access mode, if there are elements in the VData
     *      we should set the read list
     */
    if (vs->nvertices > 0)
      {
          rlist = &(vs->rlist);
          rlist->n = 0;
          if(rlist->item!=NULL)
              HDfree(rlist->item);
          rlist->item=NULL;

          /* Allocate enough space for the read list */
          if((rlist->item=(intn *)HDmalloc(sizeof(intn)*(size_t)(ac)))==NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
          for (i = 0; i < ac; i++)
            {
                found = FALSE;
                for (j = 0; j < vs->wlist.n; j++)
                    if (!HDstrcmp(av[i], vs->wlist.name[j]))
                      {     /*  see if field exist */
                          found = TRUE;

                          rlist->item[rlist->n] = j;    /* save as index into wlist->name */
                          rlist->n++;
                          break;
                      }
                if (!found)     /* field does not exist - error */
                    HGOTO_ERROR(DFE_BADFIELDS, FAIL);
            }
        ret_value=SUCCEED;
      }     /* setting read list */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* VSsetfields */

/* ------------------------------------------------------------------ */
/*
   ** defines a (one) new field within the vdata
   ** return FAIL if error
   ** return SUCCEED if success
 */
intn 
VSfdefine(int32 vkey, const char *field, int32 localtype, int32 order)
{
    char      **av;
    int32       ac;
    int16       isize, replacesym;
    intn        usymid;
    intn j;
    vsinstance_t *w;
    VDATA      *vs;
    intn       ret_value = SUCCEED;
    CONSTR(FUNC, "VSfdefine");

    if (HAatom_group(vkey)!=VSIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* locate vs's index in vstab */
    if (NULL == (w = (vsinstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    vs = w->vs;
    if ((vs == NULL) || (scanattrs(field, &ac, &av) == FAIL) || (ac != 1))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* The order of a variable is stored in a 16-bit number, so have to keep this limit -QAK */
    if (order < 1 || order > MAX_ORDER)
        HGOTO_ERROR(DFE_BADORDER, FAIL);
    /* don't forget to check for field size limit */
    isize = (int16)DFKNTsize(localtype);
    if ( (isize == FAIL) || (isize * order) > MAX_FIELD_SIZE )
        HGOTO_ERROR(DFE_BADFIELDS, FAIL);

    /*
     ** check for any duplicates
     */
#ifdef OLD_WAY
/* It's OK to over-ride pre-defined symbols with the user's own -QAK */
    /* --- first look in the reserved symbol table --- */
    for (j = 0; j < NRESERVED; j++)
        if (!HDstrcmp(av[0], rstab[j].name))
          {
              if (localtype != rstab[j].type && order != rstab[j].order)
                  break;
          }
#endif /* OLD_WAY */

    /* --- then look in the user's symbol table --- */
    for (replacesym = 0, j = 0; j < vs->nusym; j++)
        if (!HDstrcmp(av[0], vs->usym[j].name))
          {
              if (localtype != rstab[j].type && order != rstab[j].order)
                {
                    replacesym = 1;
                    break;
                }
          }

    if (replacesym)
        usymid = j;     /* new definition will replace old at this index */
    else
      {
          SYMDEF *tmp_sym=vs->usym;  /* temp. pointer to the new symdef list */

          usymid = (intn)vs->nusym;
          /* use temporary pointer in case we run out of memory, so we don't loose original list */
          if (tmp_sym == NULL)
           {
             if((tmp_sym=(SYMDEF *) HDmalloc(sizeof(SYMDEF)*(size_t)(usymid+1)))==NULL)
              HGOTO_ERROR(DFE_NOSPACE,FAIL);
            }
          else
            {
              if((tmp_sym=(SYMDEF *) HDrealloc(tmp_sym,sizeof(SYMDEF)*(size_t)(usymid+1)))==NULL)
                  HGOTO_ERROR(DFE_NOSPACE,FAIL);
            }
          vs->usym=tmp_sym;
      } /* end else */

      vs->usym[usymid].isize = (uint16) isize; /* ok, because number
          type sizes are smaller than max(int16) */

    /* Copy the symbol [field] information */
    if ((vs->usym[usymid].name = (char *) HDstrdup(av[0]) ) == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);
    vs->usym[usymid].type = (int16) localtype;
    vs->usym[usymid].order = (uint16) order;

    /* increment vs->nusym only if no user field has been redefined */
    if (!replacesym)
        vs->nusym++;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}	/* VSfdefine */

/* ------------------------------ VFnfields ------------------------------- */
/*
   Return the number of fields in this Vdata
   Return FAIL on failure
 */
int32
VFnfields(int32 vkey)
{
    vsinstance_t *w;
    VDATA      *vs;
    int32      ret_value = SUCCEED;
    CONSTR(FUNC, "VFnfields");

    if (HAatom_group(vkey)!=VSIDGROUP)
        HGOTO_ERROR(DFE_ARGS,FAIL);

    /* locate vs's index in vstab */
    if (NULL == (w = (vsinstance_t *) HAatom_object(vkey)))
      HGOTO_ERROR(DFE_NOVS,FAIL);

    vs = w->vs;
    if ((vs == NULL) || (vs->otag != VSDESCTAG))
      HGOTO_ERROR(DFE_ARGS,FAIL);

    ret_value = ((int32) vs->wlist.n);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}	/* VFnfields */

/* ----------------------------- VFfieldname ------------------------------ */
/*
   Return the name of the given field in this Vdata.
   This is just a pointer to the string in local memory and is only guarenteed
   to be valid as long as we are VSattached() to this Vdata

   Return NULL on failure
 */
char       *
VFfieldname(int32 vkey, int32 index)
{
    vsinstance_t *w;
    VDATA      *vs;
    char       *ret_value = NULL; /* FAIL */
    CONSTR(FUNC, "VFfieldname");

    if (HAatom_group(vkey)!=VSIDGROUP)
      HGOTO_ERROR(DFE_ARGS,NULL);

    /* locate vs's index in vstab */
    if (NULL == (w = (vsinstance_t *) HAatom_object(vkey)))
      HGOTO_ERROR(DFE_NOVS,NULL);

    vs = w->vs;
    if ((vs == NULL) || (vs->otag != VSDESCTAG))
      HGOTO_ERROR(DFE_ARGS,NULL);

    if (vs->wlist.n == 0)
      HGOTO_ERROR(DFE_BADFIELDS,NULL);

    ret_value = ((char *) vs->wlist.name[index]);

done:
  if(ret_value == NULL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* VFfieldname */

/* ----------------------------- VFfieldtype ------------------------------ */
/*
   Return the type of the given field in this Vdata.

   Return FAIL on failure
 */
int32
VFfieldtype(int32 vkey, int32 index)
{
    vsinstance_t *w;
    VDATA      *vs;
    int32      ret_value = SUCCEED;
    CONSTR(FUNC, "VFfeildtype");

    if (HAatom_group(vkey)!=VSIDGROUP)
      HGOTO_ERROR(DFE_ARGS,FAIL);

    /* locate vs's index in vstab */
    if (NULL == (w = (vsinstance_t *) HAatom_object(vkey)))
      HGOTO_ERROR(DFE_NOVS,FAIL);

    vs = w->vs;
    if ((vs == NULL) || (vs->otag != VSDESCTAG))
      HGOTO_ERROR(DFE_ARGS,FAIL);

    if (vs->wlist.n == 0)
      HGOTO_ERROR(DFE_BADFIELDS,FAIL);

    ret_value = ((int32) vs->wlist.type[index]);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* VFfieldtype */

/* ----------------------------- VFfieldisize ------------------------------ */
/*
   Return the internal size of the given field in this Vdata.
   (internal to HDF file, see VWRITELIST in vg.h. 4/3/96) 

   Return FAIL on failure
 */
int32
VFfieldisize(int32 vkey, int32 index)
{
    vsinstance_t *w;
    VDATA      *vs;
    int32      ret_value = SUCCEED;
    CONSTR(FUNC, "VFfieldisize");

    if (HAatom_group(vkey)!=VSIDGROUP)
      HGOTO_ERROR(DFE_ARGS,FAIL);

    /* locate vs's index in vstab */
    if (NULL == (w = (vsinstance_t *) HAatom_object(vkey)))
      HGOTO_ERROR(DFE_NOVS,FAIL);

    vs = w->vs;
    if ((vs == NULL) || (vs->otag != VSDESCTAG))
      HGOTO_ERROR(DFE_ARGS,FAIL);

    if (vs->wlist.n == 0)
      HGOTO_ERROR(DFE_BADFIELDS,FAIL);

    ret_value = ((int32) vs->wlist.isize[index]);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* VFfieldisize */

/* ----------------------------- VFfieldesize ------------------------------ */
/*
   Return the external size of the given field in this Vdata.
   (external to HDF file, see VWRITELIST in vg.h. 4/3/96) 

   Return FAIL on failure
 */
int32
VFfieldesize(int32 vkey, int32 index)
{
    vsinstance_t *w;
    VDATA      *vs;
    int32      ret_value = SUCCEED;
    CONSTR(FUNC, "VFfieldesize");

    if (HAatom_group(vkey)!=VSIDGROUP)
      HGOTO_ERROR(DFE_ARGS,FAIL);

    /* locate vs's index in vstab */
    if (NULL == (w = (vsinstance_t *) HAatom_object(vkey)))
      HGOTO_ERROR(DFE_NOVS,FAIL);

    vs = w->vs;
    if ((vs == NULL) || (vs->otag != VSDESCTAG))
      HGOTO_ERROR(DFE_ARGS,FAIL);

    if (vs->wlist.n == 0)
      HGOTO_ERROR(DFE_BADFIELDS,FAIL);

    ret_value = ((int32) vs->wlist.esize[index]);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* VFfieldesize */

/* ----------------------------- VFfieldorder ------------------------------ */
/*
   Return the order of the given field in this Vdata.

   Return FAIL on failure
 */
int32
VFfieldorder(int32 vkey, int32 index)
{
    vsinstance_t *w;
    VDATA      *vs;
    int32      ret_value = SUCCEED;
    CONSTR(FUNC, "VFfieldorder");

    if (HAatom_group(vkey)!=VSIDGROUP)
      HGOTO_ERROR(DFE_ARGS,FAIL);

    /* locate vs's index in vstab */
    if (NULL == (w = (vsinstance_t *) HAatom_object(vkey)))
      HGOTO_ERROR(DFE_NOVS,FAIL);

    vs = w->vs;
    if ((vs == NULL) || (vs->otag != VSDESCTAG))
      HGOTO_ERROR(DFE_ARGS,FAIL);

    if (vs->wlist.n == 0)
      HGOTO_ERROR(DFE_BADFIELDS,FAIL);

    ret_value = ((int32) vs->wlist.order[index]);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}	/* VFfieldorder */

/* -------------------------- VSsetexternalfile --------------------------- */
/*

 NAME
	VSsetexternalfile -- store Vdat info in a separate file
 USAGE
	int32 VSsetexternalfile(id, filename, offset)
        int32   id;                  IN: vdata ID
        char  * filename;            IN: name of external file
        int32   offset;              IN: offset in external file
 RETURNS
        Return SUCCEED or FAIL

 DESCRIPTION
        Specify that the actual data for this Vdata be stored in a 
        separate file (an "external file" in HDF terms).

        Only the data (as in VSwrite()) will be stored externally.  
        Attributes and such will still be in the main file

        IMPORTANT:  It is the user's responsibility to see that the 
        separate files are transported with the main file.
FORTRAN
	vsfsextf

--------------------------------------------------------------------------- */

intn VSsetexternalfile(int32 vkey, const char *filename, int32 offset)
{
    CONSTR(FUNC, "VSsetexternalfile");
    int32       ret_value = SUCCEED;

    vsinstance_t *w;
    VDATA      *vs;
    intn       status;

    if(!filename || offset < 0)
	HGOTO_ERROR(DFE_ARGS, FAIL);

    if (HAatom_group(vkey)!=VSIDGROUP)
	    HGOTO_ERROR(DFE_ARGS, FAIL);

    /* locate vs's index in vstab */
    if (NULL == (w = (vsinstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    vs = w->vs;
    if (vs->access != 'w')
        HGOTO_ERROR(DFE_BADACC, FAIL);

    if (FAIL == vexistvs(vs->f, vs->oref))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    if(!w->ref)
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* no need to give a length since the element already exists */
    /* The Data portion of a Vdata is always stored in linked blocks. */
    /* So, use the special tag */
    status = (intn)HXcreate(vs->f, (uint16)VSDATATAG, (uint16) w->ref,
		      filename, offset, (int32)0);
    if(status != FAIL)
      {
        if((vs->aid != 0) && (vs->aid != FAIL))
            Hendaccess(vs->aid);
        vs->aid = status;
      }
    else
        ret_value = FAIL;

done:
    if(ret_value == FAIL)
    { /* Error condition cleanup */

    } /* end if */

    /* Normal function cleanup */

    return ret_value;
} /* VSsetexternalfile */

/*----------------------------------------------------------------- 
NAME
    VSfpack -- pack into or unpack from a buf the values of fully
              interlaced fields.
USAGE
    intn VSfpack(int32 vsid, intn packtype, char *fields_in_buf,
         void * buf, intn bufsz, intn n_records, char *fields, void * fldbufpt[])
    int32 vsid; IN: vdata id.
    intn packtype; IN: 
         _HDF_VSPACK(0) -- pack field values into vdata buf;
         _HDF_VSUNPACK(1) -- unpack vdata value into filed bufs.
    char *fields_in_buf; IN: 
         fields in buf to write to or read from vdata. NULL 
         stands for all fields in the vdata.
    void * buf; IN: buffer for vdata values.
    intn bufsz; IN: buf size in byte.
    intn n_records; IN: number of records to pack or unpack.
    char *fields; IN: 
         names of the fields to be pack/unpack. It may be a 
         subset of the fields_in_buf. NULL stands for all 
         fields in buf. 
    void * fldbufpt[]; IN: array of pointers to field buffers.
RETURNS
    SUCCEED(0) on success; FIAL(-1) otherwise.
DESCRIPTION
    1. This pack/unpack routine is convenient for users. It also 
       serves for FORTRAN programs to pack/unpack numeric and 
       non-numeric fields.
    2. The caller should supply correct number of field buffers, 
       which should agree with the number of fields to be 
       packed/unpacked. 
    3. For packtype = _HDF_VSPACK, the calling sequence should be:
          VSsetfields,  VSfpack, and VSwrite;
       For packtype = _HDF_VSUNPACK, the calling sequence should be:
          VSsetfields, VSread and VSfpack.
*/

/*---------------------------------------------------------*/

intn VSfpack(int32 vsid, intn packtype, const char *fields_in_buf,
         void * buf, intn bufsz, intn n_records, const char *fields, void * fldbufpt[])
{
    CONSTR(FUNC, "VSfpack");

    int32 ac;
    char **av, *s;
    uint8 *bufp = (uint8 *)buf;
    uint8 **fbufps=NULL;
    int32 b_rec_size, *fmsizes=NULL, *foffs=NULL; 
    intn i, j, found, ret_value = SUCCEED;
    vsinstance_t *wi;
    VDATA *vs;
    DYN_VWRITELIST *w;
    struct blist_t  { /* contains info about fields in buf */
       intn n;       /* number of fields in buf     */
       int32 *idx;  /* index of buf fields in vdata */
       int32 *offs; /* offset of buf fields in buf */
    }  blist;

    if (HAatom_group(vsid)!=VSIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);
    /* locate vs' index in vgtab */
    if (NULL == (wi = (vsinstance_t *) HAatom_object(vsid)))
        HGOTO_ERROR(DFE_NOVS, FAIL);
    vs = wi->vs;
    if (vs == NULL) 
        HGOTO_ERROR(DFE_NOVS, FAIL); 
    w = &vs->wlist;
         /* build blist based on info in w */
    if (fields_in_buf == NULL)   
         ac = w->n;
    else    {           /* build our own blist */
       if (scanattrs(fields_in_buf, &ac, &av) == FAIL)
           HGOTO_ERROR(DFE_BADFIELDS, FAIL);
       if ((av == NULL) || (ac < 1))
            HGOTO_ERROR(DFE_ARGS, FAIL);
    } 
    blist.n = ac;
    blist.idx = (int32 *)HDmalloc((size_t)ac * sizeof(int32));
    blist.offs = (int32 *)HDmalloc((size_t)ac * sizeof(int32));
    if ((blist.idx == NULL) || (blist.offs == NULL))
         HGOTO_ERROR(DFE_NOSPACE, FAIL);
      /* fill arrays blist.msizes and blist.offs; calculate
           buf record size */
    b_rec_size = 0;
    if (fields_in_buf != NULL) 
        /* a subset of vdata fields are contained in buf */
        for (i=0; i<ac; i++) {
           /* look for the field in vdata fields */
           found = 0;
           s = av[i];
           for (j=0; j< w->n; j++)  {
#ifdef VDATA_FIELDS_ALL_UPPER
               if (matchnocase(s, w->name[j]))
               {
                    found = 1;
                    break;
                }
#else
                if (HDstrcmp(s, w->name[j]) == 0)
                {
                     found = 1;
                     break;
                 }
#endif /* VDATA_FIELDS_ALL_UPPER */
            }     /* for j */
            if (!found)
                HGOTO_ERROR(DFE_BADFIELDS, FAIL);
               /* get field info */
            blist.idx[i] = j;
            blist.offs[i] =
               (i==0? 0 : blist.offs[i-1] + w->esize[blist.idx[i-1]]);
            b_rec_size += w->esize[j];
        }  /* for i */
    else  /* buf contains all vdata fields  */
       for (i=0; i< ac; i++) {
           blist.idx[i] = i;
           blist.offs[i] =
                (i==0? 0 : blist.offs[i-1] + w->esize[i-1]);
           b_rec_size += w->esize[i];
       }
 
       /* check bufsz */
    if (bufsz < b_rec_size * n_records)
        HGOTO_ERROR(DFE_NOTENOUGH, FAIL);
    if (fields != NULL) { /* convert field names into tokens. */
        if (scanattrs(fields, &ac, &av) == FAIL )
            HGOTO_ERROR(DFE_BADFIELDS, FAIL);
        if ((av == NULL) || (ac < 1))
            HGOTO_ERROR(DFE_ARGS, FAIL);
    }
    else 
        ac = blist.n;
       /* fill array of fmsizes, foffs, fbufps */
    if ((fmsizes=(int32 *)HDmalloc((size_t)ac*sizeof(int32))) == NULL) 
         HGOTO_ERROR(DFE_NOSPACE, FAIL);
    if ((foffs = (int32 *)HDmalloc((size_t)ac*sizeof(int32))) == NULL) 
         HGOTO_ERROR(DFE_NOSPACE, FAIL);
    if ((fbufps=(uint8 **)HDmalloc((size_t)ac*sizeof(uint8 *))) == NULL) 
         HGOTO_ERROR(DFE_NOSPACE, FAIL);
    if (fields != NULL)  { /* a subset of buf fields */
        for (i=0; i<ac; i++) {
           /* find field info */
           found = 0;
           s = av[i];
           for (j=0; j< blist.n; j++)  {
#ifdef VDATA_FIELDS_ALL_UPPER
               if (matchnocase(s, w->name[blist.idx[j]]))
               {
                    found = 1;
                    break;
                }
#else
                if (HDstrcmp(s, w->name[blist.idx[j]]) == 0)
                {
                     found = 1;
                     break;
                 }
#endif /* VDATA_FIELDS_ALL_UPPER */
            }     /* for */
            if (!found)
                HGOTO_ERROR(DFE_BADFIELDS, FAIL); 
            fmsizes[i] = (int32)w->esize[blist.idx[j]];
            foffs[i] = blist.offs[j];
	    fbufps[i] = fldbufpt[i];
            if (fbufps[i] == NULL)  
                HGOTO_ERROR(DFE_BADPTR,FAIL);  
        }
    }
    else
    {     /* all buf fields */
        for (i=0; i < ac; i++)   {
            fmsizes[i] = (int32)w->esize[blist.idx[i]];
            foffs[i] = blist.offs[i];
	    fbufps[i] = fldbufpt[i];
            if (fbufps[i] == NULL)  
                HGOTO_ERROR(DFE_BADPTR,FAIL); 
        }
     }
    if (packtype == _HDF_VSPACK ) {
        /* memory copy fields data to vdata buf */    
        for (i=0; i<n_records; i++)   {
            for (j=0; j<ac; j++)       {
                HDmemcpy(bufp + foffs[j], fbufps[j], fmsizes[j]);
                fbufps[j] += fmsizes[j];
            }
            bufp += b_rec_size;
        }
    }
    else  { /* unpack from buf to fields */
        for (i=0; i<n_records; i++)   {
            for (j=0; j<ac; j++)       {
                HDmemcpy(fbufps[j], bufp + foffs[j], fmsizes[j]);
                fbufps[j] += fmsizes[j];
            }
            bufp += b_rec_size;
        }
    }

done:
    if (ret_value == FAIL) {
    }
    if (blist.idx != NULL)
       HDfree(blist.idx);
    if (blist.offs != NULL)
       HDfree(blist.offs);
    if (fmsizes != NULL)
       HDfree(fmsizes);
    if (foffs != NULL)
       HDfree(foffs);
    if (fbufps != NULL)
       HDfree(fbufps);

  return ret_value;
}       /* VSfpack */
/*--------------------------------------------------------- */
