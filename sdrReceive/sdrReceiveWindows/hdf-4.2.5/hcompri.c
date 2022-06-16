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

/* $Id: hcompri.c 4932 2007-09-07 17:17:23Z bmribler $ */
/*LINTLIBRARY */
/* ------------------------------ hcompri.c -------------------------------

   Routines for reading & writing old-style (i.e. non-special compressed)
   compressed raster images, such as JPEG, (raster specific) RLE and IMCOMP.
   These routines are designed to be layered underneath the buffered special
   element code and only provide access to reading/writing the entire image
   at once through the DFgetcomp/DFputcomp code.

   File Organization
  ******************
    These special elements are invoked at run-time only, information about
    whether an element was written/read through this interface is not stored in
    the file.  Unless specificly asked for by an API routine or required for a
    particular kind of access by the library, these routines aren't called.

 LOCAL ROUTINES

 EXPORTED BUT LIBRARY PRIVATE ROUTINES
   HRPcloseAID      -- close object but keep AID active
   HRPendacess      -- close object, free AID
   HRPinfo          -- return info about a compressed raster element
   HRPinquire       -- retrieve information about a compressed raster element
   HRPread          -- read some data out of a compressed raster element
   HRPreset         -- replace the current comp. raster info with new info (NOP)
   HRPseek          -- set the seek position
   HRPsetaccesstype -- set the I/O access type of the compressed raster element
   HRPstread        -- open an access record for reading
   HRPstwrite       -- open an access record for reading
   HRPwrite         -- write some data out to a compressed raster element
   HRPconvert       -- wrap access to a compress raster object

------------------------------------------------------------------------- */

#include "hdf.h"
#include "hfile.h"
#include <assert.h>

/* crinfo_t -- compressed raster information structure */

typedef struct
  {
      intn        attached;     /* number of access records attached
                                   to this information structure */
      int32 fid;                /* File ID of image */
      uint16 tag, ref;          /* Tag & ref of compressed raster image */
      int32 xdim, ydim;         /* Image dimensions */
      int16 scheme;             /* Compression scheme */
      comp_info cinfo;          /* Compression information */
      uintn image_size;         /* Size of the uncompressed image in memory */
  }
crinfo_t;

/* forward declaration of the functions provided in this module */

/* cr_funcs -- table of the accessing functions of the compressed raster
   data element function modules.  The position of each function in
   the table is standard */
funclist_t  cr_funcs =
{
    HRPstread,
    HRPstwrite,
    HRPseek,
    HRPinquire,
    HRPread,
    HRPwrite,
    HRPendaccess,
    HRPinfo,
    NULL         /* no routine registered */
};

/*------------------------------------------------------------------------ 
NAME
   HRPconvert -- wrap an existing raster image with the special element routines.
USAGE
   intn HRPconvert(fid, tag, ref, xdim, ydim, scheme, cinfo, pixel_size)
        int32 fid;          IN: File ID for raster image
        uint16 tag, ref;    IN: Tag & Ref of raster image to wrap
        int32 xdim, ydim;   IN: Dimensions of raster image
        int16 scheme;       IN: Compression scheme used
        comp_info *cinfo;   IN: Additional compression parameters
        uintn pixel_size;   IN: Size of the pixels in the image
RETURNS
   AID on SUCCEED/FAIL on failure
DESCRIPTION
   Wraps an existing compressed raster image with the special element API.
   This is designed only for use under a buffered special element.

FORTRAN
   None

--------------------------------------------------------------------------*/
int32
HRPconvert(int32 fid, uint16 tag, uint16 ref, int32 xdim, int32 ydim,int16 scheme, comp_info *cinfo, uintn pixel_size)
{
    CONSTR(FUNC, "HRPconvert");     /* for HERROR */
    filerec_t  *file_rec;           /* file record */
    accrec_t   *access_rec=NULL;    /* access element record */
    crinfo_t  *info;                /* information for the compressed raster element */
    int32      ret_value = SUCCEED;

    HEclear();

    file_rec = HAatom_object(fid);
    if (BADFREC(file_rec) || SPECIALTAG(tag))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* allocate special info struct for buffered element */
    if ((info = HDmalloc((uint32) sizeof(crinfo_t)))==NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* fill in special info struct */
    info->attached= 1;
    info->fid     = fid;
    info->tag     = tag;
    info->ref     = ref;
    info->xdim    = xdim;
    info->ydim    = ydim;
    info->image_size  = xdim*ydim*pixel_size;
    info->scheme  = scheme;
    HDmemcpy(&(info->cinfo),cinfo,sizeof(comp_info));

    /* get empty access record */
    access_rec = HIget_access_rec();
    if (access_rec == NULL)
        HGOTO_ERROR(DFE_TOOMANY, FAIL);

    /* set up the information in the access record */
    access_rec->special_info = info;

    /* Check if the tag/ref pair exists */
    if(Hexist(fid,tag,ref)<0) {
        access_rec->new_elem=TRUE;
        if((access_rec->ddid=HTPcreate(file_rec,tag,ref))==FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);
      } /* end if */
    else {
        if((access_rec->ddid=HTPselect(file_rec,tag,ref))==FAIL)
            HGOTO_ERROR(DFE_INTERNAL, FAIL);
      } /* end else */
    access_rec->special_func = &cr_funcs;
    access_rec->special      = SPECIAL_COMPRAS;
    access_rec->posn         = 0;
    access_rec->access       = DFACC_RDWR;
    access_rec->file_id      = fid;
    access_rec->appendable   = FALSE;     /* data is non-appendable */
    file_rec->attach++;

    ret_value = HAregister_atom(AIDGROUP,access_rec);  /* return access id */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */
    } /* end if */

  return ret_value; 
} /* HRPconvert */

