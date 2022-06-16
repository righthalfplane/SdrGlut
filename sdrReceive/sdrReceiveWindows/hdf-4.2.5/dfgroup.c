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

/* $Id: dfgroup.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*-----------------------------------------------------------------------------
 * File:    dfgroup.c
 * Purpose: Low level functions for implementing groups
 * Invokes: df.c df.h
 * Contents:
 *  DFdiread   : read in the data identifier list from the group
 *  DFdiget    : get next data identifier from list
 *  DFdisetup  : get ready to store a list of data identifiers to write out
 *  DFdiput    : add a data identifier to the list to be written out
 *  DFdiwrite  : write out the list of data identifiers
 * Remarks: A group is a way of associating data elements with each other.
 *          It is a tag whose data is a list of tag/refs
 *          Each tag/ref combination is called a data identifier (DI).
 *---------------------------------------------------------------------------*/

#include "hdf.h"
#include "hfile.h"

#if 0
#define MAX_GROUPS 8
#endif

typedef struct DIlist_struct
  {
      uint8      *DIlist;
      intn        num;
      intn        current;
  }
DIlist     , *DIlist_ptr;

static DIlist_ptr Group_list[MAX_GROUPS] = {NULL};

#define GSLOT2ID(s) ((((uint32)GROUPTYPE & 0xffff) << 16) | ((s) & 0xffff))
#define VALIDGID(i) (((((uint32)(i) >> 16) & 0xffff) == GROUPTYPE) && \
                    (((uint32)(i) & 0xffff) < MAX_GROUPS))
#define GID2REC(i)  ((VALIDGID(i) ? (Group_list[(uint32)(i) & 0xffff]) : NULL))

/*-----------------------------------------------------------------------------
 * Name:    setgroupREC
 * Purpose: Add a group list into the internal structure and return an ID
 * Inputs:  list_rec: list to remember
 * Returns: FAIL on failure else a group ID to the list
 * Users:   other group routines
 * Invokes:
 * Remarks: Allocates internal storeage if necessary
 *---------------------------------------------------------------------------*/
PRIVATE int32
setgroupREC(DIlist_ptr list_rec)
{
    CONSTR(FUNC, "setgroupREC");
    uintn       i;

    for (i = 0; i < MAX_GROUPS; i++)
        if (Group_list[i]==NULL)
          {
              Group_list[i] = list_rec;
              return (int32)GSLOT2ID(i);
          }

    HRETURN_ERROR(DFE_INTERNAL, FAIL)
}   /* setgroupREC */

/*-----------------------------------------------------------------------------
 * Name:    DFdiread
 * Purpose: Read a list of DIs into memory
 * Inputs:  file_id: HDF file pointer
 *          tag, ref: id of group which is to be read in
 * Returns: FAIL on failure else a group ID to the list
 * Users:   HDF systems programmers, DF8getrig, other routines
 * Invokes: HDvalidfid, DFIfind, DFgetelement
 * Remarks: assumes tag is a group
 *---------------------------------------------------------------------------*/

int32
DFdiread(int32 file_id, uint16 tag, uint16 ref)
{
    DIlist_ptr  new_list;
    CONSTR(FUNC, "DFdiread");
    int32       length;

    HEclear();

    if (!HDvalidfid(file_id))
        HRETURN_ERROR(DFE_ARGS, FAIL);

    /* Find the group. */
    length = Hlength(file_id, tag, ref);
    if (length == FAIL)
        HRETURN_ERROR(DFE_INTERNAL, FAIL);

    /* allocate a new structure to hold the group */
    new_list = (DIlist_ptr) HDmalloc((uint32) sizeof(DIlist));
    if (!new_list)
        HRETURN_ERROR(DFE_NOSPACE, FAIL);

    new_list->DIlist = (uint8 *) HDmalloc((uint32) length);
    if (!new_list->DIlist)
      {
          HDfree((VOIDP) new_list);
          HRETURN_ERROR(DFE_NOSPACE, FAIL)
      }

    new_list->num = (intn) (length / 4);
    new_list->current = 0;  /* no DIs returned so far */

    /* read in group */
    if (Hgetelement(file_id, tag, ref, (uint8 *) new_list->DIlist) < 0)
      {
          HDfree((VOIDP) new_list->DIlist);
          HDfree((VOIDP) new_list);
          HRETURN_ERROR(DFE_READERROR, FAIL)
      }
    return (int32) setgroupREC(new_list);
}

