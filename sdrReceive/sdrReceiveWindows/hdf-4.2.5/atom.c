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
static char RcsId[] = "@(#)$Revision: 5028 $";
#endif


/* $Id: atom.c 5028 2008-01-27 16:49:37Z bmribler $ */

/*
FILE
    atom.c - Internal storage routines for handling "atoms"

REMARKS
    Atoms are just ID's which allow objects (void *'s currently) to be
    bundled into "groups" for more general storage.

DESIGN
    The groups are stored in an array of pointers to store each group in an
    element. Each "atomic group" node contains a link to a hash table to
    manage the atoms in each group.  The allowed "atomic groups" are stored
    in an enum (called group_t) in atom.h.

BUGS/LIMITATIONS
    Can't interate over the atoms in a group.

LOCAL ROUTINES
  HAIfind_atom      - Returns a pointer to an atom_info_t from a atom ID
  HAIget_atom_node  - Gets an atom node (uses the atom free list)
  HAIrelease_atom_node - Releases an atom node (uses the atom free list)
EXPORTED ROUTINES
 Atom Functions:
  HAregister_atom   - Register an object in a group and get an atom for it
  HAatom_object     - Get the object for an atom
  HAatom_group      - Get the group for an atom
  HAremove_atom     - Remove an atom from a group
  HAsearch_atom     - Search a group for a particular object
 Atom Group Functions:
  HAinit_group      - Initialize a group to store atoms in
  HAdestroy_group   - Destroy an atomic group
 Atom Group Cleanup:
  HAshutdown        - Terminate various static buffers.

AUTHOR
   Quincey Koziol

MODIFICATION HISTORY
   1/3/96  - Starting writing specs & coding prototype
   1/7/96  - Finished coding prototype
*/

#define ATOM_MASTER
#include "hdf.h"
#include "atom.h"
#include <assert.h>

/* Private function prototypes */
static atom_info_t *HAIfind_atom(atom_t atm);

static atom_info_t *HAIget_atom_node(void);

static void HAIrelease_atom_node(atom_info_t *atm);

