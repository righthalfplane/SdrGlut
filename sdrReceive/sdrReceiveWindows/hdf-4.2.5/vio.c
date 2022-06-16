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

/* $Id: vio.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*****************************************************************************
 file - vio.c

 Part of the HDF Vset interface.

 VDATAs are handled by routines in here.
 PRIVATE functions manipulate vsdir and are used only within this file.
 PRIVATE data structures in here pertain to vdatas in vsdir only.


LOCAL ROUTINES
 VSIget_vdata_node      -- allocate a new VDATA record
 VSIrelease_vdata_node  -- Releases a vdata node
 VSIget_vsinstance_node -- allocate a new vsinstance_t record
 VSIrelease_vsinstance_node -- Releases a vsinstance node

LIBRARY PRIVATE ROUTINES
 VSPhshutdown  --  shutdown the Vset interface

EXPORTED ROUTINES
 vinst         -- Looks thru vstab for vsid and return the addr of the vdata 
                   instance where vsid is found.
 vexistvs      -- Tests if a vdata with id vsid is in the file's vstab.
 vpackvs       -- Packs a VDATA structure into a compact form suitable for 
                   storing in the HDF file.
 vunpackvs     -- Convert a packed form(from HDF file) to a VDATA structure.
                   This routine will also initalize the VDATA structure as 
                   much as it can.
 vsdestroynode -- Frees B-Tree nodes.
 VSPgetinfo    -- Read in the "header" information about the Vdata.
 VSattach      -- Attaches/Creates a new vs in vg depending on "vsid" value.
 VSdetach      -- Detaches vs from vstab.
 VSappendable  -- Make it possible to append unlimitedly to an existing VData.
 VSgetid       -- Returns the id of the next VDATA from the file.
 VSQuerytag    -- Return the 'otag' of the given Vdata.
 VSQueryref    -- Return the ref of the given Vdata.
 vswritelist   -- Return the writelist of a Vdata.
 VSgetversion  -- Return the version number of a Vdata.
 VSdelete      -- Remove a Vdata from its file.  This function will both 
                   remove the Vdata from the internal Vset data structures 
                   as well as from the file.

 NOTE: Another pass needs to made through this file to update some of
       the comments about certain sections of the code. -GV 9/8/97

*************************************************************************/

#define VSET_INTERFACE
#include "hdf.h"

/* Private Function Prototypes */
PRIVATE intn vunpackvs
            (VDATA * vs, uint8 buf[], int32 len);

/* Temporary buffer for I/O */
PRIVATE uint32 Vhbufsize = 0;
PRIVATE uint8 *Vhbuf = NULL;

/* Pointers to the VDATA & vsinstance node free lists */
static VDATA *vdata_free_list=NULL;
static vsinstance_t *vsinstance_free_list=NULL;

/* vpackvs is prototyped in vg.h since vconv.c needs to call it */

