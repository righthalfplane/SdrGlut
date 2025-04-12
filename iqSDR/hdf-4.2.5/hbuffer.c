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

/* $Id: hbuffer.c 4932 2007-09-07 17:17:23Z bmribler $ */
/*LINTLIBRARY */
/* ------------------------------ hbuffer.c -------------------------------

   Routines for buffered elements, i.e., data elements that reside in memory
   until they are closed, and are then flushed out to the file.  Buffered
   elements are held in memory while they are being accessed and are only
   written back to the file if they are modified.

   File Organization
  ******************
    These special elements are invoked at run-time only, information about
    whether an element was cached is not stored in the file.  Unless specificly
    asked for by an API routine or required for a particular kind of access by
    the library, these routines aren't called.

 LOCAL ROUTINES

 EXPORTED BUT LIBRARY PRIVATE ROUTINES
   HBPcloseAID      -- close file but keep AID active
   HBPendacess      -- close file, free AID
   HBPinfo          -- return info about an buffered element
   HBPinquire       -- retrieve information about an buffered element
   HBPread          -- read some data out of an buffered element
   HBPreset         -- replace the current buffered info with new info (NOP)
   HBPseek          -- set the seek position
   HBPsetaccesstype -- set the I/O access type of the buffered element
   HBPstread        -- open an access record for reading
   HBPstwrite       -- open an access record for reading
   HBPwrite         -- write some data out to a buffered element

EXPORTED ROUTINES
   HBconvert        -- start buffering an AID

------------------------------------------------------------------------- */

#include "hdf.h"
#include "hfile.h"
#include <assert.h>

/* extinfo_t -- external elt information structure */

typedef struct
  {
      intn        attached;     /* number of access records attached
                                   to this information structure */
      intn        modified;     /* has the buffered element been modified? */
      int32       length;       /* length of this element */
      uint8      *buf;          /* pointer to the buffered data */
      int32       buf_aid;      /* AID for buffered access record (below) */
      accrec_t   *buf_access_rec;   /* "Real" access record for buffered data */
  }
bufinfo_t;

/* forward declaration of the functions provided in this module */

/* buf_funcs -- table of the accessing functions of the buffered
   data element function modules.  The position of each function in
   the table is standard */
funclist_t  buf_funcs =
{
    HBPstread,
    HBPstwrite,
    HBPseek,
    HBPinquire,
    HBPread,
    HBPwrite,
    HBPendaccess,
    HBPinfo,
    NULL         /* no routine registered */
};

