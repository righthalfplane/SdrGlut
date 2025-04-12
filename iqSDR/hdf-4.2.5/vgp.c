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
static char RcsId[] = "@(#)$Revision: 5333 $";
#endif

/* $Id: vgp.c 5333 2010-01-28 05:57:54Z bmribler $ */
/*****************************************************************************

 file - vgp.c

 Part of the Vset interface.
 VGROUPs are handled by routines in here.


LOCAL ROUTINES
==============
 VIget_vgroup_node -- allocate a new VGROUP record
 VIrelease_vgroup_node -- Releases a vgroup node
 VIget_vginstance_node -- allocate a new vginstance_t record
 VIrelease_vginstance_node -- Releases a vginstance node
 Get_vfile    -- get vgroup file record
 New_vfile    -- create new vgroup file record
 Load_vfile   -- loads vgtab table with info of all vgroups in file.
 Remove_vfile -- removes the file ptr from the vfile[] table. 

 VPgetinfo  --  Read in the "header" information about the Vgroup.
 VIstart    --  V-level initialization routine
 VPshutdown  --  Terminate various static buffers.

EXPORTED ROUTINES
=================
 Following 4 routines are solely for B-tree routines.
 vcompare     -- Compares two TBBT-tree keys for equality.  Similar to memcmp.
 vprint       -- Prints out the key and reference number of VDatas and Vgroups
 vdestroynode -- destroy vgroup node in TBBT
 vfdestroynode  -- destroy vgroup file record node in TBBT
 vtfreekey    -- Frees B-Tree index (actually doesn't anything at all)

 Vinitialize  -- initialize Vxxx interface
 Vfinish      -- end Vxxx access to file

 vginstance   -- Looks thru vgtab for vgid and return the addr of the vg 
                  instance where vgid is found. 
 vexistvg     -- Tests if a vgroup with id "vgid" is in the file's vgtab.
 vpackvg      -- Extracts fields from a VGROUP struct "vg" and packs the 
                  fields into array buf in preparation for storage in the 
                  HDF file.
 vunpackvg    -- Unpacks the fields from a buf (ie a DFTAG_VG data object 
                  just read in from the HDF file), into a VGROUP structure vg.

 Vattach      -- Attaches to an existing vgroup or creates a new vgroup.
 Vdetach      -- Detaches access to vg.    
 Vinsert      -- Inserts a velt (vs or vg) into a vg 
 Vflocate     -- Checks to see if the given field exists in a vdata 
                  belonging to this vgroup.
 Vinqtagref   -- Checks whether the given tag/ref pair already exists 
                  in the vgroup.
 Vntagrefs    -- Returns the number (0 or +ve integer) of tag/ref pairs 
                  in a vgroup.
 Vnrefs       --
 Vgettagrefs  -- Returns n tag/ref pairs from the vgroup into the 
                  caller-supplied arrays(tagrarray and refarray).
 Vgettagref   -- Returns a specified tag/ref pair from the vgroup.
 VQuerytag    -- Return the tag of this Vgroup.
 VQueryref    -- Return the ref of this Vgroup.
 Vaddtagref   -- Inserts a tag/ref pair into the attached vgroup vg.
 vinsertpair  -- Inserts a tag/ref pair into the attached vgroup vg.
 Ventries     -- Returns the num of entries (+ve integer) in the vgroup vgid.
 Vsetname     -- Gives a name to the VGROUP vg.
 Vsetclass    -- Assigns a class name to the VGROUP vg.
 Visvg        -- Tests if the given entry in the vgroup vg is a VGROUP.
 Visvs        -- Checks if an id in a vgroup refers to a VDATA.
 Vgetid       -- Given a vgroup's id, returns the next vgroup's id in the file.
 Vgetnext     -- Given the id of an entry from a vgroup vg, looks in vg 
                  for the next entry after it, and returns its id.
 Vgetnamelen  -- Retrieves the length of the vgroup's name.
 Vgetclassnamelen  -- Retrieves the length of the vgroup's classname.
 Vgetname     -- Returns the vgroup's name.
 Vgetclass    -- Returns the vgroup's class name .
 Vinquire     -- General inquiry routine for VGROUP. 
 Vopen        -- This routine opens the HDF file and initializes it for 
                  Vset operations.(i.e." Hopen(); Vinitialize(f)").
 Vclose       -- This routine closes the HDF file, after it has freed 
                  all memory and updated the file.
                  (i.e." Vfinish(f); Hclose(f);").
 Vdelete      -- Remove a Vgroup from its file.  This function will both 
                  remove the Vgoup from the internal Vset data structures 
                  as well as from the file.
 Vdeletetagref - delete tag/ref pair in Vgroup

 NOTE: Another pass needs to made through this file to update some of
       the comments about certain sections of the code. -GV 9/8/97

*************************************************************************/

#define VSET_INTERFACE
#include "hdf.h"

/* Prototypes */
extern VOID vprint(VOIDP k1);

PRIVATE intn Load_vfile
            (HFILEID f);

PRIVATE intn Remove_vfile
            (HFILEID f);

PRIVATE intn vunpackvg
            (VGROUP * vg, uint8 buf[], intn len);

PRIVATE intn VIstart(void);

/*
   * --------------------------------------------------------------------
   * PRIVATE  data structure and routines.
   *
   * Info about all vgroups in the file are loaded into vgtab  at start;
   * and the vg field set to NULL until that vgroup is attached,
   * and reset back to NULL when that vgroup is detached.
   * Info about all vdatas in the file are loaded into vstab  at start;
   * and the vs field set to NULL until that vdata is attached,
   * and reset back to NULL when that vdata is detached.
   * --------------------------------------------------------------------
 */

TBBT_TREE *vtree = NULL;

/* Whether we've installed the library termination function yet for this interface */
PRIVATE intn library_terminate = FALSE;

/* Temporary buffer for I/O */
PRIVATE uint32 Vgbufsize = 0;
PRIVATE uint8  *Vgbuf = NULL;

/* Pointers to the VGROUP & vginstance node free lists */
static VGROUP       *vgroup_free_list     = NULL;
static vginstance_t *vginstance_free_list = NULL;

