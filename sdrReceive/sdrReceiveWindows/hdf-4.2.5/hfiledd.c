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

/* $Id: hfiledd.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*
FILE
    hfiledd.c - DD & DD block management routines.

REMARKS
    These routines provide all the management for the DD list, both on disk
    and in memory.  The DD list is read in from disk (or created in memory
    for a new file) here and the tag tree and all the DD modifying functions
    are in this module.  DO NOT MODIFY THE DD BLOCKS FROM OUTSIDE THIS FILE!

DESIGN
    The DD blocks are stored in memory in a very similar way to the way they
    are stored in the file: in a doubly-linked list of DD blocks with each
    block having a pointer to the array of DDs in it (in the file).  There
    are additional memory structures which are indexed into the DD list which
    are designed for faster access to certain manipulations of the DD list.
    The tag_tree is a tbbt of the tags contained within the file.  Each
    node of the tag_tree has a link to a bit-vector for keeping track of the
    refs used for that tag and a link to a dynamic array pointers into the
    DD list for each ref # used.

BUGS/LIMITATIONS

EXPORTED ROUTINES
  User-level functions:
    Hdupdd      - Duplicate a data descriptor
    Hnumber     - Count number of occurrances of tag/ref in file
    Hnewref     - Returns a ref that is unique in the file
    Htagnewref  - Returns a ref that is unique in the file for a given tag
    Hfind       - Locate the next object of a search in an HDF file
    Hdeldd      - Delete a data descriptor

  Developer-level routines
    HDcheck_tagref - Checks to see if tag/ref is in DD list i.e. created already
    HDreuse_tagref - reuse a data descriptor preserving tag/refw(assumes DD exists)

  Tag/ref functions:
    HTPcreate   - Create (& attach to) a tag/ref pair (inserts into DD list also)
    HTPselect   - Attach to an existing DD in the DD list
    HTPendaccess- End access to an attached DD in the DD list
    HTPdelete   - Mark a tag/ref pair as free (marks space as free in DD list)
                    (ends access to the tag/ref also)
    HTPupdate   - Change the offset and/or length of a data object
    HTPinquire  - Get the DD information for a DD (i.e. tag/ref/offset/length)
    HTPis_special- Check if a DD id is associated with a special tag
  DD list functions:
    HTPstart    - Initialize the DD list from disk (creates the DD list in memory)
    HTPinit     - Create a new DD list (creates the DD list in memory)
    HTPsync     - Flush the DD list to disk (syncronizes with disk)
    HTPend      - Close the DD list to disk (syncronizes with disk too)
LOCAL ROUTINES
    HTIfind_dd      - find a specific DD in the file
    HTInew_dd_block - create a new (empty) DD block
    HTIupdate_dd    - update a DD on disk
    HTIcount_dd     - counts the dd's of a certain type in file
    HTIregister_tag_ref     - insert a ref into the tag tree for a file
    HTIunregister_tag_ref   - remove a ref from the tag tree for a file

OLD ROUTINES
    HIlookup_dd             - find the dd record for an element
    HIflush_dds             - flush changed DD blocks to file
    HIinit_file_dds         - Initialize DD blocks for a new file
    HIfill_file_rec         - read in all of the DDs
    HIadd_hash_dd           - add a dd to the hash table
    HIdel_hash_dd           - remove a dd from the hash table
    HIfind_dd               - find the dd record for an element
    HIcount_dd              - counts the dd's of a certain type in file
    HInew_dd_block          - create a new (empty) DD block
    HIupdate_dd             - write an updated dd to the file
    HIregister_tag_ref      - mark a ref as used for a tag
    HIunregister_tag_ref    - mark a ref as un-used for a tag

AUTHOR
   Quincey Koziol

MODIFICATION HISTORY
   12/20/95  - Starting writing specs & coding prototype
*/

#include "hdf.h"
#include "hfile.h"

/* Private routines */
static intn HTIfind_dd(filerec_t * file_rec, uint16 look_tag, uint16 look_ref,
            dd_t ** pdd, intn direction);

static intn HTInew_dd_block(filerec_t * file_rec);

static intn HTIupdate_dd(filerec_t * file_rec, dd_t * dd);

static intn HTIcount_dd(filerec_t * file_rec, uint16 cnt_tag, uint16 cnt_ref,
            uintn *all_cnt, uintn *real_cnt);

static intn HTIregister_tag_ref(filerec_t * file_rec, dd_t *dd);

static intn HTIunregister_tag_ref(filerec_t * file_rec, dd_t *dd_ptr);

/* Local definitions */
/* The initial size of a ref dynarray */
#define REF_DYNARRAY_START  64
/* The increment of a ref dynarray */
#define REF_DYNARRAY_INCR   256
/* macros to encode and decode a DD */
#define DDENCODE(p, tag,ref,offset,length) \
   {UINT16ENCODE(p, tag); \
    UINT16ENCODE(p, ref); \
    INT32ENCODE(p, offset); \
    INT32ENCODE(p, length); \
   }
#define DDDECODE(p, tag,ref,offset,length) \
   {UINT16DECODE(p, tag); \
    UINT16DECODE(p, ref); \
    INT32DECODE(p, offset); \
    INT32DECODE(p, length); \
   }