/*------------------------------------------------------------------------ 
NAME
   HBconvert -- cause an existing AID to be buffered.
USAGE
   intn HBcreate(aid)
       int32  aid;          IN: AID of data element to buffer
RETURNS
   SUCCEED/FAIL
DESCRIPTION
   Buffers an existing data element (referred to with the AID passed in) in
   memory for faster access.  This is especially useful when repeatedly
   accessing a compressed special element object which would otherwise have
   to be repeatedly decompressed over many I/O accesses.

   If the ALLOW_BUFFER_GROWING flag is defined during compile time,
   the buffered object is allowed to grow, it is assumed that a higher-level
   API will prevent this if it is not allowed through that API.

FORTRAN
   None

--------------------------------------------------------------------------*/
intn
HBconvert(int32 aid)
{
    CONSTR(FUNC, "HBconvert");   /* for HERROR */
    accrec_t   *access_rec=NULL;/* access element record */
    accrec_t   *new_access_rec; /* newly created access record */
    accrec_t   *tmp_access_rec; /* temp. access record */
    bufinfo_t  *info;           /* information for the buffered element */
    uint16 data_tag,data_ref;   /* tag/ref of the data we are checking */
    int32       data_off;		/* offset of the data we are checking */
    int32       data_len;		/* length of the data we are checking */
    intn        ret_value = SUCCEED;

    HEclear();
    if ((access_rec = HAatom_object(aid)) == NULL)	/* get the access_rec pointer */
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get the info for the dataset */
    if (HTPis_special(access_rec->ddid) || access_rec->special!=0) {
        if((*access_rec->special_func->inquire) (access_rec, NULL,
                           &data_tag, &data_ref, &data_len, &data_off, NULL, NULL, NULL)==FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);
    } /* end if */
    else
        if(HTPinquire(access_rec->ddid,&data_tag,&data_ref,&data_off,&data_len)==FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);

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
      
    /* allocate special info struct for buffered element */
    if ((info = HDmalloc((uint32) sizeof(bufinfo_t)))==NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* fill in special info struct */
    info->attached     = 1;
    info->modified     = 0;         /* Data starts out not modified */
    info->length       = data_len;  /* initial buffer size */

    /* Get space for buffer */
    if(data_len>0) {
        if((info->buf = HDmalloc((uint32) data_len))==NULL)
            HGOTO_ERROR(DFE_NOSPACE, FAIL);
      } /* end if */
    else
        info->buf=NULL;

    /* Read in existing data into buffer */
    if(data_len>0) {
        if (Hseek(aid, 0, DF_START) == FAIL)
            HGOTO_ERROR(DFE_SEEKERROR, FAIL);
        if (Hread(aid, data_len, info->buf) == FAIL)
            HGOTO_ERROR(DFE_READERROR, FAIL);
    } /* end if */

    /* get empty access record */
    new_access_rec = HIget_access_rec();
    if (new_access_rec == NULL)
        HGOTO_ERROR(DFE_TOOMANY, FAIL);

    /* Copy the old access record information to the new access record */
    /*
     * Don't get a new copy of the DD id or increment the number of attached
     * elements, buffered elements are supposed to be "transparent".
     * We "inherit" the appendable flag if it's set and ALLOW_BUFFER_GROW is
     * defined to support it.
     */
    tmp_access_rec=new_access_rec->next;    /* preserve free list pointer */
    HDmemcpy(new_access_rec,access_rec,sizeof(accrec_t));
    new_access_rec->next=tmp_access_rec;    /* restore free list pointer */

    /* Preserve the actual access record for the buffered element */
    info->buf_access_rec = new_access_rec;  /* Access record of actual data on disk */

    /* Create AID for actual access record */
    info->buf_aid = HAregister_atom(AIDGROUP,new_access_rec);

    /* Modify access record to point to buffered element functions */
    access_rec->special_info = (void *)info;
    access_rec->special_func = &buf_funcs;
    access_rec->special      = SPECIAL_BUFFERED;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */
    } /* end if */

  return ret_value;
} /* HBconvert */

/* ------------------------------ HBPstread ------------------------------- */
/*
NAME
   HBPstread -- open an access record for reading
USAGE
   int32 HBPstread(access_rec)
       access_t * access_rec;   IN: access record to fill in
RETURNS
   The AID of the access record on success FAIL on error.
DESCRIPTION
   This is a stub routine and should never be called

---------------------------------------------------------------------------*/
int32
HBPstread(accrec_t * rec)
{
    /* shut compilers up*/
    rec=rec;

assert(0 && "Should never be called");
  return (FAIL);
}   /* HBPstread */

/* ------------------------------ HBPstwrite ------------------------------- */
/*
NAME
   HBPstwrite -- open an access record for reading
USAGE
   int32 HBPstwrite(access_rec)
       access_t * access_rec;   IN: access record to fill in
RETURNS
   The AID of the access record on success FAIL on error.
DESCRIPTION
   This is a stub routine and should never be called.

---------------------------------------------------------------------------*/
int32
HBPstwrite(accrec_t * rec)
{
    /* shut compilers up*/
    rec=rec;

assert(0 && "Should never be called");
  return (FAIL);
}   /* HBPstwrite */

