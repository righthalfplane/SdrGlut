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

/* $Id: hblocks.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*LINTLIBRARY */
/* ------------------------------ hblocks.c -------------------------------
   routines to implement linked-block elements

   Linked element in HDF files created in two ways
   -- created from the start or
   -- converted from a normal data element

   A linked-block element is a special element.

   Special elements are
   flagged with a set high-bit in their tag.  Thus, a tag t has
   BASETAG == t & 0x7f and is a special tag if t & 0x80 != 0x00

   The first 16 bits of the meta-element that this tag/ref points to
   tells us what type of special element this is.  If these 16 bits is
   SPECIAL_LINKED, then it contains information about the linked blocks.
   After this 16 bits, 32 bit which is the length of each block, after
   which is the information header:

   ----------------------------------------------------------------------
   | # blocks in | tag/ref of | tag/ref of blocks list .......          |
   | this header | next header|                                         |
   ----------------------------------------------------------------------

   File Description of Linked Block Element
   ****************************************
   DD for Linked Block pointing to Linked Block Description Record
   ==============================================================
   <-  2 bytes -> <- 2 bytes -> <- 4 bytes -> <- 4bytes ->
   --------------------------------------------------------
   |extended tag | reference # |  Offset     |  Length    |
   --------------------------------------------------------
                                    \______________/
   __________________________________________|
   V
   LINKED BLOCK DESCRIPTION RECORD(LBDR - 22 bytes)
   ===============================================
   <-  4 bytes -> <- 4 bytes  -> <-   4 bytes  -> <- 4bytes ->
   --------------------------------------------------------------
   |ext_tag_desc | elem_tot_len | blk_first_len  | blk_length |   ... cont'd
   --------------------------------------------------------------
    
   <- 4 bytes -> <- 2 bytes ->
   --------------------------
...  num_blk   | link_ref   |
   --------------------------

   ext_tag_desc   - SPECIAL_LINKED(16 bit constant), identifies this as
                    a linked block description record
   elem_tot_len   - Length of the entire element(32 bit field)
   blk_first_len  - Length of the first data block(32 bit field)
   blk_length     - Length of successive data blocks(32 bit field)
   num_blk        - Number of blocks per block table(32 bit field)
   link_ref       - Reference number of the first block table(16 bit field)

   Linked Block Table(12 + 2 + 2 + 2 + 2 + ... bytes)
   ===================================================
   <-  2 bytes -> <- 2 bytes -> <- 4 bytes -> <- 4bytes ->
   --------------------------------------------------------
   |link_blk_tag | link_ref    |  Offset     |  Length    |
   --------------------------------------------------------
                                    \______________/
   __________________________________________|
   V
   <-  2 bytes -> <- 2 bytes -> <- 2 bytes -> <- 2 bytes -> <-...
   -----------------------------------------------------------...
   | next_ref    | block_ref_1 | block_ref_2 | block_ref_3 |  ...
   -----------------------------------------------------------...
    
   link_blk_tag   - DFTAG_LINKED(16 bit)
   link_ref       - Reference number for this table(16 bit)
   next_ref       - Reference number for next block table(16 bit)
                    Zero(0) signifies no more block tables for this element.
   blk_ref_x      - Reference number for data block X (16 bit). 
                  e.g. for data block 1
                  <-  2 bytes ->  <- 2 bytes -> <- 4 bytes -> <- 4bytes ->
                  --------------------------------------------------------
                  | DFTAG_LINKED | block_ref_1 |  Offset     |  Length    |
                  --------------------------------------------------------
                                                    \______________/
                  __________________________________________|
                  V
                  -----------------------
                  | Data_block          |
                  -----------------------
                  Note: The "Length" here is specified by either 
                        "elem_first_len" or "blk_length".

   For now, HLcreate() has the best description of what the on-disk
   representation of a linked block element looks like.

EXPORTED ROUTINES

   HLcreate       -- create a linked block element
   HLconvert      -- convert an AID into a linked block element
   HDinqblockinfo -- return info about linked blocks
   HLPstread      -- open an access record for reading
   HLPstwrite     -- open an access record for writing
   HLPseek        -- set the seek posn
   HLPread        -- read some data out of a linked block element
   HLPwrite       -- write out some data to a linked block
   HLPinquire     -- Hinquire for linked blocks
   HLPendacess    -- close a linked block AID
   HLPinfo        -- return info about a linked block element
LOCAL ROUTINES
   HLIstaccess -- set up AID to access a linked block elem
   HLIgetlink  -- get link information
   HLInewlink  -- write out some data to a linked block
*/

#include "hdf.h"
#include "hfile.h"

/* block_t - record of a linked block. contains the tag and ref of the
   data elt that forms the linked block */
typedef struct block_t
{
    uint16      ref;          /* ref of the linked block */
}
block_t;

/* link_t - a linked list block table.
   Very similar to the dd block structure */
typedef struct link_t
{
    uint16          nextref;   /* ref of the next block table */
    struct link_t  *next;      /* ptr to the next block table */
    struct block_t *block_list;/* ptr to the block list for this table */
}
link_t;

/* information on this special linked block data elt */
typedef struct linkinfo_t
{
    int      attached;     /* how many access records refer to this elt */
    int32    length;       /* the actual length of the data elt */
    int32    first_length; /* length of first block */
    int32    block_length; /* the length of the remaining blocks */
    int32    number_blocks;/* total number of blocks in each link/block table */
    uint16   link_ref;     /* ref of the first block table structure */
    link_t  *link;         /* pointer to the first block table */
    link_t  *last_link;    /* pointer to the last block table */
}
linkinfo_t;

/* private functions */
PRIVATE int32 HLIstaccess(accrec_t *access_rec, 
                          int16     acc_mode);

PRIVATE link_t *HLInewlink(int32  file_id, 
                           int32  number_blocks, 
                           uint16 link_ref, 
                           uint16 first_block_ref);

PRIVATE link_t *HLIgetlink(int32  file_id, 
                           uint16 ref, 
                           int32  number_blocks);

/* the accessing function table for linked blocks */
funclist_t  linked_funcs =
{
    HLPstread,
    HLPstwrite,
    HLPseek,
    HLPinquire,
    HLPread,
    HLPwrite,
    HLPendaccess,
    HLPinfo,
    NULL         /* no routine registered */
};

/* ------------------------------------------------------------------------
NAME
   HLcreate -- create a linked block element
USAGE
   int32 HLcreate(fid, tag, ref, blocklen, numblocks)
   int32   fid;         IN: file to put linked block element in
   uint16  tag;         IN: tag of element
   uint16  ref;         IN: ref of element
   int32   blocklen;    IN: length of standard block
   int32   numblocks;   IN: number of blocks per block list
RETURNS
   The AID of newly created linked block element, FAIL on error.
DESCRIPTION
   This routine takes an HDF element and promotes it into a linked
   block element.  Basically, the element becomes a linked list
   allowing easy appending.  If the element already exists, it
   is promoted to being a linked block element, otherwise a new
   element is created.

   All of the pieces of the linked list are the same size (blocklen)
   except for the first one which stays the size of the element
   at the time HLcreate was called.

   numblocks gives the number of linked list objects in each
   block header.

   The ideal setting for numblocks and blocklen are very data
   and application depedent.

 --------------------------------------------------------------------------- */
