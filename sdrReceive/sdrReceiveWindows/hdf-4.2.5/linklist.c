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

/* $Id: linklist.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*
FILE
    linklist.c - Internal storage routines for handling generic linked lists

REMARKS
    Generic linked lists are used in several places in the HDF library when
    a variable number of items are needed to be kept track of without the
    additional overhead of a threaded binary tree.  The linked list can be
    either sorted or un-sorted, chosen at creation time.  The interface allows
    objects (void *'s currently) to be stored and searched and interated
    through in a fairly easy manner

DESIGN
    The lists are stored in a singly-linked list of node, each containing
    a pointer to a managed object.  The list may be sorted or unsorted, based
    on the comparison function.

BUGS/LIMITATIONS

LOCAL ROUTINES
    HULIget_list_node - Gets a list node
    HULIrelease_list_node - Releases a list node
EXPORTED ROUTINES
    HULcreate_list  - Create a new linked list
    HULdestroy_list - Destroys a linked list
    HULadd_node     - Adds an object to a linked-list
    HULsearch_node  - Search for an object in a linked-list
    HULfirst_node   - Get the first object in a linked-list
    HULnext_node    - Get the next object in a linked-list
    HULremove_node  - Removes an object from a linked-list
    HULshutdown     - Close down the HUL interface

AUTHOR
   Quincey Koziol

MODIFICATION HISTORY
   2/5/96  - Starting writing specs & coding prototype
   2/12/96  - Finished writing specs & coding prototype, start testing
*/

#define LIST_MASTER
#include "hdf.h"
#include "linklist.h"


/* Private function prototypes */
static node_info_t *HULIget_list_node(void);
static void HULIrelease_list_node(node_info_t *nod);