/*******************************************************************************
 NAME
    VIget_vgroup_node -- allocate a new VGROUP record

 DESCRIPTION
    Return a pointer to a new VGROUP to use for a new VGID.

 RETURNS
    returns VGROUP record pointer or NULL if failed.

*******************************************************************************/
VGROUP *
VIget_vgroup_node(void)
{
    VGROUP   *ret_value = NULL;
    CONSTR(FUNC, "VIget_vgroup_node");

    /* clear error stack */
    HEclear();

    /* Grab from free list if possible */
    if(vgroup_free_list != NULL)
      {
        ret_value = vgroup_free_list;
        vgroup_free_list = vgroup_free_list->next;
      } /* end if */
    else
      {
        if((ret_value = (VGROUP *)HDmalloc(sizeof(VGROUP))) == NULL)
            HGOTO_ERROR(DFE_NOSPACE, NULL);
      } /* end else */

    /* Initialize to zeros */
    HDmemset(ret_value,0,sizeof(VGROUP));

done:
  if(ret_value == NULL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return(ret_value);
}	/* VIget_vgroup_node */

/******************************************************************************
 NAME
   VIrelease_vgroup_node -- Releases a vgroup node

 DESCRIPTION
    Puts an VGROUP node into the free list

 RETURNS
    No return value

*******************************************************************************/
void VIrelease_vgroup_node(VGROUP *vg)
{
#ifdef LATER
    CONSTR(FUNC, "VIrelease_vgroup_node");	/* for HERROR */
#endif /* LATER */

    /* Insert the atom at the beginning of the free list */
    vg->next         = vgroup_free_list;
    vgroup_free_list = vg;

}   /* end VIrelease_vgroup_node() */

/******************************************************************************
 NAME
    VIget_vginstance_node -- allocate a new vginstance_t record

 DESCRIPTION
    Return an pointer to a new vginstance to use for a new VGID.

 RETURNS
    returns vginstance_t pointer or NULL if failed.
*******************************************************************************/
vginstance_t *
VIget_vginstance_node(void)
{
    vginstance_t   *ret_value = NULL;
    CONSTR(FUNC, "VIget_vginstance_node");

    /* clear error stack */
    HEclear();

    /* Grab from free list if possible */
    if(vginstance_free_list != NULL)
      {
        ret_value = vginstance_free_list;
        vginstance_free_list = vginstance_free_list->next;
      } /* end if */
    else
      {
        if((ret_value = (vginstance_t *)HDmalloc(sizeof(vginstance_t))) == NULL)
            HGOTO_ERROR(DFE_NOSPACE, NULL);
      } /* end else */

    /* Initialize to zeros */
    HDmemset(ret_value,0,sizeof(vginstance_t));

done:
  if(ret_value == NULL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return(ret_value);
}	/* VIget_vginstance_node */

/******************************************************************************
 NAME
    VIrelease_vginstance_node -- Releases a vginstance node

 DESCRIPTION
    Puts a vginstance node into the free list

 RETURNS
    No return value

*******************************************************************************/
void 
VIrelease_vginstance_node(vginstance_t *vg /* IN: vgroup instance to release */)
{
#ifdef LATER
    CONSTR(FUNC, "VIrelease_vginstance_node");	/* for HERROR */
#endif /* LATER */

    /* Insert the vsinstance at the beginning of the free list */
    vg->next = vginstance_free_list;
    vginstance_free_list = vg;

}   /* end VIrelease_vginstance_node() */

/*******************************************************************************
NAME
  Get_vfile  -- get vgroup file record

DESCRIPTION
   Looks in the TBBT vtree for the file ID of the file.

RETURNS
   Returns a pointer to the vfile_t for that file on success, otherwise NULL.

*******************************************************************************/
vfile_t *
Get_vfile(HFILEID f /* IN: file handle */)
{
    VOIDP *t = NULL;       /* vfile_t pointer from tree */
    int32 key = (int32)f;  /* initialize key to file handle */

    /* find file record */
    t = (VOIDP *) tbbtdfind(vtree, (VOIDP) &key, NULL);

    return((vfile_t *)(t==NULL ? NULL : *t));
} /* end Get_vfile() */

        
/*******************************************************************************
NAME
   New_vfile  -- create new vgroup file record

DESCRIPTION
   Creates vfile_t structure and adds it to the tree

RETURNS
   Returns a pointer to the vfile_t for that file on success, otherwise NULL.

*******************************************************************************/
PRIVATE vfile_t *
New_vfile(HFILEID f /* IN: file handle */)
{
    vfile_t *v = NULL;
    
    /* Allocate the vfile_t structure */
    if (NULL == (v = (vfile_t *) HDcalloc(1,sizeof(vfile_t))))
      return(NULL);

    /* Assign the file ID & insert into the tree */
    v->f = f;

    /* insert the vg instance in B-tree */
    tbbtdins(vtree, (VOIDP) v, NULL);    

    /* return vfile_t struct */
    return(v); 
} /* end New_vfile() */

/*******************************************************************************
NAME
   Load_vfile -- loads vgtab table with info of all vgroups in file.

DESCRIPTION
   *** Only called by Vinitialize()  ***

   loads vgtab table with info of all vgroups in file f.
   Will allocate a new vfile_t, then proceed to load vg instances.

RETURNS
   RETURNS FAIL if error or no more file slots available.
   RETURNS SUCCEED if ok.

*******************************************************************************/
PRIVATE intn
Load_vfile(HFILEID f /* IN: file handle */)
{
    vfile_t      *vf = NULL;
    vginstance_t *v = NULL;
    vsinstance_t *w = NULL;
    int32       aid;
    int32       ret;
    uint16      tag = DFTAG_NULL;
    uint16      ref = DFTAG_NULL;
    intn        ret_value = SUCCEED;
    CONSTR(FUNC, "Load_vfile");

    /* clear error stack */
    HEclear();

    /* Check if vfile buffer has been allocated */
    if (vtree == NULL)
      {
          vtree = tbbtdmake(vcompare, sizeof(int32), TBBT_FAST_INT32_COMPARE);
          if (vtree == NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);

          /* Initialize the atom groups for Vdatas and Vgroups */
          if (HAinit_group(VSIDGROUP,VATOM_HASH_SIZE) == FAIL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);

          if (HAinit_group(VGIDGROUP,VATOM_HASH_SIZE) == FAIL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);
      }

    /* Grab the existing vfile_t structure first, otherwise create a new one */
    if ((vf = Get_vfile(f)) == NULL)
      {
        if ((vf = New_vfile(f)) == NULL)
            HGOTO_ERROR(DFE_FNF, FAIL);
      }

    /* the file is already loaded (opened twice) do nothing */
    if (vf->access++)
        HGOTO_DONE(SUCCEED);

    /* load all the vg's  tag/refs from file */
    vf->vgtabn = 0; /* intialize to number of current entries to zero */
    vf->vgtree = tbbtdmake(vcompare, sizeof(int32), TBBT_FAST_INT32_COMPARE);
    if (vf->vgtree == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    ret = aid = Hstartread(f, DFTAG_VG, DFREF_WILDCARD);
    while (ret != FAIL)
      {
          /* get tag/ref for this vgroup */
          HQuerytagref(aid, &tag, &ref);

          /* get a vgroup struct to fill */
          if (NULL == (v = VIget_vginstance_node()))
            {
                tbbtdfree(vf->vgtree, vdestroynode, NULL);
                HGOTO_ERROR(DFE_NOSPACE, FAIL);
            }

          vf->vgtabn++; /* increment number of vgroups found in file */

          v->key = (int32) ref;   /* set the key for the node */
          v->ref = (uintn) ref;

          /* get the header information */
          v->vg = VPgetinfo(f,ref);  
          if (v->vg == NULL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);

          /* insert the vg instance in B-tree */
          tbbtdins(vf->vgtree, (VOIDP) v, NULL); 

          /* get next vgroup */
          ret = Hnextread(aid, DFTAG_VG, DFREF_WILDCARD, DF_CURRENT);
      }

    if (aid != FAIL)
        Hendaccess(aid);

    /* clear error stack - this is to remove the faux errors about DD not
       found from when Hstartread is called on a new file */
    HEclear();

    /* load all the vs's  tag/refs from file */
    vf->vstabn = 0;
    vf->vstree = tbbtdmake(vcompare, sizeof(int32), TBBT_FAST_INT32_COMPARE);
    if (vf->vstree == NULL)
      {
          tbbtdfree(vf->vgtree, vdestroynode, NULL);
          HGOTO_ERROR(DFE_NOSPACE, FAIL);
      }     /* end if */

    ret = aid = Hstartread(f, VSDESCTAG, DFREF_WILDCARD);
    while (ret != FAIL)
      {
          /* get tag/ref for this vdata */
          HQuerytagref(aid, &tag, &ref);

          /* attach new vs to file's vstab */
          if (NULL == (w = VSIget_vsinstance_node()))
            {
                tbbtdfree(vf->vgtree, vdestroynode, NULL);
                tbbtdfree(vf->vstree, vsdestroynode, NULL);
                HGOTO_ERROR(DFE_NOSPACE, FAIL);
            }

          vf->vstabn++; /* increment number of vdatas found in file */

          w->key = (int32) ref;   /* set the key for the node */
          w->ref = (uintn)ref;

          /* get the header information */
          w->vs = VSPgetinfo(f,ref);  
          if (w->vs == NULL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);

          w->nattach = 0;
          w->nvertices = 0;

          /* insert the vg instance in B-tree */
          tbbtdins(vf->vstree, (VOIDP) w, NULL);    

          /* get next vdata */
          ret = Hnextread(aid, VSDESCTAG, DFREF_WILDCARD, DF_CURRENT);
      }

    if (aid != FAIL)
        Hendaccess(aid);

    /* clear error stack - this is to remove the faux errors about DD not
       found from when Hstartread is called on a new file */
    HEclear();

    /* file may be incompatible with vset version 2.x. Need to check it */
    if (((int32) 0 == vf->vgtabn) && ((int32) 0 == vf->vstabn))
      {
        if ((int32) 0 == vicheckcompat(f))
          {     /* not compatible */
#if 0
              nvfile--;     /* delete the structure for that file */
#endif
              tbbtdfree(vf->vgtree, vdestroynode, NULL);
              tbbtdfree(vf->vstree, vsdestroynode, NULL);
              HGOTO_ERROR(DFE_BADOPEN, FAIL);
          }
      }

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* Load_vfile */

/******************************************************************************
NAME
   Remove_vfile -- removes the file ptr from the vfile[] table. 

DESCRIPTION
   removes the file ptr from the vfile[] table.
   *** Only called by Vfinish() ***

RETURNS

*******************************************************************************/
PRIVATE intn
Remove_vfile(HFILEID f /* IN: file handle */)
{
    VOIDP      *t  = NULL;
    vfile_t    *vf = NULL;
    intn       ret_value = SUCCEED;
    CONSTR(FUNC, "Remove_vfile");

    /* clear error stack */
    HEclear();

    /* Check if vfile buffer has been allocated */
    if (vtree == NULL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Figure out what file to work on */
    if ((vf = Get_vfile(f)) == NULL)
        HGOTO_ERROR(DFE_FNF, FAIL);

    /* If someone still has an active pointer to this file 
       we don't remove it. */
    if (--vf->access)
        HGOTO_DONE(SUCCEED);

    /* clear out the tbbt's */
    tbbtdfree(vf->vgtree, vdestroynode, NULL);
    tbbtdfree(vf->vstree, vsdestroynode, NULL);

    /* Find the node in the tree */
    if (( t = (VOIDP *) tbbtdfind(vtree, (VOIDP) &f, NULL)) == NULL)
        HGOTO_DONE(FAIL);

    /* Delete the node and free the vfile_t stucture */
    vf = tbbtrem((TBBT_NODE **) vtree, (TBBT_NODE *) t, NULL);
    HDfree(vf);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* Remove_vfile */

/******************************************************************************
NAME
   vcompare  -- compare two TBBT keys for equality

DESCRIPTION
   Compares two B-tree keys for equality.  Similar to memcmp.

   *** Only called by B-tree routines, should _not_ be called externally ***

RETURNS

*******************************************************************************/
intn
vcompare(VOIDP k1,   /* IN: first key to compare*/
         VOIDP k2,   /* IN: second key to compare */
         intn cmparg /* IN: not used */)
{
  /* shut compiler up */
  cmparg = cmparg;

  return (intn) ((*(int32 *)k1) - (*(int32 *)k2));    /* valid for integer keys */
}   /* vcompare */

/******************************************************************************
NAME
  vprint -- print key and reference number of vgroup/vdata node in TBBT

DESCRIPTION
   Prints out the key and reference number of VDatas and Vgroups

   *** Only called by B-tree routines, should _not_ be called externally ***

RETURNS

*******************************************************************************/
VOID
vprint(VOIDP k1 /* IN: key to print */)
{
    printf("Ptr=%p, key=%d, ref=%d\n", 
           k1, (int) ((vginstance_t *) k1)->key, (int) ((vginstance_t *) k1)->ref);
}   /* vprint */

/******************************************************************************
NAME
   vdestroynode -- destroy vgroup node in TBBT

DESCRIPTION
   Frees B-Tree nodes

   *** Only called by B-tree routines, should _not_ be called externally ***

RETURNS
   Nothing

*******************************************************************************/
VOID
vdestroynode(VOIDP n /* IN: node to free */)
{
    VGROUP     *vg = NULL;

    if (n != NULL)
      {
          vg = ((vginstance_t *) n)->vg;
          if (vg != NULL)
            {
                HDfree((VOIDP) vg->tag);
                HDfree((VOIDP) vg->ref);

                if (vg->vgname != NULL)
                    HDfree((VOIDP) vg->vgname);

                if (vg->vgclass != NULL)
                    HDfree((VOIDP) vg->vgclass);

                if (vg->alist != NULL)
                    HDfree((VOIDP) vg->alist);

                VIrelease_vgroup_node(vg);
            }

          VIrelease_vginstance_node((vginstance_t *)n);
      } /* end if n */
}   /* vdestroynode */

/*******************************************************************************
NAME
   vfdestroynode  -- destroy vgroup file record node in TBBT

DESCRIPTION
   Frees B-Tree vfile_t nodes

   *** Only called by B-tree routines, should _not_ be called externally ***

RETURNS
   Nothing

*******************************************************************************/
VOID
vfdestroynode(VOIDP n /* IN: vfile_t record to free */)
{
    vfile_t      *vf = NULL;

    if (n != NULL)
      {
          vf=(vfile_t *)n;

          /* clear out the tbbt's */
          tbbtdfree(vf->vgtree, vdestroynode, NULL);
          tbbtdfree(vf->vstree, vsdestroynode, NULL);

          HDfree(vf);
      }
}   /* vfdestroynode */

#ifdef NOTNEEDED
/* ---------------------------- vtfreekey ------------------------- */
/*
   Frees B-Tree index (actually doesn't anything at all)

   *** Only called by B-tree routines, should _not_ be called externally ***
 */
VOID
vtfreekey(VOIDP k)
{
    k = k;  /* i.e. do nothing */
}   /* vtfreekey */
#endif

/*******************************************************************************
NAME
   Vinitialize  -- initialize Vxxx interface

DESCRIPTION
    Initialize Vxxx stuff/interface ?

RETURNS
    SUCCEED / FAIL

*******************************************************************************/
intn
Vinitialize(HFILEID f /* IN: file handle */)
{
    intn   ret_value = SUCCEED;
    CONSTR(FUNC, "Vinitialize");

    /* clear error stack */
    HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
      {
        if(VIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);
      }

    /* load Vxx stuff from file? */
    if (Load_vfile(f) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
} /* Vinitialize() */


/*******************************************************************************
NAME
   Vfinish  -- end Vxxx access to file

DESCRIPTION
   End Vxxx acess to file?

RETURNS
   SUCCEED / FAIL

*******************************************************************************/
intn
Vfinish(HFILEID f /* IN: file handle */)
{
  intn    ret_value = SUCCEED;
  CONSTR(FUNC, "Vfinish");

  /* clear error stack */
  HEclear();

  /* remove Vxxx file record ? */
  if (Remove_vfile(f) == FAIL)
      HGOTO_ERROR(DFE_INTERNAL, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
} /* Vfinish() */

/*******************************************************************************
NAME
   vginst

DESCRIPTION
   Looks thru vgtab for vgid and return the addr of the vg instance
   where vgid is found.

RETURNS
   RETURNS NULL if error or not found.
   RETURNS vginstance_t pointer if ok.
   
*******************************************************************************/
vginstance_t *
vginst(HFILEID f,   /* IN: file handle */
       uint16 vgid  /* IN: vgroup id */)
{
    VOIDP        *t = NULL;
    vfile_t      *vf = NULL;
    vginstance_t *ret_value = NULL;
    int32        key;
    CONSTR(FUNC, "vginstance");

    /* clear error stack */
    HEclear();

    /* get file Vxxx file record */
    if (NULL == (vf = Get_vfile(f)))
        HGOTO_ERROR(DFE_FNF, NULL);

    /* tbbtdfind returns a pointer to the vginstance_t pointer */
    key = (int32)vgid;
    t = (VOIDP *) tbbtdfind(vf->vgtree, (VOIDP) &key, NULL);
    if (t != NULL)
      {
        ret_value = ((vginstance_t *) * t);  /* return the actual vginstance_t ptr */
        goto done;
      }

    /* we get here then we did find vgroup */
    HGOTO_ERROR(DFE_NOMATCH, NULL);

done:
  if(ret_value == NULL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* vginst */

/*******************************************************************************
NAME
   vexistvg

DESCRIPTION
   Tests if a vgroup with id vgid is in the file's vgtab.

RETURNS
   returns FAIL if not found,
   returns TRUE if found.

*******************************************************************************/
int32
vexistvg(HFILEID f,   /* IN: file handle */
         uint16 vgid  /* IN: vgroup id */)
{
    int32   ret_value;
#ifdef LATER
    CONSTR(FUNC, "vexistvg");
#endif

    if (NULL == (vginstance_t *) vginst(f, vgid))
        ret_value = (FAIL);
    else
        ret_value =  (TRUE);
  
  return ret_value;
}   /* vexistvg */

/* ==================================================================== */
/*
   * vpackvg() and vunpackvg() : Packing and unpacking routines.
   * For use in retrieving and storing vgroups to/from the HDF file.
   *
   *    Fields of VGROUP  that gets stored in HDF as a DFTAG_VG data object:
   *            int16           nvelt (no of entries )
   *            char*           vgname
   *            char*           vgclass
   *            int16           tag[1..nvelt]
   *            int16           ref[1..nvelt]
   *    (fields for version 4) 
   *            uint32   flags
   *    (if bit0 of flags is set, the vg has attributes )
   *            int32    nattrs
   *            vg_attr_t alist[1..nattrs]
 */
/* ==================================================================== */

/*******************************************************************************
NAME
   vpackvg

DESCRIPTION
   Extracts fields from a VGROUP struct vg and packs the fields
   into array buf in preparation for storage in the HDF file.

RETRUNS   
   NO RETURN VALUES.

*******************************************************************************/
intn
vpackvg(VGROUP * vg, /* IN: */
        uint8 buf[], /* IN/OUT: */
        int32 *size  /* IN/OUT: */)
{
#ifdef LATER
    CONSTR(FUNC, "vpackvg");
#endif
    uintn  i;
    int16 slen = 0;
    uint16 temp_len = 0;
    uint8 *bb = NULL;
    int32 ret_value = SUCCEED;

    /* clear error stack */
    HEclear();

    bb = &buf[0];

    /* save nvelt */
    UINT16ENCODE(bb, vg->nvelt);

    /* save all tags */
    for (i = 0; i < (uintn)vg->nvelt; i++)
        UINT16ENCODE(bb, vg->tag[i]);

    /* save all refs */
    for (i = 0; i < (uintn)vg->nvelt; i++)
        UINT16ENCODE(bb, vg->ref[i]);

    /* save the vgnamelen and vgname - omit the null */
    if (vg->vgname != NULL)
        slen = (int16)HDstrlen(vg->vgname);
    temp_len = slen > 0 ? slen : 0;
    UINT16ENCODE(bb, temp_len);

    if (vg->vgname != NULL)
        HDstrcpy((char *) bb, vg->vgname);
    bb += temp_len;

    /* save the vgclasslen and vgclass- omit the null */
    if (vg->vgclass != NULL)
        slen = (int16)HDstrlen(vg->vgclass);
    temp_len = slen > 0 ? slen : 0;
    UINT16ENCODE(bb, temp_len);

    if (vg->vgclass != NULL)
        HDstrcpy((char *) bb, vg->vgclass);
    bb += temp_len;

    /* save the expansion tag/ref pair */
    UINT16ENCODE(bb, vg->extag);    /* the vg's expansion tag */
    UINT16ENCODE(bb, vg->exref);    /* the vg's expansion ref */

    if (vg->flags)  
      {   /* save the flag and update version num */
          if (vg->version < VSET_NEW_VERSION)   
              vg->version = VSET_NEW_VERSION;

          UINT32ENCODE(bb, vg->flags);

          if (vg->flags & VG_ATTR_SET)  
            {   /* save the attrs */
                INT32ENCODE(bb, vg->nattrs);

                for (i=0; i<(uintn)vg->nattrs; i++)  
                  {
                      UINT16ENCODE(bb, vg->alist[i].atag);
                      UINT16ENCODE(bb, vg->alist[i].aref);
                  }
            }
      }
       
    /*  save the vg's version field */
    UINT16ENCODE(bb, vg->version);

    /* save the vg's more field */
    UINT16ENCODE(bb, vg->more);

    /* returns the size of total fields saved */
    *size = (int32) (bb - buf) + 1;     
    *bb = 0;       /* NOTE: the '+1' part shouldn't be there */
    /* but since files have been created with */
    /* it there (and the size calc. wrong) it */
    /* has to be left alone -QAK */

#ifdef LATER
done:
    if (ret_value == FAIL)
    { /* Error condition cleanup */

    } /* end if */
#endif /* LATER */

  /* Normal function cleanup */
  return ret_value;
}   /* vpackvg */

/*******************************************************************************
NAME
   vunpackvg:

DESCRIPTION
   Unpacks the fields from a buf (ie a DFTAG_VG data object just
   read in from the HDF file), into a VGROUP structure vg.
   Will first zero out vg, unpack fields, then inits as much of
   vg as it can.

RETURNS   
   NO RETURN VALUES

*******************************************************************************/
PRIVATE intn
vunpackvg(VGROUP * vg, /* IN/OUT: */
          uint8 buf[], /* IN: */ 
          intn len     /* IN: */)
{
    uint8 *bb = NULL;
    uintn u;
    uint16 uint16var;
    intn i;
    int32 ret_value = SUCCEED;
    CONSTR(FUNC, "vunpackvg");

    /* clear error stack */
    HEclear();

    /* '5' is a magic number, the exact amount of space for 2 uint16's */
    /* the magic number _should_ be '4', but the size of the Vgroup */
    /* information is incorrectly calculated (in vpackvg() above) when the */
    /* info is written to the file and it's too late to change it now :-( */
    bb = &buf[len - 5];

    UINT16DECODE(bb, uint16var);  /* retrieve the vg's version field */
    vg->version=(int16)uint16var;

    UINT16DECODE(bb, uint16var);     /* retrieve the vg's more field */
    vg->more=(int16)uint16var;

    bb = &buf[0];

    /* retrieve nvelt */
    if (vg->version <= 4)
      {     /* current Vset version number */
          UINT16DECODE(bb, vg->nvelt);

          vg->msize = ((uintn)vg->nvelt > (uintn)MAXNVELT ? vg->nvelt : MAXNVELT);
          vg->tag = (uint16 *) HDmalloc(vg->msize * sizeof(uint16));
          vg->ref = (uint16 *) HDmalloc(vg->msize * sizeof(uint16));
    
          if ((vg->tag == NULL) || (vg->ref == NULL))
              HGOTO_ERROR(DFE_NOSPACE, FAIL);

          /* retrieve the tags */
          for (u = 0; u < (uintn)vg->nvelt; u++)
              UINT16DECODE(bb, vg->tag[u]);

          /* retrieve the refs */
          for (u = 0; u < (uintn)vg->nvelt; u++)
              UINT16DECODE(bb, vg->ref[u]);

          /* retrieve vgname (and its len)  */
          UINT16DECODE(bb, uint16var);

	  vg->vgname = (char *)HDmalloc(uint16var+1);
          HIstrncpy(vg->vgname, (char *) bb, (int32) uint16var + 1);
          bb += (size_t)uint16var;

          /* retrieve vgclass (and its len)  */
          UINT16DECODE(bb, uint16var);

	  vg->vgclass = (char *)HDmalloc(uint16var+1);
          HIstrncpy(vg->vgclass, (char *) bb, (int32) uint16var + 1);
          bb += (size_t)uint16var;

          UINT16DECODE(bb, vg->extag);  /* retrieve the vg's expansion tag */
          UINT16DECODE(bb, vg->exref);  /* retrieve the vg's expansion ref */

          if (vg->version == VSET_NEW_VERSION) 
            {
                UINT32DECODE(bb, vg->flags);  /* retrieve new features in
                                               version 4, or higher */
                if (vg->flags & VG_ATTR_SET)   
                  {   /* the vg has attrs */
                      INT32DECODE(bb, vg->nattrs); 

                      if (NULL == (vg->alist = HDmalloc(vg->nattrs * sizeof(vg_attr_t))))
                          HGOTO_ERROR(DFE_NOSPACE, FAIL);

                      for (i = 0; i < vg->nattrs; i++) 
                        {
                            UINT16DECODE(bb, vg->alist[i].atag);
                            UINT16DECODE(bb, vg->alist[i].aref);
                        } /* for */
                  }  /* attributes set */
            }  /* new version */
      }     /* end if */
done: 
    if (ret_value == FAIL)
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;

}   /* vunpackvg */

/*******************************************************************************
 NAME
    VPgetinfo  --  Read in the "header" information about the Vgroup.

 DESCRIPTION
    This routine pre-reads the header information for a Vgroup into memory
    so that it can be accessed more quickly by the routines that need it.

 RETURNS
    Return a pointer to a VGROUP filled with the Vgroup information on success,
    NULL on failure.
*******************************************************************************/
VGROUP *
VPgetinfo(HFILEID f,  /* IN: file handle */
          uint16 ref  /* IN: ref of vgroup */)
{
    VGROUP         *vg = NULL;
/*  intn          len;    intn mismatches Vgbufsize type -- uint32 */
    size_t          len;
    VGROUP *ret_value = NULL; /* FAIL */
    CONSTR(FUNC, "VPgetinfo");

    /* clear error stack */
    HEclear();

    /* Find out how long the VGroup information is */
    if (( len = Hlength(f, DFTAG_VG, (uint16) ref)) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL,NULL);
 
    if(len > Vgbufsize)
      {
        Vgbufsize = (uint32)len;

        if (Vgbuf)
            HDfree((VOIDP) Vgbuf);

        if ((Vgbuf = (uint8 *) HDmalloc(Vgbufsize)) == NULL)
            HGOTO_ERROR(DFE_NOSPACE, NULL);
      } /* end if */

    /* Get the raw Vgroup info */
    if (Hgetelement(f, DFTAG_VG, (uint16)ref, Vgbuf) == (int32)FAIL) 
        HGOTO_ERROR(DFE_NOMATCH,NULL);
       
    /* allocate space for vg */
    if (NULL == (vg =VIget_vgroup_node()))
        HGOTO_ERROR(DFE_NOSPACE,NULL);
       
    /* unpack vgpack into structure vg, and init  */
    vg->f             = f;
    vg->oref          = ref;
    vg->otag          = DFTAG_VG;
    if (FAIL == vunpackvg(vg,Vgbuf,(intn)len))
         HGOTO_ERROR(DFE_INTERNAL, NULL);

    /* return vgroup */
    ret_value = vg;

done:
  if(ret_value == NULL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
} /* end VPgetinfo */

/*******************************************************************************
NAME
  Vattach:

DESCRIPTION
   attaches to an existing vgroup or creates a new vgroup.
   returns NULL if  error, else ptr to vgroup.

   IGNORE accesstype. (but save it)

   if vgid == -1,
     create a NEW vg if vgdir is not full.
     Also set nattach =1, nentries=0.
   if vgid +ve,
        look in vgdir to see if already attached,
         if yes, incr nattach
         if not, fetch from file. attach, set nattach=1, netries= val from file
   
       In any case, set marked flag to 0.

RETRUNS
     SUCCEED/FAIL

*******************************************************************************/
int32
Vattach(HFILEID f,             /* IN: file handle */
        int32 vgid,            /* IN: vgroup id */
        const char *accesstype /* IN: access type */)
{
    VGROUP     *vg = NULL;
    vginstance_t *v = NULL;
    vfile_t    *vf = NULL;
    filerec_t  *file_rec = NULL;       /* file record */
    int16       acc_mode;
    atom_t      ret_value = FAIL;
    CONSTR(FUNC, "Vattach");

    /* clear error stack */
    HEclear();

    /* check file id */
    if (f == FAIL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get Vxxx file record */
    if ((vf = Get_vfile(f))==NULL)
        HGOTO_ERROR(DFE_FNF, FAIL);

    /* check access type to vgroup */
    if (accesstype[0] == 'R' || accesstype[0]=='r')
        acc_mode = 'r';
    else if (accesstype[0] == 'W' || accesstype[0]=='w')
        acc_mode = 'w';
    else
        HGOTO_ERROR(DFE_BADACC, FAIL);

    /* convert file id to file record and check for write-permission */
    file_rec = HAatom_object(f);
    if((file_rec==NULL || acc_mode=='w') && !(file_rec->access&DFACC_WRITE))
        HGOTO_ERROR(DFE_BADACC, FAIL);

    if (vgid == -1)
      {
          /******* create a NEW vg in vgdir ******/
          if (acc_mode == 'r')
              HGOTO_ERROR(DFE_ARGS, FAIL);

          /* allocate space for vg, & zero it out */
          if ((vg = VIget_vgroup_node()) == NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);

          /* initialize new vg */
          vg->msize = MAXNVELT;
          vg->tag = (uint16 *) HDmalloc(vg->msize * sizeof(uint16));
          vg->ref = (uint16 *) HDmalloc(vg->msize * sizeof(uint16));

          if ((vg->tag == NULL) || (vg->ref == NULL))
              HGOTO_ERROR(DFE_NOSPACE, FAIL);

          vg->f = f;
          vg->otag = DFTAG_VG;
          vg->oref = Hnewref(f);    /* create a new unique ref for it */
          if (vg->oref == 0)
              HGOTO_ERROR(DFE_NOREF, FAIL);

          vg->access = (intn)acc_mode;

          vg->marked = 1;
          vg->new_vg = 1;
          vg->version = VSET_VERSION;

          /* attach new vg to file's vgtab  */
          if (NULL == (v = VIget_vginstance_node()))
              HGOTO_ERROR(DFE_NOSPACE, FAIL);

          vf->vgtabn++;
          v->key = (int32) vg->oref;  /* set the key for the node */
          v->ref = (uintn)vg->oref;
          v->vg = vg;
          v->nattach = 1;
          tbbtdins(vf->vgtree, (VOIDP) v, NULL);    /* insert the vg instance in B-tree */

          ret_value=HAregister_atom(VGIDGROUP,v);
      }
    else
      {
          /******* access an EXISTING vg *********/
          if (NULL == (v = vginst(f, (uint16) vgid)))
              HGOTO_ERROR(DFE_NOMATCH, FAIL);

          /* vg already attached.  inc nattach and return existing ptr */
          if (v->nattach > 0)
            {
                v->vg->access = MAX(v->vg->access, acc_mode);
                v->nattach++;
            }
          else
            {
              vg=v->vg;
              vg->access = (intn)acc_mode;
              vg->marked = 0;

              /* attach vg to file's vgtab at the vg instance v */
              v->nattach = 1;
              v->nentries = (int32)vg->nvelt;
            } /* end else */

          ret_value=HAregister_atom(VGIDGROUP,v);
      }

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* Vattach */

/*******************************************************************************
NAME
   Vdetach

DESCRIPTION
   Detaches access to vg.
   
     if marked flag is 1, write out vg to file.
       if vg still has velts attached to it, cannot detach vg.
       decr  nattach. if (nattach is 0), free vg from vg instance.
       (check that no velts are still attached to vg before freeing)
   
     if attached with read access, just return.
   
    after detach, set marked flag to 0.

RETURNS
   SUCCEED / FAIL

*******************************************************************************/
int32
Vdetach(int32 vkey /* IN: vgroup key */)
{
    VGROUP       *vg = NULL;
    vginstance_t *v = NULL;
    int32       vgpacksize;
    int32      ret_value = SUCCEED;
    CONSTR(FUNC, "Vdetach");

    /* clear error stack */
    HEclear();

    /* check if vgroup is valid */
    if (HAatom_group(vkey) != VGIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *)HAremove_atom(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check it */
    vg = v->vg;
    if ((vg == NULL) || (vg->otag != DFTAG_VG))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Now, only update the Vgroup if it has actually changed. */
    /* Since only Vgroups with write-access are allowed to change, there is */
    /* no reason to check for access... (I hope) -QAK */
    if (vg->marked == 1)
      {
          size_t need, vgnamelen=0, vgclasslen=0;
	  if (vg->vgname != NULL)
	      vgnamelen = strlen(vg->vgname);

	  if (vg->vgclass != NULL)
	      vgclasslen = strlen(vg->vgclass);

          need = sizeof(VGROUP)
		+ vgnamelen	/* vgname dynamic, vpackvg omits null */
		+ vgclasslen	/* vgclass dynamic, vpackvg omits null */
		+ (size_t)vg->nvelt*4 + (size_t)vg->nattrs*sizeof(vg_attr_t) + 1;
          if(need > Vgbufsize)
            {
                Vgbufsize = (uint32)need;

                if (Vgbuf)
                    HDfree((VOIDP) Vgbuf);

                if ((Vgbuf = (uint8 *) HDmalloc(Vgbufsize)) == NULL)
                    HGOTO_ERROR(DFE_NOSPACE, FAIL);
            } /* end if */

          if (FAIL == vpackvg(vg, Vgbuf, &vgpacksize))
              HGOTO_ERROR(DFE_INTERNAL, FAIL);

          /*
           *  If vgroup alreay exists, try to re-use the same
           *  tag/ref. This will cause the pointer to the original
           *  vgroup to be lost but this is okay.
           */
          if(!vg->new_vg)
            {
                /* check if tag/ref exists in DD list already */
                switch(HDcheck_tagref(vg->f, DFTAG_VG, vg->oref))
                  {
                  case 0: /* not found */
                      break;
                  case 1: /* found, reuse tag/ref */
                      if (HDreuse_tagref(vg->f, DFTAG_VG, vg->oref) == FAIL)
                          HGOTO_ERROR(DFE_INTERNAL, FAIL);
                      break;
                  case -1: /* error */
                      HGOTO_ERROR(DFE_INTERNAL, FAIL);
                  default: /* should never get here */
                      HGOTO_ERROR(DFE_INTERNAL, FAIL);
                  } 
            }

          /* write out vgroup */
          if (Hputelement(vg->f, DFTAG_VG, vg->oref, Vgbuf, vgpacksize) == FAIL)
              HERROR(DFE_WRITEERROR);

          vg->marked = 0;
          vg->new_vg = 0;
      }

    v->nattach--;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* Vdetach */

/*******************************************************************************
NAME
   Vinsert

DESCRIPTION
   inserts a velt (vs or vg)  into a vg
   
   checks and prevents duplicate links.

   Since multiple files are now possible, check that both vg and velt
   are from the same file. else error.

RETURNS
    RETURNS entry position within vg (0 or +ve) or FAIL on error.

*******************************************************************************/
int32
Vinsert(int32 vkey,      /* IN: vgroup key */
        int32 insertkey  /* IN: */)
{
    VGROUP       *vg = NULL;
    vginstance_t *v = NULL;
    uint16      newtag = 0;
    uint16      newref = 0;
    int32       newfid;
    uintn       u;
    int32       ret_value = SUCCEED;
    CONSTR(FUNC, "Vinsert");

    /* clear error stack */
    HEclear();

    /* check to see if vgroup is valid */
    if (HAatom_group(vkey) != VGIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check it */
    vg = v->vg;
    if (vg == NULL)
        HGOTO_ERROR(DFE_BADPTR, FAIL);

    /* check write access */
    if (vg->otag != DFTAG_VG || vg->access != 'w')
        HGOTO_ERROR(DFE_ARGS, FAIL);

    newfid = FAIL;
    if (HAatom_group(insertkey) == VSIDGROUP)
      {   /* locate vs's index in vstab */
        vsinstance_t *w;

        if (NULL == (w = (vsinstance_t *) HAatom_object(insertkey)))
            HGOTO_ERROR(DFE_NOVS, FAIL);

        if (w->vs == NULL)
            HGOTO_ERROR(DFE_ARGS, FAIL);

        newtag = DFTAG_VH;
        newref = w->vs->oref;
        newfid = w->vs->f;
      }
    else
      {
        vginstance_t *x;

        if (HAatom_group(insertkey) == VGIDGROUP)
          {   /* locate vs's index in vgtab */
            if (NULL == (x = (vginstance_t *) HAatom_object(insertkey)))
                HGOTO_ERROR(DFE_NOVS, FAIL);

            if (x->vg == NULL)
                HGOTO_ERROR(DFE_ARGS, FAIL);

            newtag = DFTAG_VG;
            newref = x->vg->oref;
            newfid = x->vg->f;
          }
      }

    /* make sure we found something */
    if (newfid == FAIL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    if (vg->f != newfid)
        HGOTO_ERROR(DFE_DIFFFILES, FAIL);

    /* check and prevent duplicate links */
    for (u = 0; u < (uintn)vg->nvelt; u++)
      {
        if ((vg->ref[u] == newref) && (vg->tag[u] == newtag))
            HGOTO_ERROR(DFE_DUPDD, FAIL);
      }

    /* Finally, ok to insert */
    if (vinsertpair(vg, newtag, newref) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    ret_value = (vg->nvelt - 1);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* Vinsert */

/*******************************************************************************
NAME
  Vflocate

DESCRIPTION
   Checks to see if the given field exists in a vdata belonging to this vgroup.

   28-MAR-91 Jason Ng NCSA

RETURNS
   If found, returns the ref of the vdata.
   If not found, or error, returns FAIL

*******************************************************************************/
int32
Vflocate(int32 vkey,  /* IN: vdata key */
         char *field  /* IN: field to locate */)
{
    uintn u;
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
    int32       vskey;
    int32       ret_value = SUCCEED;
    CONSTR(FUNC, "Vflocate");

    /* clear error stack */
    HEclear();

    /* check if vgroup is valid */
    if (HAatom_group(vkey) != VGIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check */
    vg = v->vg;
    if (vg == NULL)
        HGOTO_ERROR(DFE_BADPTR, FAIL);

    for (u = 0; u < (uintn)vg->nvelt; u++)
      {
          intn       s;

          if (vg->tag[u] != VSDESCTAG)
              continue;

          vskey = VSattach(vg->f, (int32)vg->ref[u], "r");
          if (vskey == FAIL)
              HGOTO_DONE(FAIL);

          s = VSfexist(vskey, field);

          if (VSdetach(vskey) == FAIL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);

          if (s == 1)
              HGOTO_DONE((int32)vg->ref[u]);  /* found. return vdata's ref */
      }

    ret_value = (FAIL);  /* field not found */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* Vflocate */

/*******************************************************************************
NAME
   Vinqtagref

DESCRIPTION
    Checks whether the given tag/ref pair already exists in the vgroup.
    28-MAR-91 Jason Ng NCSA

RETURNS
    RETURNS TRUE if exist
    RETURNS FALSE if not.

*******************************************************************************/
intn
Vinqtagref(int32 vkey, /* IN: vgroup key */ 
           int32 tag,  /* IN: tag to check in vgroup */
           int32 ref   /* IN: ref to check in vgroup */) 
{
    uintn u;
    uint16 ttag;
    uint16 rref;
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
    intn       ret_value = FALSE;
    CONSTR(FUNC, "Vinqtagref");

    /* clear error stack */
    HEclear();

    /* check if vgroup is valid */
    if (HAatom_group(vkey) != VGIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FALSE);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FALSE);

    /* get vgroup itself and check */
    vg = v->vg;
    if (vg == NULL)
        HGOTO_ERROR(DFE_BADPTR, FALSE);

    ttag = (uint16) tag;
    rref = (uint16) ref;

    for (u = 0; u < (uintn)vg->nvelt; u++)
      {
        if ((ttag == vg->tag[u]) && (rref == vg->ref[u]))
          HGOTO_DONE(TRUE);
      }

done:
  if(ret_value == FALSE)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* Vinqtagref */

/*******************************************************************************
 NAME
   Vdeletetagref - delete tag/ref pair in Vgroup

 DESCRIPTION
    Deletes the given tag/ref pair from the Vgroup.  If the given tag/ref pair 
    does not exist in the vgroup the routine will return FAIL. Users should use 
    Vinqtagref() to check if the tag/ref pair exists before deleting.

 RETURNS
    Returns SUCCEED if the tag/ref pair is deleted from Vgroup and
    FAIL if unsuccessful.

 Author -GeorgeV 10/10/97
*******************************************************************************/
intn
Vdeletetagref(int32 vkey, /* IN: vgroup key */ 
              int32 tag,  /* IN: tag to delete in vgroup */
              int32 ref   /* IN: ref to delete in vgroup */) 
{
    uintn         i,j;       /* loop indices */
    uint16        ttag;      /* tag for comparison */
    uint16        rref;      /* ref for comparison */
    vginstance_t *v  = NULL; /* vgroup instance struct */
    VGROUP       *vg = NULL; /* in-memory vgroup struct */
    intn          ret_value = SUCCEED;
    CONSTR(FUNC, "Vdeletetagref");

   /* NOTE: Move the following comments to the DESCRIPTION of the
            fcn when the issue with duplicate tag/refs is decided.

    If duplicate tag/ref pairs exist, then it deletes the first occurence.
    If the case of duplicate tag/ref pairs the user can call Vinqtagref() 
    to see if there are more occurences and then delete them. */

    /* clear error stack */
    HEclear();

    /* check if vgroup is valid */
    if (HAatom_group(vkey) != VGIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check */
    vg = v->vg;
    if (vg == NULL)
        HGOTO_ERROR(DFE_BADPTR, FAIL);

    /* set comparsion tag/ref pair */
    ttag = (uint16) tag;
    rref = (uint16) ref;

    /* look through elements in vgroup */
    for (i = 0; i < (uintn)vg->nvelt; i++)
      { /* see if element tag/ref matches search tag/ref */
          if ((ttag == vg->tag[i]) && (rref == vg->ref[i]))
            { /* found tag/ref pair to delete. 
                 If duplicate tag/ref pairs exist, then it deletes 
                 the first occurence. If the case of duplicate tag/ref 
                 pairs the user can call Vinqtagref() to see if there 
                 are more occurences and then delete them.*/

                /* check if element found is last one in vgroup */
                if ( i != ((uintn)vg->nvelt - 1))
                  { /* Basically shifts the contents of the array down by one. 
                       This method will preserve the order without using
                       extra memory for storage etc. If speed/performance
                       is an issue you can use memove()/memcpy(). */
                      for (j = i; j < (uintn)vg->nvelt - 1; j++)
                        {
                            vg->tag[j] = vg->tag[j+1];
                            vg->ref[j] = vg->ref[j+1];
                        }
#if 0
                      /* This method is quick but does not preserve the
                         order of elements in a vgroup.
                         swap i'th element with last one. */
                      vg->tag[i] = vg->tag[(uintn)vg->nvelt - 1];
                      vg->ref[i] = vg->ref[(uintn)vg->nvelt - 1];
#endif

                  }
                /* else if last one , do nothing and allow the 
                   number of elements to be decrementd. */

                /* reset last ones, just to be sure  */
                vg->tag[(uintn)vg->nvelt - 1] = DFTAG_NULL;
                vg->ref[(uintn)vg->nvelt - 1] = 0; /* invalid ref */

                vg->nvelt--; /* decrement number of elements in vgroup */
                vg->marked = TRUE; /* mark vgroup as changed. 
                                      forces re-writing of new vgroup. */
                ret_value = SUCCEED;
                goto done; /* we are done */
            } /* if found */
      } /* for all items in vgroup */

    /* reaching here means tag/ref pair not found. The user
       should have used Vinqtagref() before calling this fcn. 
       Oh well...*/
    ret_value = FAIL;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* Vdeletetagref */

/*******************************************************************************
NAME
   Vntagrefs

DESCRIPTION
    Returns the number (0 or +ve integer) of tag/ref pairs in a vgroup.
    If error, returns FAIL
    28-MAR-91 Jason Ng NCSA.

RETURNS

*******************************************************************************/
int32
Vntagrefs(int32 vkey /* IN: vgroup key */)
{
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
    int32      ret_value = SUCCEED;
    CONSTR(FUNC, "Vntagrefs");

    /* clear error stack */
    HEclear();

    /* check if vgroup is valid */
    if (HAatom_group(vkey)!=VGIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check */
    vg = v->vg;
    if (vg == NULL)
        HGOTO_ERROR(DFE_BADPTR, FAIL);

    ret_value = ((vg->otag == DFTAG_VG) ? (int32) vg->nvelt : FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* Vntagrefs */

/*******************************************************************************
NAME
   Vnrefs

DESCRIPTION
   Returns the number (0 or +ve integer) of tags of a given type in a vgroup.
   If error, returns FAIL
   05-NOV-94 Quincey Koziol.

RETURNS

*******************************************************************************/
int32
Vnrefs(int32 vkey, /* IN: vgroup key */
       int32 tag   /* IN: tag to find refs for */)
{
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
    uint16 ttag = (uint16)tag;    /* alias for faster comparison */
    uintn u;                    /* local counting variable */
    int32     ret_value = 0;   /* zero refs to start */
    CONSTR(FUNC, "Vnrefs");

    /* clear error stack */
    HEclear();

    /* check if vgroup is valid */
    if (HAatom_group(vkey) != VGIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check it */
    vg = v->vg;
    if (vg == NULL)
        HGOTO_ERROR(DFE_BADPTR, FAIL);

    for (u = 0; u < (uintn)vg->nvelt; u++)
      {
        if (ttag == vg->tag[u])
            ret_value++;
      }

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* Vnrefs */

/*******************************************************************************
NAME
   Vgettagrefs

DESCRIPTION
    Returns n tag/ref pairs from the vgroup into the caller-supplied arrays
    tagrarray and refarray.
    n can be any +ve number, but arrays must be this big.
    28-MAR-91 Jason Ng NCSA.
   
    NOTE: Do not confuse with Vgettagref().

RETURNS
    The total number of (0 or +ve #)  tag/ref pairs returned.
   
*******************************************************************************/
int32
Vgettagrefs(int32 vkey,       /* IN: vgroup key */
            int32 tagarray[], /* IN/OUT: tag array to fill */
            int32 refarray[], /* IN/OUT: ref array to fill */
            int32 n           /* IN: number of pairs to return */)
{
    int32       i;
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
    int32      ret_value = SUCCEED;
    CONSTR(FUNC, "Vgettagrefs");

    /* clear error stack */
    HEclear();

    /* check if vgroup is valid */
    if (HAatom_group(vkey)!=VGIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check */
    vg = v->vg;
    if (vg == NULL)
        HGOTO_ERROR(DFE_BADPTR, FAIL);

    if (n > (int32) vg->nvelt)
        n = (int32)vg->nvelt;

    for (i = 0; i < n; i++)
      {
          tagarray[i] = (int32) vg->tag[i];
          refarray[i] = (int32) vg->ref[i];
      }

    ret_value = (n);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* Vgettagrefs */

/*******************************************************************************
NAME
   Vgettagref

DESCRIPTION
   Returns a specified tag/ref pair from the vgroup.
   User specifies an index.
   12-MAY-91 Jason Ng NCSA.

   NOTE: Do not confuse with Vgettagrefs().

RETURNS
   RETURNS FAIL if OK.
   RETURNS SUCCEED if error.

*******************************************************************************/
intn
Vgettagref(int32 vkey,   /* IN: vgroup key */
           int32 which,  /* IN: hmm */
           int32 *tag,   /* IN/OUT: tag to return */
           int32 *ref    /* IN/OUT: ref to return */)
{
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
    intn       ret_value = SUCCEED;
    CONSTR(FUNC, "Vgettagref");

    /* clear error stack */
    HEclear();

    /* check if vgroup is valid */
    if (HAatom_group(vkey) != VGIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check */
    vg = v->vg;
    if (vg == NULL)
        HGOTO_ERROR(DFE_BADPTR, FAIL);

    if (which < 0 || which > (int32) (vg->nvelt - 1))
        HGOTO_ERROR(DFE_RANGE, FAIL);     /* range err */

    *tag = (int32) vg->tag[which];
    *ref = (int32) vg->ref[which];

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* Vgettagref */

/*******************************************************************************
NAME
   VQuerytag

DESCRIPTION
  Return the tag of this Vgroup.
  Return 0 on failure

RETURNS

*******************************************************************************/
int32
VQuerytag(int32 vkey /* IN: vgroup key */)
{
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
    int32      ret_value = SUCCEED;
    CONSTR(FUNC, "Vgettagref");

    /* clear error stack */
    HEclear();

    /* check if vgroup is valid */
    if (HAatom_group(vkey)!=VGIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check */
    vg = v->vg;
    if (vg == NULL)
        HGOTO_ERROR(DFE_BADPTR, FAIL);

    ret_value = ((int32) vg->otag);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* VQuerytag */

/*******************************************************************************
NAME
   VQueryref

DESCRIPTION
   Return the ref of this Vgroup.
   Return FAIL on failure

RETURN
*******************************************************************************/
int32
VQueryref(int32 vkey /* IN: vgroup id */)
{
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
    int32     ret_value = SUCCEED;
    CONSTR(FUNC, "Vgettagref");

    /* clear error stack */
    HEclear();

    /* check if vgroup is valid */
    if (HAatom_group(vkey)!=VGIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check */
    vg = v->vg;
    if (vg == NULL)
        HGOTO_ERROR(DFE_BADPTR, FAIL);

    ret_value = ((int32) vg->oref);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* VQueryref */

/*******************************************************************************
NAME
  Vaddtagref

DESCRIPTION
  Inserts a tag/ref pair into the attached vgroup vg.
  First checks that the tag/ref is unique. (6/20/96 Maybe the original
   design required the uniqueness. However, the current code allows
   duplication if NO_DUPLICATES is not defined. The SD interface needs
   this feature to create SDS's with duplicated dimensions. For example
   a 3D SDS has dimensions "time", "presure" and "presure".)
  If error, returns FAIL or tag/ref is not inserted.
  If OK, returns the total number of tag/refs in the vgroup (a +ve integer).
  28-MAR-91 Jason Ng NCSA.

RETURNS

*******************************************************************************/
int32
Vaddtagref(int32 vkey, /* IN: vgroup key */
           int32 tag,  /* IN: tag to add */
           int32 ref   /* IN: ref to add */)
{
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
#ifdef NO_DUPLICATES
    uintn        i;
#endif /* NO_DUPLICATES */
    int32      ret_value = SUCCEED;
    CONSTR(FUNC, "Vaddtagref");

    /* clear error stack */
    HEclear();

    /* check if vgroup is valid */
    if (HAatom_group(vkey) != VGIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check */
    vg = v->vg;
    if (vg == NULL)
        HGOTO_ERROR(DFE_BADPTR, FAIL);

#ifdef NO_DUPLICATES
    /* SD interface needs duplication if two dims have the same name.
       So, don't remove the ifdef/endif pair.   */
    /* make sure doesn't already exist in the Vgroup */
    for (i = 0; i < vg->nvelt; i++)
        if ((tag == vg->tag[i]) && (ref == vg->ref[i]))
            HGOTO_DONE(FAIL);
#endif  /* NO_DUPLICATES  */

    ret_value = vinsertpair(vg, (uint16) tag, (uint16) ref);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* Vaddtagref */

/*******************************************************************************
NAME
  vinsertpair

DESCRIPTION
   Inserts a tag/ref pair into the attached vgroup vg.
   Expand the tag/ref space if necessary

RETURNS
    Returns the total number of tag/refs in the vgroup.

*******************************************************************************/
int32
vinsertpair(VGROUP * vg, /* IN: vgroup struct */
            uint16 tag,  /* IN: tag to insert */
            uint16 ref   /* IN: ref to insert */)
{
    int32    ret_value = SUCCEED;
    CONSTR(FUNC, "vinsertpair");

    /* clear error stack */
    HEclear();

    if ((intn)vg->nvelt >= vg->msize)
      {
          vg->msize *= 2;

          vg->tag = (uint16 *) HDrealloc((VOIDP) vg->tag, vg->msize * sizeof(uint16));
          vg->ref = (uint16 *) HDrealloc((VOIDP) vg->ref, vg->msize * sizeof(uint16));

          if ((vg->tag == NULL) || (vg->ref == NULL))
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
      }

    vg->tag[(uintn)vg->nvelt] = tag;
    vg->ref[(uintn)vg->nvelt] = ref;
    vg->nvelt++;

    vg->marked = TRUE;
    ret_value = ((int32) vg->nvelt);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
} /* vinsertpair() */

/*******************************************************************************
NAME
   Ventries

DESCRIPTION
    returns the no of entries (+ve integer) in the vgroup vgid.
    vgid must be an actual id

    undocumented

RETURNS
    RETURNS FAIL if error

*******************************************************************************/
int32
Ventries(HFILEID f,  /* IN: file handle */
         int32 vgid  /* IN: vgroup id */)
{
    vginstance_t *v = NULL;
    int32      ret_value = SUCCEED;
    CONSTR(FUNC, "Ventries");

    /* clear error stack */
    HEclear();

    /* check vgroup id? */
    if (vgid < 1)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    if((v = vginst(f,(uint16)vgid))==NULL)
        HGOTO_ERROR(DFE_NOMATCH,FAIL);          /* error */

    if (v->vg != NULL)
        ret_value = (int32)v->vg->nvelt;
    else
        ret_value = FAIL;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* Ventries */

/*******************************************************************************
NAME
   Vsetname

DESCRIPTION
   gives a name to the VGROUP vg.

RETURNS
    RETURN VALUES: SUCCEED for success, FAIL for failure (big suprise, eh?)
   
*******************************************************************************/
int32
Vsetname(int32 vkey,         /* IN: vgroup key */
         const char *vgname  /* IN: name to set for vgroup */) 
{
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
    uint16 name_len;
    int32      ret_value = SUCCEED;
    CONSTR(FUNC, "Vsetname");

    /* clear error stack */
    HEclear();

    /* check if vgroup is valid and if vgroup name is okay */
    if (HAatom_group(vkey) != VGIDGROUP || vgname == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check */
    vg = v->vg;
    if (vg == NULL || vg->access!='w')
        HGOTO_ERROR(DFE_BADPTR, FAIL);

    /* copy the name over */
    name_len = (uint16)HDstrlen(vgname);
    vg->vgname = (char *)HDmalloc(name_len+1);
    HIstrncpy(vg->vgname, vgname, name_len+1);

    vg->marked = TRUE;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* Vsetname */

/*******************************************************************************
NAME
   Vsetclass

DESCRIPTION
    assigns a class name to the VGROUP vg.
   
RETURNS
    RETURN VALUES: SUCCEED for success, FAIL for failure (big suprise, eh?)

MODIFICATION
    2010/01/26 No longer truncates classname to max length of VGNAMELENMAX.
   
*******************************************************************************/
int32
Vsetclass(int32 vkey,          /* IN: vgroup key */
          const char *vgclass  /* IN: class to set for vgroup */)
{
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
    uint16       classname_len;
    int32        ret_value = SUCCEED;
    CONSTR(FUNC, "Vsetclass");

    /* clear error stack */
    HEclear();

    /* check if vgroup is valid */
    if (HAatom_group(vkey) != VGIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* check if class is valid */
    if (vgclass == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check. Also check write access to vgroup  */
    vg = v->vg;
    if (vg == NULL || vg->access != 'w')
        HGOTO_ERROR(DFE_BADPTR, FAIL);

    /* copy class over */
    classname_len = (uint16)HDstrlen(vgclass);
    vg->vgclass = (char *)HDmalloc(classname_len+1);
    HIstrncpy(vg->vgclass, vgclass, classname_len+1);

    vg->marked = TRUE;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* Vsetclass */

/*******************************************************************************
NAME
   Visvg

DESCRIPTION
   Tests if an entry in the vgroup vg is a VGROUP, given the entry's id.
   
RETURNS
    RETURNS TRUE if so
    RETURNS FALSE if not, or if error
   
*******************************************************************************/
intn
Visvg(int32 vkey, /* IN: vgroup key */
      int32 id    /* IN: id of entry in vgroup */)
{
    uintn        u;
    uint16       ID;
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
    intn         ret_value = FALSE; /* initialize to FALSE */
    CONSTR(FUNC, "Visvg");

    /* clear error stack */
    HEclear();

    /* check if vgroup is valid */
    if (HAatom_group(vkey) != VGIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FALSE);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FALSE);

    /* get vgroup itself and check */
    vg = v->vg;
    if (vg == NULL)
        HGOTO_ERROR(DFE_BADPTR, FALSE);

    ID = (uint16) id; /* cast 32bit ID to 16bit id */

    for (u = 0; u < (uintn)vg->nvelt; u++)
      {
        if (vg->ref[u] == ID &&     /* if the ids match, */
            vg->tag[u] == DFTAG_VG)     /* and it is a vgroup */
          {
                HGOTO_DONE(TRUE);
          }
      }

done:
  if(ret_value == FALSE)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* Visvg */

/*******************************************************************************
NAME
   Visvs

DESCRIPTION
   Checks if an id in a vgroup refers to a VDATA

RETURNS
   RETURNS 1 if so
   RETURNS 0 if not, or if error.

*******************************************************************************/
intn
Visvs(int32 vkey, /* IN: vgroup key */
      int32 id    /* IN: id of entry in vgroup */)
{
    intn         i;
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
    intn         ret_value = FALSE; /* initialize to false */
    CONSTR(FUNC, "VSisvs");

    /* clear error stack */
    HEclear();

    /* check if vgroup is valid */
    if (HAatom_group(vkey) != VGIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FALSE);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FALSE);

    /* get vgroup itself and check */
    vg = v->vg;
    if (vg == NULL)
        HGOTO_ERROR(DFE_BADPTR, FALSE);

    i = (intn)vg->nvelt;
    while (i)
      {
        if (vg->ref[--i] == (uint16) id && vg->tag[i] == VSDESCTAG)
          HGOTO_DONE(TRUE);
      }

done:
  if(ret_value == FALSE)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* Visvs */

/*******************************************************************************
NAME
   Vgetid

DESCRIPTION
   Given a vgroup's id, returns the next vgroup's id in the file f .
   The call Vgetid(f,-1) returns the id of the FIRST vgroup in the file.

   This id is actually the "ref" of the vgroup "tag/ref".

RETURNS
   RETURNS -1 if error
   RETURNS the next vgroup's id (0 or +ve integer).
   
*******************************************************************************/
int32
Vgetid(HFILEID f,  /* IN: file handle */
       int32 vgid  /* IN: vgroup id */)
{
    vginstance_t *v = NULL;
    vfile_t      *vf = NULL;
    VOIDP        *t = NULL;
    int32        key;
    int32        ret_value = SUCCEED;
    CONSTR(FUNC, "Vgetid");


    /* clear error stack */
    HEclear();

    /* check if vgroup id is valid */
    if (vgid < -1)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get vgroup file record */
    if (NULL == (vf = Get_vfile(f)))
        HGOTO_ERROR(DFE_FNF, FAIL);

    if (vgid == (-1))
      {     /* check for magic value to return the first group */
          if (vf->vgtree == NULL )
              HGOTO_DONE(FAIL); /* just return FAIL, no error */

          if (NULL == (t = (VOIDP *) tbbtfirst((TBBT_NODE *) * (vf->vgtree))))
              HGOTO_DONE(FAIL); /* just return FAIL, no error */

          /* t is assumed to valid at this point */
          v = (vginstance_t *) * t;   /* get actual pointer to the vginstance_t */
          HGOTO_DONE((int32)v->ref); /* rets 1st vgroup's ref */
      }     
    else /* vgid >= 0 */
      {
          /* look in vgtab for vgid */
          /* tbbtdfind returns a pointer to the vginstance_t pointer */
          key = (int32)vgid;
          t = (VOIDP *) tbbtdfind(vf->vgtree, (VOIDP) &key, NULL);

          if (t == NULL ||
              t == (VOIDP *) tbbtlast((TBBT_NODE *) * (vf->vgtree)))  
            { /* couldn't find the old vgid or at the end */
              ret_value = (FAIL);  
            }
          else if (NULL == (t = (VOIDP *) tbbtnext((TBBT_NODE *) t))) /* get the next node in the tree */
              ret_value = (FAIL);
          else
            {
                v = (vginstance_t *) * t;     /* get actual pointer to the vginstance_t */
                ret_value = (int32)v->ref;  /* rets next vgroup's ref */
            }     /* end else */
      }
done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* Vgetid */

/*******************************************************************************
NAME
   Vgetnext

DESCRIPTION
   Given the id of an entry from a vgroup vg, looks in vg for the next
   entry after it, and returns its id.
   The call Vgetnext (vg,-1) returns the id of the FIRST entry in the vgroup.

   Vgetnext will look at only VSET elements in the vgroup.
   To look at all links in a vgroup, use Vgettagrefs instead.

  This id is actually the "ref" of the entry's "tag/ref".
   
RETURNS
   RETURNS -1 if error
   RETURNS the id of the next entry( 0 or +ve integer)  in the vgroup.
   
   
*******************************************************************************/
int32
Vgetnext(int32 vkey, /* IN: vgroup key */
         int32 id    /* IN: id of entry in vgroup */)
{
    uintn        u;
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
    int32        ret_value = FAIL;
    CONSTR(FUNC, "Vgetnext");

    /* clear error stack */
    HEclear();

    /* check if vgroup is valid. Also check if 'id' is valid */
    if (HAatom_group(vkey) != VGIDGROUP || id < (-1))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check. */
    vg = v->vg;
    if ((vg == NULL) || (vg->otag != DFTAG_VG))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* hmm..if no elements we return FAIL */
    if (vg->nvelt == 0)
        HGOTO_DONE(FAIL);

    /* if id is '-1' then the first entry in the vgroup is returned 
       if it is a vgroup ? */
    if (id == -1)
      {
          if ((vg->tag[0] == DFTAG_VG) || (vg->tag[0] == VSDESCTAG))
              HGOTO_DONE((int32)vg->ref[0]);  /* id of first entry */
      }     /* end if */

    /* look in vgroup for 'id' */
    for (u = 0; u < (uintn)vg->nvelt; u++)
      { /* only look for vgroups? */
        if ((vg->tag[u] == DFTAG_VG) || (vg->tag[u] == VSDESCTAG))
          {
              if (vg->ref[u] == (uint16) id)
                {
                    if (u == (uintn)(vg->nvelt - 1))
                      {
                        HGOTO_DONE(FAIL);
                      } /* end if */
                    else
                      {
                          if ((vg->tag[u + 1] == DFTAG_VG) || (vg->tag[u + 1] == VSDESCTAG))
                            {
                              HGOTO_DONE((int32)vg->ref[u + 1]);  /* return the id of next entry */
                            }
                          else
                            {
                              HGOTO_DONE(FAIL);
                            }
                      }     /* end else */
                }   /* end if */
          }     /* end if */
      }

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* Vgetnext  */

/*******************************************************************************
NAME
   Vgetnamelen

DESCRIPTION
   Retrieves the length of the vgroup's name.

RETURNS
   Returns SUCCEED/FAIL
   BMR - 2006/09/10
   
*******************************************************************************/
int32
Vgetnamelen(int32 vkey,   /* IN: vgroup key */
	    uint16 *name_len /* OUT: length of vgroup's name */)
{
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
    size_t       temp_len;
    int32        ret_value = SUCCEED;
    CONSTR(FUNC, "Vgetnamelen");


    /* clear error stack */
    HEclear();

    /* check if vgroup is valid and the vgname */
    if (HAatom_group(vkey)!=VGIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check */
    vg = v->vg;
    if (vg == NULL)
        HGOTO_ERROR(DFE_BADPTR, FAIL);

    /* obtain the name length */
    temp_len = HDstrlen(vg->vgname);
    *name_len = temp_len > 0 ? (uint16)temp_len : 0;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* Vgetnamelen */

/*******************************************************************************
NAME
   Vgetclassnamelen

DESCRIPTION
   Retrieves the length of the vgroup's name.

RETURNS
   Returns SUCCEED/FAIL
   BMR - 2006/09/10
   
*******************************************************************************/
int32
Vgetclassnamelen(int32 vkey,   /* IN: vgroup key */
	    uint16 *classname_len /* OUT: length of vgroup's classname */)
{
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
    size_t       temp_len;
    int32        ret_value = SUCCEED;
    CONSTR(FUNC, "Vgetclassnamelen");


    /* clear error stack */
    HEclear();

    /* check if vgroup is valid and the vgname */
    if (HAatom_group(vkey)!=VGIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check */
    vg = v->vg;
    if (vg == NULL)
        HGOTO_ERROR(DFE_BADPTR, FAIL);

    /* obtain the name length */
    temp_len = HDstrlen(vg->vgclass);
    *classname_len = temp_len > 0 ? (uint16)temp_len : 0;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* Vgetclassnamelen */

/*******************************************************************************
NAME
   Vgetname

DESCRIPTION
   returns the vgroup's name
   ASSUME that vgname has been allocated large enough to hold
   the name

RETURNS
   SUCCEED / FAIL
   
*******************************************************************************/
int32
Vgetname(int32 vkey,   /* IN: vgroup key */
         char *vgname  /* IN/OUT: vgroup name */)
{
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
    int32        ret_value = SUCCEED;
    CONSTR(FUNC, "Vgetname");


    /* clear error stack */
    HEclear();

    /* check if vgroup is valid and the vgname */
    if (HAatom_group(vkey)!=VGIDGROUP || vgname==NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check */
    vg = v->vg;
    if (vg == NULL)
        HGOTO_ERROR(DFE_BADPTR, FAIL);

    /* copy vgroup name over */
    HDstrcpy(vgname, vg->vgname);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* Vgetname */

/*******************************************************************************
NAME
   Vgetclass

DESCRIPTION
   returns the vgroup's class name
   ASSUME that vgclass has been allocated large enough to hold
   the name

RETURNS
   SUCCEED/FAIL

*******************************************************************************/
int32
Vgetclass(int32 vkey,    /* IN: vgroup key */
          char *vgclass  /* IN/OUT: vgroup class */)
{
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
    int32        ret_value = SUCCEED;
    CONSTR(FUNC, "Vgetclass");


    /* clear error stack */
    HEclear();

    /* check if vgroup is valid and also vgroup class */
    if (HAatom_group(vkey) != VGIDGROUP || vgclass==NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check */
    vg = v->vg;
    if (vg == NULL)
        HGOTO_ERROR(DFE_BADPTR, FAIL)
            ;
    /* copy class over */
    HDstrcpy(vgclass, vg->vgclass);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* Vgetclass */

/*******************************************************************************
NAME
   Vinquire

DESCRIPTION
   General inquiry routine for VGROUP.
   output parameters:
         nentries - no of entries in the vgroup
         vgname  - the vgroup's name
   
RETURNS
   RETURNS FAIL if error
   RETURNS SUCCEED if ok

*******************************************************************************/
intn
Vinquire(int32 vkey,        /* IN: vgroup key */
         int32 *nentries,   /* IN/OUT: number of entries in vgroup */
         char *vgname       /* IN/OUT: vgroup name */)
{
    vginstance_t *v = NULL;
    VGROUP       *vg = NULL;
    intn    ret_value = SUCCEED;
    CONSTR(FUNC, "Vinquire");


    /* clear error stack */
    HEclear();

    /* check if vgroup is valid */
    if (HAatom_group(vkey) != VGIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get instance of vgroup */
    if (NULL == (v = (vginstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vgroup itself and check */
    vg = v->vg;
    if (vg == NULL)
        HGOTO_ERROR(DFE_BADPTR, FAIL);

    /* check tag of vgroup */
    if (vg->otag != DFTAG_VG)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* copy vgroup name if requested.  Assumes 'vgname' has sufficient space */
    if (vgname != NULL)
        HDstrcpy(vgname, vg->vgname);

    /* set number of entries in vgroup if requested */
    if (nentries != NULL)
        *nentries = (int32)vg->nvelt;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* Vinquire */


/*******************************************************************************
NAME
   Vopen

DESCRIPTION
   This routine will replace the code segment " Hopen(); Vinitialize(f)".
   Thus, if Vopen is used, do not call Vinitialize after that.
   
   Similar to Hopen().
   
   This routine opens the HDF file and initializes it for Vset operations.

   See also Vclose().

   By: Jason Ng 10 Aug 92

RETURNS
    RETURN VALUE:
    if error:  -1 (FAIL).
    if successful: the id of the file (>0).
   
*******************************************************************************/
HFILEID
Vopen(char *path,     /* IN: file name */
      intn acc_mode,  /* IN: type of file access */
      int16 ndds      /* IN: number of DD in a block */)
{
    HFILEID    ret_value=SUCCEED;
    CONSTR(FUNC, "Vopen");

    /* clear error stack */
    HEclear();

    /* use 'Hopen' to open file */
    if ((ret_value = Hopen(path, acc_mode, ndds))== FAIL)
      HGOTO_ERROR(DFE_BADOPEN,FAIL);

    /* Initialize Vxx interface */
    if(Vinitialize(ret_value)==FAIL)
      HGOTO_ERROR(DFE_CANTINIT,FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
} /* Vopen() */

/*******************************************************************************
NAME
  Vclose

DESCRIPTION
   This routine will replace the code segment " Vfinish(f); Hclose(f);".
   Thus, if Vclose is used, do not call Vfinish before that.
   
   This routine closes the HDF file, after it has freed all memory and
   updated the file.
   
   See also Vopen().

RETURNS
   RETURN VALUE:  intn status - result of Hopen().
   
*******************************************************************************/
intn
Vclose(HFILEID f /* IN: file handle */)
{
#ifdef LATER
    CONSTR(FUNC, "Vclose");
#endif
    intn ret_value = SUCCEED;

    if (Vfinish(f) == FAIL)
        ret_value = FAIL;
    else
        ret_value = (Hclose(f));

    return ret_value;
} /* Vclose() */

/*******************************************************************************
NAME
   Vdelete

DESCRIPTION
   Remove a Vgroup from its file.  This function will both remove the Vgoup
   from the internal Vset data structures as well as from the file.

   'vgid' here is actually the ref of the Vgroup.

   (i.e. it calls tbbt_delete() and Hdeldd())

RETURNS
   Return FAIL / SUCCEED

*******************************************************************************/
int32
Vdelete(int32 f,     /* IN: file handle */
        int32 vgid   /* IN: vgroup id i.e. ref */)
{
    VOIDP       v;
    vfile_t    *vf = NULL;
    VOIDP      *t = NULL;
    int32       key;
    filerec_t  *file_rec = NULL;       /* file record */
    int32       ret_value = SUCCEED;
    CONSTR(FUNC, "Vdelete");

    /* clear error stack */
    HEclear();

    /* check vgroup id is valid */
    if (vgid < 0)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* convert file id to file record */
    if ((file_rec = HAatom_object(f)) == NULL)
        HGOTO_ERROR(DFE_BADACC, FAIL);

    /* check for write-permission to file*/
    if(!(file_rec->access & DFACC_WRITE))
        HGOTO_ERROR(DFE_BADACC, FAIL);

    /* get vgroup file record */
    if (NULL == (vf = Get_vfile(f)))
        HGOTO_ERROR(DFE_FNF, FAIL);

    /* find vgroup node in TBBT using it's ref */
    key = (int32)vgid;
    if (( t = (VOIDP *) tbbtdfind(vf->vgtree, (VOIDP) &key, NULL))== NULL)
      HGOTO_DONE(FAIL);

    /* remove vgroup node from TBBT */
    if((v = tbbtrem((TBBT_NODE **) vf->vgtree, (TBBT_NODE *) t, NULL))!=NULL)
        vdestroynode((VOIDP) v);

    /* Delete vgroup from file */
    if(Hdeldd(f, DFTAG_VG, (uint16) vgid) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* Vdelete */

/*******************************************************************************
 NAME
    VIstart  --  V-level initialization routine

 DESCRIPTION
    Register the shut-down routines (VPshutdown & VSPshutdown) for call
    with atexit.

 RETURNS
    Returns SUCCEED/FAIL
*******************************************************************************/
PRIVATE intn 
VIstart(void)
{
    intn        ret_value = SUCCEED;
    CONSTR(FUNC, "VIstart");    /* for HERROR */

    /* Don't call this routine again... */
    library_terminate = TRUE;

    /* Install atexit() library cleanup routine */
    if (HPregister_term_func(&VPshutdown) != 0)
      HGOTO_ERROR(DFE_CANTINIT, FAIL);

    /* Install atexit() library cleanup routine */
    if (HPregister_term_func(&VSPshutdown) != 0)
      HGOTO_ERROR(DFE_CANTINIT, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

    return(ret_value);
} /* end VIstart() */

/*******************************************************************************
 NAME
    VPshutdown  --  Terminate various static buffers.

 DESCRIPTION
    Free various buffers allocated in the V routines.

 RETURNS
    Returns SUCCEED/FAIL

*******************************************************************************/
intn 
VPshutdown(void)
{
    VGROUP       *v  = NULL;
    vginstance_t *vg = NULL;
    intn         ret_value = SUCCEED;
    CONSTR(FUNC, "VPshutdown");

    /* Release the vdata free-list if it exists */
    if(vgroup_free_list != NULL)
      {
          while(vgroup_free_list != NULL)
            {
                v = vgroup_free_list;
                vgroup_free_list = vgroup_free_list->next;
                v->next = NULL;
                HDfree(v);
            } /* end while */
      } /* end if */

    /* Release the vginstance free-list if it exists */
    if(vginstance_free_list != NULL)
      {
          while(vginstance_free_list != NULL)
            {
                vg = vginstance_free_list;
                vginstance_free_list = vginstance_free_list->next;
                vg->next = NULL;
                HDfree(vg);
            } /* end while */
      } /* end if */

    if(vtree != NULL)
      {
          /* Free the vfile tree */
          tbbtdfree(vtree, vfdestroynode, NULL);

          /* Destroy the atom groups for Vdatas and Vgroups */
          if (HAdestroy_group(VSIDGROUP) == FAIL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);

          if (HAdestroy_group(VGIDGROUP) == FAIL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);

          vtree = NULL;
      } /* end if */

    if(Vgbuf != NULL)
      {
          HDfree(Vgbuf);
          Vgbuf = NULL;
          Vgbufsize = 0;
      } /* end if */


done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
    return ret_value;
} /* end VPshutdown() */