/* ------------------------------ HRPstread ------------------------------- */
/*
NAME
   HRPstread -- open an access record for reading
USAGE
   int32 HRPstread(access_rec)
       access_t * access_rec;   IN: access record to fill in
RETURNS
   The AID of the access record on success FAIL on error.
DESCRIPTION
   This is a stub routine and should never be called

---------------------------------------------------------------------------*/
int32
HRPstread(accrec_t * rec)
{
    /* shut compilers up*/
    rec=rec;

assert(0 && "Should never be called");
  return (FAIL);
}   /* HRPstread */

/* ------------------------------ HRPstwrite ------------------------------- */
/*
NAME
   HRPstwrite -- open an access record for reading
USAGE
   int32 HRPstwrite(access_rec)
       access_t * access_rec;   IN: access record to fill in
RETURNS
   The AID of the access record on success FAIL on error.
DESCRIPTION
   This is a stub routine and should never be called.

---------------------------------------------------------------------------*/
int32
HRPstwrite(accrec_t * rec)
{
    /* shut compilers up*/
    rec=rec;

assert(0 && "Should never be called");
  return (FAIL);
}   /* HRPstwrite */

/* ------------------------------ HRPseek ------------------------------- */
/*
NAME
   HRPseek -- set the seek posn
USAGE
   int32 HRPseek(access_rec, offset, origin)
       access_t * access_rec;      IN: access record to mess with
       int32      offset;          IN: seek offset
       int32      origin;          IN: where we should calc the offset from
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Set the seek posn in the given compressed raster element.  The only valid
   position is 0 bytes from the beginning.

---------------------------------------------------------------------------*/
int32
HRPseek(accrec_t * access_rec, int32 offset, int origin)
{
    int32     ret_value = SUCCEED;
    CONSTR(FUNC, "HRPseek");    /* for HERROR */

    /* Adjust offset according to origin.  There is no upper bound to posn */
    if (origin != DF_START || offset !=0)
        HGOTO_ERROR(DFE_RANGE, FAIL);

    /* set the offset */
    access_rec->posn = offset;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* HRPseek */

/* ------------------------------ HRPread ------------------------------- */
/*
NAME
   HRPread -- read some data out of compressed raster element
USAGE
   int32 HRPread(access_rec, length, data)
       access_t * access_rec;      IN: access record to mess with
       int32      length;          IN: number of bytes to read
       void *      data;           IN: buffer for data
RETURNS
   The number of bytes read or FAIL on error
DESCRIPTION
    Uncompress a compressed raster image into the buffer provided.  Support is
    only provided for reading the entire image in.

---------------------------------------------------------------------------*/
int32
HRPread(accrec_t * access_rec, int32 length, void * data)
{
    CONSTR(FUNC, "HRPread");    /* for HERROR */
    crinfo_t  *info =          /* information on the special element */
        (crinfo_t *) access_rec->special_info;
    int32    ret_value = SUCCEED;

    /* validate length */
    if (length!=0 && length!=(int32)info->image_size)
        HGOTO_ERROR(DFE_RANGE, FAIL);

    /* adjust length */
    if (length == 0)
        length = info->image_size;

    /* Copy data from buffer */
    DFgetcomp(info->fid,info->tag,info->ref,data,info->xdim,info->ydim,info->scheme);

    ret_value = length;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return(ret_value);
}	/* HRPread */

/* ------------------------------ HRPwrite ------------------------------- */
/*
NAME
   HRPwrite -- write data out to a compressed raster image
USAGE
   int32 HRPwrite(access_rec, length, data)
       access_t * access_rec;      IN: access record to mess with
       int32      length;          IN: number of bytes to read
       void *     data;            IN: buffer of data
RETURNS
   The number of bytes written or FAIL on error
DESCRIPTION
   Write out data to a compressed raster image.  The entire image must be
   written.

---------------------------------------------------------------------------*/
int32
HRPwrite(accrec_t * access_rec, int32 length, const void * data)
{
    CONSTR(FUNC, "HRPwrite");   /* for HERROR */
    crinfo_t  *info =          /* information on the special element */
                    (crinfo_t *) (access_rec->special_info);
    int32      ret_value = SUCCEED;

    /* validate length */
    if (length!=0 && length!=(int32)info->image_size)
        HGOTO_ERROR(DFE_RANGE, FAIL);

    /* adjust length */
    if (length == 0)
        length = info->image_size;

    /* Copy data to buffer */
    DFputcomp(info->fid,info->tag,info->ref,data,info->xdim,info->ydim,NULL,NULL,info->scheme,&(info->cinfo));

    ret_value = length;    /* return length of bytes written */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return(ret_value);
}	/* HRPwrite */

/* ------------------------------ HRPinquire ------------------------------ */
/*
NAME
   HRPinquire -- retreive information about a compressed raster element
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
   Return interesting information about a compressed raster element.
   NULL can be passed for any of the OUT parameters if their
   value is not needed.

---------------------------------------------------------------------------*/
int32
HRPinquire(accrec_t * access_rec, int32 *pfile_id, uint16 *ptag,
           uint16 *pref, int32 *plength, int32 *poffset,
           int32 *pposn, int16 *paccess, int16 *pspecial)
{
    CONSTR(FUNC, "HRPinquire");   /* for HERROR */
    crinfo_t  *info =          /* special information record */
        (crinfo_t *) access_rec->special_info;
    uint16 data_tag,data_ref;   /* tag/ref of the data we are checking */
    int32       data_off;		/* offset of the data we are checking */
    int32    ret_value = SUCCEED;

    /* Get the data's offset & length */
    if(HTPinquire(access_rec->ddid,&data_tag,&data_ref,&data_off,NULL)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* fill in the variables if they are present */
    if (pfile_id)
        *pfile_id = access_rec->file_id;
    if (ptag)
        *ptag = data_tag;
    if (pref)
        *pref = data_ref;
    if (plength)
        *plength = (access_rec->new_elem ? -1 : info->image_size);
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
}	/* HRPinquire */

/* ----------------------------- HRPendaccess ----------------------------- */
/*
NAME
   HRPendacess -- free AID
USAGE
   intn HRPendaccess(access_rec)
       access_t * access_rec;      IN:  access record to close
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Free the AID

---------------------------------------------------------------------------*/
intn
HRPendaccess(accrec_t * access_rec)
{
#ifdef LATER
    CONSTR(FUNC, "HRPendaccess");   /* for HERROR */
#endif /* LATER */
    filerec_t  *file_rec; 	    /* file record */
    intn     ret_value = SUCCEED;

    /* convert file id to file record */
    file_rec = HAatom_object(access_rec->file_id);

    /* shut down dependant access record */
    HRPcloseAID(access_rec);

    /* free the access record */
    HIrelease_accrec_node(access_rec);

    /* detach from the file */
    file_rec->attach--;


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
}	/* HRPendaccess */

/* ----------------------------- HRPcloseAID ------------------------------ */
/*
NAME
   HRPcloseAID -- free memory but keep AID active
USAGE
   int32 HRPcloseAID(access_rec)
       access_t * access_rec;      IN:  access record of file to close
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Free special element information, but do *NOT* free the AID.

---------------------------------------------------------------------------*/
int32
HRPcloseAID(accrec_t * access_rec)
{
#ifdef LATER
    CONSTR(FUNC, "HRPcloseAID");    /* for HERROR */
#endif /* LATER */
    crinfo_t  *info =          /* special information record */
        (crinfo_t *) access_rec->special_info;
    int32      ret_value = SUCCEED;

    /* detach the special information record.
       If no more references to that, free the record */

    if (--(info->attached) == 0)
      {
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

    return(ret_value);
}   /* HRPcloseAID */

/* ------------------------------- HRPinfo -------------------------------- */
/*
NAME
   HRPinfo -- return info about a compressed raster element
USAGE
   int32 HRPinfo(access_rec, info_block)
       accrec_t        * access_rec; IN: access record of element
       sp_info_block_t * info_block; OUT: information about the special element 
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Return information about the given external element.  Info_block is
   assumed to be non-NULL.  

   --------------------------------------------------------------------------- */
int32
HRPinfo(accrec_t * access_rec, sp_info_block_t * info_block)
{
    CONSTR(FUNC, "HRPinfo");    /* for HERROR */
    int32      ret_value = SUCCEED;

    /* validate access record */
    if (access_rec->special != SPECIAL_COMPRAS)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* fill in the info_block */
    info_block->key = SPECIAL_COMPRAS;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return(ret_value);
}   /* HRPinfo */