/*-----------------------------------------------------------------------------
 * Name:    DFdiget
 * Purpose: return next DI from the list of DIs in a group
 * Inputs:  list: handle to group (which is list of DIs)
 * Outputs: ptag: pointer to tag part of DI to be returned
 *          pref: pointer to ref part of DI to be returned
 * Returns: 0 on success, -1 on failure with error set
 * Users:   HDF systems programmers, DF8getrig, other routines
 * Invokes: none
 * Remarks: frees Dilist space when all DIs returned
 *---------------------------------------------------------------------------*/

intn
DFdiget(int32 list, uint16 *ptag, uint16 *pref)
{
    CONSTR(FUNC, "DFdiget");
    uint8      *p;
    DIlist_ptr  list_rec;

    list_rec = GID2REC(list);

    if (!list_rec)
        HRETURN_ERROR(DFE_ARGS, FAIL);
    if (list_rec->current >= list_rec->num)
        HRETURN_ERROR(DFE_INTERNAL, FAIL);

    /* compute address of Ndi'th di */
    p = (uint8 *) list_rec->DIlist + 4 * list_rec->current++;
    UINT16DECODE(p, *ptag);
    UINT16DECODE(p, *pref);

    if (list_rec->current == list_rec->num)
      {
          HDfree((VOIDP) list_rec->DIlist);    /*if all returned, free storage */
          HDfree((VOIDP) list_rec);
          Group_list[list & 0xffff] = NULL;     /* YUCK! BUG! */
      }
    return SUCCEED;
}

/*-----------------------------------------------------------------------------
 * Name:    DFdinobj
 * Purpose: return number of tag/refs in the group
 * Inputs:  list: handle to group (which is list of DIs)
 * Returns: number of tag/refs in the group on success,
 *  -1 on failure with error set
 * Users:   HDF systems programmers, hdp utility
 * Invokes: none
 * Remarks: nuttin'
 *---------------------------------------------------------------------------*/
intn
DFdinobj(int32 list)
{
    CONSTR(FUNC, "DFdinobj");
    DIlist_ptr  list_rec;

    list_rec = GID2REC(list);

    if (!list_rec)
        HRETURN_ERROR(DFE_ARGS, FAIL);

    return (list_rec->num);
}   /* DFdinobj() */

/*-----------------------------------------------------------------------------
 * Name:    DFdisetup
 * Purpose: setup space for storing a list of DIs to be written out
 * Inputs:  maxsize: maximum number of DIs expected in the list
 * Returns: FAIL on failure with error set
 *          else a group ID
 * Users:   HDF systems programmers, DF8putrig, other routines
 * Invokes: none
 * Remarks: This call should go away sometime.  Need better way to allocate
 *          space, possibly just use a big block of static space
 *---------------------------------------------------------------------------*/

int32
DFdisetup(int maxsize)
{
    CONSTR(FUNC, "DFdisetup");
    DIlist_ptr  new_list;

    new_list = (DIlist_ptr) HDmalloc((uint32) sizeof(DIlist));

    if (!new_list)
        HRETURN_ERROR(DFE_NOSPACE, FAIL);

    new_list->DIlist = (uint8 *) HDmalloc((uint32) (maxsize * 4));
    if (!new_list->DIlist)
      {
          HDfree((VOIDP) new_list);
          HRETURN_ERROR(DFE_NOSPACE, FAIL)
      }

    new_list->num = maxsize;
    new_list->current = 0;

    return setgroupREC(new_list);
}