/******************************************************************************
 NAME
     HAinit_group - Initialize an atomic group

 DESCRIPTION
    Creates a global atomic group to store atoms in.  If the group has already
    been initialized, this routine just increments the count of # of
    initializations and returns without trying to change the size of the hash
    table.

 RETURNS
    Returns SUCCEED if successful and FAIL otherwise

*******************************************************************************/
intn HAinit_group(group_t grp,      /* IN: Group to initialize */
    intn hash_size                  /* IN: Minimum hash table size to use for group */
)
{
    CONSTR(FUNC, "HAinit_group");	/* for HERROR */
    atom_group_t *grp_ptr=NULL;     /* ptr to the atomic group */
    intn ret_value=SUCCEED;

    HEclear();
    if((grp<=BADGROUP || grp>=MAXGROUP) && hash_size>0)
        HGOTO_ERROR(DFE_ARGS, FAIL);

#ifdef ATOMS_CACHE_INLINE
/* Assertion necessary for faster pointer swapping */
assert(sizeof(hdf_pint_t)==sizeof(void *));
#endif /* ATOMS_CACHE_INLINE */

#ifdef HASH_SIZE_POWER_2
    if(hash_size & (hash_size-1))
        HGOTO_ERROR(DFE_ARGS, FAIL);
#endif /* HASH_SIZE_POWER_2 */

    if(atom_group_list[grp]==NULL)
      {     /* Allocate the group information */
          grp_ptr=(atom_group_t *)HDcalloc(1,sizeof(atom_group_t));
          if(grp_ptr==NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
          atom_group_list[grp]=grp_ptr;
      } /* end if */
    else /* Get the pointer to the existing group */
        grp_ptr=atom_group_list[grp];

    if(grp_ptr->count==0)
      {     /* Initialize the atom group structure */
        grp_ptr->hash_size=hash_size;
        grp_ptr->atoms=0;
        grp_ptr->nextid=0;
        if((grp_ptr->atom_list=(atom_info_t **)HDcalloc(hash_size,sizeof(atom_info_t *)))==NULL)
            HGOTO_ERROR(DFE_NOSPACE, FAIL);
      } /* end if */

    /* Increment the count of the times this group has been initialized */
    grp_ptr->count++;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */
        if(grp_ptr!=NULL)
          {
            if(grp_ptr->atom_list!=NULL)
                HDfree(grp_ptr->atom_list);
            HDfree(grp_ptr);
          } /* end if */
    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end HAinit_group() */

/******************************************************************************
 NAME
     HAdestroy_group - Destroy an atomic group

 DESCRIPTION
    Destroys an atomic group which atoms are stored in.  If the group still
    has atoms which are registered, this routine fails.  If there have been
    multiple initializations of the group, this routine just decrements the
    count of initializations and does not check the atoms out-standing.

 RETURNS
    Returns SUCCEED if successful and FAIL otherwise

*******************************************************************************/
intn HAdestroy_group(group_t grp       /* IN: Group to destroy */
)
{
    CONSTR(FUNC, "HAdestroy_group");	/* for HERROR */
    atom_group_t *grp_ptr=NULL;     /* ptr to the atomic group */
    intn ret_value=SUCCEED;

    HEclear();
    if(grp<=BADGROUP || grp>=MAXGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    grp_ptr=atom_group_list[grp];
    if(grp_ptr==NULL || grp_ptr->count<=0)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Decrement the number of users of the atomic group */
    if((--(grp_ptr->count))==0)
      {
#ifdef ATOMS_ARE_CACHED
      {
        uintn i;

        for(i=0; i<ATOM_CACHE_SIZE; i++)
            if(ATOM_TO_GROUP(atom_id_cache[i])==grp)
              {
                atom_id_cache[i]=(-1);
                atom_obj_cache[i]=NULL;
              } /* end if */
      } /* end block */
#endif /* ATOMS_ARE_CACHED */
        HDfree(grp_ptr->atom_list);
	grp_ptr->atom_list = NULL;
      } /* end if */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end HAdestroy_group() */

/******************************************************************************
 NAME
     HAregister_atom - Register an object in a group and get an atom for it.

 DESCRIPTION
    Registers an object in a group and returns an atom for it.  This routine
    does _not_ check for unique-ness of the objects, if you register an object
    twice, you will get two different atoms for it.  This routine does make
    certain that each atom in a group is unique.  Atoms are created by getting
    a unique number for the group the atom is in and incorporating the group
    into the atom which is returned to the user.

 RETURNS
    Returns atom if successful and FAIL otherwise

*******************************************************************************/
atom_t HAregister_atom(group_t grp,     /* IN: Group to register the object in */
    VOIDP object                        /* IN: Object to attach to atom */
)
{
    CONSTR(FUNC, "HAregister_atom");	/* for HERROR */
    atom_group_t *grp_ptr=NULL;     /* ptr to the atomic group */
    atom_info_t *atm_ptr=NULL;      /* ptr to the new atom */
    atom_t atm_id;                  /* new atom ID */
    uintn hash_loc;                 /* new item's hash table location */
    atom_t ret_value=SUCCEED;

    HEclear();
    if(grp<=BADGROUP || grp>=MAXGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    grp_ptr=atom_group_list[grp];
    if(grp_ptr==NULL || grp_ptr->count<=0)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    if((atm_ptr=HAIget_atom_node())==NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* Create the atom & it's ID */
    atm_id=MAKE_ATOM(grp,grp_ptr->nextid);
    atm_ptr->id=atm_id;
    atm_ptr->obj_ptr=object;
    atm_ptr->next=NULL;

    /* hash bucket already full, prepend to front of chain */
    hash_loc=grp_ptr->nextid%(uintn)grp_ptr->hash_size;
    if(grp_ptr->atom_list[hash_loc]!=NULL)
        atm_ptr->next=grp_ptr->atom_list[hash_loc];

    /* Insert into the group */
    grp_ptr->atom_list[hash_loc]=atm_ptr;
    grp_ptr->atoms++;
    grp_ptr->nextid++;

    ret_value=atm_id;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end HAregister_atom() */

/******************************************************************************
 NAME
     HAatom_object - Returns to the object ptr for the atom 

 DESCRIPTION
    Retrieves the object ptr which is associated with the atom.

 RETURNS
    Returns object ptr if successful and NULL otherwise

*******************************************************************************/
#ifdef ATOMS_CACHE_INLINE
VOIDP HAPatom_object(atom_t atm   /* IN: Atom to retrieve object for */
)
#else /* ATOMS_CACHE_INLINE */
VOIDP HAatom_object(atom_t atm   /* IN: Atom to retrieve object for */
)
#endif /* ATOMS_CACHE_INLINE */
{
    CONSTR(FUNC, "HAatom_object");	/* for HERROR */
#ifndef ATOMS_CACHE_INLINE
#ifdef ATOMS_ARE_CACHED
    uintn i;                        /* local counter */
#endif /* ATOMS_ARE_CACHED */
#endif /* ATOMS_CACHE_INLINE */
    atom_info_t *atm_ptr=NULL;      /* ptr to the new atom */
    VOIDP ret_value=NULL;

    HEclear();

#ifndef ATOMS_CACHE_INLINE
#ifdef ATOMS_ARE_CACHED
    /* Look for the atom in the cache first */
    for(i=0; i<ATOM_CACHE_SIZE; i++)
        if(atom_id_cache[i]==atm)
          {
            ret_value=atom_obj_cache[i];
            if(i>0)
              { /* Implement a simple "move forward" caching scheme */
                atom_t t_atom=atom_id_cache[i-1];
                VOIDP  t_obj=atom_obj_cache[i-1];

                atom_id_cache[i-1]=atom_id_cache[i];
                atom_obj_cache[i-1]=atom_obj_cache[i];
                atom_id_cache[i]=t_atom;
                atom_obj_cache[i]=t_obj;
              } /* end if */
            HGOTO_DONE(ret_value);
          } /* end if */
#endif /* ATOMS_ARE_CACHED */
#endif /* ATOMS_CACHE_INLINE */

    /* General lookup of the atom */
    if((atm_ptr=HAIfind_atom(atm))==NULL)
        HGOTO_ERROR(DFE_INTERNAL, NULL);

    /* Check if we've found the correct atom */
    if(atm_ptr!=NULL)
        ret_value=atm_ptr->obj_ptr;

done:
  if(ret_value == NULL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end HAatom_object() */

/******************************************************************************
 NAME
     HAatom_group - Returns to the group for the atom 

 DESCRIPTION
    Retrieves the group which is associated with the atom.

 RETURNS
    Returns group if successful and BADGROUP otherwise

*******************************************************************************/
group_t HAatom_group(atom_t atm   /* IN: Atom to retrieve group for */
)
{
    CONSTR(FUNC, "HAatom_group");	/* for HERROR */
    group_t ret_value=BADGROUP;

    HEclear();
    ret_value=ATOM_TO_GROUP(atm);
    if(ret_value<=BADGROUP || ret_value>=MAXGROUP)
        HGOTO_ERROR(DFE_ARGS, BADGROUP);

done:
  if(ret_value == BADGROUP)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end HAatom_group() */

/******************************************************************************
 NAME
     HAremove_atom - Removes an atom from a group

 DESCRIPTION
    Removes an atom from a group.

 RETURNS
    Returns atom's object if successful and NULL otherwise

*******************************************************************************/
VOIDP HAremove_atom(atom_t atm   /* IN: Atom to remove */
)
{
    CONSTR(FUNC, "HAremove_atom");	/* for HERROR */
    atom_group_t *grp_ptr=NULL;     /* ptr to the atomic group */
    atom_info_t *curr_atm,          /* ptr to the current atom */
        *last_atm;                  /* ptr to the last atom */
    group_t grp;                    /* atom's atomic group */
    uintn hash_loc;                 /* atom's hash table location */
#ifdef ATOMS_ARE_CACHED
    uintn i;                        /* local counting variable */
#endif /* ATOMS_ARE_CACHED */
    VOIDP ret_value=NULL;

    HEclear();
    grp=ATOM_TO_GROUP(atm);
    if(grp<=BADGROUP || grp>=MAXGROUP)
        HGOTO_ERROR(DFE_ARGS, NULL);

    grp_ptr=atom_group_list[grp];
    if(grp_ptr==NULL || grp_ptr->count<=0)
        HGOTO_ERROR(DFE_INTERNAL, NULL);

    /* Get the location in which the atom is located */
    hash_loc=(uintn)ATOM_TO_LOC(atm,grp_ptr->hash_size);
    curr_atm=grp_ptr->atom_list[hash_loc];
    if(curr_atm==NULL)
        HGOTO_ERROR(DFE_INTERNAL, NULL);

    last_atm=NULL;
    while(curr_atm!=NULL)
      {
          if(curr_atm->id==atm)
              break;
          last_atm=curr_atm;
          curr_atm=curr_atm->next;
      } /* end while */

    if(curr_atm!=NULL)
      {
          if(last_atm==NULL)    /* atom is the first the chain */
              grp_ptr->atom_list[hash_loc]=curr_atm->next;
          else
              last_atm->next=curr_atm->next;
          ret_value=curr_atm->obj_ptr;
          HAIrelease_atom_node(curr_atm);
      } /* end if */
    else    /* couldn't find the atom in the proper place */
        HGOTO_ERROR(DFE_INTERNAL, NULL);
    
#ifdef ATOMS_ARE_CACHED
    /* Delete object from cache */
    for(i=0; i<ATOM_CACHE_SIZE; i++)
        if(atom_id_cache[i]==atm)
          {
            atom_id_cache[i]=(-1);
            atom_obj_cache[i]=NULL;
            break;  /* we assume there is only one instance in the cache */
          } /* end if */
#endif /* ATOMS_ARE_CACHED */

    /* Decrement the number of atoms in the group */
    (grp_ptr->atoms)--;

done:
  if(ret_value == NULL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end HAremove_atom() */

/******************************************************************************
 NAME
     HAsearch_atom - Search for an object in a group and get it's pointer.

 DESCRIPTION
    Searchs for an object in a group and returns the pointer to it.
    This routine calls the function pointer passed in for each object in the
    group until it finds a match.  Currently there is no way to resume a
    search.

 RETURNS
    Returns pointer an atom's object if successful and NULL otherwise

*******************************************************************************/
void * HAsearch_atom(group_t grp,        /* IN: Group to search for the object in */
    HAsearch_func_t func,               /* IN: Ptr to the comparison function */
    const void * key                     /* IN: pointer to key to compare against */
)
{
    CONSTR(FUNC, "HAsearch_atom");	/* for HERROR */
    atom_group_t *grp_ptr=NULL;     /* ptr to the atomic group */
    atom_info_t *atm_ptr=NULL;      /* ptr to the new atom */
    intn i;                         /* local counting variable */
    void * ret_value=NULL;

    HEclear();
    if(grp<=BADGROUP || grp>=MAXGROUP)
        HGOTO_ERROR(DFE_ARGS, NULL);

    grp_ptr=atom_group_list[grp];
    if(grp_ptr==NULL || grp_ptr->count<=0)
        HGOTO_ERROR(DFE_INTERNAL, NULL);

    /* Start at the beginning of the array */
    for(i=0; i<grp_ptr->hash_size; i++)
      {
        atm_ptr=grp_ptr->atom_list[i];
        while(atm_ptr!=NULL)
          {
              if((*func)(atm_ptr->obj_ptr,key))
                  HGOTO_DONE(atm_ptr->obj_ptr); /* found the item we are looking for */
              atm_ptr=atm_ptr->next;
          } /* end while */
      } /* end for */

done:
  if(ret_value == NULL)
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end HAsearch_atom() */

/******************************************************************************
 NAME
     HAIfind_atom - Finds a atom in a group

 DESCRIPTION
    Retrieves the atom ptr which is associated with the atom.

 RETURNS
    Returns atom ptr if successful and NULL otherwise

*******************************************************************************/
static atom_info_t *HAIfind_atom(atom_t atm   /* IN: Atom to retrieve atom for */
)
{
    CONSTR(FUNC, "HAIfind_atom");	/* for HERROR */
    atom_group_t *grp_ptr=NULL;     /* ptr to the atomic group */
    atom_info_t *atm_ptr=NULL;      /* ptr to the new atom */
    group_t grp;                    /* atom's atomic group */
    uintn hash_loc;                 /* atom's hash table location */
    atom_info_t *ret_value=NULL;

    HEclear();
    grp=ATOM_TO_GROUP(atm);
    if(grp<=BADGROUP || grp>=MAXGROUP)
        HGOTO_ERROR(DFE_ARGS, NULL);

    grp_ptr=atom_group_list[grp];
    if(grp_ptr==NULL || grp_ptr->count<=0)
        HGOTO_ERROR(DFE_INTERNAL, NULL);

    /* Get the location in which the atom is located */
    hash_loc=(uintn)ATOM_TO_LOC(atm,grp_ptr->hash_size);
    atm_ptr=grp_ptr->atom_list[hash_loc];
    if(atm_ptr==NULL)
        HGOTO_ERROR(DFE_INTERNAL, NULL);

    while(atm_ptr!=NULL)
      {
          if(atm_ptr->id==atm)
              break;
          atm_ptr=atm_ptr->next;
      } /* end while */
    ret_value=atm_ptr;

#ifdef ATOMS_ARE_CACHED
    atom_id_cache[ATOM_CACHE_SIZE-1]=atm;
    atom_obj_cache[ATOM_CACHE_SIZE-1]=atm_ptr->obj_ptr;
#endif /* ATOMS_ARE_CACHED */

done:
  if(ret_value == NULL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end HAIfind_atom() */

/******************************************************************************
 NAME
     HAIget_atom_node - Gets an atom node

 DESCRIPTION
    Either gets an atom node from the free list (if there is one available)
    or allocate a node.

 RETURNS
    Returns atom ptr if successful and NULL otherwise

*******************************************************************************/
static atom_info_t *HAIget_atom_node(void)
{
    CONSTR(FUNC, "HAIget_atom_node");	/* for HERROR */
    atom_info_t *ret_value=NULL;

    HEclear();
    if(atom_free_list!=NULL)
      {
        ret_value=atom_free_list;
        atom_free_list=atom_free_list->next;
      } /* end if */
    else
      {
        if((ret_value=(atom_info_t *)HDmalloc(sizeof(atom_info_t)))==NULL)
            HGOTO_ERROR(DFE_NOSPACE, NULL);
      } /* end else */

done:
  if(ret_value == NULL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end HAIget_atom_node() */

/******************************************************************************
 NAME
     HAIrelease_atom_node - Releases an atom node

 DESCRIPTION
    Puts an atom node into the free list

 RETURNS
    No return value

*******************************************************************************/
static void HAIrelease_atom_node(atom_info_t *atm)
{
#ifdef LATER
    CONSTR(FUNC, "HAIrelease_atom_node");	/* for HERROR */
#endif /* LATER */

    /* Insert the atom at the beginning of the free list */
    atm->next=atom_free_list;
    atom_free_list=atm;
}   /* end HAIrelease_atom_node() */

/*--------------------------------------------------------------------------
 NAME
    HAshutdown
 PURPOSE
    Terminate various static buffers.
 USAGE
    intn HAshutdown()
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Free various buffers allocated in the HA routines.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Should only ever be called by the "atexit" function HDFend
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn 
HAshutdown(void)
{
    atom_info_t *curr;
    intn i;

    /* Release the free-list if it exists */
    if(atom_free_list!=NULL)
      {
        while(atom_free_list!=NULL)
          {
            curr=atom_free_list;
            atom_free_list=atom_free_list->next;
            HDfree(curr);
          } /* end while */
      } /* end if */

    for(i=0; i<(intn)MAXGROUP; i++)
        if(atom_group_list[i]!=NULL)
          {
            HDfree(atom_group_list[i]);
            atom_group_list[i]=NULL;
          } /* end if */
  return (SUCCEED);
}	/* end HAshutdown() */