/******************************************************************************
 NAME
     HTPstart - Initialize the DD list in memory

 DESCRIPTION
    Reads the DD blocks from disk and creates the in-memory structures for
    handling them.  This routine should only be called once for a given
    file and HTPend should be called when finished with the DD list (i.e.
    when the file is being closed).

 RETURNS
    Returns SUCCEED if successful and FAIL otherwise

*******************************************************************************/
intn HTPstart(filerec_t *file_rec       /* IN:  File record to store info in */
)
{
  CONSTR(FUNC, "HTPstart");	/* for HERROR */
  uint8      *tbuf=NULL;  /* temporary buffer */
  uintn       tbuf_size=0;    /* temporary buffer size */
  int32       end_off = 0;	/* offset of the end of the file */
  intn        ret_value = SUCCEED;

  HEclear();
  /* Alloc start of linked list of ddblocks. */
  file_rec->ddhead = (ddblock_t *) HDmalloc(sizeof(ddblock_t));
  if (file_rec->ddhead == (ddblock_t *) NULL)
    HGOTO_ERROR(DFE_NOSPACE, FAIL);

  /* Keep the filerec_t pointer around for each ddblock */
  file_rec->ddhead->frec=file_rec;

  /* Only one elt in linked list so head is also last. */
  file_rec->ddlast = file_rec->ddhead;
  file_rec->ddlast->next = (ddblock_t *) NULL;
  file_rec->ddlast->prev = (ddblock_t *) NULL;

  /* The first ddblock always starts after the magic number.
  Set it up so that we start reading from there. */
  file_rec->ddlast->myoffset = MAGICLEN;	/* set offset of block in file */
  file_rec->ddlast->dirty = 0;	/* block does not need to be flushed */

  /* Initialize the tag tree */
  file_rec->tag_tree = tbbtdmake(tagcompare, sizeof(uint16), TBBT_FAST_UINT16_COMPARE);

  /* Initialize the DD atom group (trying 256 hash currently, feel free to change */
  if(HAinit_group(DDGROUP,256)==FAIL)
    HGOTO_ERROR(DFE_INTERNAL, FAIL);

  /* Read in the dd's one at a time and determine the max ref in the file
	   at the same time. */
  file_rec->maxref = 0;
  for (;;)
    {
        ddblock_t *ddcurr;      /* ptr to the current DD block */
        dd_t *curr_dd_ptr;      /* pointer to the current DD being read in */
        uint8       ddhead[NDDS_SZ+OFFSET_SZ];   /* storage for the DD header */
        uint8      *p;          /* Temporary buffer pointer. */
        intn        ndds;       /* number of DDs in a block */
        intn        i;          /* Temporary integer */

        /* Get a short-cut for the current DD block being read-in */
        ddcurr=file_rec->ddlast;
  
        /* Go to the beginning of the DD block */
        if (HPseek(file_rec, ddcurr->myoffset) == FAIL)
          HGOTO_ERROR(DFE_SEEKERROR, FAIL);
  
        /* Read in the start of this dd block.
           Read data consists of ndds (number of dd's in this block) and
           offset (offset to the next ddblock). */
        if (HP_read(file_rec, ddhead, NDDS_SZ + OFFSET_SZ) == FAIL)
          HGOTO_ERROR(DFE_READERROR, FAIL);
  
        /* Decode the numbers. */
        p = &ddhead[0];
        INT16DECODE(p, ddcurr->ndds);
        ndds = (intn)ddcurr->ndds;
        if (ndds <= 0)		/* validity check */
          HGOTO_ERROR(DFE_CORRUPT, FAIL);
        INT32DECODE(p, ddcurr->nextoffset);
  
        /* check if the DD block is the last thing in the file */
        /* (Unlikely, but possible (I think)) */
        if (ddcurr->myoffset + (NDDS_SZ + OFFSET_SZ) + (ndds * DD_SZ) > end_off)
          end_off = ddcurr->myoffset + (NDDS_SZ + OFFSET_SZ) + (ndds * DD_SZ);
  
        /* Now that we know how many dd's are in this block,
           alloc memory for the records. */
        ddcurr->ddlist = (dd_t *) HDmalloc((uint32) ndds * sizeof(dd_t));
        if (ddcurr->ddlist==(dd_t *)NULL)
          HGOTO_ERROR(DFE_NOSPACE, FAIL);
  
        /* Allocate memory for the temporary buffer also */
        if(tbuf==NULL || ((uintn)ndds*DD_SZ)>tbuf_size)
          {
              if (tbuf!=(uint8 *)NULL)
                  HDfree(tbuf);
              tbuf_size=(uintn)ndds*DD_SZ;
              tbuf=(uint8 *)HDmalloc(tbuf_size);
              if (tbuf==(uint8 *)NULL)
                HGOTO_ERROR(DFE_NOSPACE, FAIL);
          } /* end if */
  
        /* Index of current dd in ddlist of this ddblock is 0. */
        curr_dd_ptr=ddcurr->ddlist;
  
        /* Read in a chunk of dd's from the file. */
        if (HP_read(file_rec, tbuf, ndds * DD_SZ) == FAIL)
          HGOTO_ERROR(DFE_READERROR, FAIL);
  
      /* decode the dd's */
        p = tbuf;
        for (i = 0; i < ndds; i++, curr_dd_ptr++)
          {
	    DDDECODE(p, curr_dd_ptr->tag, curr_dd_ptr->ref,
		curr_dd_ptr->offset, curr_dd_ptr->length);
            curr_dd_ptr->blk=ddcurr;
  
             /* check if maximum ref # exceeded */
            if (file_rec->maxref < curr_dd_ptr->ref)
              file_rec->maxref = curr_dd_ptr->ref;
  
            /* check if the data element is the last thing in the file */
            if ((curr_dd_ptr->offset + curr_dd_ptr->length) > end_off)
              end_off = curr_dd_ptr->offset + curr_dd_ptr->length;
  
            /* Add to the tag info tree */
            if(curr_dd_ptr->tag!=DFTAG_NULL)
                if(HTIregister_tag_ref(file_rec,curr_dd_ptr)==FAIL)
                    HGOTO_ERROR(DFE_INTERNAL, FAIL);
          }
  
        if (ddcurr->nextoffset != 0)
          {	/* More ddblocks in the file */
              ddblock_t *ddnew;    /* ptr to the new DD block */
  
      /* extend the linked list */
            ddcurr->next = ddnew = (ddblock_t *) HDmalloc((uint32) sizeof(ddblock_t));
            if (ddnew == (ddblock_t *) NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
  
            ddnew->prev = ddcurr;
            ddnew->next = (ddblock_t *) NULL;
            ddnew->ddlist = (dd_t *) NULL;
            ddnew->myoffset = ddcurr->nextoffset;
            ddnew->dirty= FALSE;
            file_rec->ddlast = ddnew;

            /* Keep the filerec_t pointer around for each ddblock */
            ddnew->frec=file_rec;
          }	/* end if */
        else
              break;
      } /* end for */
    
    /* Update the DFTAG_NULL pointers */
    file_rec->ddnull=NULL;
    file_rec->ddnull_idx=(-1);

    /* Update the end of the file from the DD's we have read in */
    file_rec->f_end_off = end_off;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  if(tbuf!=NULL)
      HDfree(tbuf);

  return ret_value;
} /* end HTPstart() */

/******************************************************************************
 NAME
     HTPinit - Create a new DD list in memory

 DESCRIPTION
    Creates a new DD list in memory for a newly created file.  This routine
    should only be called once for a given file and HTPend should be called
    when finished with the DD list (i.e.  when the file is being closed).

 RETURNS
    Returns SUCCEED if successful and FAIL otherwise

*******************************************************************************/
intn HTPinit(filerec_t *file_rec,       /* IN: File record to store info in */
    int16 ndds                          /* IN: # of DDs to store in each block */
)
{
    CONSTR(FUNC, "HTPinit");    /* for HERROR */
    ddblock_t  *block;          /* dd block to intialize */
    uint8       ddhead[NDDS_SZ+OFFSET_SZ];   /* storage for the DD header */
    uint8      *tbuf=NULL;      /* temporary buffer */
    uint8      *p;              /* temp buffer ptr */
    dd_t       *list;           /* list of dd */
    intn        ret_value = SUCCEED;
  
    HEclear();
    if (file_rec == NULL || ndds<0)	/* valid arguments */
      HGOTO_ERROR(DFE_ARGS, FAIL);
  
    /* 'reasonablize' the value of ndds.  0 means use default */
    if (0 == ndds)
      ndds = DEF_NDDS;
    else if (ndds < MIN_NDDS)
      ndds = MIN_NDDS;
  
    /* allocate the dd block in memory and initialize it */
    file_rec->ddhead = (ddblock_t *) HDmalloc(sizeof(ddblock_t));
    if (file_rec->ddhead == (ddblock_t *) NULL)
      HGOTO_ERROR(DFE_NOSPACE, FAIL);
    block = file_rec->ddlast = file_rec->ddhead;
    block->prev = (ddblock_t *) NULL;
    block->ndds = ndds;
    block->next = (ddblock_t *) NULL;
    block->nextoffset = 0;
    block->myoffset = MAGICLEN;
    block->dirty = FALSE;

    /* Keep the filerec_t pointer around for each ddblock */
    block->frec=file_rec;
  
    /* write first dd block header to file */
    p = &ddhead[0];
    INT16ENCODE(p, block->ndds);
    INT32ENCODE(p, (int32) 0);
    if (HP_write(file_rec, ddhead, NDDS_SZ + OFFSET_SZ) == FAIL)
      HGOTO_ERROR(DFE_WRITEERROR, FAIL);
  
    /* allocate and initialize dd list */
    list = block->ddlist = (dd_t *) HDmalloc((uint32) ndds * sizeof(dd_t));
    if (list == (dd_t *) NULL)
      HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* Fill the first memory DD block with NIL dd's */
    list[0].tag = DFTAG_NULL;
    list[0].ref = DFREF_NONE;
    list[0].length = INVALID_LENGTH;
    list[0].offset = INVALID_OFFSET;
    list[0].blk = block;
    HDmemfill(&list[1],&list[0],sizeof(dd_t),(uint32)(ndds-1));

    tbuf=(uint8 *)HDmalloc(ndds*DD_SZ);
    if (tbuf == NULL)	/* check for DD list */
      HGOTO_ERROR(DFE_NOSPACE, FAIL);
  
    /* Fill the first disk DD block with NIL dd's */
    p = tbuf;
    DDENCODE(p, (uint16) DFTAG_NULL, (uint16) DFREF_NONE,
	(int32) INVALID_LENGTH, (int32) INVALID_OFFSET);
    HDmemfill(p,tbuf,DD_SZ,(uint32)(ndds-1));

    /* Write the NIL dd's out into the DD block on disk */
    if (HP_write(file_rec, tbuf, ndds * DD_SZ) == FAIL)
      HGOTO_ERROR(DFE_WRITEERROR, FAIL);

    /* Update the DFTAG_NULL pointers */
    file_rec->ddnull=block;
    file_rec->ddnull_idx=(-1);
  
    /* set the end of the file currently to the end of the first DD block */
    file_rec->f_end_off = block->myoffset + (NDDS_SZ + OFFSET_SZ) + (block->ndds * DD_SZ);
  
    /* no dd's yet, so maximum ref is 0 */
    file_rec->maxref = 0;
  
    /* Initialize the tag tree */
    file_rec->tag_tree = tbbtdmake(tagcompare, sizeof(uint16), TBBT_FAST_UINT16_COMPARE);

    /* Initialize the DD atom group (trying 256 hash currently, feel free to change */
    if(HAinit_group(DDGROUP,256)==FAIL)
      HGOTO_ERROR(DFE_INTERNAL, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  HDfree(tbuf);

  return ret_value;
} /* end HTPinit() */

/******************************************************************************
 NAME
     HTPsync - Flush the DD list in memory

 DESCRIPTION
    Syncronizes the in-memory copy of the DD list with the copy on disk by
    writing out the DD blocks which have changed to disk.
    
 RETURNS
    Returns SUCCEED if successful and FAIL otherwise

*******************************************************************************/
intn HTPsync(filerec_t *file_rec       /* IN:  File record to store info in */
)
{
    CONSTR(FUNC, "HTPsync");	/* for HERROR */
    ddblock_t  *block;		/* dd block to intialize */
    uint8       ddhead[NDDS_SZ+OFFSET_SZ];   /* storage for the DD header */
    uint8      *tbuf=NULL;  /* temporary buffer */
    uintn       tbuf_size=0;    /* temporary buffer size */
    uint8      *p;		/* temp buffer ptr */
    dd_t       *list;		/* list of dd */
    int16       ndds;		/* # of DDs per block */
    intn        i;		/* temp ints */
    intn        ret_value = SUCCEED;
  
    HEclear();
    block = file_rec->ddhead;
    if (block == NULL)	/* check for DD list */
      HGOTO_ERROR(DFE_BADDDLIST, FAIL);
  
    while (block != NULL)
      {	/* check all the blocks for flushing */
        if (block->dirty == TRUE)
          {	/* flush this block? */
            if (HPseek(file_rec, block->myoffset) == FAIL)
              HGOTO_ERROR(DFE_SEEKERROR, FAIL);
  
      /* write dd block header to file */
            p = ddhead;
            INT16ENCODE(p, block->ndds);
            INT32ENCODE(p, block->nextoffset);
            if (HP_write(file_rec, ddhead, NDDS_SZ + OFFSET_SZ) == FAIL)
              HGOTO_ERROR(DFE_WRITEERROR, FAIL);
  
      /* n is the maximum number of dd's in tbuf */
            ndds = block->ndds;
            /* Allocate memory for the temporary buffer also */
            if(tbuf==NULL || ((uintn)ndds*DD_SZ)>tbuf_size)
              {
                  if (tbuf!=(uint8 *)NULL)
                      HDfree(tbuf);
                  tbuf_size=(uintn)ndds*DD_SZ;
                  tbuf=(uint8 *)HDmalloc(tbuf_size);
                  if (tbuf==(uint8 *)NULL)
                    HGOTO_ERROR(DFE_NOSPACE, FAIL);
              } /* end if */
  
      /* write dd list to file */
            list = &block->ddlist[0];	/* start at the first DD, go from there */
            p = tbuf;
            for (i = 0; i < ndds; i++, list++)
		DDENCODE(p, list->tag, list->ref, list->offset, list->length);

            if (HP_write(file_rec, tbuf, ndds * DD_SZ) == FAIL)
              HGOTO_ERROR(DFE_WRITEERROR, FAIL);
  
            block->dirty = FALSE;	/* block has been flushed */
          }	/* end if */
        block = block->next;	/* advance to next block for file */
      }		/* end while */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  if (tbuf!=(uint8 *)NULL)
      HDfree(tbuf);

  return ret_value;
} /* end HTPsync() */

/******************************************************************************
 NAME
     HTPend - Terminate the DD list in memory

 DESCRIPTION
    Terminates access to the DD list in memory, writing the DD blocks out to
    the disk (if they've changed).  After this routine is called, no further
    access to tag/refs (or essentially any other HDF objects) can be performed
    on the file.

 RETURNS
    Returns SUCCEED if successful and FAIL otherwise

*******************************************************************************/
intn HTPend(filerec_t *file_rec       /* IN:  File record to store info in */
) 
{
    CONSTR(FUNC, "HTPend");	/* for HERROR */
    ddblock_t  *bl, *next;	/* current ddblock and next ddblock pointers.
				   for freeing ddblock linked list */
    intn        ret_value = SUCCEED;

    HEclear();
    if(HTPsync(file_rec)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    for (bl = file_rec->ddhead; bl!=NULL; bl = next)
      {
        next = bl->next;
        if (bl->ddlist)
          HDfree((VOIDP) bl->ddlist);
        HDfree((VOIDP) bl);
      }

    /* Chuck the tag info tree too */
    tbbtdfree(file_rec->tag_tree,tagdestroynode,NULL);

    /* Shutdown the DD atom group */
    if(HAdestroy_group(DDGROUP)==FAIL)
      HGOTO_ERROR(DFE_INTERNAL, FAIL);

    file_rec->ddhead = (ddblock_t *) NULL;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
} /* end HTPend() */

/******************************************************************************
 NAME
     HTPcreate - Create (and attach to) a tag/ref pair

 DESCRIPTION
    Creates a new tag/ref pair in memory and inserts the tag/ref pair into the
    DD list to be written out to disk.  This routine returns a DD id which can
    be used in the other tag/ref routines to modify the DD.

 RETURNS
    Returns DD id if successful and FAIL otherwise

*******************************************************************************/
atom_t HTPcreate(filerec_t *file_rec,   /* IN: File record to store info in */
    uint16 tag,                         /* IN: Tag to create */
    uint16 ref                          /* IN: ref to create */
)
{
    CONSTR(FUNC, "HTPcreate");  /* for HERROR */
    dd_t *dd_ptr=NULL;          /* ptr to dd created */
    atom_t ret_value=SUCCEED;

    HEclear();
    if(file_rec==NULL || (tag==DFTAG_NULL || tag==DFTAG_WILDCARD) ||
            ref==DFREF_WILDCARD)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    if(HTIfind_dd(file_rec,(uint16)DFTAG_NULL,(uint16)DFTAG_WILDCARD,
            &dd_ptr,DF_FORWARD)==FAIL)
      {
        if (HTInew_dd_block(file_rec) == FAIL)
          {
            HGOTO_ERROR(DFE_NOFREEDD, FAIL);
          }	/* end if */
        else
            dd_ptr=&file_rec->ddlast->ddlist[0];
      } /* end if */

    /* Insert DD information into the DD list in memory */
    dd_ptr->tag=tag;
    dd_ptr->ref=ref;
    /* the following assures object defintion in DD list 
       without data written for object. */
    dd_ptr->offset=INVALID_OFFSET;
    dd_ptr->length=INVALID_LENGTH;

    /* dd_ptr->blk should already be correctly set */

    /* Update the disk, etc. */
    if(HTIupdate_dd(file_rec,dd_ptr)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Mark off the ref # as 'used' in the tag tree & add to dynarray of refs */
    if(HTIregister_tag_ref(file_rec,dd_ptr)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Get the atom to return */
    if((ret_value=HAregister_atom(DDGROUP,dd_ptr))==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* HTPcreate() */

/******************************************************************************
 NAME
     HTPselect - Attach to an existing tag/ref pair

 DESCRIPTION
    Attaches to an existing tag/ref pair.  This routine returns a DD id which
    can be used in the other tag/ref routines to modify the DD.

 RETURNS
    Returns DD id if successful and FAIL otherwise

*******************************************************************************/
atom_t HTPselect(filerec_t *file_rec,   /* IN: File record to store info in */
    uint16 tag,                         /* IN: Tag to select */
    uint16 ref                          /* IN: ref to select */
)
{
    CONSTR(FUNC, "HTPselect");  /* for HERROR */
    dd_t *dd_ptr;         /* ptr to the DD info for the tag/ref */
    tag_info **tip_ptr;   /* ptr to the ptr to the info for a tag */
    tag_info *tinfo_ptr;  /* pointer to the info for a tag */
    uint16 base_tag=BASETAG(tag);    /* corresponding base tag (if the tag is special) */
    atom_t ret_value=SUCCEED;

    HEclear();
    if(file_rec==NULL || (tag==DFTAG_NULL || tag==DFTAG_WILDCARD) ||
            ref==DFREF_WILDCARD)
        HGOTO_ERROR(DFE_ARGS, FAIL);
        
    /* Try to find the regular tag in the tag info tree */
    if((tip_ptr=(tag_info **)tbbtdfind(file_rec->tag_tree,(VOIDP)&base_tag,NULL))==NULL)
        HGOTO_DONE(FAIL); /* Not an error, we just didn't find the object */

    tinfo_ptr=*tip_ptr; /* get the pointer to the tag info */
    if((dd_ptr=DAget_elem(tinfo_ptr->d,(intn)ref))==NULL)
        HGOTO_DONE(FAIL); /* Not an error, we just didn't find the object */

    /* Get the atom to return */
    if((ret_value=HAregister_atom(DDGROUP,dd_ptr))==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* HTPselect() */

/******************************************************************************
 NAME
     HTPendaccess - End access to an existing tag/ref pair

 DESCRIPTION
    Ends access to an existing tag/ref pair.  Any further access to the tag/ref
    pair may result in incorrect information being recorded about the DD in
    memory or on disk.

 RETURNS
    Returns SUCCEED if successful and FAIL otherwise

*******************************************************************************/
intn HTPendaccess(atom_t ddid           /* IN: DD id to end access to */
)
{
#ifdef LATER
    CONSTR(FUNC, "HTPendaccess"); /* for HERROR */
#endif /* LATER */
    int32 ret_value=SUCCEED;

    /* Chuck the atom */
    if(HAremove_atom(ddid)==NULL)
        HGOTO_DONE(FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* HTPendaccess() */

/******************************************************************************
 NAME
     HTPdelete - Delete an existing tag/ref pair

 DESCRIPTION
    Deletes a tag/ref from the file.  Also ends access to the tag/ref pair.
    Any further access to the tag/ref pair may result in incorrect information
    being recorded about the DD in memory or on disk.

 RETURNS
    Returns SUCCEED if successful and FAIL otherwise

*******************************************************************************/
intn HTPdelete(atom_t ddid              /* IN: DD id to delete */
)
{
    CONSTR(FUNC, "HTPdelete"); /* for HERROR */
    dd_t *dd_ptr;           /* ptr to the DD info for the tag/ref */
    filerec_t * file_rec;
    int32 ret_value=SUCCEED;

    HEclear();
    /* Retrieve the atom's object, so we can delete the tag/ref */
    if((dd_ptr=HAatom_object(ddid))==NULL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Grab this information, because the global dd_info will be deleted in HTIunregister_tag_ref */
    file_rec=dd_ptr->blk->frec;

    /* Since we don't know where we are, reset the DFTAG_NULL pointers */
    file_rec->ddnull=NULL;
    file_rec->ddnull_idx=(-1);
  
    if (HPfreediskblock(file_rec,dd_ptr->offset,dd_ptr->length) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Update the disk, etc. */
    if(HTIupdate_dd(file_rec,dd_ptr)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Remove the ref # as 'used' in the tag tree & delete from dynarray of refs */
    if(HTIunregister_tag_ref(file_rec,dd_ptr)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Destroy everything */
    if(HAremove_atom(ddid)==NULL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* HTPdelete() */

/******************************************************************************
 NAME
     HTPupdate - Change the offset or length of an existing tag/ref pair

 DESCRIPTION
    Updates a tag/ref in the file, allowing the length and/or offset to be
    modified. 

    Note: a value of '-2' for both 'length' and 'offset' are used to indicate
    that the length or offset (respectively) is unchanged and should
    remain the same. Kind of ugly but works for now.

 RETURNS
    Returns SUCCEED if successful and FAIL otherwise

*******************************************************************************/
intn HTPupdate(atom_t ddid,             /* IN: DD id to update */
    int32 new_off,                      /* IN: new offset for DD */
    int32 new_len                       /* IN: new length for DD */
)
{
    CONSTR(FUNC, "HTPupdate");  /* for HERROR */
    dd_t *dd_ptr    = NULL;     /* ptr to the DD info for the tag/ref */
    int32 dont_change = -2;     /* initialize to '-2' */
    int32 ret_value = SUCCEED;

    HEclear();
    /* Retrieve the atom's object, so we can update the DD */
    if((dd_ptr=HAatom_object(ddid))==NULL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Update the tag/ref in memory */
    if(new_len != dont_change)
        dd_ptr->length=new_len;
    if(new_off != dont_change)
        dd_ptr->offset=new_off;

    /* Update the disk, etc. */
    if(HTIupdate_dd(dd_ptr->blk->frec,dd_ptr)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* HTPupdate() */

/******************************************************************************
 NAME
     HTPinquire - Get the DD information for a DD (i.e. tag/ref/offset/length)

 DESCRIPTION
    Get the DD information for a DD id from the DD block.  Passing NULL for
    any parameter does not try to update that parameter.

 RETURNS
    Returns SUCCEED if successful and FAIL otherwise

*******************************************************************************/
intn HTPinquire(atom_t ddid,            /* IN: DD id to inquire about */
    uint16 *tag,                        /* IN: tag of DD */
    uint16 *ref,                        /* IN: ref of DD */
    int32 *off,                         /* IN: offset of DD */
    int32 *len                          /* IN: length of DD */
)
{
    CONSTR(FUNC, "HTPinquire"); /* for HERROR */
    dd_t *dd_ptr;               /* ptr to the DD info for the tag/ref */
    intn ret_value=SUCCEED;

    HEclear();
    /* Retrieve the atom's object, so we can update the DD */
    if((dd_ptr=HAatom_object(ddid))==NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Get the information requested */
    if(tag!=NULL)
        *tag=dd_ptr->tag;
    if(ref!=NULL)
        *ref=dd_ptr->ref;
    if(off!=NULL)
        *off=dd_ptr->offset;
    if(len!=NULL)
        *len=dd_ptr->length;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* HTPinquire() */

/******************************************************************************
 NAME
     HTPis_special - Check if a DD id is associated with a special tag

 DESCRIPTION
    Checks if the tag for the DD id is a special tag.

 RETURNS
    Returns TRUE(1)/FALSE(0)

*******************************************************************************/
intn HTPis_special(atom_t ddid             /* IN: DD id to inquire about */
)
{
    CONSTR(FUNC, "HTPis_special"); /* for HERROR */
    dd_t *dd_ptr;               /* ptr to the DD info for the tag/ref */
    int32 ret_value=FAIL;

    HEclear();
    /* Retrieve the atom's object, so we can update the DD */
    if((dd_ptr=HAatom_object(ddid))==NULL)
        HGOTO_ERROR(DFE_ARGS, FALSE);

    /* Get the information requested */
    if(SPECIALTAG(dd_ptr->tag))
        ret_value=TRUE;
    else
        ret_value=FALSE;

done:
  if(ret_value == FALSE)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* HTPis_special() */

/******************************************************************************
 NAME
     Hdupdd - Duplicate a data descriptor

 DESCRIPTION
    Duplicates a data descriptor so that the new tag/ref points to the
    same data element pointed to by the old tag/ref.  Return FAIL if
    the given tag/ref are already in use.

 RETURNS
    returns SUCCEED (0) if successful, FAIL (-1) otherwise

*******************************************************************************/
intn Hdupdd(int32 file_id,      /* IN: File ID the tag/refs are in */
        uint16 tag,             /* IN: Tag of new tag/ref */
        uint16 ref,             /* IN: Ref of new tag/ref */
        uint16 old_tag,         /* IN: Tag of old tag/ref */
        uint16 old_ref          /* IN: Ref of old tag/ref */
)
{
    CONSTR(FUNC, "Hdupdd"); /* for HERROR */
    filerec_t  *file_rec;		/* file record */
    atom_t      old_dd;         /* The DD id for the old DD */
    atom_t      new_dd;         /* The DD id for the new DD */
    int32       old_len;        /* The length of the old DD */
    int32       old_off;        /* The offset of the old DD */
    intn ret_value=SUCCEED;

    /* clear error stack and check validity of file id */
    HEclear();
    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
      HGOTO_ERROR(DFE_ARGS, FAIL);
  
    /* Attach to the old DD in the file */
    if((old_dd=HTPselect(file_rec,old_tag,old_ref))==FAIL)
      HGOTO_ERROR(DFE_NOMATCH, FAIL);

    /* Create the new DD in the file */
    if((new_dd=HTPcreate(file_rec,tag,ref))==FAIL)
      HGOTO_ERROR(DFE_DUPDD, FAIL);

    /* Retrieve the old offset & length */
    if(HTPinquire(old_dd,NULL,NULL,&old_off,&old_len)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Set the new DD's offset & length to the same as the old DD */
    if(HTPupdate(new_dd,old_off,old_len)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* End access to the old & new DDs */
    if(HTPendaccess(old_dd)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);
    if(HTPendaccess(new_dd)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* Hdupdd() */

/******************************************************************************
 NAME
     Hnumber - Determine the number of objects of a given type

 DESCRIPTION
    Determine how many objects of the given tag are in the file.
    tag may be set to DFTAG_WILDCARD to get back the total number
    of objects in the file.

    Note, a return value of zero is not a fail condition.

 RETURNS
    the number of objects of type 'tag' else FAIL

*******************************************************************************/
int32 Hnumber(int32 file_id,    /* IN: File ID the tag/refs are in */
        uint16 tag              /* IN: Tag to count */
)
{
    CONSTR(FUNC, "Hnumber");
    uintn       all_cnt;
    uintn       real_cnt;
    filerec_t  *file_rec;           /* file record */
    int32 ret_value=SUCCEED;

    /* convert file id to file record */
    file_rec = HAatom_object(file_id);

    HEclear();
    if (BADFREC(file_rec))
      HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Go count the items with that tag */
    if (HTIcount_dd(file_rec, tag, DFREF_WILDCARD, &all_cnt, &real_cnt) == FAIL)
      HGOTO_ERROR(DFE_INTERNAL, FAIL);

    ret_value = (int32) real_cnt;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* Hnumber() */

/******************************************************************************
 NAME
     Hnewref - Returns a ref that is guaranteed to be unique in the file

 DESCRIPTION
    Returns a ref number that can be used with any tag to produce a
    unique tag/ref.  Successive calls to Hnewref will generate a
    strictly increasing sequence until the highest possible ref had been
    returned, then Hnewref will return unused ref's starting from 1.

 RETURNS
    returns the ref number, 0 otherwise

*******************************************************************************/
uint16 
Hnewref(int32 file_id /* IN: File ID the tag/refs are in */)
{
    CONSTR(FUNC, "Hnewref");
    filerec_t  *file_rec;	   /* file record */
    uint16      ref;		   /* the new ref */
    uint16      ret_value = DFREF_NONE;
    uint32      i_ref;        /* index for FOR loop */

    /* clear error stack and check validity of file record id */
    HEclear();
    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, 0);
  
    /* if maxref of this file is still below the maximum,
     just return next number */
    if (file_rec->maxref < MAX_REF)
        ret_value = ++(file_rec->maxref);
    else
      { /* otherwise, search for an empty ref */
        /* incredibly slow but unlikely situation */
/* This could possibly get replaced with some sort of bit-vector manipulation -QAK */
        for (i_ref = 1; i_ref <= (uint32)MAX_REF; i_ref++)
          {
            dd_t *dd_ptr=NULL;
            ref = (uint16)i_ref; 
            if (HTIfind_dd(file_rec, (uint16) DFTAG_WILDCARD, ref, &dd_ptr, DF_FORWARD) == FAIL)
              {
               ret_value = ref; /* set return value to ref found */
               break; /* break out of loop */
              } /* end if */
          } /* end for */
      } /* end else */

done:
  if(ret_value == DFREF_NONE)
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* Hnewref() */

/******************************************************************************
 NAME
    Htagnewref  - returns a ref that is unique in the file for a given tag

 DESCRIPTION
    Returns a ref number that can be used with any tag to produce a
    unique tag/ref.  Successive calls to Hnewref will generate a
    strictly increasing sequence until the highest possible ref had been
    returned, then Hnewref will return unused ref's starting from 1.

 RETURNS
    returns the ref number, 0 otherwise

*******************************************************************************/
uint16 
Htagnewref(int32 file_id,/* IN: File ID the tag/refs are in */
           uint16 tag    /* IN: Tag to search for a new ref for */)
{
    CONSTR(FUNC, "Htagnewref");
    filerec_t  *file_rec;  /* file record */
    tag_info   *tinfo_ptr; /* pointer to the info for a tag */
    tag_info  **tip_ptr;   /* ptr to the ptr to the info for a tag */
    uint16      base_tag = BASETAG(tag); /* corresponding base tag (if the tag is special) */
    uint16      ret_value = DFREF_NONE;

    /* clear error stack and check validity of file record id */
    HEclear();
    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, 0);
  
    if((tip_ptr = (tag_info **)tbbtdfind(file_rec->tag_tree,(VOIDP)&base_tag,NULL))==NULL)
        ret_value = 1;  /* The first available ref */
    else
      {   /* found an existing tag */
          tinfo_ptr = *tip_ptr; /* get the pointer to the tag info */
          if((ret_value = (uint16)bv_find(tinfo_ptr->b,-1,BV_FALSE)) == (uint16)FAIL)
              HGOTO_ERROR(DFE_BVFIND, 0);
      } /* end else */

done:
  if(ret_value == 0)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* Htagnewref() */

/******************************************************************************
 NAME
    Hfind - locate the next object of a search in an HDF file

 DESCRIPTION
    Searches for the `next' DD that fits the search tag/ref.  Wildcards
    apply.  If origin is DF_FORWARD, search from current position forwards
    in the file, otherwise DF_BACKWARD searches backward from the current
    position in the file.  If *find_tag and *find_ref are both set to
    0, this indicates the beginning of a search, and the search will
    start from the beginning of the file if the direction is DF_FORWARD
    and from the and of the file if the direction is DF_BACKWARD.

 RETURNS
    returns SUCCEED (0) if successful and FAIL (-1) otherwise

*******************************************************************************/
intn Hfind(int32 file_id,       /* IN: file ID to search in */
        uint16 search_tag,      /* IN: the tag to search for (can be DFTAG_WILDCARD) */
        uint16 search_ref,      /* IN: ref to search for (can be DFREF_WILDCARD) */
        uint16 *find_tag,       /* IN: if (*find_tag==0) and (*find_ref==0) then start search */
                                /* OUT: tag matching the search tag */
        uint16 *find_ref,       /* IN: if (*find_tag==0) and (*find_ref==0) then start search */
                                /* OUT: ref matching the search ref */
        int32 *find_offset,     /* OUT: offset of the data element found */
        int32 *find_length,     /* OUT: length of the data element found */
        intn direction          /* IN: Direction to search in: */
                                /*  DF_FORWARD searches forward from the current location */
                                /*  DF_BACKWARD searches backward from the current location */
)
{
    CONSTR(FUNC, "Hfind");	/* for HERROR */
    filerec_t  *file_rec;		/* file record */
    dd_t       *dd_ptr;		   /* ptr to current ddlist searched */
    intn    ret_value = SUCCEED;

    /* clear error stack and check validity of the access id */
    HEclear();
    if (file_id == FAIL || /* search_ref > MAX_REF || */ find_tag == NULL
        || find_ref == NULL || find_offset == NULL || find_length == NULL
        || (direction != DF_FORWARD && direction != DF_BACKWARD))
      HGOTO_ERROR(DFE_ARGS, FAIL);
  
    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
      HGOTO_ERROR(DFE_INTERNAL, FAIL);
  
    dd_ptr = NULL;
    if (*find_ref != 0 || *find_tag != 0)
      {		/* continue a search */
        /* get the block and index of the last tag/ref found, to continue */
        if (HTIfind_dd(file_rec, *find_tag, *find_ref, &dd_ptr, direction) == FAIL)
          HGOTO_ERROR(DFE_NOMATCH, FAIL);
      }		/* end else */
  
    /* Go get the next match in the given direction */
    if (HTIfind_dd(file_rec, search_tag, search_ref, &dd_ptr, direction) == FAIL)
      HGOTO_DONE(FAIL); /* Not an error, we just didn't find the object */
  
    *find_tag = dd_ptr->tag;
    *find_ref = dd_ptr->ref;
    *find_offset = dd_ptr->offset;
    *find_length = dd_ptr->length;

done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

  /* Normal function cleanup */
    return ret_value;
}	/* end Hfind() */

/******************************************************************************
 NAME
     HDcheck_tagref - Checks to see if tag/ref is in DD list i.e. created already

 DESCRIPTION
     Routine checks to see if tag/ref exists in the DD list i.e. has
     been created.

 RETURNS
     0-> tag/ref does not exist
     1-> tag/ref exists
    -1-> function failed

*******************************************************************************/
intn 
HDcheck_tagref(int32  file_id, /* IN: id of file */
               uint16 tag,     /* IN: Tag to check */
               uint16 ref      /* IN: ref to check */)
{
    CONSTR(FUNC, "HDcheck_tagref");  /* for HERROR */
    filerec_t *file_rec = NULL;  /* file record */
    dd_t      *dd_ptr = NULL;    /* ptr to the DD info for the tag/ref */
    tag_info **tip_ptr = NULL;   /* ptr to the ptr to the info for a tag */
    tag_info  *tinfo_ptr = NULL; /* pointer to the info for a tag */
    uint16     base_tag;         /* corresponding base tag (if the tag is special) */
    intn       ret_value = 1;  /* default tag/ref exists  */

    /* clear error stack */
    HEclear();

    /* check args */
    file_rec = HAatom_object(file_id);
    if(file_rec == NULL 
       || (tag == DFTAG_NULL || tag==DFTAG_WILDCARD) 
       ||  ref == DFREF_WILDCARD)
        HGOTO_ERROR(DFE_ARGS, -1);

    base_tag = BASETAG(tag);
        
    /* Try to find the regular tag in the tag info tree */
    if((tip_ptr = (tag_info **)tbbtdfind(file_rec->tag_tree,
                                         (VOIDP)&base_tag,NULL)) == NULL)
        HGOTO_DONE(0); /* Not an error, we just didn't find the object */

    tinfo_ptr = *tip_ptr; /* get the pointer to the tag info */
    if((dd_ptr = DAget_elem(tinfo_ptr->d,(intn)ref)) == NULL)
        HGOTO_DONE(0); /* Not an error, we just didn't find the object */

    /* found if we reach here*/
    ret_value = 1;

done:
  if(ret_value == -1)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* HDcheck_tagref() */

/************************************************************************
NAME
   HDreuse_tagref -- reuse a data descriptor preserving tag/ref(assumes DD exists)

DESCRIPTION
   Reuses the data descriptor of tag/ref in the dd list of the file.
   The tag/ref must already exist in the DD list.
   This routine is unsafe and may leave a file in a condition that is
   not usable by some routines.  Use with care. Not valid for
   special elments right now. Used for allowing the data to change
   and move somewhere else in the file for non-special elements.
   Must be carefully if apply to higher-level objects like GR's and SDS
   that are comprised of other objects.
   Usefull when re-writing simple elements whose size changes while
   preserving the original tag/ref of the element since other elements
   might refer to this element by tag/ref e.g. in a Vgroup.

   NOTE: this routine is similiar to Hdeldd() but with a different name

RETURNS
   returns SUCCEED (0) if successful, FAIL (-1) otherwise
************************************************************************/
intn 
HDreuse_tagref(int32 file_id, /* IN: id of file */
               uint16 tag,    /* IN: tag of data descriptor to reuse */
               uint16 ref     /* IN: ref of data descriptor to reuse */ )
{
  CONSTR(FUNC, "HDreusedd");   /* for HERROR */
  filerec_t  *file_rec = NULL; /* file record */
  atom_t      ddid;            /* ID for the DD */
  intn        ret_value = SUCCEED;

  /* clear error stack and check validity of file record id */
  HEclear();

  file_rec = HAatom_object(file_id);
  if (BADFREC(file_rec) || tag == DFTAG_WILDCARD || ref == DFREF_WILDCARD)
    HGOTO_ERROR(DFE_ARGS, FAIL);

  /* look for the dd to reuse */
  if ((ddid = HTPselect(file_rec, tag, ref)) == FAIL)
    HGOTO_ERROR(DFE_NOMATCH, FAIL);

  /* could reuse space in file by calling HPfreediskblock() routine 
     but it does nothing for now. For later. */
  /* if (HPfreediskblock(file_rec,dd_ptr->offset,dd_ptr->length) == FAIL)
      HGOTO_ERROR(DFE_INTERNAL, FAIL); */

  /* reuse the dd by setting the offset and length to
     INVALID_OFFSET and INVALID_LENGTH*/
  if (HTPupdate(ddid,INVALID_OFFSET, INVALID_LENGTH) == FAIL)
    HGOTO_ERROR(DFE_INTERNAL, FAIL);

 /* We leave the ref # as 'used' in the tag tree and
    dont' delete from dynarray of refs. */

  /* Remove DD from atom group since it should get re-created in Hstartaccess().
     This could be handled better if Hstartaccess() was revamped
     to not create new access records for existing tag/ref pairs
     as well as revamping a few other routines. */
  if(HAremove_atom(ddid)==NULL)
      HGOTO_ERROR(DFE_INTERNAL, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}	/* end HDreuse_tagref */

/*--------------------------------------------------------------------------
NAME
   Hdeldd -- delete a data descriptor
USAGE
   intn Hdeldd(file_id, tag, ref)
   int32 file_id;            IN: id of file
   int16 tag;                IN: tag of data descriptor to delete
   int16 ref;                IN: ref of data descriptor to delete
RETURNS
   returns SUCCEED (0) if successful, FAIL (-1) otherwise
DESCRIPTION
   Deletes a data descriptor of tag/ref from the dd list of the file.
   This routine is unsafe and may leave a file in a condition that is
   not usable by some routines.  Use with care.
   For example, if this element is contained in a Vgroup, that group
   will *NOT* get updated to reflect that this element has been deleted.

--------------------------------------------------------------------------*/
intn Hdeldd(int32 file_id, uint16 tag, uint16 ref)
{
  CONSTR(FUNC, "Hdeldd");		/* for HERROR */
  filerec_t  *file_rec;		/* file record */
  atom_t      ddid;         /* ID for the DD */
  intn        ret_value = SUCCEED;

  /* clear error stack and check validity of file record id */
  HEclear();
  file_rec = HAatom_object(file_id);
  if (BADFREC(file_rec) || tag == DFTAG_WILDCARD || ref == DFREF_WILDCARD)
    HGOTO_ERROR(DFE_ARGS, FAIL);

  /* look for the dd to delete */
  if ((ddid=HTPselect(file_rec, tag, ref)) == FAIL)
    HGOTO_ERROR(DFE_NOMATCH, FAIL);

  /* delete the dd */
  if (HTPdelete(ddid) == FAIL)
    HGOTO_ERROR(DFE_CANTDELDD, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}	/* end Hdeldd */

#ifdef DEBUGGING
/*--------------------------------------------------------------------------
 NAME
    HTPdump_dds -- Dump out the dd information for a file
 USAGE
    intn HTPdump_dds(file_id)
        int32 file_id;              IN: file ID of HDF file to dump
        FILE *fout;                 IN: file stream to output to
 RETURNS
    returns SUCCEED (0) if successful and FAIL (-1) if failed.
 DESCRIPTION
    Prints out all the information (that you could _ever_ want to know) about
    the dd blocks and dd list for a file.

--------------------------------------------------------------------------*/
intn HTPdump_dds(int32 file_id, FILE *fout)
{
  CONSTR(FUNC, "HTPdump_dds");
  filerec_t  *file_rec;		/* file record */
  int         ret_value = SUCCEED;

  /* clear error stack and check validity of file record id */
  HEclear();
  file_rec = HAatom_object(file_id);
  if (BADFREC(file_rec))
    HGOTO_ERROR(DFE_ARGS, FAIL);

/* Print out each DD block */
    {
        ddblock_t   *block=file_rec->ddhead;    /* dd block currently dumping */
        intn blk_count=0;       /* Count of the number of blocks we've dumped */
        intn i;                 /* local counting variable */

        while(block!=NULL)
          {
            dd_t *curr_dd;      /* current dd to dump */

            fprintf(fout,"DD block %d\n",blk_count);
            fprintf(fout,"# of DDs: %d, next block offset=%ld\n",(int)block->ndds,(long)block->nextoffset);
            fprintf(fout,"DD block offset: %ld, dirty?=%d\n",(long)block->myoffset,(int)block->dirty);
            for(i=0, curr_dd=block->ddlist; i<block->ndds; i++,curr_dd++)
                fprintf(fout,"%5d: tag/ref=(%5u/%5u), offset=%7ld, length=%7ld\n",(int)i,(unsigned)curr_dd->tag,(unsigned)curr_dd->ref,(long)curr_dd->offset,(long)curr_dd->length);
            blk_count++;
            block=block->next;
          } /* end while */
    } /* End of ddblock dumping code */

/* Dump the tag tree */
    {
        VOIDP      *t;

        if (NULL != (t = (VOIDP *) tbbtfirst((TBBT_NODE *) * (file_rec->tag_tree))))
          {   /* found at least one node in the tree */
            tag_info *tinfo_ptr;  /* pointer to the info for a tag */

            do
              { /* dump each node */
                intn size;      /* # of elements in the array */
                intn i;         /* local counting variable */

                tinfo_ptr = (tag_info *) * t;   /* get actual pointer to the tag info */
                fprintf(fout,"Tag: %u\n",tinfo_ptr->tag);
                
                /* Dump the ref # dynarray */
                if((size=DAsize_array(tinfo_ptr->d))!=FAIL)
                  {
                    VOIDP elem;

                    fprintf(fout,"dynarray size=%d\n",size);
                    for(i=0; i<size; i++)
                      {
                        elem=DAget_elem(tinfo_ptr->d,i);
                        if(elem!=NULL)
                            fprintf(fout,"dynarray[%d]=%p\n",i,elem);
                      } /* end for */
                  } /* end if */
                
                /* Dump the ref # bit-vector */
                if((size=bv_size(tinfo_ptr->b))!=FAIL)
                  {
                    intn bit;

                    fprintf(fout,"bitvector size=%d\n",size);
                    fprintf(fout,"bits set:");
                    for(i=0; i<size; i++)
                      {
                        bit=bv_get(tinfo_ptr->b,i);
                        if(bit!=BV_FALSE)
                            fprintf(fout,"%5d",i);
                      } /* end for */
                    fprintf(fout,"\n");
                  } /* end if */

                /* Get the next tag node */
                t = (VOIDP *) tbbtnext((TBBT_NODE *) t);
              } while(t!=NULL);
          }
        else
            fprintf(fout,"No nodes in tag tree\n");
    } /* End of tag node dumping */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}	/* HTPdump_dds */
#endif /* DEBUGGING */



/* Private, static, internal routines.  Do not call from outside this module */

/*--------------------------------------------------------------------------
 NAME
    HTInew_dd_block -- create a new (empty) DD block
 USAGE
    intn HTInew_dd_block(file_rec)

    filerec_t  * file_rec;        IN: file record
 RETURNS
    returns SUCCEED (0) if successful and FAIL (-1) if failed.
 DESCRIPTION
    Create a new DDblock in the file.  Update the previously last DDblock so
    that its offset points to the newly created one.

--------------------------------------------------------------------------*/
static intn HTInew_dd_block(filerec_t * file_rec)
{
    CONSTR(FUNC, "HTInew_dd_block");    /* for HERROR */
    int32       nextoffset;		/* offset of new ddblock */
    uint8       ddhead[NDDS_SZ+OFFSET_SZ];   /* storage for the DD header */
    int32       offset;			/* offset to the offset of new ddblock */
    ddblock_t *block;           /* Block the DD is located in */
    dd_t       *list;			/* dd list array of new dd block */
    uint8      *p;              /* Temporary buffer pointer. */
    intn        ndds;                   /* number of ndds in new DD block */
    intn        ret_value = SUCCEED;

    HEclear();
    /* check integrity of file record */
    if (file_rec->ddhead==NULL || file_rec->ddlast==NULL)
      HGOTO_ERROR(DFE_INTERNAL, FAIL);
  
    /* allocate new dd block record and fill in data */
    if ((block = (ddblock_t *) HDmalloc(sizeof(ddblock_t))) == NULL)
      HGOTO_ERROR(DFE_NOSPACE, FAIL);
    block->ndds = (int16)(ndds = (intn)file_rec->ddhead->ndds);    /* snarf from first block */
    block->next = (ddblock_t *) NULL;
    block->nextoffset = 0;

    /* Keep the filerec_t pointer around for each ddblock */
    block->frec=file_rec;
  
    /* get room for the new DD block in the file */
    if ((nextoffset = HPgetdiskblock(file_rec, NDDS_SZ + OFFSET_SZ + (ndds * DD_SZ), TRUE)) == FAIL)
      HGOTO_ERROR(DFE_SEEKERROR, FAIL);
    block->myoffset = nextoffset;	/* set offset of new block */
    block->dirty = (uintn)file_rec->cache; /* if we're caching, wait to write DD block */
  
    if (file_rec->cache)	/* if we are caching, wait to update previous DD block */
      file_rec->dirty |= DDLIST_DIRTY;	/* indicate file needs to be flushed */
    else
      {
        p = ddhead;
        INT16ENCODE(p, block->ndds);
        INT32ENCODE(p, (int32) 0);
        if (HP_write(file_rec, ddhead, NDDS_SZ + OFFSET_SZ) == FAIL)
          HGOTO_ERROR(DFE_WRITEERROR, FAIL);
      }		/* end else */
  
    /* set up the dd list of this dd block and put it in the file
     after the dd block header */
    list = block->ddlist = (dd_t *) HDmalloc((uint32) ndds * sizeof(dd_t));
    if (list == (dd_t *) NULL)
      HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* Fill the block with NIL tags */
    list[0].tag = DFTAG_NULL;
    list[0].ref = DFREF_NONE;
    list[0].length = INVALID_LENGTH;
    list[0].offset = INVALID_OFFSET;
    list[0].blk = block;
    HDmemfill(&list[1],&list[0],sizeof(dd_t),(uint32)ndds-1);
  
    if (file_rec->cache!=0)
      {	/* if we are caching, wait to update previous DD block */
        uint8 *tbuf;    /* temporary buffer */

        tbuf=(uint8 *)HDmalloc(ndds*DD_SZ);
        if(tbuf==(uint8 *)NULL)
            HGOTO_ERROR(DFE_NOSPACE, FAIL);

        p = tbuf;
	DDENCODE(p, (uint16) DFTAG_NULL, (uint16) DFREF_NONE,
	    (int32) INVALID_LENGTH, (int32) INVALID_OFFSET);
        HDmemfill(p,tbuf,DD_SZ,(uint32)(ndds-1));

        if (HP_write(file_rec, tbuf, ndds * DD_SZ) == FAIL)
          HGOTO_ERROR(DFE_WRITEERROR, FAIL);

        HDfree(tbuf);
      }		/* end if */
  
    /* update previously last ddblock to point to this new dd block */
    file_rec->ddlast->nextoffset = nextoffset;
    block->prev = file_rec->ddlast;
    file_rec->ddlast->next = block;
    if (file_rec->cache)
      {	/* if we are caching, wait to update previous DD block */
        file_rec->dirty |= DDLIST_DIRTY;	/* indicate file needs to be flushed */
        file_rec->ddlast->dirty = TRUE;	/* indicate this block needs to be flushed */
      }	/* end if */
    else
      {
        if (file_rec->ddhead == file_rec->ddlast)
          offset = MAGICLEN + NDDS_SZ;
        else
          offset = file_rec->ddlast->prev->nextoffset + NDDS_SZ;
        p = ddhead;
        INT32ENCODE(p, nextoffset);
        if (HPseek(file_rec, offset) == FAIL)
          HGOTO_ERROR(DFE_SEEKERROR, FAIL);
        if (HP_write(file_rec, ddhead, OFFSET_SZ) == FAIL)
          HGOTO_ERROR(DFE_WRITEERROR, FAIL);
      }	/* end else */
  
    /* update file record */
    file_rec->ddlast = block;
  
    /* set the end of the file to the end of the current DD block */
    file_rec->f_end_off = block->myoffset + (NDDS_SZ + OFFSET_SZ) + (block->ndds * DD_SZ);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}	/* HTInew_dd_block */

/*--------------------------------------------------------------------------
 NAME
    HTIfind_dd -- find a specific DD in the file
 USAGE
    int HTIfind_dd(file_rec, tag, ref, dd_ptr, direction)
        filerec_t *  file_rec;       IN:  file record to search
        uint16       tag;            IN:  tag of element to find
        uint16       ref;            IN:  ref of element to find
        dd_t      ** pdd;            OUT: pointer to the DD in memory
        intn         direction;      IN:  direction to search
                                        (DF_FORWARD / DF_BACKWARD)
 RETURNS
    returns SUCCEED (0) if successful and FAIL (-1) if failed.
 DESCRIPTION
    Find the dd with tag and ref, by returning the block where the dd resides
    and the index of the dd in the ddblock ddlist.

--------------------------------------------------------------------------*/
static intn HTIfind_dd(filerec_t * file_rec, uint16 look_tag, uint16 look_ref,
            dd_t ** pdd, intn direction)
{
#ifdef LATER
    CONSTR(FUNC, "HTIfind_dd");    /* for HERROR */
#endif /* LATER */
    intn idx;          /* index into ddlist of current dd searched */
    ddblock_t *block;  /* ptr to current ddblock searched */
    dd_t *list;        /* ptr to current ddlist searched */
    uint16      special_tag;    /* corresponding special tag */
    intn        ret_value = SUCCEED;

    HEclear();
    /* Create the special version of the tag to search for also */
    special_tag = MKSPECIALTAG(look_tag);

    if(look_tag!=DFTAG_WILDCARD && look_ref!=DFTAG_WILDCARD)
      { /* easy to optimize case, looking for a specific tag/ref pair */
          tag_info **tip_ptr;   /* ptr to the ptr to the info for a tag */
          tag_info *tinfo_ptr;  /* pointer to the info for a tag */
          dd_t *dd_ptr;         /* ptr to the DD info for a tag/ref */
          uint16 base_tag=BASETAG(look_tag);    /* corresponding base tag (if the tag is special) */

          /* Try to find the regular tag in the tag info tree */
          if((tip_ptr=(tag_info **)tbbtdfind(file_rec->tag_tree,(VOIDP)&base_tag,NULL))==NULL)
              HGOTO_DONE(FAIL); /* Not an error, we just didn't find the object */

          tinfo_ptr=*tip_ptr; /* get the pointer to the tag info */
          if((dd_ptr=DAget_elem(tinfo_ptr->d,(intn)look_ref))==NULL)
              HGOTO_DONE(FAIL); /* Not an error, we just didn't find the object */

          *pdd=dd_ptr;
          HGOTO_DONE(SUCCEED);
      } /* end if */
    else
      { /* handle wildcards, etc. */
        if (direction == DF_FORWARD)
          {   /* search forward through the DD list */
            if(*pdd==NULL)
              {
                block=file_rec->ddhead;
                idx=0;
              } /* end if */
            else
              {
                block=(*pdd)->blk;
                idx=(intn)(((*pdd)-&block->ddlist[0])+1);
              } /* end else */
            if(look_tag==DFTAG_WILDCARD && look_ref==DFREF_WILDCARD)
              { /* Both tag & ref are wildcards */
                for (; block; block = block->next)
                  {
                    list = &block->ddlist[idx];
                    for (; idx < block->ndds; idx++, list++)
                      {
                        /* skip the empty dd's */
                        if (list->tag == DFTAG_NULL)
                            continue;
      
                        /* we have a match !! (anything matches! :-) */
                        *pdd=list;
                        HGOTO_DONE(SUCCEED);
                      }	/* end for */
        
                    /* start from beginning of the next dd list */
                    idx = 0;
                  }	/* end for */
              } /* end if */
            else if(look_tag==DFTAG_NULL && look_ref==DFTAG_WILDCARD)
              { /* special case for quick lookup of empty DD's */
                if(file_rec->ddnull==NULL)
                    block=file_rec->ddhead;
                else
                    block=file_rec->ddnull;
                if(file_rec->ddnull_idx<0)
                    idx=0;
                else
                    idx=file_rec->ddnull_idx+1;

                for (; block; block = block->next)
                  {
                    list = &block->ddlist[idx];
                    for (; idx < block->ndds; idx++, list++)
                      {
                        /* skip the empty dd's */
                        if (list->tag == DFTAG_NULL)
                          {
                            /* we have a match !! */
                            *pdd=list;

                            /* Update the DFTAG_NULL pointers */
                            file_rec->ddnull=block;
                            file_rec->ddnull_idx=idx;

                            HGOTO_DONE(SUCCEED);
                          } /* end if */
                      }	/* end for */
        
                    /* start from beginning of the next dd list */
                    idx = 0;
                  }	/* end for */
              } /* end if */
            else if(look_tag==DFTAG_WILDCARD)
              { /* tag is wildcard */
                for (; block; block = block->next)
                  {
                    list = &block->ddlist[idx];
                    for (; idx < block->ndds; idx++, list++)
                      {
                        /* skip the empty dd's */
                        if (list->tag == DFTAG_NULL)
                            continue;
      
                        if (list->ref == look_ref)
                          {
                            /* we have a match !! */
                            *pdd=list;
                            HGOTO_DONE(SUCCEED);
                          } /* end if */
                      }	/* end for */
        
                    /* start from beginning of the next dd list */
                    idx = 0;
                  }	/* end for */
              } /* end if */
            else if(look_ref==DFREF_WILDCARD)
              { /* ref is wildcard */
#ifndef OLD_WAY
                if(special_tag==DFTAG_NULL)
                  {
/* Change this to lookup the next used ref # in the bitvector or dynarray -QAK */
                    for (; block; block = block->next)
                      {
                        list = &block->ddlist[idx];
                        for (; idx < block->ndds; idx++, list++)
                          {
                            /* skip the empty dd's */
                            if (list->tag == DFTAG_NULL && look_tag != DFTAG_NULL)
                                continue;
          
                            if (list->tag == look_tag)
                              {
                                /* we have a match !! */
                                *pdd=list;
                                HGOTO_DONE(SUCCEED);
                              } /* end if */
                          }	/* end for */
            
                        /* start from beginning of the next dd list */
                        idx = 0;
                      }	/* end for */
                  } /* end if */
                else
                  {
                    for (; block; block = block->next)
                      {
                        list = &block->ddlist[idx];
                        for (; idx < block->ndds; idx++, list++)
                          {
                            /* skip the empty dd's */
                            if (list->tag == DFTAG_NULL && look_tag != DFTAG_NULL)
                                continue;
          
                            if (list->tag == look_tag || list->tag == special_tag)
                              {
                                /* we have a match !! */
                                *pdd=list;
                                HGOTO_DONE(SUCCEED);
                              } /* end if */
                          }	/* end for */
            
                        /* start from beginning of the next dd list */
                        idx = 0;
                      }	/* end for */
                  } /* end else */
#else /* OLD_WAY */
/* Hmm, not working yet?... -QAK */
                tag_info **tip_ptr;   /* ptr to the ptr to the info for a tag */
                tag_info *tinfo_ptr;  /* pointer to the info for a tag */
                dd_t *dd_ptr;         /* ptr to the DD info for a tag/ref */
                uint16 base_tag=BASETAG(look_tag);    /* corresponding base tag (if the tag is special) */
                int32 last_ref;       /* the last ref # found */
                uint16 found_ref;     /* next ref # found */

                /* Try to find the regular tag in the tag info tree */
                if((tip_ptr=(tag_info **)tbbtdfind(file_rec->tag_tree,(VOIDP)&base_tag,NULL))==NULL)
                    HGOTO_ERROR(DFE_BADTAG, FAIL);

                tinfo_ptr=*tip_ptr; /* get the pointer to the tag info */
                if(*pdd==NULL) /* check if we are searching from the beginning */
                    last_ref=-1;
                else
                    last_ref=block->ddlist[idx].ref;
                if((found_ref=bv_find(tinfo_ptr->b,last_ref,1))==(uint16)FAIL)
                  HGOTO_ERROR(DFE_BVFIND, FAIL);

                if((dd_ptr=DAget_elem(tinfo_ptr->d,found_ref))==NULL)
                  HGOTO_ERROR(DFE_BADREF, FAIL);

                *pdd=dd_ptr;
                HGOTO_DONE(SUCCEED);
#endif /* OLD_WAY */
              } /* end if */
            else
              { /* Both tag & ref are not wildcards */
                for (; block; block = block->next)
                  {
                    list = &block->ddlist[idx];
                    for (; idx < block->ndds; idx++, list++)
                      {
                        /* skip the empty dd's */
                        if (list->tag == DFTAG_NULL && look_tag != DFTAG_NULL)
                            continue;
      
                        if ((list->tag == look_tag
                            || (special_tag != DFTAG_NULL && list->tag == special_tag))
                            && list->ref == look_ref)
                        {
                            /* we have a match !! */
                            *pdd=list;
                            HGOTO_DONE(SUCCEED);
                        }	/* end if */
                      }	/* end for */
        
                    /* start from beginning of the next dd list */
                    idx = 0;
                  }	/* end for */
              } /* end else */
          }		/* end if */
        else if (direction == DF_BACKWARD)
          {	  /* search backward through the DD list */
            if(*pdd==NULL)
              {
                block=file_rec->ddlast;
                idx=block->ndds-1;
              } /* end if */
            else
              {
                block=(*pdd)->blk;
                idx=(intn)(((*pdd)-&block->ddlist[0])-1);
              } /* end else */
            for (; block;)
              {
                list = block->ddlist;
                for (; idx >= 0; idx--)
                  {
                      /* skip the empty dd's */
                    if (list[idx].tag == DFTAG_NULL && look_tag != DFTAG_NULL)
                      continue;
    
                    if (((look_tag == DFTAG_WILDCARD || list[idx].tag == look_tag)
                       || (special_tag != DFTAG_NULL && list[idx].tag == special_tag))
                       && (look_ref == DFREF_WILDCARD || list[idx].ref == look_ref))
                      {
    
                        /* we have a match !! */
                        *pdd=&list[idx];
                        HGOTO_DONE(SUCCEED);
                      }	/* end if */
                }	/* end for */

              /* start from beginning of the next dd list */
              block = block->prev;
              if (block != NULL)
                  idx = block->ndds - 1;
              }	/* end for */
          }		/* end if */
      } /* end else */

    /* If we get here, we've failed */
    ret_value=FAIL;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}	/* HTIfind_dd */

/*--------------------------------------------------------------------------
 NAME
    HTIupdate_dd -- update a DD on disk
 USAGE
    int HTIupdate_dd(file_rec, dd_ptr)
        filerec_t *file_rec;    IN: id of file
        dd_t      *dd_ptr;      IN: pointer to dd to update
 RETURNS
    returns SUCCEED (0) if successful and FAIL (-1) if failed.
 DESCRIPTION
   Takes appropriate action to note that a DD in a DD block has changed

--------------------------------------------------------------------------*/
static intn HTIupdate_dd(filerec_t * file_rec, dd_t * dd_ptr)
{
    CONSTR(FUNC, "HTIupdate_dd");   /* for HERROR */
    ddblock_t  *block;              /* DD block the dd is in */
    int32       idx;                /* index of the DD in the DD block */
    intn        ret_value = SUCCEED;

    HEclear();
    block=dd_ptr->blk;
    idx=(int32)(dd_ptr-&block->ddlist[0]);
    if (file_rec->cache)
      {		/* if caching is on, postpone update until later */
        file_rec->dirty |= DDLIST_DIRTY;
        block->dirty = TRUE;
      } /* end if */
    else
      {
        int32       offset;         /* offset of updated dd in file */
        uint8       tbuf[DD_SZ];    /* storage for the DD */
        uint8      *p;              /* temp buffer ptr */

        /* look for offset of updated dd block in the file */
        offset = block->myoffset + (NDDS_SZ + OFFSET_SZ) + (idx * DD_SZ);

        /* write in the updated dd */
        if (HPseek(file_rec, offset) == FAIL)
          HGOTO_ERROR(DFE_SEEKERROR, FAIL);
  
        p = tbuf;
	DDENCODE(p, dd_ptr->tag, dd_ptr->ref, dd_ptr->offset, dd_ptr->length);
        if (HP_write(file_rec, tbuf, DD_SZ) == FAIL)
          HGOTO_ERROR(DFE_WRITEERROR, FAIL);
      } /* end else */
  
    /* check whether to incr. offset of end of file */
    /* not certain whether this is actually necessary, but better safe than */
    /* sorry later... -QAK */
    if ((dd_ptr->offset != INVALID_OFFSET && dd_ptr->length != INVALID_LENGTH) &&
        ( dd_ptr->offset +  dd_ptr->length) > file_rec->f_end_off)
      file_rec->f_end_off = dd_ptr->offset + dd_ptr->length;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}	/* HTIupdate_dd */

/* ----------------------------- HTIcount_dd ------------------------------ */
/*
NAME
   HTIcount_dd -- counts the dd's of a certain type in file
USAGE
   intn HTIcount_dd(file_rec, tag, ref, all_cnt, real_cnt)
   filerec_t *  file_rec;       IN:  file record to search
   uint16       tag;            IN:  tag of element to find
                                     (can be DFTAG_WILDCARD)
   uint16       ref;            IN:  ref of element to find
                                     (can be DFREF_WILDCARD)
   uintn       *all_cnt;        OUT: Count of all the tag/ref pairs
                                     found, including DFTAG_NULL and
                                     DFTAG_FREE
   uintn       *real_cnt;       OUT: Count of all the tag/ref pairs
                                     found, excluding DFTAG_NULL and 
                                     DFTAG_FREE
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Counts the number of tag/ref pairs in a file.

   This routine keeps track of and returns to the user the number
   of all tag/refs and the number of "real" tag/refs found.
   "Real" tag/refs are any except DFTAG_NULL & DFTAG_FREE.

   This routine always counts the total tag/refs in the file, no
   provision is made for partial searches.

---------------------------------------------------------------------------*/
static intn HTIcount_dd(filerec_t * file_rec, uint16 cnt_tag, uint16 cnt_ref,
           uintn *all_cnt, uintn *real_cnt)
{
    uintn       t_all_cnt = 0;  /* count of all tag/refs found */
    uintn       t_real_cnt = 0; /* count of all tag/refs except NULL & FREE */
    intn        idx;            /* index into ddlist of current dd searched */
    ddblock_t  *block;          /* ptr to current ddblock searched */
    dd_t       *dd_ptr;         /* ptr to current ddlist searched */
    uint16      special_tag;    /* corresponding special tag */

    HEclear();
    /* search for special version also */
    special_tag = MKSPECIALTAG(cnt_tag);

/* Change these algorithms to take advantage of the dynamic arrays for tags -QAK */
    switch(cnt_tag)
      {
          case DFTAG_WILDCARD:
              for (block = file_rec->ddhead; block != NULL; block = block->next)
                {
                    t_all_cnt += (uintn)block->ndds;

                    dd_ptr = block->ddlist;
                    for (idx = 0; idx < block->ndds; idx++, dd_ptr++)
                      {
                          /* skip the empty dd's */
                          if (dd_ptr->tag == DFTAG_NULL || dd_ptr->tag == DFTAG_FREE)
                              continue;

                          if ((cnt_ref == DFREF_WILDCARD || dd_ptr->ref == cnt_ref))
                                t_real_cnt++;
                  }	/* end for */
                }		/* end for */
              break;

          case DFTAG_NULL:
          case DFTAG_FREE:
              for (block = file_rec->ddhead; block != NULL; block = block->next)
                {
                    t_all_cnt += (uintn)block->ndds;

                    dd_ptr = block->ddlist;
                    for (idx = 0; idx < block->ndds; idx++, dd_ptr++)
                          if ((dd_ptr->tag == cnt_tag
                            || (special_tag != DFTAG_NULL && dd_ptr->tag == special_tag))
                           && (cnt_ref == DFREF_WILDCARD || dd_ptr->ref == cnt_ref))
                                t_real_cnt++;
                }		/* end for */
              break;

          default:
              if(special_tag==DFTAG_NULL)
                {
                  for (block = file_rec->ddhead; block != NULL; block = block->next)
                    {
                        t_all_cnt += (uintn)block->ndds;

                        dd_ptr = block->ddlist;
                        for (idx = 0; idx < block->ndds; idx++, dd_ptr++)
                            if (dd_ptr->tag == cnt_tag
                             && (dd_ptr->ref == cnt_ref || cnt_ref == DFREF_WILDCARD))
                                  t_real_cnt++;
                    }		/* end for */
                } /* end if */
              else
                {
                  if(cnt_ref==DFREF_WILDCARD)
                    {
                      for (block = file_rec->ddhead; block != NULL; block = block->next)
                        {
                            t_all_cnt += (uintn)block->ndds;

                            idx=0;
                            dd_ptr = block->ddlist;
                            if(block->ndds%2 == 1)
                                if (dd_ptr->tag == cnt_tag || dd_ptr->tag == special_tag)
                                  {
                                    t_real_cnt++;
                                    idx++;
                                    dd_ptr++;
                                  } /* end if */
                            for (; idx < block->ndds; idx++, dd_ptr++)
                              {
                                  if (dd_ptr->tag == cnt_tag || dd_ptr->tag == special_tag)
                                      t_real_cnt++;
                                  idx++;
                                  dd_ptr++;
                                  if (dd_ptr->tag == cnt_tag || dd_ptr->tag == special_tag)
                                      t_real_cnt++;
                              } /* end for */
                        }		/* end for */
                    } /* end if */
                  else
                    {
                      for (block = file_rec->ddhead; block != NULL; block = block->next)
                        {
                            t_all_cnt += (uintn)block->ndds;

                            dd_ptr = block->ddlist;
                            for (idx = 0; idx < block->ndds; idx++, dd_ptr++)
                                if ((dd_ptr->tag == cnt_tag || dd_ptr->tag == special_tag)
                                   && dd_ptr->ref == cnt_ref)
                                      t_real_cnt++;
                        }		/* end for */
                    } /* end else */
                } /* end else */
              break;
      } /* end switch */

    *all_cnt = t_all_cnt;
    *real_cnt = t_real_cnt;
    return (SUCCEED);
}	/* HTIcount_dd */

/*--------------------------------------------------------------------------
 NAME
    HTIregister_tag_ref -- mark a ref # as used for a tag
 USAGE
    intn HTIregister_tag_ref(file_rec, dd_ptr)
        filerec_t  * file_rec;        IN: file record
        dd_t  * dd_ptr;               IN: pointer to the dd the tag/ref is in
 RETURNS
    returns SUCCEED (0) if successful and FAIL (-1) if failed.
 DESCRIPTION
    Marks a ref # as used for a given tag.

--------------------------------------------------------------------------*/
static intn
HTIregister_tag_ref(filerec_t * file_rec, dd_t *dd_ptr)
{
  CONSTR(FUNC, "HTIregister_tag_ref");
  tag_info *tinfo_ptr;  /* pointer to the info for a tag */
  tag_info **tip_ptr;   /* ptr to the ptr to the info for a tag */
  uint16 base_tag=BASETAG(dd_ptr->tag);      /* the base tag for the tag tree */
  int         ret_value = SUCCEED;

  HEclear();
  /* Add to the tag info tree */
  if((tip_ptr=(tag_info **)tbbtdfind(file_rec->tag_tree,(VOIDP)&base_tag,NULL))==NULL)
    {   /* a new tag was found */
      if((tinfo_ptr=(tag_info *)HDcalloc(1,sizeof(tag_info)))==NULL)
          HGOTO_ERROR(DFE_NOSPACE, FAIL);
      tinfo_ptr->tag=base_tag;

      /* Insert the tag node into the tree */
      tbbtdins(file_rec->tag_tree, (VOIDP) tinfo_ptr, NULL);

      /* Take care of the bit-vector */
      if((tinfo_ptr->b=bv_new(-1,BV_EXTENDABLE))==NULL)
          HGOTO_ERROR(DFE_BVNEW, FAIL);
      /* Set the 0'th bit in the bit-vector (cannot be stored in HDF files) */
      /* Yes, this is a kludge due to ref # zero not being used -QAK */
      if(bv_set(tinfo_ptr->b,0,BV_TRUE)==FAIL)
          HGOTO_ERROR(DFE_BVSET, FAIL);

      /* Take care of the dynarray */
      if((tinfo_ptr->d=DAcreate_array(REF_DYNARRAY_START,REF_DYNARRAY_INCR))==NULL)
          HGOTO_ERROR(DFE_INTERNAL, FAIL);
    } /* end if */
  else
    {   /* found an existing tag */
        intn ref_bit;    /* bit of the ref # in the tag info */

        tinfo_ptr=*tip_ptr; /* get the pointer to the tag info */
        if((ref_bit=bv_get(tinfo_ptr->b,(intn)dd_ptr->ref))==FAIL)
            HGOTO_ERROR(DFE_BVGET, FAIL);
        if(ref_bit==BV_TRUE)
            HGOTO_ERROR(DFE_DUPDD, FAIL);
    } /* end else */

  /* Set the bit in the bit-vector */
  if(bv_set(tinfo_ptr->b,(intn)dd_ptr->ref,BV_TRUE)==FAIL)
      HGOTO_ERROR(DFE_BVSET, FAIL);

  /* Insert the DD info into the dynarray for later use */
  if(DAset_elem(tinfo_ptr->d,(intn)dd_ptr->ref,(VOIDP)dd_ptr)==FAIL)
      HGOTO_ERROR(DFE_INTERNAL, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}	/* HTIregister_tag_ref */

/*--------------------------------------------------------------------------
 NAME
    HTIunregister_tag_ref -- mark a ref # as free for a tag
 USAGE
    intn HTIunregister_tag_ref(file_rec, tag, ref)
        filerec_t  * file_rec;        IN: file record
        dd_t  *dd_ptr;                IN: DD of the tag/ref to unregister
 RETURNS
    returns SUCCEED (0) if successful and FAIL (-1) if failed.
 DESCRIPTION
    Marks a ref # as free for a given tag.

--------------------------------------------------------------------------*/
static intn HTIunregister_tag_ref(filerec_t * file_rec, dd_t *dd_ptr)
{
  CONSTR(FUNC, "HTIunregister_tag_ref");
  tag_info *tinfo_ptr;  /* pointer to the info for a tag */
  tag_info **tip_ptr;   /* ptr to the ptr to the info for a tag */
  uint16 base_tag=BASETAG(dd_ptr->tag);      /* the base tag for the tag tree */
  int         ret_value = SUCCEED;

  HEclear();
  /* Add to the tag info tree */
  if((tip_ptr=(tag_info **)tbbtdfind(file_rec->tag_tree,(VOIDP)&base_tag,NULL))==NULL)
    {
      HGOTO_ERROR(DFE_BADTAG, FAIL);
    } /* end if */
  else
    {   /* found an existing tag */
        intn ref_bit;    /* bit of the ref # in the tag info */

        tinfo_ptr=*tip_ptr; /* get the pointer to the tag info */
        if((ref_bit=bv_get(tinfo_ptr->b,(intn)dd_ptr->ref))==FAIL)
            HGOTO_ERROR(DFE_BVGET, FAIL);
        if(ref_bit==BV_FALSE)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);
        if(bv_set(tinfo_ptr->b,(intn)dd_ptr->ref,BV_FALSE)==FAIL)
            HGOTO_ERROR(DFE_BVSET, FAIL);

        /* Delete the DD info from the tag tree */
        if(DAdel_elem(tinfo_ptr->d,(intn)dd_ptr->ref)==NULL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);

        /* Delete the tag/ref from the file */
        dd_ptr->tag=DFTAG_NULL;
    } /* end else */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}	/* HTIunregister_tag_ref */

/* ---------------------------- tagcompare ------------------------- */
/*
   Compares two tag B-tree keys for equality.  Similar to memcmp.

   *** Only called by B-tree routines, should _not_ be called externally ***
 */
intn
tagcompare(VOIDP k1, VOIDP k2, intn cmparg)
{
  intn  ret_value;
  /* shut compiler up */
  cmparg = cmparg;

  ret_value = ((intn) ((*(uint16 *) k1) - (*(uint16 *) k2)));    /* valid for integer keys */

  return ret_value;
}   /* tagcompare */

/* ---------------------------- tagdestroynode ------------------------- */
/*
   Frees tag B-Tree nodes

   *** Only called by B-tree routines, should _not_ be called externally ***
 */
VOID
tagdestroynode(VOIDP n)
{
    tag_info *t=(tag_info *)n;

    if(t->b!=NULL)
        bv_delete(t->b);
    if(t->d!=NULL)
        DAdestroy_array(t->d,0);
    HDfree((VOIDP) n);
}   /* tagdestroynode */