/******************************************************************************
 NAME
     HULcreate_list - Create a linked list

 DESCRIPTION
    Creates a linked list.  The list may either be sorted or un-sorted, based
    on the comparison function.

 RETURNS
    Returns a pointer to the list if successful and NULL otherwise

*******************************************************************************/
list_head_t *HULcreate_list(HULfind_func_t find_func    /* IN: object comparison function */
)
{
    CONSTR(FUNC, "HULcreate_list");	/* for HERROR */
    list_head_t *ret_value=NULL;     /* ptr to the linked list "head" node */

    HEclear();

    /* Allocate the head information */
    if((ret_value=(list_head_t *)HDcalloc(1,sizeof(list_head_t)))==NULL)
        HGOTO_ERROR(DFE_NOSPACE, NULL);

    /* Set the counter */
    ret_value->count=0;

    /* Store the creation flags, etc */
    if(find_func==NULL)
        ret_value->flags=HUL_UNSORTED_LIST;
    else
        ret_value->flags=HUL_SORTED_LIST;
    ret_value->cmp_func=find_func;

done:
  if(ret_value == NULL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* end HULcreate_list() */

/******************************************************************************
 NAME
     HULdestroy_list - Destroys a linked list

 DESCRIPTION
    Destroys a linked list created by HULcreate_list().  This function
    walks through the list and frees all the nodes, then frees the list head.
    Note: this function does not (currently) free the objects in the nodes,
    it just leaves 'em hanging.

 RETURNS
    Returns SUCCEED/FAIL.

*******************************************************************************/
intn HULdestroy_list(list_head_t *lst    /* IN: list to destroy */
)
{
    CONSTR(FUNC, "HULdestroy_list");	/* for HERROR */
    node_info_t *curr_node,         /* current node while walking through list */
        *next_node;                 /* next node in the list */
    intn ret_value=SUCCEED;         /* return value */

    HEclear();
    if(lst==NULL)
        HGOTO_ERROR(DFE_ARGS,FAIL);

    /* Chuck the list */
    curr_node=lst->node_list;
    while(curr_node!=NULL)
      {
          next_node=curr_node->next;
          HULIrelease_list_node(curr_node);
          curr_node=next_node;
      } /* end while */

    /* Chuck the list-head */
    HDfree(lst);

done:
  if(ret_value == FAIL)
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* end HULdestroy_list() */

/******************************************************************************
 NAME
     HULadd_node - Adds an object to a linked-list

 DESCRIPTION
    Adds an object to the linked list.  If the list is sorted, the comparison
    function is used to determine where to insert the node, otherwise it is
    inserted at the head of the list.

 RETURNS
    Returns SUCCEED/FAIL.

*******************************************************************************/
intn HULadd_node(list_head_t *lst,  /* IN: list to modify */
    VOIDP obj                       /* IN: object to add to the list */
)
{
    CONSTR(FUNC, "HULadd_node");	/* for HERROR */
    node_info_t *new_node;          /* new node to insert into the list */
    intn ret_value=SUCCEED;         /* return value */

    HEclear();
    if(lst==NULL || obj==NULL)
        HGOTO_ERROR(DFE_ARGS,FAIL);

    /* Allocate & initialize the new node */
    if((new_node=HULIget_list_node())==NULL)
        HGOTO_ERROR(DFE_NOSPACE,FAIL);
    new_node->obj_ptr=obj;

    if(((lst->flags)&HUL_SORTED_LIST)!=0)
      { /* insert node into a sorted list */
        node_info_t *curr_node,         /* current node while walking through list */
            *prev_node;                 /* previous node in the list */

        prev_node=NULL;
        curr_node=lst->node_list;
        while(curr_node!=NULL)
          {
            if(lst->cmp_func(curr_node->obj_ptr,new_node->obj_ptr)>=0)
              { /* 'curr_node' object is greater than or equal to 'new_node' */
                new_node->next=curr_node;
                if(prev_node==NULL)
                    lst->node_list=new_node;
                else
                    prev_node->next=new_node;
                HGOTO_DONE(SUCCEED);  /* Break out of the loop */
              } /* end if */
            prev_node=curr_node;
            curr_node=curr_node->next;
          } /* end while */

        /* Walked off the list, so append to last node */
        if(prev_node==NULL)
            lst->node_list=new_node;
        else
            prev_node->next=new_node;
      } /* end if */
    else
      { /* insert node into an un-sorted list */
        new_node->next=lst->node_list;
        lst->node_list=new_node;
      } /* end else */

done:
  if(ret_value == FAIL)
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* end HULadd_node() */

/******************************************************************************
 NAME
     HULsearch_node - Search for an object in a linked-list

 DESCRIPTION
    Locate an object in a linked list using a key and comparison function.

 RETURNS
    Returns a pointer to the object found in the list, or NULL on failure.

*******************************************************************************/
VOIDP HULsearch_node(list_head_t *lst,  /* IN: list to search */
    HULsearch_func_t srch_func,       /* IN: function to use to find node */
    VOIDP key                       /* IN: key of object to search for */
)
{
    CONSTR(FUNC, "HULsearch_node");	/* for HERROR */
    node_info_t *curr_node;         /* current node we are on */
    VOIDP ret_value=NULL;           /* default return value */

    HEclear();
    if(lst==NULL || srch_func==NULL || key==NULL)
        HGOTO_ERROR(DFE_ARGS,NULL);

    curr_node=lst->node_list;
    while(curr_node!=NULL)
      {
        if(srch_func(curr_node->obj_ptr,key)==1)
            HGOTO_DONE(curr_node->obj_ptr);
      } /* end while */

done:
  if(ret_value == NULL)
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* end HULsearch_node() */

/******************************************************************************
 NAME
     HULfirst_node - Get the first object in a linked-list

 DESCRIPTION
    Returns the first object in a linked-list and prepares the list for
    interating through.

 RETURNS
    Returns a pointer to the first object found in the list, or NULL on failure.

*******************************************************************************/
VOIDP HULfirst_node(list_head_t *lst   /* IN: list to search */
)
{
    CONSTR(FUNC, "HULfirst_node");	/* for HERROR */
    VOIDP ret_value=NULL;           /* default return value */

    HEclear();
    if(lst==NULL)
        HGOTO_ERROR(DFE_ARGS,NULL);

    if(lst->node_list!=NULL)
      {
        lst->curr_node=lst->node_list;
        HGOTO_DONE(lst->node_list->obj_ptr);
      } /* end if */

done:
  if(ret_value == NULL)
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* end HULfirst_node() */

/******************************************************************************
 NAME
     HULnext_node - Get the next object in a linked-list

 DESCRIPTION
    Returns the next object in a linked-list by walking through the list

 RETURNS
    Returns a pointer to the next object found in the list, or NULL on failure.

*******************************************************************************/
VOIDP HULnext_node(list_head_t *lst   /* IN: list to search */
)
{
    CONSTR(FUNC, "HULnext_node");	/* for HERROR */
    VOIDP ret_value=NULL;           /* default return value */

    HEclear();
    if(lst==NULL)
        HGOTO_ERROR(DFE_ARGS,NULL);

    if(lst->curr_node!=NULL)
      {
        lst->curr_node=lst->curr_node->next;
        if(lst->curr_node!=NULL)
            HGOTO_DONE(lst->curr_node->obj_ptr);
      } /* end if */

done:
  if(ret_value == NULL)
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* end HULnext_node() */

/******************************************************************************
 NAME
     HULremove_node - Removes an object from a linked-list

 DESCRIPTION
    Remove an object from a linked list.  The key and comparison function are
    provided locate the object to delete.

 RETURNS
    Returns a pointer to the object deleted from the list, or NULL on failure.

*******************************************************************************/
VOIDP HULremove_node(list_head_t *lst,  /* IN: list to modify */
    HULsearch_func_t srch_func,     /* IN: function to use to find node to remove */
    VOIDP key                       /* IN: object to add to the list */
)
{
    CONSTR(FUNC, "HULremove_node");	/* for HERROR */
    node_info_t *curr_node,         /* current node we are on */
        *prev_node;                 /* previous node we looked at */
    VOIDP ret_value=NULL;           /* default return value */

    HEclear();
    if(lst==NULL || srch_func==NULL || key==NULL)
        HGOTO_ERROR(DFE_ARGS,NULL);

    prev_node=NULL;
    curr_node=lst->node_list;
    while(curr_node!=NULL)
      {
        if(srch_func(curr_node->obj_ptr,key)==1)
          {
            if(prev_node==NULL)
                lst->node_list=curr_node->next;
            else
                prev_node->next=curr_node->next;
            ret_value=curr_node->obj_ptr;
            HULIrelease_list_node(curr_node);
            break;
          } /* end if */
      } /* end while */

done:
  if(ret_value == NULL)
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* end HULremove_node() */

/******************************************************************************
 NAME
     HULIget_list_node - Gets a list node

 DESCRIPTION
    Either gets an list node from the free list (if there is one available)
    or allocate a node.

 RETURNS
    Returns list node ptr if successful and NULL otherwise

*******************************************************************************/
static node_info_t *HULIget_list_node(void)
{
    CONSTR(FUNC, "HULIget_list_node");	/* for HERROR */
    node_info_t *ret_value=NULL;

    HEclear();
    if(node_free_list!=NULL)
      {
        ret_value=node_free_list;
        node_free_list=node_free_list->next;
      } /* end if */
    else
      {
        if((ret_value=(node_info_t *)HDmalloc(sizeof(node_info_t)))==NULL)
            HGOTO_ERROR(DFE_NOSPACE, NULL);
      } /* end else */

done:
  if(ret_value == NULL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end HULIget_list_node() */

/******************************************************************************
 NAME
     HULIrelease_list_node - Releases a list node

 DESCRIPTION
    Puts a list node into the free list

 RETURNS
    No return value

*******************************************************************************/
static void HULIrelease_list_node(node_info_t *nod)
{
#ifdef LATER
    CONSTR(FUNC, "HULIrelease_list_node");	/* for HERROR */
#endif /* LATER */

    /* Insert the node at the beginning of the free list */
    nod->next=node_free_list;
    node_free_list=nod;
}   /* end HULIrelease_list_node() */

/*--------------------------------------------------------------------------
 NAME
    HULshutdown
 PURPOSE
    Terminate various global items.
 USAGE
    intn HULshutdown()
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Free various buffers allocated in the HUL routines.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Should only ever be called by the "atexit" function HDFend
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn 
HULshutdown(void)
{
    node_info_t *curr;

    /* Release the free-list if it exists */
    if(node_free_list!=NULL)
      {
        while(node_free_list!=NULL)
          {
            curr=node_free_list;
            node_free_list=node_free_list->next;
            HDfree(curr);
          } /* end while */
      } /* end if */
  return (SUCCEED);
}	/* end HULshutdown() */