int32
HLcreate(int32  file_id, 
         uint16 tag, 
         uint16 ref, 
         int32  block_length,
         int32  number_blocks)
{
    CONSTR(FUNC, "HLcreate");   /* for HERROR */
    filerec_t  *file_rec;       /* file record */
    accrec_t   *access_rec=NULL;/* access record */
    int32       dd_aid;         /* AID for writing the special info */
    linkinfo_t *info = NULL;   /* information for the linked blocks elt */
    uint16      link_ref;       /* the ref of the link structure
                                   (block table) */
    atom_t      data_id;        /* dd ID of existing regular element */
    uint16      new_data_tag, new_data_ref=0;  /* Tag/ref of the new data in the file */
    int32       data_len;		/* length of the data we are checking */
    int32       data_off;		/* offset of the data we are checking */
    uint16      special_tag;    /* special version of this tag */
    uint8       local_ptbuf[16];
    int32       ret_value = SUCCEED;

    /* clear error stack and validate file record id */
    HEclear();
    file_rec = HAatom_object(file_id);

    /* check args and create special tag */
    if (BADFREC(file_rec) || block_length < 0 || number_blocks < 0
        || SPECIALTAG(tag)
        || (special_tag = MKSPECIALTAG(tag)) == DFTAG_NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* make sure write access to file */
    if (!(file_rec->access & DFACC_WRITE))
        HGOTO_ERROR(DFE_DENIED, FAIL);

    /* get empty access record */
    access_rec = HIget_access_rec();
    if (access_rec == NULL)
        HGOTO_ERROR(DFE_TOOMANY, FAIL);

    /* search for identical dd */
    if ((data_id = HTPselect(file_rec,tag,ref))!=FAIL)
      {
          /* Check if the element is already special */
          if (HTPis_special(data_id)==TRUE)
            {
                HTPendaccess(data_id);
                HGOTO_ERROR(DFE_CANTMOD, FAIL);
            }   /* end if */

          /* If the data already was in the file, 
           * convert it into the first linked block 
           * get the info for the dataset */
          if(HTPinquire(data_id,NULL,NULL,&data_off,&data_len)==FAIL)
            {
                HTPendaccess(data_id);
                HGOTO_ERROR(DFE_INTERNAL, FAIL);
            } /* end if */

          if(data_off == INVALID_OFFSET || data_len==INVALID_LENGTH)
            { /* data object which has been created, but has no data */
              /* Delete the old data ID */
              if(HTPdelete(data_id)==FAIL)
                  HGOTO_ERROR(DFE_CANTDELHASH, FAIL);

              data_id=FAIL; /* reset this so the first block is a "regular" fixed length block */
            } /* end if */
          else
            {   /* existing data object with real data in it */
              new_data_tag = DFTAG_LINKED;
              new_data_ref = Htagnewref(file_id,new_data_tag);
              /* create new linked-block table DD to point to existing data */
              if(Hdupdd(file_id, new_data_tag, new_data_ref, tag, ref)==FAIL)
                {
                    HTPendaccess(data_id);
                    HGOTO_ERROR(DFE_CANTUPDATE, FAIL);
                } /* end if */

              /* Delete the old data ID */
              if(HTPdelete(data_id)==FAIL)
                  HGOTO_ERROR(DFE_CANTDELHASH, FAIL);

              /* Attach to the new data ID */
              if ((data_id = HTPselect(file_rec,new_data_tag,new_data_ref))==FAIL)
                  HGOTO_ERROR(DFE_INTERNAL, FAIL);
            } /* end else */
      } /* end if */

    /* get ref for next linked-block? */
    link_ref = Htagnewref(file_id,DFTAG_LINKED);

    /* allocate and fill special info struct */
    if (( info = (linkinfo_t *) HDmalloc((uint32) sizeof(linkinfo_t)))==NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    info->attached     = 1;
    info->length       = (data_id!=FAIL) ? data_len : 0;
    info->first_length = (data_id!=FAIL) ? data_len : block_length;
    info->block_length = block_length;
    info->number_blocks = number_blocks;
    info->link_ref      = link_ref;

    /* encode special information for writing to file */
    {
        uint8      *p;
        p = local_ptbuf;
        UINT16ENCODE(p, SPECIAL_LINKED);
        INT32ENCODE(p, info->length);
        INT32ENCODE(p, block_length);
        INT32ENCODE(p, number_blocks);
        UINT16ENCODE(p, link_ref);  /* link_ref */
    }

    /* write the special info structure */
    if((dd_aid = Hstartaccess(file_id,special_tag,ref,DFACC_ALL))==FAIL)
        HGOTO_ERROR(DFE_CANTACCESS, FAIL);
    if (Hwrite(dd_aid, 16, local_ptbuf) == FAIL)
        HGOTO_ERROR(DFE_WRITEERROR, FAIL);
    if(Hendaccess(dd_aid)==FAIL)
        HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

    /* write out linked block */
    info->link = HLInewlink(file_id, number_blocks, link_ref,
                            (uint16) ((data_id!=FAIL) ? new_data_ref : 0));
    if (!info->link)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Detach from the data DD ID */
    if(data_id != FAIL)
      {
        if(HTPendaccess(data_id)==FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);
      }

    /* update access record and file record */
    if((access_rec->ddid=HTPselect(file_rec,special_tag,ref))==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    access_rec->special_func = &linked_funcs;
    access_rec->special_info = (void *)info;
    access_rec->special      = SPECIAL_LINKED;
    access_rec->posn         = 0;
    access_rec->access       = DFACC_RDWR;
    access_rec->file_id      = file_id;
    access_rec->appendable   = FALSE;     /* start data as non-appendable */

    file_rec->attach++; /* increment number of elements attached to file */

    /* set return value */
    ret_value = HAregister_atom(AIDGROUP,access_rec);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */
        if (info != NULL)
            HDfree(info);
        if(access_rec!=NULL)
            HIrelease_accrec_node(access_rec);
    } /* end if */

  /* Normal function cleanup */
  return ret_value;
} /* HLcreate() */

/* ------------------------------------------------------------------------
NAME
   HLconvert -- convert an AID into a linked block element
USAGE
   intn HLconvert(aid, blocklen, numblocks)
   int32   aid;         IN: AID to convert
   int32   blocklen;    IN: length of standard block
   int32   numblocks;   IN: number of blocks per block list
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   This routine takes an HDF element and promotes it into a linked
   block element.  Basically, the element becomes a linked list
   allowing easy appending.  If the element already exists, it
   is promoted to being a linked block element, otherwise a new
   element is created.

   All of the pieces of the linked list are the same size (blocklen)
   except for the first one which stays the size of the element
   at the time HLcreate was called.

   This routine is similar to HLcreate but is used to convert an
   existing AID into a linked block element "in-place".  This is
   done for convenience and ease-of-use mostly internally to the
   library in various places, but it is allowable for user-level
   code to do this also.

   Hopefully HLcreate will get re-written to call this routine for
   most of it's work...

   numblocks gives the number of linked list objects in each
   block header.

   The ideal setting for numblocks and blocklen are very data
   and application depedent.

---------------------------------------------------------------------------*/
intn
HLconvert(int32 aid, 
          int32 block_length, 
          int32 number_blocks)
{
    CONSTR(FUNC, "HLconvert");  /* for HERROR */
    filerec_t  *file_rec;       /* file record */
    accrec_t   *access_rec=NULL;/* access record */
    linkinfo_t *info;           /* information for the linked blocks elt */
    uint16      link_ref;       /* the ref of the link structure
                                   (block table) */
    int32       dd_aid;         /* AID for writing the special info */
    uint16      new_data_tag=DFTAG_NULL, new_data_ref=0;  /* Tag/ref of the new data in the file */
    uint16      data_tag, data_ref;  /* Tag/ref of the data in the file */
    int32       data_len;		/* length of the data we are checking */
    int32       data_off;		/* offset of the data we are checking */
    uint16      special_tag;    /* special version of this tag */
    int32       file_id;        /* file ID for the access record */
    uint8       local_ptbuf[16];
    int32       old_posn;       /* position in the access element */
    intn        ret_value = SUCCEED;

    /* clear error stack */
    HEclear();

    /* start checking the func. args */
    if (HAatom_group(aid)!=AIDGROUP || block_length < 0 || number_blocks < 0)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get the access_rec pointer */
    if ((access_rec = HAatom_object(aid)) == NULL)    
        HGOTO_ERROR(DFE_ARGS, FAIL);

    file_id = access_rec->file_id;
    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    if (!(file_rec->access & DFACC_WRITE))
        HGOTO_ERROR(DFE_DENIED, FAIL);

    /* verify that the object is not already special. Can not convert
       if already special.  */
    if (HTPis_special(access_rec->ddid))
        HGOTO_ERROR(DFE_CANTMOD, FAIL);

    /* Save previous position in data element so that we can come back to it */
    old_posn=access_rec->posn;

    /* get the info for the dataset */
    if(HTPinquire(access_rec->ddid,&data_tag,&data_ref,&data_off,&data_len)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* make data tag special i.e. will be linked-block element */
    if ((special_tag = MKSPECIALTAG(data_tag)) == DFTAG_NULL)
        HGOTO_ERROR(DFE_BADDDLIST, FAIL);

    /* is data defined but does not exist in the file? */
    if(data_off==INVALID_OFFSET && data_len==INVALID_LENGTH)
      { /* catch the case where the data doesn't exist yet */

          /* set length to zero */
        if(Hsetlength(aid,0)==FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);

        /* get back new offset and length */
        if(HTPinquire(access_rec->ddid,&data_tag,&data_ref,&data_off,&data_len)==FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);
      } /* end if */
      
    /* set up new tag/ref for linked block element */
    new_data_tag = DFTAG_LINKED;
    new_data_ref = Htagnewref(file_id,new_data_tag);

    /* make new tag/ref point to existing data element */
    if(Hdupdd(file_id, new_data_tag, new_data_ref, data_tag, data_ref)==FAIL)
        HGOTO_ERROR(DFE_CANTUPDATE, FAIL);

    /* Delete the old data ID */
    if(HTPdelete(access_rec->ddid)==FAIL)
        HGOTO_ERROR(DFE_CANTDELHASH, FAIL);

    /* Attach to the new data ID */
    if ((access_rec->ddid=HTPcreate(file_rec,special_tag,data_ref))==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* get link ref for linked-block ? */
    link_ref = Htagnewref(file_id,DFTAG_LINKED);

    /* allocates special info struct for linked blocks */
    access_rec->special_info = HDmalloc((uint32) sizeof(linkinfo_t));
    if (!access_rec->special_info)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* fill in special info struct */
    info = (linkinfo_t *) access_rec->special_info;
    info->attached     = 1;
    info->length       = data_len;
    info->first_length = data_len;
    info->block_length = block_length;
    info->number_blocks = number_blocks;
    info->link_ref = link_ref;

    /* Get ready to fill and write the special info structure  */

    /* start write access on special tag/ref */
    if((dd_aid=Hstartaccess(file_id,special_tag,data_ref,DFACC_ALL))==FAIL)
        HGOTO_ERROR(DFE_CANTACCESS, FAIL);

    /* encode special information to write out */
    {
        uint8      *p;

        p = local_ptbuf;
        UINT16ENCODE(p, SPECIAL_LINKED);
        INT32ENCODE(p, info->length);
        INT32ENCODE(p, block_length);
        INT32ENCODE(p, number_blocks);
        UINT16ENCODE(p, link_ref);  /* link_ref */
    }

    /* write out special information */
    if (Hwrite(dd_aid, 16, local_ptbuf) == FAIL)
        HGOTO_ERROR(DFE_WRITEERROR, FAIL);
    if(Hendaccess(dd_aid)==FAIL)
        HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

    /* write out linked block */
    if ((info->link = HLInewlink(file_id, number_blocks, link_ref, (uint16)new_data_ref)) ==NULL)
        HGOTO_ERROR(DFE_CANTLINK, FAIL);

    /* update access record and file record */
    access_rec->special_func = &linked_funcs;
    access_rec->special = SPECIAL_LINKED;
    access_rec->appendable = FALSE;     /* start data as non-appendable */

    /* check whether we should seek out to the proper position */
    if(old_posn>0)
      {
        if(Hseek(aid,old_posn,DF_START)==FAIL)
              HGOTO_ERROR(DFE_BADSEEK, FAIL);
      } /* end if */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */
        if(access_rec->special_info != NULL)
            HDfree(access_rec->special_info);
        if(access_rec!=NULL)
            HIrelease_accrec_node(access_rec);
    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* end HLconvert() */

/* ---------------------------- HDinqblockinfo ---------------------------- */
/*
NAME
   HDinqblockinfo -- return info about linked blocks
USAGE
   int32 HDinqblockinfo(aid, length, flength, blen, nblocks)
   int32   aid;          IN:  aid of element
   int32 * length;       OUT: total element length
   int32 * flength;      OUT: length of first element
   int32 * blen;         OUT: length of block elements
   int32 * nblocks;      OUT: number of blocks per block header
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Given an aid, return low level special info for linked-block
   element in space provided.  This function works like HDinquire()
   but provides more low level info than HLPinquire.  NULL can
   be passed for any non-interesting entries.

   hdfpack is the only application that I know of which uses
   this function.

---------------------------------------------------------------------------*/
int
HDinqblockinfo(int32 aid, 
               int32 *length, 
               int32 *first_length,
               int32 *block_length, 
               int32 *number_blocks)
{
    accrec_t   *arec;
    int        ret_value = SUCCEED;
    CONSTR(FUNC, "HDinqblockinfo");

    HEclear();
    if ((arec = HAatom_object(aid)) == (accrec_t *) NULL)
        HGOTO_ERROR(DFE_BADAID, FAIL);

    if (arec->special != SPECIAL_LINKED)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    if (length)
        *length = ((linkinfo_t *) (arec->special_info))->length;
    if (first_length)
        *first_length = ((linkinfo_t *) (arec->special_info))->first_length;
    if (block_length)
        *block_length = ((linkinfo_t *) (arec->special_info))->block_length;
    if (number_blocks)
        *number_blocks = ((linkinfo_t *) (arec->special_info))->number_blocks;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* HDinqblockinfo */

/* ----------------------------- HLIstaccess ------------------------------ */
/*
NAME
   HLIstaccess -- set up AID to access a linked block elem
USAGE
   int32 HLIstaccess(access_rec, acc_mode)
   access_t * access_rec;   IN: access record to fill in
   int16      acc_mode;     IN: access mode
RETURNS
   The AID of the access record on success FAIL on error.
DESCRIPTION
   Calls to HLIstread and HLIstwrite resolve to this function.
   Given an active AID fill in all of the special information.
   If this information has already been read in for a different
   element use that else we must go out to the HDF file and
   pull in the information ourselves

----------------------------------------------------------------------------*/
PRIVATE int32
HLIstaccess(accrec_t *access_rec, 
            int16     acc_mode)
{
    CONSTR(FUNC, "HLIstaccess");    /* for HERROR */
    filerec_t  *file_rec;       /* file record */
    linkinfo_t *info = NULL;           /* information about data elt */
    int32       dd_aid;         /* AID for writing the special info */
    uint16      data_tag, data_ref;  /* Tag/ref of the data in the file */
    uint8       local_ptbuf[14];
    int32       ret_value = SUCCEED;

    /* validate file record id */
    file_rec = HAatom_object(access_rec->file_id);
    if (BADFREC(file_rec) || !(file_rec->access & acc_mode))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* set up some data in access record */
    access_rec->special = SPECIAL_LINKED;
    access_rec->posn = 0;
    access_rec->access = (uint32)(acc_mode|DFACC_READ);

    /*
     * Lets free old special info first,if one exists,
     * before copying a new one
     */
    if (access_rec->special_info != NULL)
      {   /* special information record */
          linkinfo_t *t_info = (linkinfo_t *) access_rec->special_info;

          if (--(t_info->attached) == 0)
            {
                link_t     *t_link; /* current link to free */
                link_t     *next;   /* next link to free */

                /* free the linked list of links/block tables */
                if(t_info->link!=NULL)
                  {
                    for (t_link = t_info->link; t_link; t_link = next)
                      {
                          next = t_link->next;
                          if(t_link->block_list!=NULL)
                              HDfree(t_link->block_list);
                          HDfree(t_link);
                      }
                  } /* end if */
                HDfree(t_info);
                access_rec->special_info = NULL;
            }
      }

    /* get the info for the dataset */
    if(HTPinquire(access_rec->ddid,&data_tag,&data_ref,NULL,NULL)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* if the special information are already in some other acc elt,
     * point to it */
    access_rec->special_info = HIgetspinfo(access_rec);
    if (access_rec->special_info)
      {
          ((linkinfo_t *) access_rec->special_info)->attached++;
          file_rec->attach++;
          ret_value = HAregister_atom(AIDGROUP,access_rec);
          goto done; /* we are done */
      }

    /* read the special info structure from the file */
    if((dd_aid = Hstartaccess(access_rec->file_id,data_tag,data_ref,DFACC_READ))==FAIL)
        HGOTO_ERROR(DFE_CANTACCESS, FAIL);
    if (Hseek(dd_aid, 2, DF_START) == FAIL)
        HGOTO_ERROR(DFE_SEEKERROR, FAIL);
    if (Hread(dd_aid, 14, local_ptbuf) == FAIL)
        HGOTO_ERROR(DFE_READERROR, FAIL);
    if(Hendaccess(dd_aid)==FAIL)
        HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

    /* allocate space for special information */
    access_rec->special_info = HDmalloc((uint32) sizeof(linkinfo_t));
    info = (linkinfo_t *) access_rec->special_info;
    if (!info)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* decode special information retrieved from file into info struct */
    {
        uint8      *p = local_ptbuf;
        INT32DECODE(p, info->length);
        INT32DECODE(p, info->block_length);
        INT32DECODE(p, info->number_blocks);
        UINT16DECODE(p, info->link_ref);
    }

    /* set up the block tables of the information */
    info->link = HLIgetlink(access_rec->file_id,
                            info->link_ref, info->number_blocks);
    if (!info->link)
        HGOTO_DONE(FAIL);

    /* find and set the length of the first linked-block */
    if (info->link->block_list[0].ref)
      {
          info->first_length = Hlength(access_rec->file_id, DFTAG_LINKED,
                                       info->link->block_list[0].ref);
          if (info->first_length == FAIL)
            {
                HDfree(info->link);
                HGOTO_ERROR(DFE_INTERNAL, FAIL);
            }
      }
    else
        info->first_length = info->block_length;

    /* process through all the linked-blocks in the file for this element */
    info->last_link = info->link;
    while (info->last_link->nextref != 0)
      {
          info->last_link->next = HLIgetlink(access_rec->file_id,
                 info->last_link->nextref, info->number_blocks);
          if (!info->last_link->next)
            {
                link_t     *l, *next;

                for (l = info->link; l; l = next)
                  {
                      next = l->next;
                      if (l->block_list)
                          HDfree(l->block_list);
                      HDfree(l);
                  }
                HGOTO_ERROR(DFE_INTERNAL, FAIL);
            }
          info->last_link = info->last_link->next;
      }

    /* update data */
    info->attached = 1;

    file_rec->attach++; /* increment number of elements attached to file */

    ret_value = HAregister_atom(AIDGROUP,access_rec);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */
        if(access_rec->special_info != NULL)
            HDfree(access_rec->special_info);
    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* HLIstaccess */

/* ------------------------------ HLPstread ------------------------------- */
/*
NAME
   HLPstread -- open an access record for reading
USAGE
   int32 HLPstread(access_rec)
   access_t * access_rec;   IN: access record to fill in
RETURNS
   The AID of the access record on success FAIL on error.
DESCRIPTION
   Calls to HLIstaccess to fill in the access rec for
   reading

---------------------------------------------------------------------------*/
int32
HLPstread(accrec_t * access_rec)
{
  int32 ret_value;

  ret_value = HLIstaccess(access_rec, DFACC_READ);

  return ret_value;
}   /* HLPstread */

/* ------------------------------ HLPstwrite ------------------------------- */
/*
NAME
   HLPstwrite -- open an access record for writing
USAGE
   int32 HLPstwrite(access_rec)
   access_t * access_rec;   IN: access record to fill in
RETURNS
   The AID of the access record on success FAIL on error.
DESCRIPTION
   Calls to HLIstaccess to fill in the access rec for
   writing

---------------------------------------------------------------------------*/
int32
HLPstwrite(accrec_t * access_rec)
{
  int32  ret_value;

  ret_value = HLIstaccess(access_rec, (int16)DFACC_WRITE);

  return ret_value;
}   /* HLPstwrite */

/* ------------------------------ HLIgetlink ------------------------------ */
/*
NAME
   HLIgetlink -- get link information
USAGE
   link_t * HLIgetlink(fid, ref, num_blocks)
   int32  file_id;             IN: the file
   uint16 ref;                 IN: ref number of the link table
   int32  num_blocks;          IN: number of blocks in the table
RETURNS
   A pointer to a link_t or NULL.
DESCRIPTION
   Read a block table out of the file and return a pointer to
   the internal table representing it.

   It seems that num_blocks is redundant.

---------------------------------------------------------------------------*/
PRIVATE link_t *
HLIgetlink(int32  file_id, 
           uint16 ref, 
           int32  number_blocks)
{
    CONSTR(FUNC, "HLIgetlink");     /* for HERROR */
    int32    access_id;      /* access record id */
    uint8    *buffer = NULL;
    uint16   tag     = DFTAG_LINKED;
    link_t   *new_link  = NULL;
    link_t   *ret_value = NULL; /* FAIL */

    /* allocate necessary memory for in-memory block table */
    new_link = (link_t *) HDmalloc((uint32) sizeof(link_t));

    if (new_link == NULL)
        HGOTO_ERROR(DFE_NOSPACE, NULL);

    new_link->block_list = (block_t *) HDmalloc((uint32) number_blocks
                                                  * sizeof(block_t));
    if (new_link->block_list == NULL)
        HGOTO_ERROR(DFE_NOSPACE, NULL);

    new_link->next = (link_t *) NULL;

    /* create temp buffer to read block table in */
    buffer = (uint8 *) HDmalloc((uint32) (2 + 2 * number_blocks));
    if (buffer == NULL)
        HGOTO_ERROR(DFE_NOSPACE, NULL);

    /* read block table into buffer */
    access_id = Hstartread(file_id, tag, ref);
    if (access_id == FAIL ||
        Hread(access_id, 2 + 2 * number_blocks, buffer) == FAIL)
        HGOTO_ERROR(DFE_READERROR, NULL);

    /* decode block table information read from file */
    {
        int32 i;
        uint8      *p = buffer;

        UINT16DECODE(p, new_link->nextref);
        for (i = 0; i < number_blocks; i++)
            UINT16DECODE(p, new_link->block_list[i].ref);
    }

    /* end acces to this block table */
    Hendaccess(access_id);

    /* set return value */
    ret_value = new_link;

done:
  if(ret_value == NULL)   
    { /* Error condition cleanup */
        if (new_link->block_list != NULL)
            HDfree(new_link->block_list);
        if (new_link != NULL)
            HDfree(new_link);
    } /* end if */

  /* Normal function cleanup */
  if (buffer != NULL)
      HDfree(buffer);

  return ret_value;
}   /* HLIgetlink */

/* ------------------------------- HLPseek -------------------------------- */
/*
NAME
   HLPseek -- set the seek posn
USAGE
   int32 HLPseek(access_rec, offset, origin)
   access_t * access_rec;      IN: access record to mess with
   int32      offset;          IN: seek offset
   int32      origin;          IN: where we should calc the offset from
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Set the seek posn in the given linked block element

---------------------------------------------------------------------------*/
int32
HLPseek(accrec_t *access_rec, 
        int32     offset, 
        int       origin)
{
    CONSTR(FUNC, "HLPseek");    /* for HERROR */
    int32   ret_value = SUCCEED;

    /* validate access record */
    if (access_rec->special != SPECIAL_LINKED)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* adjust the offset according to origin and validate */
    /* there is no upper bound to posn */
    if (origin == DF_CURRENT)
        offset += access_rec->posn;
    if (origin == DF_END)
        offset += ((linkinfo_t *) (access_rec->special_info))->length;
    if (offset < 0)
        HGOTO_ERROR(DFE_RANGE, FAIL);

    /* set position */
    access_rec->posn = offset;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* HLPseek */

/* ------------------------------- HLPread -------------------------------- */
/*
NAME
   HLPread -- read some data out of a linked block element
USAGE
   int32 HLPseek(access_rec, length, data)
   access_t * access_rec;      IN: access record to mess with
   int32      length;          IN: number of bytes to read
   void *      data;            IN: buffer for data
RETURNS
   The number of bytes read or FAIL on error
DESCRIPTION
   Read in some data from a linked block element.  If length
   is zero read until the end of the element.  It is assumed
   that the data buffer is big enough to store the data.
   If length would take us off the end of the element only
   read what has been written.

--------------------------------------------------------------------------- */
int32
HLPread(accrec_t *access_rec, 
        int32     length, 
        void *     datap)
{
    CONSTR(FUNC, "HLPread");    /* for HERROR */
    uint8      *data = (uint8 *) datap;
    /* information record for this special data elt */
    linkinfo_t *info = (linkinfo_t *) (access_rec->special_info);
    link_t     *t_link = info->link;    /* block table record */

    /* relative position in linked block of data elt */
    int32       relative_posn = access_rec->posn;

    int32       block_idx;      /* block table index of current block */
    int32       current_length; /* length of current block */
    int32       nbytes = 0;     /* # bytes read on any single Hread() */
    int32       bytes_read = 0; /* total # bytes read for this call of HLIread */
    int32       ret_value = SUCCEED;

    /* validate length */
    if (length == 0)
        length = info->length - access_rec->posn;
    else
        if (length < 0)
            HGOTO_ERROR(DFE_RANGE, FAIL);

    if (access_rec->posn + length > info->length)
        length = info->length - access_rec->posn;

    /* search for linked block to start reading from */
    if (relative_posn < info->first_length)
      { /* first block */
          block_idx = 0;
          current_length = info->first_length;
      }
    else /* not first block? */
      {
          relative_posn -= info->first_length;
          block_idx = relative_posn / info->block_length + 1;
          relative_posn %= info->block_length;
          current_length = info->block_length;
      }

/* calculate which block to start from? */
    {
        int32 i;

        for (i = 0; i < block_idx / info->number_blocks; i++)
          {
              if (t_link == NULL)
                  HGOTO_ERROR(DFE_INTERNAL, FAIL);
              t_link = t_link->next;
          }
    }
    block_idx %= info->number_blocks;

    /* found the starting block, now read in the data */
    do
      {
          int32 remaining =    /* remaining data in current block */
              current_length - relative_posn;

          /* read in the data in this block */
          if (remaining > length)
              remaining = length;
          if (t_link->block_list[block_idx].ref != 0)
            {
                int32       access_id;  /* access record id for this block */
                block_t    *current_block =     /* record on the current block */
                    &(t_link->block_list[block_idx]);

                access_id = Hstartread(access_rec->file_id, DFTAG_LINKED,
                                       current_block->ref);
                if (access_id == (int32) FAIL
                    || (relative_posn
                && (int32) FAIL == Hseek(access_id, relative_posn, DF_START))
                    || (int32) FAIL == (nbytes = Hread(access_id, remaining, data)))
                    HGOTO_ERROR(DFE_READERROR, FAIL);

                bytes_read += nbytes;
                Hendaccess(access_id);
            }
          else
            {   /*if block is missing, fill this part of buffer with zero's */
                HDmemset(data, 0, (size_t)remaining);
                bytes_read += nbytes;
            }

          /* move variables for the next block */
          data += remaining;
          length -= remaining;
          if (length > 0 && ++block_idx >= info->number_blocks)
            {
                block_idx = 0;
                t_link = t_link->next;
                if (t_link == NULL)
                    HGOTO_ERROR(DFE_INTERNAL, FAIL);
            }
          relative_posn = 0;
          current_length = info->block_length;
      }
    while (length > 0);     /* if still somemore to read in, repeat */

    access_rec->posn += bytes_read;
    ret_value = bytes_read;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* HLPread  */

/* ------------------------------- HLPwrite ------------------------------- */
/*
NAME
   HLPwrite -- write out some data to a linked block
USAGE
   int32 HLPwrite(access_rec, length, data)
   access_t * access_rec;      IN: access record to mess with
   int32      length;          IN: number of bytes to write
   void *      data;            IN: buffer for data
RETURNS
   The number of bytes written or FAIL on error
DESCRIPTION
   Write out some data from a linked block element.  If we write
   passed the end of the existing element new blocks are created
   as needed.

---------------------------------------------------------------------------*/
int32
HLPwrite(accrec_t   *access_rec, 
         int32       length, 
         const void * datap)
{
    CONSTR(FUNC, "HLPwrite");   /* for HERROR */
    const uint8      *data = datap;
    filerec_t  *file_rec;       /* file record */
    int32       dd_aid;         /* AID for writing the special info */
    uint16      data_tag, data_ref;  /* Tag/ref of the data in the file */
    linkinfo_t *info =          /* linked blocks information record */
        (linkinfo_t *) (access_rec->special_info);
    link_t     *t_link =        /* ptr to link block table */
        info->link;
    int32       relative_posn = /* relative position in linked block */
        access_rec->posn;
    int32       block_idx;      /* block table index of current block */
    link_t     *prev_link = NULL; /* ptr to block table before current block table.
                                       for groking the offset of
                                       current block table */
    int32       current_length; /* length of current block */
    int32       nbytes = 0;     /* #bytes written by any single Hwrite */
    int32       bytes_written = 0;  /* total #bytes written by HLIwrite */
    uint8       local_ptbuf[4];
    int32       ret_value = SUCCEED;

    /* convert file id to file record */
    file_rec = HAatom_object(access_rec->file_id);

    /* validate length and file records */
    if (length <= 0)
        HGOTO_ERROR(DFE_RANGE, FAIL);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* determine linked block and position to start writing into */
    /* determine where to start.  Setup missing block tables
       along the way. */
    if (relative_posn < info->first_length)
      {
          block_idx = 0;
          current_length = info->first_length;
      }
    else
      {
          relative_posn -= info->first_length;
          block_idx = relative_posn / info->block_length + 1;
          relative_posn %= info->block_length;
          current_length = info->block_length;
      }
    {
        /* follow the links of block tables and create missing
           block tables along the way */
        int32 num_links;   /* number of links to follow */

        for (num_links = block_idx / info->number_blocks; num_links > 0; num_links--)
          {
              if (!t_link->next)
                {   /* create missing link (block table) */
                    t_link->nextref = Htagnewref(access_rec->file_id,DFTAG_LINKED);
                    t_link->next = HLInewlink(access_rec->file_id,
                                   info->number_blocks, t_link->nextref, 0);
                    if (!t_link->next)
                        HGOTO_ERROR(DFE_NOSPACE,FAIL);
                    {   /* AA */
                        /* update previous link with information about new link */

                        uint16      link_tag = DFTAG_LINKED;
                        uint16      link_ref =  /* ref of current link */
                        (uint16) (prev_link != NULL ?
                                  prev_link->nextref : info->link_ref);

                        uint8      *p = local_ptbuf;    /* temp buf ptr */

                        /* write file the updated portion of current link */

                        int32       link_id =   /* access id for current link */
                        Hstartwrite(access_rec->file_id, link_tag, link_ref, 0);

                        if (link_id == FAIL)
                            HGOTO_ERROR(DFE_WRITEERROR, FAIL);
                        UINT16ENCODE(p, t_link->nextref);
                        if (Hwrite(link_id, 2, local_ptbuf) == FAIL)
                            HGOTO_ERROR(DFE_WRITEERROR, FAIL);
                        Hendaccess(link_id);
                    }   /* AA */
                }   /* if not t_link->next */

              /* move to the next link */
              prev_link = t_link;
              t_link = t_link->next;
          }     /* end for */
    }   /* end block statement(bad) */

    block_idx %= info->number_blocks;

    /* start writing in that block */
    do
      {
          int32       access_id;    /* access record id */
          int32       remaining =   /* remaining data length in this block */
              current_length - relative_posn;
          uint16      new_ref = 0;  /* ref of newly created block */

          /* determine length and write this block */
          if (remaining > length)
              remaining = length;

          /* this block already exist, so just set up access to it */
          if (t_link->block_list[block_idx].ref != 0)
            {
                block_t    *current_block =     /* ptr to current block record */
                &(t_link->block_list[block_idx]);

                access_id = Hstartwrite(access_rec->file_id, DFTAG_LINKED,
                                        current_block->ref, current_length);
            }   
          else
            {   /* block is missing, set up a new block */
                new_ref = Htagnewref(access_rec->file_id,DFTAG_LINKED);
                access_id = Hstartwrite(access_rec->file_id, DFTAG_LINKED,
                                        new_ref, current_length);
            }

          if (access_id == (int32) FAIL)
              HGOTO_ERROR(DFE_WRITEERROR, FAIL);

          if ((relative_posn &&
               (int32) FAIL == Hseek(access_id, relative_posn, DF_START)) ||
              (int32) FAIL == (nbytes = Hwrite(access_id, remaining, data)))
            {
                HGOTO_ERROR(DFE_WRITEERROR, FAIL);
            }
          Hendaccess(access_id);
          bytes_written += nbytes;

          if (new_ref)
            {   /* created a new block, so update the link/block table */
                uint16      link_tag = DFTAG_LINKED;
                uint16      link_ref =  /* ref of the current link/block table */
                (uint16) (prev_link ? prev_link->nextref : info->link_ref);
                uint8      *p = /* temp buffer ptr */
                local_ptbuf;
                int32       link_id =   /* access record id of the current
                                           link/block table */
                Hstartwrite(access_rec->file_id, link_tag, link_ref, 0);

                if (link_id == FAIL)
                    HGOTO_ERROR(DFE_WRITEERROR, FAIL);
                UINT16ENCODE(p, new_ref);
                if (Hseek(link_id, 2 + 2 * block_idx, DF_START) == FAIL)
                    HGOTO_ERROR(DFE_SEEKERROR, FAIL);
                if (Hwrite(link_id, 2, local_ptbuf) == FAIL)
                    HGOTO_ERROR(DFE_WRITEERROR, FAIL);
                Hendaccess(link_id);

                /* update memory structure */
                t_link->block_list[block_idx].ref = new_ref;
            }   /* if new_ref */

          /* move ptrs and counters for next phase */
          data += remaining;
          length -= remaining;

          if (length > 0 && ++block_idx >= info->number_blocks)
            {  /* move to the next link/block table */
                block_idx = 0;
                if (!t_link->next)
                  {     /* create missing link/block table */
                      t_link->nextref = Htagnewref(access_rec->file_id,DFTAG_LINKED);
                      t_link->next = HLInewlink(access_rec->file_id,
                                   info->number_blocks, t_link->nextref, 0);
                      if (!t_link->next)
                          HGOTO_ERROR(DFE_NOSPACE, FAIL);

                      {     /* BB */
                          uint16      link_tag = DFTAG_LINKED;
                          uint16      link_ref =    /* ref of current link/block table */
                          (uint16) (prev_link ? prev_link->nextref : info->link_ref);
                          uint8      *p =   /* temp buffer ptr */
                          local_ptbuf;
                          int32       link_id =     /* access record id of
                                                       current link/block table */
                          Hstartwrite(access_rec->file_id, link_tag,
                                      link_ref, 0);

                          if (link_id == FAIL)
                              HGOTO_ERROR(DFE_WRITEERROR, FAIL);
                          UINT16ENCODE(p, t_link->nextref);
                          if (Hwrite(link_id, 2, local_ptbuf) == FAIL)
                              HGOTO_ERROR(DFE_WRITEERROR, FAIL);
                          Hendaccess(link_id);
                      }     /* BB */
                  }     /* if not t_link->next  */

                /* move to the next link/block table */
                prev_link = t_link;
                t_link = t_link->next;
            }   /* end if "length" */

          /* update vars for next phase */
          relative_posn = 0;
          current_length = info->block_length;
      }
    while (length > 0);

    /* update the info for the dataset */
    if(HTPinquire(access_rec->ddid,&data_tag,&data_ref,NULL,NULL)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);
    if((dd_aid=Hstartaccess(access_rec->file_id,data_tag,data_ref,DFACC_WRITE))==FAIL)
        HGOTO_ERROR(DFE_CANTACCESS, FAIL);
    if (Hseek(dd_aid, 2, DF_START) == FAIL)
        HGOTO_ERROR(DFE_SEEKERROR, FAIL);
    {
        int32       tmp;
        uint8      *p = local_ptbuf;

        tmp = bytes_written + access_rec->posn;
        if (tmp > info->length)
            info->length = tmp;
        INT32ENCODE(p, info->length);
    }
    if (Hwrite(dd_aid, 4, local_ptbuf) == FAIL)
        HGOTO_ERROR(DFE_READERROR, FAIL);
    if(Hendaccess(dd_aid)==FAIL)
        HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

    access_rec->posn += bytes_written;
    /* return SUCCEED; */
    /* if wrong # bytes written, FAIL has already been returned */
    ret_value = bytes_written;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* HLPwrite */

/* ------------------------------ HLInewlink ------------------------------ */
/*
NAME
   HLInewlink -- write out some data to a linked block
USAGE
   link_t * HLInewlink(fid, nblocks, link_ref, first_block_ref)
   int32  fid;               IN: file ID
   int32  nblocks;           IN: number of blocks in the table
   uint16 link_ref;          IN: ref number for the table
   uint16 first_block_ref;   IN: ref number for first block
RETURNS
   A pointer to a new link/block table or NULL
DESCRIPTION
   Create a new link/block table in memory and in file returns
   ptr to the new link/block table.

---------------------------------------------------------------------------*/
PRIVATE link_t *
HLInewlink(int32  file_id, 
           int32  number_blocks,
           uint16 link_ref, 
           uint16 first_block_ref)
{
    CONSTR(FUNC, "HLInewlink");     /* for HERROR */
    int32       link_id;        /* access record id of new link */
    uint8      *buf       = NULL;            /* temp buffer */
    link_t     *t_link    = NULL;
    link_t     *ret_value = NULL; /* FAIL */

    /* set up new link record in memory */
    /* new link record */
    t_link = (link_t *) HDmalloc((uint32) sizeof(link_t));

    if (!t_link)
        HGOTO_ERROR(DFE_NOSPACE, NULL);

    t_link->block_list = (block_t *) HDmalloc((uint32) number_blocks
                                                * sizeof(block_t));
    if (!t_link->block_list)
        HGOTO_ERROR(DFE_NOSPACE, NULL);

    t_link->next = NULL;

    /* get ready to write the new link to file */
    link_id = Hstartwrite(file_id, DFTAG_LINKED, link_ref, 2 + 2 * number_blocks);
    if (link_id == FAIL)
        HGOTO_ERROR(DFE_WRITEERROR, NULL);

    /* encode this block information for writing to the file */
    {   /* CC */
        int32 i;       /* temp int index */
        uint8      *p;          /* temp buffer ptr */

        p = buf = (uint8 *) HDmalloc((uint32) (2 + 2 * number_blocks));
        if (!buf)
            HGOTO_ERROR(DFE_NOSPACE, NULL);

        /* set up the record and write to file */
        t_link->nextref = 0;
        UINT16ENCODE(p, 0);
        t_link->block_list[0].ref = first_block_ref;
        UINT16ENCODE(p, first_block_ref);

        for (i = 1; i < number_blocks; i++)
          {     /* set up each block in this link */
              t_link->block_list[i].ref = 0;
              UINT16ENCODE(p, 0);
          }
    }   /* CC */

    /* write the link */
    if (Hwrite(link_id, 2 + 2 * number_blocks, buf) == FAIL)
        HGOTO_ERROR(DFE_WRITEERROR, NULL);

    /* close down acces to this block */
    Hendaccess(link_id);

    /* set return value */
    ret_value = t_link;

done:
  if(ret_value == NULL)   
    { /* Error condition cleanup */
        if (t_link->block_list != NULL)
            HDfree(t_link->block_list);
        if (t_link != NULL)
            HDfree(t_link);
    } /* end if */

  /* Normal function cleanup */
  if (buf != NULL)
      HDfree(buf);

  return ret_value;
}   /* HLInewlink */

/* ------------------------------ HLPinquire ------------------------------ */
/*
NAME
   HLPinquire -- Hinquire for linked blocks
USAGE
   int32 HLPinquire(access_rec, fid, tag, ref, len, off, pos, acc, sp)
   access_t * access_rec;      IN:  access record to return info about
   uint16   * file;            OUT: file ID;
   uint16   * tag;             OUT: tag of info record;
   uint16   * ref;             OUT: ref of info record;
   int32    * len;             OUT: length of element;
   int32    * off;             OUT: offset of element -- meaningless
   int32    * pos;             OUT: current position in element;
   int16    * acc;             OUT: access mode;
   int16    * sp;              OUT: special code;
RETURNS
   SUCCEED
DESCRIPTION
   Return interesting information about a linked block element.
   NULL can be passed for any of the OUT parameters if their
   value is not needed.

--------------------------------------------------------------------------- */
int32
HLPinquire(accrec_t  *access_rec, 
           int32     *pfile_id, 
           uint16    *ptag,
           uint16    *pref, 
           int32     *plength, 
           int32     *poffset, 
           int32     *pposn,
           int16     *paccess, 
           int16     *pspecial)
{
    CONSTR(FUNC, "HLPinquire");   /* for HERROR */
    uint16      data_tag, data_ref;  /* Tag/ref of the data in the file */
    linkinfo_t *info =          /* special information record */
        (linkinfo_t *) access_rec->special_info;
    int32   ret_value = SUCCEED;

    /* update the info for the dataset */
    if(HTPinquire(access_rec->ddid,&data_tag,&data_ref,NULL,NULL)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* fill in the variables if they are present */
    if (pfile_id)
        *pfile_id = access_rec->file_id;
    if (ptag)
        *ptag = data_tag;
    if (pref)
        *pref = data_ref;
    if (plength)
        *plength = info->length;
    if (poffset)
        *poffset = 0;   /* meaningless */
    if (pposn)
        *pposn = access_rec->posn;
    if (paccess)
        *paccess = (int16)access_rec->access;
    if (pspecial)
        *pspecial = (int16)access_rec->special;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

    return ret_value;
}   /* HLPinquire */

/* ----------------------------- HLPendaccess ----------------------------- */
/*
NAME
   HLPendacess -- close a linked block AID
USAGE
   intn HLPendaccess(access_rec)
   access_t * access_rec;      IN:  access record to close
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Free up all of the space used to store information about a
   linked block element.  Information is flushed to disk as
   it is created so this routine does NOT have to write anything
   out.

--------------------------------------------------------------------------- */
intn
HLPendaccess(accrec_t * access_rec)
{
    CONSTR(FUNC, "HLPendaccess");   /* for HERROR */
    filerec_t  *file_rec;           /* file record */
    intn      ret_value = SUCCEED;

    /* convert file id to file record */
    file_rec = HAatom_object(access_rec->file_id);

    /* detach the special information record.
       If no more references to that, free the record */
    HLPcloseAID(access_rec);

    /* update file and access records */
    if (HTPendaccess(access_rec->ddid) == FAIL)
      HGOTO_ERROR(DFE_CANTFLUSH, FAIL);

    /* validate file record */
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* detach from the file */
    file_rec->attach--;

    /* free the access record */
    HIrelease_accrec_node(access_rec);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* HLPendaccess */

/* ----------------------------- HLPcloseAID ------------------------------ */
/*
NAME
   HLPcloseAID -- close file but keep AID active
USAGE
   int32 HLPcloseAID(access_rec)
   access_t * access_rec;      IN:  access record of file to close
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   close the file currently being pointed to by this AID but 
   do *NOT* free the AID.

   This is called by Hnextread() which reuses an AID to point to
   the 'next' object as requested.  If the current object was an
   linked object, the linked information needs to be closed before all
   reference to it is lost.

---------------------------------------------------------------------------*/
int32
HLPcloseAID(accrec_t * access_rec)
{
#ifdef LATER
    CONSTR(FUNC, "HLPcloseAID");    /* for HERROR */
#endif /* LATER */
    linkinfo_t *info =          /* special information record */
        (linkinfo_t *) access_rec->special_info;
    int32      ret_value = SUCCEED;

    /* detach the special information record.
       If no more references to that, free the record */
    if (--(info->attached) == 0)
      {
          link_t     *t_link;   /* current link to free */
          link_t     *next;     /* next link to free */

          /* free the linked list of links/block tables */
          for (t_link = info->link; t_link; t_link = next)
            {
                next = t_link->next;
                HDfree(t_link->block_list);
                HDfree(t_link);
            }

          HDfree(info);
          access_rec->special_info = NULL;
      }

#ifdef LATER
done:
#endif /* LATER */
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

    return ret_value;
}   /* HLPcloseAID */

/* ------------------------------- HLPinfo -------------------------------- */
/*
NAME
   HLPinfo -- return info about a linked block element
USAGE
   int32 HLPinfo(access_rec, info_block)
   access_t        * access_rec;
   IN: access record of access element
   sp_info_block_t * info_block;
   OUT: information about the special element
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Return information about the given linked block.  Info_block is
   assumed to be non-NULL.

--------------------------------------------------------------------------- */
int32
HLPinfo(accrec_t * access_rec, sp_info_block_t * info_block)
{
    CONSTR(FUNC, "HLPinfo");
    linkinfo_t *info =          /* special information record */
    (linkinfo_t *) access_rec->special_info;
    int32     ret_value = SUCCEED;

    /* validate access record */
    if (access_rec->special != SPECIAL_LINKED)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* fill in the info_block */
    info_block->key = SPECIAL_LINKED;

    info_block->first_len = info->first_length;
    info_block->block_len = info->block_length;
    info_block->nblocks = info->number_blocks;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* HLPinfo */

/*--------------------------------------------------------------------------
NAME
   HLsetblocksize -- set the block length of a linked-block element

USAGE
   intn HLsetblockinfo(
   int32 aid		IN: access record id
   int32 block_size	IN: length to be used for each linked-block 
   int32 num_blocks	IN: number of blocks the element will have

RETURNS
   SUCCEED / FAIL

DESCRIPTION
   HLsetblocksize sets (accrec_t).block_size and (accrec_t).num_blocks
   to block_size and num_blocks, respectively.  An error will occur, if
   either of the parameters is a 0 or a negative number, that is not
   -1, which is used to indicate that the respective field is not to be
   changed.

   In the library, this routine is used by:
	VSsetblocksize 
	VSsetnumblocks

MODIFICATION
   BMR - added in June 2001 to fix bug# 267

--------------------------------------------------------------------------*/
intn
HLsetblockinfo(int32 aid,	/* access record id */
              int32 block_size, /* length to be used for each linked-block */
              int32 num_blocks) /* number of blocks the element will have */
{
    CONSTR(FUNC, "HLsetblockinfo");  	/* for HERROR */
    accrec_t   *access_rec;               /* access record */
    intn	ret_value = SUCCEED;

    /* clear error stack */
    HEclear();

    /* validate aid */
    if (HAatom_group(aid)!=AIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* block_size and num_blocks should be either -1, to keep the original 
       values, or positive values to change the block size and/or the 
       number of blocks */ 
    if ((block_size <= 0 && block_size != -1) || 
        (num_blocks <= 0 && num_blocks != -1))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get the access record */
    if ((access_rec = HAatom_object(aid)) == NULL)        
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* change the linked-block size if requested */
    if (block_size != -1)
        access_rec->block_size = block_size;

    /* change the number of linked-blocks if requested */
    if (num_blocks != -1)
        access_rec->num_blocks = num_blocks;

done:
    if(ret_value == FAIL)
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
    return ret_value;
}       /* end HLsetblockinfo */

/*--------------------------------------------------------------------------
NAME
   HLgetblocksize -- get the block size and the number of blocks of a 
		    linked-block element

USAGE
   intn HLgetblockinfo(aid, block_size, num_blocks)
   int32  aid		IN: access record id
   int32* block_size	OUT: the returned block size of each linked-block 
   int32* num_blocks	OUT: the returned number of blocks of the element

RETURNS
   SUCCEED / FAIL

DESCRIPTION
   HLgetblocksize retrieves the values of (accrec_t).block_size and 
   (accrec_t).num_blocks to block_size and num_blocks, respectively.  
   A NULL can be passed in for an unwanted value.

   In the library, this routine is used by:
	VSgetblockinfo 

MODIFICATION
   BMR - added in June 2001 to fix bug# 267

--------------------------------------------------------------------------*/
intn
HLgetblockinfo(int32 aid,	/* access record id */
              int32* block_size, /* length being used for each linked-block */
              int32* num_blocks) /* number of blocks the element will have */
{
    CONSTR(FUNC, "HLgetblockinfo");  /* for HERROR */
    accrec_t   *access_rec;               /* access record */
    intn	ret_value = SUCCEED;

    /* clear error stack */
    HEclear();

    /* get the access record */
    if ((access_rec = HAatom_object(aid)) == NULL)        
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get the linked-block size and the number of linked-blocks if requested */
    if (block_size != NULL)
        *block_size = access_rec->block_size;
    if (num_blocks != NULL)
        *num_blocks = access_rec->num_blocks;

done:
    if(ret_value == FAIL)
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
    return ret_value;
}       /* end HLgetblockinfo */