/*******************************************************************************
 NAME
    VSIget_vdata_node -- allocate a new VDATA record

 DESCRIPTION
    Return an pointer to a new VDATA to use for a new VSID.

 RETURNS
    returns VDATA record pointer or NULL if failed.

*******************************************************************************/
VDATA *
VSIget_vdata_node(void)
{
    VDATA   *ret_value = NULL;
    CONSTR(FUNC, "VSIget_vdata_node");

    /* clear error stack */
    HEclear();

    /* Grab from free list if possible */
    if(vdata_free_list != NULL)
      {
        ret_value       = vdata_free_list;
        vdata_free_list = vdata_free_list->next;
      } 
    else /* allocate a new node */
      {
        if((ret_value=(VDATA *)HDmalloc(sizeof(VDATA)))==NULL)
            HGOTO_ERROR(DFE_NOSPACE, NULL);
      } /* end else */

    /* Initialize to zeros */
    HDmemset(ret_value,0,sizeof(VDATA));

done:
  if(ret_value == NULL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return(ret_value);
}	/* VSIget_vdata_node */

/******************************************************************************
 NAME
     VSIrelease_vdata_node -- Releases a vdata node

 DESCRIPTION
    Puts an VDATA node into the free list

 RETURNS
    No return value

*******************************************************************************/
void 
VSIrelease_vdata_node(VDATA *vs /* IN: vdata to release */)
{
#ifdef LATER
    CONSTR(FUNC, "VSIrelease_vdata_node");	/* for HERROR */
#endif /* LATER */

    /* Insert the atom at the beginning of the free list */
    vs->next        = vdata_free_list;
    vdata_free_list = vs;

}   /* end VSIrelease_vdata_node() */

/*******************************************************************************
 NAME
    VSIget_vsinstance_node -- allocate a new vsinstance_t record

 DESCRIPTION
    Return an pointer to a new VDATA to use for a new VSID.

 RETURNS
    returns vsinstance_t pointer or NULL if failed.
*******************************************************************************/
vsinstance_t *
VSIget_vsinstance_node(void)
{
    vsinstance_t   *ret_value = NULL;
    CONSTR(FUNC, "VSIget_vsinstance_node");

    /* clear error stack */
    HEclear();

    /* Grab from free list if possible */
    if(vsinstance_free_list != NULL)
      {
        ret_value            = vsinstance_free_list;
        vsinstance_free_list = vsinstance_free_list->next;
      }  
    else /* allocate a new vsinstance record */
      {
        if((ret_value=(vsinstance_t *)HDmalloc(sizeof(vsinstance_t)))==NULL)
            HGOTO_ERROR(DFE_NOSPACE, NULL);
      } /* end else */

    /* Initialize to zeros */
    HDmemset(ret_value,0,sizeof(vsinstance_t));

done:
  if(ret_value == NULL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return(ret_value);
}	/* VSIget_vsinstance_node */

/******************************************************************************
 NAME
     VSIrelease_vsinstance_node -- Releases a vsinstance node

 DESCRIPTION
    Puts a vsinstance node into the free list

 RETURNS
    No return value

*******************************************************************************/
void 
VSIrelease_vsinstance_node(vsinstance_t *vs /* IN: vinstance node to release */)
{
#ifdef LATER
    CONSTR(FUNC, "VSIrelease_vsinstance_node");	/* for HERROR */
#endif /* LATER */

    /* Insert the atom at the beginning of the free list */
    vs->next             = vsinstance_free_list;
    vsinstance_free_list = vs;

}   /* end VSIrelease_vsinstance_node() */

/*******************************************************************************
 NAME
    VSPhshutdown  -  shutdown the Vset interface 

 DESCRIPTION
    For completeness, when the VSet interface is shut-down, free the Vhbuf.

 RETURNS
    Returns SUCCEED/FAIL

 COMMENTS, BUGS, ASSUMPTIONS
    Should only ever be called by the "atexit" function HDFend
*******************************************************************************/
intn 
VSPhshutdown(void)
{
    intn  ret_value = SUCCEED;
    VDATA *v = NULL;
    vsinstance_t *vs = NULL;

    /* Release the vdata free-list if it exists */
    if(vdata_free_list != NULL)
      {
        while(vdata_free_list != NULL)
          {
            v               = vdata_free_list;
            vdata_free_list = vdata_free_list->next;
            v->next = NULL; 
            HDfree(v);
          } /* end while */
      } /* end if */

    /* Release the vsinstance free-list if it exists */
    if(vsinstance_free_list != NULL)
      {
        while(vsinstance_free_list != NULL)
          {
            vs                   = vsinstance_free_list;
            vsinstance_free_list = vsinstance_free_list->next;
            vs->next = NULL; 
            HDfree(vs);
          } /* end while */
      } /* end if */

    /* free buffer */
    if(Vhbuf != NULL)
      {
          HDfree(Vhbuf);
          Vhbuf = NULL;
          Vhbufsize = 0;
      } /* end if */

    /* free the parsing buffer */
    ret_value = VPparse_shutdown();

  return ret_value;
} /* end VSPhshutdown() */

/*******************************************************************************
NAME
  vsint

DESCRIPTION
  Looks thru vstab for vsid and return the addr of the vdata instance
  where vsid is found.

RETURNS
  RETURNS NULL if error or not found.
  RETURNS vsinstance_t pointer if ok.
   
*******************************************************************************/
vsinstance_t *
vsinst(HFILEID f,  /* IN: File handle */
       uint16 vsid /* IN: vdata id i.e. ref */)
{
    void *      *t = NULL;
    vfile_t    *vf = NULL;
    int32       key;
    vsinstance_t *ret_value = NULL; /* FAIL */
    CONSTR(FUNC, "vsinstance");

    /* clear error stack */
    HEclear();

    /* Get Vdata file record? */
    if (NULL == (vf = Get_vfile(f)))
        HGOTO_ERROR(DFE_FNF, NULL);

    /* tbbtdfind returns a pointer to the vsinstance_t pointer */
    key = (int32)vsid;
    if (( t = (void * *) tbbtdfind(vf->vstree, &key, NULL))== NULL)
        HGOTO_ERROR(DFE_NOMATCH,NULL);

    /* return the actual vsinstance_t ptr */
    ret_value = ((vsinstance_t *) * t);  

done:
  if(ret_value == NULL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* vsinst */

/*******************************************************************************
NAME
  vexistvs

DESCRIPTION
  Tests if a vdata with id vsid is in the file's vstab.

RETURNS
  returns FAIL if not found,
  returns TRUE if found.
*******************************************************************************/
int32
vexistvs(HFILEID f,  /* IN: file handle */
         uint16 vsid /* IN: vdata id i.e. ref */)
{
  return ((NULL==vsinst(f, vsid)) ? FAIL : TRUE);
}   /* vexistvs */

/* ------------------------------------------------------------------ */
/*
   The following 2 routines, vpackvs and vunpackvs, packs and unpacks
   a VDATA structure into a compact form suitable for storing in the HDF file.
 */

/****
CONTENTS of VS stored in HDF file with tag VSDESCTAG:
    int16       interlace
    int32       nvertices
    uint16       vsize
    int16       nfields

    uint16       isize[1..nfields] (internal size of each field)
    uint16       off[1..nfields] (internal offset of each field)
    char        fname[1..nfields][FIELDNAMELENMAX]
    char        vsname[VSNAMELENMAX]
    char        vsclass[VSNAMELENMAX]
    uint16      extag, exref
    uint32      flags (for vset version 4 or higher )
    int32       nattrs (if bit0 of flags is set)
    uint16      atags[1..nattrs], arefs[1..nattrs]
    int16       version, more
****/

/* ------------------------------- vpackvs ----------------------------------- */
/*
   The following 2 PRIVATE routines, vpackvs and vunpackvs, packs and unpacks
   a VDATA structure into a compact form suitable for storing in the HDF file.
 */

/****
CONTENTS of VS stored in HDF file with tag DFTAG_VH:
    int16       interlace
    int32       nvertices
    uint16      vsize
    int16       nfields

    uint16      isize[1..nfields] (internal size of each field)
    uint16      off[1..nfields] (internal offset of each field)
    char        fname[1..nfields][FIELDNAMELENMAX]
    char        vsname[VSNAMELENMAX]
    char        vsclass[VSNAMELENMAX]

****/

/*******************************************************************************
NAME
  vpackvs

DESCRIPTION
   convert a vs struct to a vspack suitable for storage in a HDF file

RETURNS
   always SUCCEED?

*******************************************************************************/
intn
vpackvs(VDATA * vs,  /* IN/OUT: */
        uint8 buf[], /* IN: */
        int32 *size  /* OUT: */)
{
#ifdef LATER
    CONSTR(FUNC, "vpackvg");
#endif /* LATER */

    int32 i;
    int16 slen;
    uint8 *bb = NULL;
    intn  ret_value = SUCCEED;

    /* clear error stack */
    HEclear();

    bb = &buf[0];

    /* save the interlace */
    INT16ENCODE(bb, vs->interlace);

    /* save nvertices */
    INT32ENCODE(bb, vs->nvertices);

    /* save ivsize */
    UINT16ENCODE(bb, vs->wlist.ivsize);

    /* save nfields */
    INT16ENCODE(bb, vs->wlist.n);

    /* Skip over all the "wheel-spinning" for 0-field vdatas */
    if(vs->wlist.n>0)
      { 
        for (i = 0; i < vs->wlist.n; i++)   /* save the type */
            INT16ENCODE(bb, vs->wlist.type[i]);

        /* save the isize */
        for (i = 0; i < vs->wlist.n; i++)
            UINT16ENCODE(bb, vs->wlist.isize[i]);

        for (i = 0; i < vs->wlist.n; i++)   /* save the offset */
            UINT16ENCODE(bb, vs->wlist.off[i]);

        for (i = 0; i < vs->wlist.n; i++)   /* save the order */
            UINT16ENCODE(bb, vs->wlist.order[i]);

        /* save each field length and name - omit the null */
        for (i = 0; i < vs->wlist.n; i++)
          {
              slen = (int16)HDstrlen(vs->wlist.name[i]);
              INT16ENCODE(bb, slen);

              HDstrcpy((char *) bb, vs->wlist.name[i]);
              bb += slen;
          }
      } /* end if */

    /* save the vsnamelen and vsname - omit the null */
    slen = (int16)HDstrlen(vs->vsname);
    INT16ENCODE(bb, slen);

    HDstrcpy((char *) bb, vs->vsname);
    bb += slen;

    /* save the vsclasslen and vsclass- omit the null */
    slen = (int16)HDstrlen(vs->vsclass);
    INT16ENCODE(bb, slen);

    HDstrcpy((char *) bb, vs->vsclass);
    bb += slen;

    /* save the expansion tag/ref pair */
    UINT16ENCODE(bb, vs->extag);
    UINT16ENCODE(bb, vs->exref);

    /* save the version field - to version_3 now if no new feature */
    INT16ENCODE(bb, vs->version);

    /* save the 'more' field - NONE now */
    INT16ENCODE(bb, vs->more);

    if (vs->flags != 0)   
      {  /* save the flags and update version # */
       UINT32ENCODE(bb, vs->flags);

       if (vs->flags & VS_ATTR_SET) 
         { /* save attributes */
          INT32ENCODE(bb, vs->nattrs);

          for (i = 0; i < vs->nattrs; i++)  
            {
              INT32ENCODE(bb, vs->alist[i].findex);
              UINT16ENCODE(bb, vs->alist[i].atag);
              UINT16ENCODE(bb, vs->alist[i].aref);
            }   /* for */
         }  /* attr set */ 
      }     /* flags set */   

   /* duplicate 'version' and 'more' - for new version of libraries */
   /* see the documentation in vattr.c */
    INT16ENCODE(bb, vs->version);

    /* save the 'more' field - NONE now */
    INT16ENCODE(bb, vs->more);

    *size = (int32) (bb - buf) + 1;
    *bb = 0;

#ifdef LATER
done:
    if (ret_value == FAIL)
    { /* Error condition cleanup */

    } /* end if */
#endif /* LATER */

  /* Normal function cleanup */
  return ret_value;
}   /* vpackvs */

/*******************************************************************************
NAME
  vunpackvs

DESCRIPTION
   Convert a packed form(from HDF file)  to a VDATA structure.
   This routine will also initalize the VDATA structure as much as it can.

RETURNS
   SUCCEED / FAIL

*******************************************************************************/
PRIVATE     intn
vunpackvs(VDATA * vs, /* IN/OUT: */
          uint8 buf[],/* IN: */ 
          int32 len   /* IN: */)
{
    uint8      *bb = NULL;
    int32       i;
    int16       int16var, temp;
    uint16      uint16var;
    int32       ret_value = SUCCEED;
    CONSTR(FUNC, "vunpackvs");

    /* clear error stack */
    HEclear();

    /* '5' is a magic number, the exact amount of space for 2 uint16's */
    /* the magic number _should_ be '4', but the size of the Vdata */
    /* information is incorrectly calculated (in vpackvs() above) when the */
    /* info is written to the file and it's too late to change it now :- ( */
    /* get version number first -- this is different from version 3
       vdata interface */
    bb = &buf[len - 5];
    UINT16DECODE(bb, uint16var); /* retrieve the vg's version field */

    vs->version = (int16)uint16var;
    UINT16DECODE(bb, uint16var);     /* retrieve the vg's more field */

    vs->more = (int16)uint16var;
    bb = &buf[0];

    if (vs->version <= 4)   
      { 
          /* retrieve interlace */
          INT16DECODE(bb, vs->interlace);

          /* retrieve nvertices */
          INT32DECODE(bb, vs->nvertices);

          /* retrieve tore ivsize */
          UINT16DECODE(bb, vs->wlist.ivsize);

          /* retrieve nfields */
          INT16DECODE(bb, int16var);
          vs->wlist.n = (intn)int16var;

          if(vs->wlist.n==0)
            {   /* Special case for Vdata with 0 fields defined */
              /* Initialize buffer to NULL & carry over to other arrays */
              vs->wlist.bptr = NULL;
              vs->wlist.type = NULL;
              vs->wlist.off = NULL;
              vs->wlist.isize = NULL;
              vs->wlist.order = NULL;
              vs->wlist.esize = NULL;

              /* Initialize the array of pointers to field names to NULL also */
              vs->wlist.name = NULL;
            } /* end if */
          else
            { /* Allocate buffer to hold all the int16/uint16 arrays */
              if(NULL==(vs->wlist.bptr = HDmalloc(sizeof(uint16)*(size_t)(vs->wlist.n*5))))
                  HGOTO_ERROR(DFE_NOSPACE, FAIL);

              /* Use buffer to support the other arrays */
              vs->wlist.type = (int16 *)vs->wlist.bptr;
              vs->wlist.off = (uint16 *)vs->wlist.type+vs->wlist.n;
              vs->wlist.isize = vs->wlist.off+vs->wlist.n;
              vs->wlist.order = vs->wlist.isize+vs->wlist.n;
              vs->wlist.esize = vs->wlist.order+vs->wlist.n;

              for (i = 0; i < vs->wlist.n; i++)   /* retrieve the type */
                  INT16DECODE(bb, vs->wlist.type[i]);

              for (i = 0; i < vs->wlist.n; i++)   /* retrieve the isize */
                  UINT16DECODE(bb, vs->wlist.isize[i]);

              for (i = 0; i < vs->wlist.n; i++)   /* retrieve the offset */
                  UINT16DECODE(bb, vs->wlist.off[i]);

              for (i = 0; i < vs->wlist.n; i++)   /* retrieve the order */
                  UINT16DECODE(bb, vs->wlist.order[i]);

              /* retrieve the field names (and each field name's length)  */
              if(NULL==(vs->wlist.name = HDmalloc(sizeof(char *)*(size_t)vs->wlist.n)))
                  HGOTO_ERROR(DFE_NOSPACE, FAIL);

              for (i = 0; i < vs->wlist.n; i++) 
                {
                    INT16DECODE(bb, int16var);    /* this gives the length */
                    if(NULL==(vs->wlist.name[i] = HDmalloc((int16var+1)*sizeof(char))))
                        HGOTO_ERROR(DFE_NOSPACE, FAIL);

                    HIstrncpy(vs->wlist.name[i], (char *) bb, int16var + 1);

                    bb += (size_t)int16var;
                }
            } /* end else */

          /* retrieve the vsname (and vsnamelen)  */
          INT16DECODE(bb, int16var);  /* this gives the length */

          HIstrncpy(vs->vsname, (char *) bb, int16var + 1);
          bb += (size_t)int16var;

          /* retrieve the vsclass (and vsclasslen)  */
          INT16DECODE(bb, int16var);  /* this gives the length */

          HIstrncpy(vs->vsclass, (char *) bb, int16var + 1);
          bb += (size_t)int16var;

          /* retrieve the expansion tag and ref */
          UINT16DECODE(bb, vs->extag);
          UINT16DECODE(bb, vs->exref);

          /* retrieve the middle version field */
          INT16DECODE(bb, temp);
          if (temp != vs->version) 
              HGOTO_ERROR(DFE_BADVH, FAIL);

          /* retrieve the 'more' field */
          INT16DECODE(bb, temp);
          if (temp != vs->more) 
              HGOTO_ERROR(DFE_BADVH, FAIL);

          if (vs->version == VSET_NEW_VERSION) 
            { /* new features exist */
                UINT32DECODE(bb, vs->flags);
                if (vs->flags & VS_ATTR_SET)  
                  {    /* get attr info */
                      INT32DECODE(bb, vs->nattrs);

                      if (NULL == (vs->alist = (vs_attr_t *)HDmalloc(vs->nattrs*sizeof(vs_attr_t))))
                          HGOTO_ERROR(DFE_NOSPACE, FAIL);

                      for (i=0; i<vs->nattrs; i++)  
                        {
                            INT32DECODE(bb, vs->alist[i].findex);
                            UINT16DECODE(bb, vs->alist[i].atag); 
                            UINT16DECODE(bb, vs->alist[i].aref); 
                        }  /* for */
                  }     /* attr set */
            }   /* new version */

          if (vs->version <= VSET_OLD_TYPES)
            {
                for (i = 0; i < vs->wlist.n; i++)   /* save the type */
                    vs->wlist.type[i] = map_from_old_types((intn)vs->wlist.type[i]);
            }

          /* --- EXTRA --- fill in the machine-dependent size fields */
          for (i = 0; i < vs->wlist.n; i++)
            {
                vs->wlist.esize[i] = (uint16) (vs->wlist.order[i] *
                                               DFKNTsize((int32) vs->wlist.type[i] | (int32) DFNT_NATIVE));
            }
      }  /* if version <= 4 */

done:
    if (ret_value == FAIL)
    { /* Error condition cleanup */

    } /* end if */

	  /* Normal function cleanup */
  return ret_value;
}   /* vunpackvs */

/*******************************************************************************
NAME
   vsdestroynode

DESCRIPTION
   Frees B-Tree nodes

   *** Only called by B-tree routines, should _not_ be called externally ***
   *** unless you know what your are doing                               ***

RETURNS
   Nothing

*******************************************************************************/
void
vsdestroynode(void * n /* IN: Node in TBBT-tree */)
{
    VDATA  *vs = NULL;
    intn   i;

    if (n != NULL)
      {
          vs = ((vsinstance_t *) n)->vs;
          if (vs != NULL)
            {
                /* Free the dynamicly allocated VData fields */
                for(i=0; i<vs->wlist.n; i++)
                    HDfree(vs->wlist.name[i]);

                HDfree(vs->wlist.name);
                HDfree(vs->wlist.bptr);

                if(vs->rlist.item != NULL)
                    HDfree(vs->rlist.item);

                if (vs->alist != NULL)
                    HDfree(vs->alist);

                VSIrelease_vdata_node(vs);
            } /* end if */

          /* relase this instance to the free list ? */
          VSIrelease_vsinstance_node((vsinstance_t *)n);
      } /* end if 'n' */

}   /* vsdestroynode */

/*******************************************************************************
 NAME
    VSPgetinfo -- Read in the "header" information about the Vdata.

 DESCRIPTION
    This routine pre-reads the header information for a Vdata into memory
    so that it can be accessed more quickly by the routines that need it.

 RETURNS
    Return a pointer to a VDATA filled with the Vdata information on success,
    NULL on failure.

*******************************************************************************/
VDATA *
VSPgetinfo(HFILEID f, /* IN: file handle */
           uint16 ref /* IN: ref of the Vdata */)
{
	VDATA 		*vs = NULL;   /* new vdata to be returned */
  /* int32       vh_length;   int32 is mismatches Vhbuf's type -- uint32 */
    size_t vh_length;         /* length of the vdata header */
    VDATA *ret_value = NULL;  /* FAIL */
    CONSTR(FUNC, "VSPgetinfo");

    /* clear error stack */
    HEclear();
 
    /* get a free Vdata node? */
    if ((vs = VSIget_vdata_node()) == NULL)
        HGOTO_ERROR(DFE_NOSPACE, NULL);
 
    /* need to fetch length of vdata from file */
    if ((vh_length = Hlength(f,DFTAG_VH,ref)) == FAIL)
        HGOTO_ERROR(DFE_BADLEN, NULL);

    if(vh_length > Vhbufsize)
      {
        Vhbufsize = (uint32)vh_length;

        if (Vhbuf != NULL)
            HDfree(Vhbuf);

        if ((Vhbuf = (uint8 *) HDmalloc(Vhbufsize)) == NULL)
            HGOTO_ERROR(DFE_NOSPACE, NULL);
      } /* end if */

    /* get Vdata header from file */
    if (Hgetelement(f,DFTAG_VH,ref,Vhbuf) == FAIL)
        HGOTO_ERROR(DFE_NOVS, NULL);
 
    /* init all other fields in vdata 
       and then unpack the vdata */
    vs->otag    = DFTAG_VH;
    vs->oref    = ref;
    vs->f       = f;
    if (FAIL == vunpackvs (vs,Vhbuf, (int32)vh_length))
        HGOTO_ERROR(DFE_INTERNAL, NULL);
 
    /* return vdata */
    ret_value = (vs);

done:
  if(ret_value == NULL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
} /* end VSPgetinfo() */


/*******************************************************************************
NAME
   VSattach

DESCRIPTION
   NEW VSattach:
   (a)  if vsid == -1
       if "r" access return error.
       if "w" access
       create a new vs in vg and attach it.
       add to vsdir, set nattach= 1, nvertices = 0.

   (b)  if (vsid > 0)
       if "r" access => look in vsdir
       if not found,
       fetch  vs from file, add to vsdir,
       set nattach= 1, nvertices = val from file.
       if found,
       check access of found vs
       if "w" => being written, unstable! forbidden
       if "r" => ok. incr nattach.

       if "w" access => new data may be added BUT must be same format
       as existing vdata.
       (ie. VSsetfields must match old format exactly!!)

   Allows for seeks to write.

   in all cases, set the marked flag to 0.
   returns NULL if error.

   OLD VSattach:
   if vsid == -1, then
   (a) if vg is "w", create a new vs in vg and attach it.
   add to vsdir, set nattach= 1, nvertices = 0.
   (b) if vg is "r", forbidden.
   if vsid is +ve, then
   (a) if vg is "w"  => new data may be added BUT must be same format
   as existing vdata.
   (ie. VSsetfields must match old format exactly!!)

   (b) if vg is "r"  => look in vsdir
   if not found,
   fetch  vs from file, add to vsdir,
   set nattach= 1, nvertices = val from file.
   if found,
   check access of found vs
   if "w" => being written, unstable! forbidden
   if "r" => ok. incr nattach.

   in all cases, set the marked flag to 0.
   returns NULL if error.

RETURNS


*******************************************************************************/
int32 
VSattach(HFILEID f,             /* IN: file handle */
         int32 vsid,            /* IN: vdata id i.e. ref */
         const char *accesstype /* IN: access type */)
{
    VDATA        *vs = NULL;    /* new vdata to be returned */
    vsinstance_t *w = NULL;
    vfile_t      *vf = NULL;
    int32        acc_mode;
    int32      ret_value = FAIL;
    CONSTR(FUNC, "VSattach");

    /* clear error stack */
    HEclear();

    /* check file and vdata handles */
    if ((f == FAIL) || (vsid < -1))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get vdata file record */
    if (NULL == (vf = Get_vfile(f)))
        HGOTO_ERROR(DFE_FNF, FAIL);

    /* check access type and covert to internal mode? */
    if (accesstype[0] == 'R' || accesstype[0] == 'r')
        acc_mode = 'r';
    else if (accesstype[0] == 'W' || accesstype[0] == 'w')
        acc_mode = 'w';
    else
        HGOTO_ERROR(DFE_BADACC, FAIL);

    /*      */
    if (vsid == -1)
      {  /* ---------- VSID IS -1 ----------------------- 
            if "r" access return error.
            if "w" access
            create a new vs in vg and attach it.
            add to vsdir, set nattach= 1, nvertices = 0.
          */
          if (acc_mode == 'r')
              HGOTO_ERROR(DFE_BADACC, FAIL);

          /* otherwise 'w' */
          /* allocate space for vs,  & zero it out  */
          if ((vs = VSIget_vdata_node()) == NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);

          vs->otag = DFTAG_VH;
          vs->oref = Hnewref(f);
          if (vs->oref == 0)
            {
                VSIrelease_vdata_node(vs);
                HGOTO_ERROR(DFE_NOREF, FAIL);
            }

          vs->interlace = FULL_INTERLACE;   /* DEFAULT */
          vs->access = 'w';
          vs->f = f;
          vs->version = VSET_VERSION;

          /* attach new vs to file's vstab */
          if (NULL == (w = VSIget_vsinstance_node()))
              HGOTO_ERROR(DFE_NOSPACE, FAIL);

          vf->vstabn++;
          w->key = (int32) vs->oref;  /* set the key for the node */
          w->ref = (uintn) vs->oref;
          w->vs = vs;
          w->nattach = 1;
          w->nvertices = 0;

          /* insert the vs instance in B-tree */
          tbbtdins(vf->vstree, w, NULL);    

          vs->instance = w;
      }     /* end of case where vsid is -1 */
    else
      { /*  --------  VSID IS NON_NEGATIVE ------------- 
            if "r" access => look in vsdir
            if not found,
            fetch  vs from file, add to vsdir,
            set nattach= 1, nvertices = val from file.
            if found,
            check access of found vs
            if "w" => being written, unstable! forbidden
            if "r" => ok. incr nattach.

            if "w" access => new data may be added BUT must be same format
            as existing vdata.
            (ie. VSsetfields must match old format exactly!!)

            Allows for seeks to write.

            in all cases, set the marked flag to 0.
            returns NULL if error.
        */

        if (NULL == (w = vsinst(f, (uint16) vsid)))
            HGOTO_ERROR(DFE_VTAB, FAIL);

        if (acc_mode == 'r')
          {     /* reading an existing vdata */
              /* this vdata is already attached for 'r', ok to do so again */
	      /* then reset the read position to the beginning of the vdata */
              if (w->nattach && w->vs->access == 'r')
		{
		  accrec_t *access_rec;	/* access record */

                  w->nattach++;

		  /* get the access_rec pointer to reset position */
		  if ((access_rec = HAatom_object(w->vs->aid)) == NULL)
		      HGOTO_ERROR(DFE_ARGS, FAIL);
		  access_rec->posn = 0;	/* to fix bugzilla #486 - BMR, Dec, 05 */
		}
              else
                {
                  vs = w->vs;

                  vs->access = 'r';
                  vs->aid = Hstartread(vs->f, VSDATATAG, vs->oref);
                  if (vs->aid == FAIL)
                    HGOTO_ERROR(DFE_BADAID, FAIL);

                  vs->instance = w;

                  /* attach vs to vsdir  at the vdata instance w */
                  w->nattach = 1;
                  w->nvertices = vs->nvertices;
                } /* end else */
          }		/* end of case where vsid is positive, and "r"  */
        else
          {		/* writing to an existing vdata */
              if (w->nattach)	/* vdata previously attached before */
                  HGOTO_ERROR(DFE_BADATTACH, FAIL);

              vs = w->vs;

              vs->access = 'w';
              vs->aid = Hstartwrite(vs->f, VSDATATAG, vs->oref, 0);
              if (vs->aid == FAIL)
                HGOTO_ERROR(DFE_BADAID, FAIL);

              vs->instance = w;
              vs->new_h_sz = 0;

              /* attach vs to vsdir  at the vdata instance w */
              w->nattach = 1;
              w->nvertices = vs->nvertices;
          }		/* end of case where vsid is positive, and "w"  */
      } /* end else */

    /* register this vdata with group */
    ret_value = HAregister_atom(VSIDGROUP,w);

    /* Make VDatas appendable by default */
    if (FAIL == VSappendable(ret_value,VDEFAULTBLKSIZE))
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}	/* VSattach */

/*******************************************************************************
NAME
  VSdetach

DESCRIPTION
   Detach vs from vstab.

   if vs has "w" access,   ( <=> only attached ONCE! )
   decr nattach.
   if (nattach is not  0)  => bad error in code.
   if nvertices (in vs) is 0) just free vs from vstab.

   if marked flag is 1
   write out vs to file and set marked flag to 0.
   free vs from vsdir.

   if vs has "r" access,   ( <=> only attached ONCE! )
   decr nattach.
   if (nattach is 0)   just free vs from vstab.

RETURNS
   SUCCEED / FAIL

*******************************************************************************/
int32 
VSdetach(int32 vkey /* IN: vdata key? */)
{
    int32       i;
    int32       ret;
    int32       vspacksize;
    vsinstance_t *w = NULL;
    VDATA        *vs = NULL;
    int32        ret_value = SUCCEED;
    CONSTR(FUNC, "VSdetach");

    /* clear error stack */
    HEclear();

    /* check if vdata is part of vdata group */
    if (HAatom_group(vkey) != VSIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get vdata instance */
    if (NULL == (w = (vsinstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vdata itself and check it */
    vs = w->vs;
    if ((vs == NULL) || (vs->otag != VSDESCTAG))
      HGOTO_ERROR(DFE_ARGS,FAIL);

    w->nattach--; /* detach from vdata */

    /* --- case where access was 'r' --- */
    if (vs->access == 'r')
      {
          if (w->nattach == 0)
            { /* end access to vdata */
              if (Hendaccess(vs->aid) == FAIL)
                  HGOTO_ERROR(DFE_INTERNAL, FAIL);

              /* remove from atom list */
              if(HAremove_atom(vkey)==NULL)
                  HGOTO_ERROR(DFE_INTERNAL, FAIL);
            } /* end if */

          /* we are done */
          HGOTO_DONE(SUCCEED);
      }
    else /* must be write */
      {
          /* --- case where access was 'w' --- */
          if (w->nattach != 0)
              HGOTO_ERROR(DFE_CANTDETACH, FAIL);

          if (vs->marked)
            {	/* if marked , write out vdata's VSDESC to file */
                size_t need;

                need = sizeof(VWRITELIST) + 
                       (size_t)vs->nattrs*sizeof(vs_attr_t) + sizeof(VDATA) + 1;
        
                if(need > Vhbufsize)
                  {
                      Vhbufsize = (uint32)need;
                      if (Vhbuf)
                          HDfree(Vhbuf);

                      if ((Vhbuf = HDmalloc(Vhbufsize)) == NULL)
                          HGOTO_ERROR(DFE_NOSPACE, FAIL);
                  } /* end if */

                if (FAIL == vpackvs(vs, Vhbuf, &vspacksize))
                    HGOTO_ERROR(DFE_INTERNAL, FAIL);

                /* if VH size changed we need to re-use the tag/ref
                 * for new header. This will cause the pointer to the
                 * original vdata header to be lost but this is okay.  */
                if (vs->new_h_sz)
                  { 
                      /* check if tag/ref exists in DD list already */
                      switch(HDcheck_tagref(vs->f, DFTAG_VH, vs->oref))
                        {
                        case 0: /* not found */
                            break;
                        case 1: /* found, reuse tag/ref */
                            if (HDreuse_tagref(vs->f, DFTAG_VH, vs->oref) == FAIL)
                                HGOTO_ERROR(DFE_INTERNAL, FAIL);
                            break;
                        case -1: /* error */
                            HGOTO_ERROR(DFE_INTERNAL, FAIL);
                        default: /* should never get here */
                            HGOTO_ERROR(DFE_INTERNAL, FAIL);
                        }
                  }

                /* write new one */
                ret = Hputelement(vs->f, VSDESCTAG, vs->oref, Vhbuf, vspacksize);
                if (ret == FAIL)
                    HGOTO_ERROR(DFE_WRITEERROR, FAIL);

                vs->marked = 0;
                vs->new_h_sz = 0;
            }

          /* remove all defined symbols */
          for (i = 0; i < vs->nusym; i++)
              HDfree(vs->usym[i].name);

          if(vs->usym!=NULL)
              HDfree(vs->usym);   /* free the actual array */

          vs->nusym = 0;
          vs->usym=NULL;

          /* end access to vdata */
          if (Hendaccess(vs->aid) == FAIL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);

          /* remove vdata from atom list */
          if(HAremove_atom(vkey)==NULL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);

      } /* end of 'write' case */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}	/* VSdetach */

/*******************************************************************************
 NAME
   VSappendable

 DESCRIPTION
    Make it possible to append unlimitedly to an existing VData
 
  RETURNS   
      SUCCEED, or FAIL for error
*******************************************************************************/ 
int32 
VSappendable(int32 vkey, /* IN: vdata key */
             int32 blk   /* IN: */)
{
    vsinstance_t *w = NULL;
    VDATA        *vs = NULL;
    int32      ret_value = SUCCEED;
    CONSTR(FUNC, "VSappendable");

    /* clear error stack */
    HEclear();

    /* shut compiler up */
    blk = blk;

    /* check vdata key is a valid */
    if (HAatom_group(vkey) != VSIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get vdata instance */
    if (NULL == (w = (vsinstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get Vdata itself and check */
    vs = w->vs;
    if ((vs == NULL) || (vs->otag != VSDESCTAG))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    if(vs->aid == 0)
        vs->aid = Hstartaccess(vs->f, VSDATATAG, vs->oref, DFACC_RDWR|DFACC_APPENDABLE);
    else
        ret_value = Happendable(vs->aid);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* VSappendable */

/*******************************************************************************
NAME
   VSgetid

DESCRIPTION
   returns the id of the next  VDATA from the file f .
   (vsid = -1 gets the 1st vDATA).

RETURNS
   RETURNS -1 on error.
   RETURNS vdata id (0 or +ve integer)

*******************************************************************************/
int32
VSgetid(HFILEID f,  /* IN: file handle */
        int32 vsid  /* IN: vdata id i.e. ref */)
{
    vsinstance_t *w = NULL;
    vfile_t      *vf = NULL;
    void *        *t = NULL;
    int32        key;
    int32        ret_value = SUCCEED;
    CONSTR(FUNC, "VSgetid");

    /* clear error stack */
    HEclear();

    /* check valid vdata id */
    if (vsid < -1)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get vdata file record */
    if (NULL == (vf = Get_vfile(f)))
        HGOTO_ERROR(DFE_FNF, FAIL);

    if (vsid == -1)
      { /* vsid '-1' case */

        if (vf->vstree==NULL) 
            HGOTO_DONE(FAIL);

        if ((t = (void **)tbbtfirst((TBBT_NODE *) * (vf->vstree))) == NULL)
            HGOTO_DONE(FAIL);

        /* we assume 't' is valid at this point */
        w = (vsinstance_t *) * t; /* get actual pointer to the vsinstance_t */
        HGOTO_DONE((int32)w->ref);/* rets 1st vdata's ref */
      }
    else /* vsid >= 0 */ 
      {
          /* tbbtdfind returns a pointer to the vsinstance_t pointer */
          key = (int32)vsid;
          t = (void * *) tbbtdfind(vf->vstree, &key, NULL);

          if (t == NULL)  /* couldn't find the old vsid */
              ret_value = (FAIL);
          else if (NULL == (t = (void * *) tbbtnext((TBBT_NODE *) t)))  /* get the next node in the tree */
              ret_value = (FAIL);
          else
            {
                w = (vsinstance_t *) * t;   /* get actual pointer to the vsinstance_t */
                ret_value = (int32)w->ref;  /* rets vdata's ref */
            }     /* end else */
      }

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* VSgetid */

/*******************************************************************************
NAME
   VSQuerytag

DESCRIPTION
   get the 'otag' of the given Vdata

RETURNS
   Return the 'otag' of the given Vdata
   Return FAIL on failure

*******************************************************************************/
int32
VSQuerytag(int32 vkey /* IN: vdata key */)
{
    vsinstance_t *w = NULL;
    VDATA        *vs = NULL;
    int32      ret_value = SUCCEED;
    CONSTR(FUNC, "VSQuerytag");

    /* clear error stack */
    HEclear();

    /* check vdata key is a valid */
    if (HAatom_group(vkey) != VSIDGROUP)
      HGOTO_ERROR(DFE_ARGS,FAIL);

    /* get vdata instance */
    if (NULL == (w = (vsinstance_t *) HAatom_object(vkey)))
      HGOTO_ERROR(DFE_NOVS,FAIL);

    /* get Vdata itself and check */
    vs = w->vs;
    if ((vs == NULL) || (vs->otag != VSDESCTAG))
      HGOTO_ERROR(DFE_ARGS,FAIL);

    /* return otag of vdata */
    ret_value = ((int32) vs->otag);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}	/* VSQuerytag */

/*******************************************************************************
NAME
  VSQueryref

DESCRIPTION
  get the ref of the the given Vdata

RETURNS
   Return the ref of the given Vdata
   Return FAIL on failure

*******************************************************************************/
int32
VSQueryref(int32 vkey /* IN: vdata key */)
{
    vsinstance_t *w = NULL;
    VDATA        *vs = NULL;
    int32      ret_value = SUCCEED;
    CONSTR(FUNC, "VSQueryref");

    /* clear error stack */
    HEclear();

    /* check vdata key is a valid */
    if (HAatom_group(vkey) != VSIDGROUP)
      HGOTO_ERROR(DFE_ARGS,FAIL);

    /* get vdata instance */
    if (NULL == (w = (vsinstance_t *) HAatom_object(vkey)))
      HGOTO_ERROR(DFE_NOVS,FAIL);

    /* get Vdata itself and check */
    vs = w->vs;
    if ((vs == NULL) || (vs->otag != VSDESCTAG))
      HGOTO_ERROR(DFE_ARGS,FAIL);

    /* return ref of vdata */
    ret_value = ((int32) vs->oref);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}	/* VSQueryref */

/*******************************************************************************
NAME
   vswritelist

DESCRIPTION
   get the 'writelist' of the given vdata

RETURNS
   return the 'writelist' of the vdata if successful else NULL

*******************************************************************************/
DYN_VWRITELIST *
vswritelist(int32 vkey /* IN: vdata key */)
{
    vsinstance_t *w = NULL;
    VDATA        *vs = NULL;
    DYN_VWRITELIST *ret_value = NULL; /* Failure */
    CONSTR(FUNC, "VSwritelist");

    /* clear error stack */
    HEclear();

    /* check vdata key is a valid */
    if (HAatom_group(vkey) != VSIDGROUP)
        HGOTO_ERROR(DFE_ARGS, NULL);

    /* get vdata instance */
    if (NULL == (w = (vsinstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, NULL);

    /* get Vdata itself and check */
    vs = w->vs;
    if ((vs == NULL) || (vs->otag != VSDESCTAG))
        HGOTO_ERROR(DFE_ARGS, NULL);

    /* return 'writelist' */
    ret_value = (&(vs->wlist));

done:
  if(ret_value == NULL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* vswritelist() */

/*******************************************************************************
NAME
  VSgetversion

DESCRIPTION
  get the version nuber of the vdata

RETURNS
  return the version number if successful else '0'.

*******************************************************************************/
int32
VSgetversion(int32 vkey /* IN: vdata key */)
{
    vsinstance_t *w = NULL;
    VDATA        *vs = NULL;
    int32      ret_value = SUCCEED;
    CONSTR(FUNC, "VSgetversion");

    /* clear error stack */
    HEclear();

    /* check vdata key is a valid */
    if (HAatom_group(vkey) != VSIDGROUP)
        HGOTO_ERROR(DFE_ARGS, 0);

    /* get vdata instance */
    if (NULL == (w = (vsinstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, 0);

    /* get Vdata itself and check */
    vs = w->vs;
    if ((vs == NULL) || (vs->otag != VSDESCTAG))
        HGOTO_ERROR(DFE_ARGS, 0);

    /* return version number */
    ret_value = (int32)vs->version;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end VSgetversion() */

/*******************************************************************************
NAME
  VSdelete

DESCRIPTION
   Remove a Vdata from its file.  This function will both remove the Vdata
   from the internal Vset data structures as well as from the file.
   'vsid' is actually the 'ref' of the vdata.

   (i.e. it calls tbbt_delete() and Hdeldd())

RETURNS
   Return FAIL / SUCCEED

*******************************************************************************/
int32
VSdelete(int32 f,    /* IN: file handle */
         int32 vsid  /* IN: vdata id i.e. ref */)
{
    void *       v;
    vfile_t    *vf = NULL;
    void *      *t = NULL;
    int32       key;
    int32       ret_value = SUCCEED;
    CONSTR(FUNC, "VSdelete");

    /* clear error stack */
    HEclear();

    /* check valid vdata id */
    if (vsid < -1)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get vdata file record */
    if (NULL == (vf = Get_vfile(f)))
      HGOTO_ERROR(DFE_FNF,FAIL);

    /* find vdata in TBBT using it's ref */
    key = vsid;
    if (( t = (void * *) tbbtdfind(vf->vstree, &key, NULL))== NULL)
        HGOTO_DONE(FAIL);

    /* remove vdata from TBBT */
    v = tbbtrem((TBBT_NODE **) vf->vstree, (TBBT_NODE *) t, NULL);

    /* destroy vdata node itself*/
    if (v != NULL)
        vsdestroynode(v);

    /* delete vdata header and data from file */
    if (Hdeldd(f, DFTAG_VS, (uint16) vsid) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    if (Hdeldd(f, DFTAG_VH, (uint16) vsid) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}	/* VSdelete */