/*-----------------------------------------------------------------------------
 * Name:    DFdiput
 * Purpose: add a DI to the list to be written out
 * Inputs:  tag, ref: DI to add
 * Returns: 0 on success, -1 on failure with error set
 * Users:   HDF systems programmers, DF8putrig, other routines
 * Invokes: none
 * Remarks: arg is tag/ref rather than DI for convenience
 *---------------------------------------------------------------------------*/

intn
DFdiput(int32 list, uint16 tag, uint16 ref)
{
    CONSTR(FUNC, "DFdiput");
    uint8      *p;
    DIlist_ptr  list_rec;

    list_rec = GID2REC(list);

    if (!list_rec)
        HRETURN_ERROR(DFE_ARGS, FAIL);
    if (list_rec->current >= list_rec->num)
        HRETURN_ERROR(DFE_INTERNAL, FAIL);

    /* compute address of Ndi'th di to put tag/ref in */
    p = (uint8 *) list_rec->DIlist + 4 * list_rec->current++;
    UINT16ENCODE(p, tag);
    UINT16ENCODE(p, ref);

    return SUCCEED;
}

/*-----------------------------------------------------------------------------
 * Name:    DFdiwrite
 * Purpose: Write DI list out to HDF file
 * Inputs:  file_id: HDF file pointer
 *          tag, ref: tag and ref of group whose contents is the list
 * Returns: 0 on success, -1 on failure with error set
 * Users:   HDF systems programmers, DF8putrig, other routines
 * Invokes: none
 * Remarks: frees storage for Dilist
 *---------------------------------------------------------------------------*/

intn
DFdiwrite(int32 file_id, int32 list, uint16 tag, uint16 ref)
{
    CONSTR(FUNC, "DFdiwrite");
    int32       ret;            /* return value */
    DIlist_ptr  list_rec;

    if (!HDvalidfid(file_id))
        HRETURN_ERROR(DFE_ARGS, FAIL);

    list_rec = GID2REC(list);

    if (!list_rec)
        HRETURN_ERROR(DFE_ARGS, FAIL);

    ret = Hputelement(file_id, tag, ref, list_rec->DIlist,
                      (int32) list_rec->current * 4);
    HDfree((VOIDP) list_rec->DIlist);
    HDfree((VOIDP) list_rec);
    Group_list[list & 0xffff] = NULL;   /* YUCK! BUG! */
    return (intn) ret;
}


/*-----------------------------------------------------------------------------
 * Name:    DFdifree
 * Purpose: Cleanup DI group
 * Inputs:  groupID
 * Returns: none
 * Users:   callers of DFdiget() when it is NOT called for every pair in the group.
 * Invokes: none
 * Remarks: Notes from Fortner Build Notes:
 *		While working on a group, its info is stored in RAM, and the pointer to 
 *		that info is kept in a global array called Group_List. the size of Group_List 
 *		is fixed, and contains MAX_GROUPS pointers. When DFdiget() has returned 
 *		the last tag-ref pair from a given group's info, that info is freed, and 
 *		the corresponding slot in the Group_List array is available for re-use.
 *
 *		If DFdiget() is NOT called for every pair in the group, the group info is 
 *		never freed, except by the use of this routine. So when a loop based on 
 *		DFdiget() exits early, it should first call freeDIGroup() to recover the 
 *		group slot for future use. 
 *
 *		The typical example seems to be an error occuring within the DFdiget() 
 *		loop or finding an element while doing a search.
 *
 *---------------------------------------------------------------------------*/
void DFdifree(int32 groupID)
{
#ifdef LATER
    CONSTR(FUNC, "DFdifree");
#endif /* LATER */
	DIlist_ptr	list_rec;
	
	list_rec = GID2REC( groupID );
	if (list_rec == NULL )
		return;
	
	HDfree((void*) list_rec->DIlist );
	HDfree((void*) list_rec );
	Group_list[groupID & 0xffff ] = NULL;
}