/* ------------------------------ HBPseek ------------------------------- */
/*
NAME
   HBPseek -- set the seek posn
USAGE
   int32 HXPseek(access_rec, offset, origin)
       access_t * access_rec;      IN: access record to mess with
       int32      offset;          IN: seek offset
       int32      origin;          IN: where we should calc the offset from
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Set the seek posn in the given buffered element

---------------------------------------------------------------------------*/
int32
HBPseek(accrec_t * access_rec, int32 offset, int origin)
{
    int32     ret_value = SUCCEED;
    CONSTR(FUNC, "HBPseek");    /* for HERROR */

    /* Adjust offset according to origin.  There is no upper bound to posn */
    if (origin == DF_CURRENT)
        offset += access_rec->posn;
    if (origin == DF_END)
        offset += ((bufinfo_t *) (access_rec->special_info))->length;
    if (offset < 0)
        HGOTO_ERROR(DFE_RANGE, FAIL);

    /* set the offset */
    access_rec->posn = offset;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* HBPseek */

/* ------------------------------ HBPread ------------------------------- */
/*
NAME
   HBPread -- read some data out of buffered element
USAGE
   int32 HBPread(access_rec, length, data)
       access_t * access_rec;      IN: access record to mess with
       int32      length;          IN: number of bytes to read
       void *      data;           IN: buffer for data
RETURNS
   The number of bytes read or FAIL on error
DESCRIPTION
   Read in some data from a buffered element.  If length is zero
   read until the end of the element.  It is assumed that the
   data buffer is big enough to store the data.

---------------------------------------------------------------------------*/
int32
HBPread(accrec_t * access_rec, int32 length, void * data)
{
    CONSTR(FUNC, "HBPread");    /* for HERROR */
    bufinfo_t  *info =          /* information on the special element */
        (bufinfo_t *) access_rec->special_info;
    int32    ret_value = SUCCEED;

    /* validate length */
    if (length < 0)
        HGOTO_ERROR(DFE_RANGE, FAIL);

    /* adjust length if it falls off the end of the element */
    if ((length == 0) || (access_rec->posn + length > info->length))
        length = info->length - access_rec->posn;
    else if (length < 0)
        HGOTO_ERROR(DFE_RANGE, FAIL);

    /* Copy data from buffer */
    HDmemcpy(data,info->buf+access_rec->posn,length);

    /* adjust access position */
    access_rec->posn += length;

    ret_value = length;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return(ret_value);
}	/* HBPread */

/* ------------------------------ HBPwrite ------------------------------- */
/*
NAME
   HBPwrite -- write some data out to a buffered element
USAGE
   int32 HBPwrite(access_rec, length, data)
       access_t * access_rec;      IN: access record to mess with
       int32      length;          IN: number of bytes to read
       void *     data;            IN: buffer of data
RETURNS
   The number of bytes written or FAIL on error
DESCRIPTION
   Write out some data to a buffered element.

---------------------------------------------------------------------------*/
int32
HBPwrite(accrec_t * access_rec, int32 length, const void * data)
{
    CONSTR(FUNC, "HBPwrite");   /* for HERROR */
    bufinfo_t  *info =          /* information on the special element */
                    (bufinfo_t *) (access_rec->special_info);
    int32 new_len;              /* new length of object */
    int32      ret_value = SUCCEED;

    /* validate length */
    if (length < 0)
        HGOTO_ERROR(DFE_RANGE, FAIL);

    /* Check if the data to write will overrun the buffer and realloc it if so */
    if(access_rec->posn+length>info->length) {
        /* Calc. the new size of the object */
        new_len=access_rec->posn+length;

        /* Resize buffer in safe manner */
        /* Realloc should handle this, but the Sun is whining about it... -QAK */
        if(info->buf==NULL) {
            if((info->buf = HDmalloc((uint32)new_len))==NULL)
                HGOTO_ERROR(DFE_NOSPACE, FAIL);
        }
        else {
            uint8 *temp_buf=info->buf;  /* temporary buffer pointer in case realloc fails */

            if((info->buf = HDrealloc(info->buf, (uint32)new_len))==NULL) {
                info->buf=temp_buf;
                HGOTO_ERROR(DFE_NOSPACE, FAIL);
            } /* end if */
        }

        /* update length */
        info->length=new_len;
    } /* end if */
    
    /* Copy data to buffer */
    HDmemcpy(info->buf+access_rec->posn,data,length);

    /* Mark the buffer as modified */
    info->modified=TRUE;

    /* update access record */
    access_rec->posn += length;

    ret_value = length;    /* return length of bytes written */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return(ret_value);
}	/* HBPwrite */

/* ------------------------------ HBPinquire ------------------------------ */
/*
NAME
   HBPinquire -- retreive information about a buffered element
USAGE
   int32 HBPinquire(access_rec, file, tag, ref, len, off, pos, acc, sp)
   access_t * access_rec;      IN:  access record to return info about
   uint16   * file;            OUT: file ID;
   uint16   * tag;             OUT: tag of info record;
   uint16   * ref;             OUT: ref of info record;
   int32    * len;             OUT: length of element;
   int32    * off;             OUT: offset of element (NOT correct);
   int32    * pos;             OUT: current position in element;
   int16    * acc;             OUT: access mode;
   int16    * sp;              OUT: special code;
RETURNS
   SUCCEED
DESCRIPTION
   Return interesting information about a buffered element.
   NULL can be passed for any of the OUT parameters if their
   value is not needed.

---------------------------------------------------------------------------*/
int32
HBPinquire(accrec_t * access_rec, int32 *pfile_id, uint16 *ptag,
           uint16 *pref, int32 *plength, int32 *poffset,
           int32 *pposn, int16 *paccess, int16 *pspecial)
{
    CONSTR(FUNC, "HBPinquire");   /* for HERROR */
    bufinfo_t  *info =          /* special information record */
        (bufinfo_t *) access_rec->special_info;
    uint16 data_tag,data_ref;   /* tag/ref of the data we are checking */
    int32       data_off;		/* offset of the data we are checking */
    int32    ret_value = SUCCEED;

    /* Get the data's offset & length */
    if(HTPinquire(info->buf_access_rec->ddid,&data_tag,&data_ref,&data_off,NULL)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* fill in the variables if they are present */
    if (pfile_id)
        *pfile_id = access_rec->file_id;
    if (ptag)
        *ptag = data_tag;
    if (pref)
        *pref = data_ref;
    if (plength)
        *plength = info->length;    /* pass along our value, which might be different from that on disk */
    if (poffset)
        *poffset = data_off;
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
}	/* HBPinquire */

/* ----------------------------- HBPendaccess ----------------------------- */
/*
NAME
   HBPendacess -- flush buffer, free AID
USAGE
   intn HBPendaccess(access_rec)
       access_t * access_rec;      IN:  access record to close
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Flush the buffer (if modified) and free the AID

---------------------------------------------------------------------------*/
intn
HBPendaccess(accrec_t * access_rec)
{
#ifdef LATER
    CONSTR(FUNC, "HBPendaccess");   /* for HERROR */
#endif /* LATER */
    intn     ret_value = SUCCEED;

    /* shut down the memory buffer and dependant access record */
    HBPcloseAID(access_rec);

    /* free the access record */
    HIrelease_accrec_node(access_rec);

#ifdef LATER
done:
#endif /* LATER */
  if(ret_value == FAIL)   
    { /* Error condition cleanup */
      if(access_rec!=NULL)
          HIrelease_accrec_node(access_rec);

    } /* end if */

  /* Normal function cleanup */

  return ret_value; 
}	/* HBPendaccess */

/* ----------------------------- HBPcloseAID ------------------------------ */
/*
NAME
   HBPcloseAID -- flush buffer and free memory but keep AID active
USAGE
   int32 HXPcloseAID(access_rec)
       access_t * access_rec;      IN:  access record of file to close
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Flush the buffered data (if modified) and free special element information,
   but do *NOT* free the AID.

   This is called by Hnextread() which reuses an AID to point to
   the 'next' object as requested.  If the current object was an
   buffered object, the buffer needs to be flushed and freed before all
   reference to it is lost.

---------------------------------------------------------------------------*/
int32
HBPcloseAID(accrec_t * access_rec)
{
    CONSTR(FUNC, "HBPcloseAID");    /* for HERROR */
    bufinfo_t  *info =          /* special information record */
        (bufinfo_t *) access_rec->special_info;
    int32      ret_value = SUCCEED;

    /* detach the special information record.
       If no more references to that, free the record */

    if (--(info->attached) == 0)
      {
        /* Flush the data if it's been modified */
        if(info->modified) {
            if (Hwrite(info->buf_aid, info->length, info->buf) == FAIL)
                HGOTO_ERROR(DFE_WRITEERROR, FAIL);
          } /* end if */

        /* Free the memory buffer */
        HDfree(info->buf);

        /* Close the dependent access record */
        Hendaccess(info->buf_aid);

        HDfree(info);
        access_rec->special_info = NULL;
      }

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

    return(ret_value);
}   /* HBPcloseAID */

/* ------------------------------- HBPinfo -------------------------------- */
/*
NAME
   HBPinfo -- return info about an external element
USAGE
   int32 HBPinfo(access_rec, info_block)
       accrec_t        * access_rec; IN: access record of element
       sp_info_block_t * info_block; OUT: information about the special element 
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Return information about the given external element.  Info_block is
   assumed to be non-NULL.  

   --------------------------------------------------------------------------- */
int32
HBPinfo(accrec_t * access_rec, sp_info_block_t * info_block)
{
    CONSTR(FUNC, "HBPinfo");    /* for HERROR */
    bufinfo_t  *info =          /* special information record */
        (bufinfo_t *) access_rec->special_info;
    int32      ret_value = SUCCEED;

    /* validate access record */
    if (access_rec->special != SPECIAL_BUFFERED)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* fill in the info_block */
    info_block->key = SPECIAL_BUFFERED;

    info_block->buf_aid = info->buf_aid;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return(ret_value);
}   /* HBPinfo */

