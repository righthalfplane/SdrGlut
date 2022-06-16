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

/* $Id: hextelt.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*LINTLIBRARY */
/* ------------------------------ hextelt.c -------------------------------

   Routines for external elements, i.e., data elements that reside on
   some other file.  These elements have no limits on their length.
   While users are prevented from reading beyond what is written, a
   user can write an unlimited amount of data.

   17-Mar-93
   Adding offset and "starting length" to elements so that a user can
   take an existing file with some data in it and create an HDF file
   which has a pointer to that data.

   File Organization
  ******************
   DD for External Element pointing to External Element Description Record
   =======================================================================
   <-  2 bytes -> <- 2 bytes -> <- 4 bytes -> <- 4bytes ->
   --------------------------------------------------------
   |extended tag | reference # |  Offset     |  Length    |
   --------------------------------------------------------
                                    \______________/
   __________________________________________|
   V
   EXTERNAL ELEMENT DESCRIPTION RECORD(EEDR - 12 + file_name_length bytes)
   ========================================================================
   <-  4 bytes  -> <- 4 bytes -> <- 4 bytes -> <- variable ->
   ---------------------------------------------------------
   | ext_tag_desc |   offset    |  length     | filename   |
   ---------------------------------------------------------

   ext_tag_desc  - EXT_EXTERN(16 bit constant). Identifies this as an
                   external element description record.
   offset        - Location of the element(data) within the external 
                   file(32 bit field)
   length        - Length in bytes of the element(data) in the external
                   file(32 bit field)
   filename      - Non-null terminated ASCII string naming the external
                   file(variable length)

 LOCAL ROUTINES
   HXIstaccess      -- set up AID to access an ext elem
   HXIbuildfilename -- Build the Filename for the External Element

 EXPORTED BUT LIBRARY PRIVATE ROUTINES
   HXPcloseAID      -- close file but keep AID active
   HXPendacess      -- close file, free AID
   HXPinfo          -- return info about an external element
   HXPinquire       -- retreive information about an external element
   HXPread          -- read some data out of an external file
   HXPreset         -- replace the current external info with new info
   HXPseek          -- set the seek position
   HXPsetaccesstype -- set the I/O access type of the external file
   HXPstread        -- open an access record for reading
   HXPstwrite       -- open an access record for reading
   HXPwrite         -- write some data out to an external file

EXPORTED ROUTINES
   HXcreate         -- create an external element
   HXsetcreatedir   -- set the directory variable for creating external file
   HXsetdir         -- set the directory variable for locating external file

------------------------------------------------------------------------- */

#include "hdf.h"
#include "hfile.h"

#ifdef _MSC_VER
#define strdup _strdup
#endif

/* Directory seperator definitions relating to a path. 
 * Note this does not provide a universal way to recognize
 * different path name conventions and translate between them */
#if defined (MAC) || defined (macintosh) || defined (SYMANTEC_C) 
#define DIR_SEPC  58  /* Integer value of ':' */
#define DIR_SEPS  ":"
#else /* not Macintosh */
#if defined WIN386 | defined DOS386
/* DOS-Windows seperator */
#define DIR_SEPC  92  /* Integer value of '\' */
#define DIR_SEPS  "\\"
#else 
#if defined VMS
/* VMS -made it the same as POSIX for now eventhought it should be '.' */
#define DIR_SEPC  47  /* Integer value of '/' */
#define DIR_SEPS  "/"
#else
/* Unix - POSIX */
#define DIR_SEPC  47  /* Integer value of '/' */
#define DIR_SEPS  "/"
#endif /* !VMS */
#endif /* !WIN386 & !DOS386 */
#endif /* !Macintosh */

/* directory path seperator from other directory paths */
#define DIR_PATH_SEPC 124
#define DIR_PATH_SEPS "|" 

/* extinfo_t -- external elt information structure */

typedef struct
  {
      int         attached;     /* number of access records attached
                                   to this information structure */
      int32       extern_offset;
      int32       length;       /* length of this element */
      int32       length_file_name;     /* length of the external file name */
      int32       para_extfile_id;  /* parallel ID of the external file */
      hdf_file_t  file_external;    /* external file descriptor */
      char       *extern_file_name;     /* name of the external file */
      intn        file_open;    /* has the file been opened yet ? */
  }
extinfo_t;

/* forward declaration of the functions provided in this module */
PRIVATE int32 HXIstaccess
            (accrec_t * access_rec, int16 access);
PRIVATE char *HXIbuildfilename
	(const char *ext_fname, const intn acc_mode);

/* ext_funcs -- table of the accessing functions of the external
   data element function modules.  The position of each function in
   the table is standard */
funclist_t  ext_funcs =
{
    HXPstread,
    HXPstwrite,
    HXPseek,
    HXPinquire,
    HXPread,
    HXPwrite,
    HXPendaccess,
    HXPinfo,
    HXPreset,
};

/*------------------------------------------------------------------------ 
NAME
   HXcreate -- create an external element
USAGE
   int32 HXcreate(file_id, tag, ref, ext_name, offset, len)
   int32  file_id;      IN: file ID for HDF file
   int16  tag;          IN: tag number for external elem
   int16  ref;          IN: ref number for external elem
   char * ext_name;     IN: external file name
   int32  offset;       IN: offset where elem should start in ext file
   int32  len;          IN: current len of element if already in
                            ext file (see desc below)
RETURNS
   returns AID to external element if succeed, else FAIL
DESCRIPTION
   Create a data element in an external file starting at the location
   of _offset_.  If the external file does not exist, it is created.
   If it already exists, we will simply open it, not delete it and
   start over.

   If the data element does not exist, it is created in reference to
   the external file starting at location _offset_.  Its data length is
   set as _len_.  If the data element already exists, it is "promoted"
   as an external element and its data is copied to the external file,
   again, starting at location _offset_.  In this case, since the
   length of the existing element is defined, it is set as the length
   of the external element.  The given _len_ value is ignored.

   Currently, all ordinary data element plus link-block and external
   elements can be set as an external element.  (For the case of
   setting an existing external element to a new external element has
   the effect of copying the data of the element from an old external
   file to a new one.)

   All further reference (e.g., read, write, seek) to this external
   element applies to the content of the external file.

   The AID which refers to this new external element, is returned upon
   successiful execution.  FAIL is returned if any error is encountered.
FORTRAN
   None

--------------------------------------------------------------------------*/
int32
HXcreate(int32 file_id, uint16 tag, uint16 ref, const char *extern_file_name, int32 offset, int32 start_len)
{
    CONSTR(FUNC, "HXcreate");   /* for HERROR */
    filerec_t  *file_rec;       /* file record */
    accrec_t   *access_rec=NULL;/* access element record */
    int32       dd_aid;         /* AID for writing the special info */
    hdf_file_t  file_external;  /* external file descriptor */
    extinfo_t  *info=NULL;      /* special element information */
    atom_t      data_id=FAIL;   /* dd ID of existing regular element */
    int32       data_len;		/* length of the data we are checking */
    uint16      special_tag;    /* special version of tag */
    uint8       local_ptbuf[20 + MAX_PATH_LEN];     /* temp working buffer */
    char	   *fname=NULL;    /* filename built from external filename */
    void *       buf = NULL;      /* temporary buffer */
    int32       ret_value = SUCCEED;

    /* clear error stack and validate args */
    HEclear();
    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec) || !extern_file_name || (offset < 0) || SPECIALTAG(tag)
        || (special_tag = MKSPECIALTAG(tag)) == DFTAG_NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    if (!(file_rec->access & DFACC_WRITE))
        HGOTO_ERROR(DFE_DENIED, FAIL);

    /* get a access records */
    access_rec = HIget_access_rec();
    if (access_rec == NULL)
        HGOTO_ERROR(DFE_TOOMANY, FAIL);

    /* search for identical dd */
    if ((data_id=HTPselect(file_rec,tag,ref))!=FAIL)
      {
          /* Check if the element is already special */
          if (HTPis_special(data_id)==TRUE)
            {
                sp_info_block_t sp_info;
                int32	aid, retcode;

                aid = Hstartread(file_id, tag, ref);
                retcode = HDget_special_info(aid, &sp_info);
                Hendaccess(aid);
                if ((retcode == FAIL) || (sp_info.key == FAIL))
                    HGOTO_ERROR(DFE_CANTMOD, FAIL);
		
                switch(sp_info.key)
                  {
                    /* we can proceed with these types of special elements */
                    case SPECIAL_LINKED:
                    case SPECIAL_EXT:
                        break;

                    /* abort since we cannot convert the data element to an external data element */
                    case SPECIAL_COMP:
                    default:
                        HTPendaccess(data_id);
                        HGOTO_ERROR(DFE_CANTMOD, FAIL);
                  } /* switch */
            }   /* end if */

          /* get the info for the dataset */
          if(HTPinquire(data_id,NULL,NULL,NULL,&data_len)==FAIL)
            {
                HTPendaccess(data_id);
                HGOTO_ERROR(DFE_INTERNAL, FAIL);
            } /* end if */
      } /* end if */

    /* build the customized external file name. */
    if (!(fname = HXIbuildfilename(extern_file_name, DFACC_CREATE)))
        HGOTO_ERROR(DFE_BADOPEN, FAIL);

    /* create the external file */
    file_external = (hdf_file_t)HI_OPEN(fname, DFACC_WRITE);
    if (OPENERR(file_external))
    {
        file_external = (hdf_file_t)HI_CREATE(fname);
        if (OPENERR(file_external))
            HGOTO_ERROR(DFE_BADOPEN, FAIL);
    }
    HDfree(fname);

    /* set up the special element information and write it to file */
    access_rec->special_info = HDmalloc((uint32) sizeof(extinfo_t));
    info = (extinfo_t *) access_rec->special_info;
    if (!info)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    if (data_id!=FAIL && data_len>0)
      {
          if ((buf = HDmalloc((uint32) data_len)) == NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
          if (Hgetelement(file_id, tag, ref, buf) == FAIL)
                HGOTO_ERROR(DFE_READERROR, FAIL);
          if (HI_SEEK(file_external, offset) == FAIL)
                HGOTO_ERROR(DFE_SEEKERROR, FAIL);
          if (HI_WRITE(file_external, buf, (int)data_len) == FAIL)
                HGOTO_ERROR(DFE_WRITEERROR, FAIL);
          info->length = data_len;
      }
    else
      info->length = start_len;

    info->attached         = 1;
    info->file_open        = TRUE;
    info->file_external    = file_external;
    info->extern_offset    = offset;
    info->extern_file_name = (char *) HDstrdup(extern_file_name);
    if (!info->extern_file_name)
      HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* Getting ready to write out special info struct */
    info->length_file_name = (int32)HDstrlen(extern_file_name);
    {
        uint8      *p = local_ptbuf;
    
        INT16ENCODE(p, SPECIAL_EXT);
        INT32ENCODE(p, info->length);
        INT32ENCODE(p, info->extern_offset);
        INT32ENCODE(p, info->length_file_name);
        HDstrcpy((char *) p, extern_file_name);
    }
    if(data_id!=FAIL)
        if (HTPdelete(data_id) == FAIL)
            HGOTO_ERROR(DFE_CANTDELDD, FAIL);

    /* write the special info structure to fill */
    if((dd_aid=Hstartaccess(file_id,special_tag,ref,DFACC_ALL))==FAIL)
        HGOTO_ERROR(DFE_CANTACCESS, FAIL);
    if (Hwrite(dd_aid, 14+info->length_file_name, local_ptbuf) == FAIL)
        HGOTO_ERROR(DFE_WRITEERROR, FAIL);
    if(Hendaccess(dd_aid)==FAIL)
        HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

    /* update access record and file record */
    if((access_rec->ddid=HTPselect(file_rec,special_tag,ref))==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);
    access_rec->special_func = &ext_funcs;
    access_rec->special      = SPECIAL_EXT;
    access_rec->posn         = 0;
    access_rec->access       = DFACC_RDWR;
    access_rec->file_id      = file_id;
    access_rec->appendable   = FALSE;     /* start data as non-appendable */
    file_rec->attach++;

    ret_value = HAregister_atom(AIDGROUP,access_rec);  /* return access id */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */
        if(access_rec!=NULL)
            HIrelease_accrec_node(access_rec);
        if(info!=NULL)
            HDfree(info);
        if(fname!=NULL)
            HDfree(fname);
        if(data_id!=FAIL)
            HTPendaccess(data_id);
    } /* end if */

  /* Normal function cleanup */
  if (buf != NULL)
      HDfree(buf);

  return ret_value; 
} /* HXcreate */

/*------------------------------------------------------------------------ 
NAME
   HXPsetaccesstype -- set the I/O access type of the external file
USAGE
   intn HXPsetaccesstype(access_rec)
   accrec_t *access_rec   IN/OUT: access record of the external element
RETURNS
   SUCCEED if no error, else FAIL
DESCRIPTION
   Open the external file according to the access type specified.

--------------------------------------------------------------------------*/
intn
HXPsetaccesstype(accrec_t * access_rec)
{
    CONSTR(FUNC, "HXPsetaccesstype");
    hdf_file_t  file_external;  /* external file descriptor */
    extinfo_t  *info;           /* special element information */
    char	*fname=NULL;
    intn       ret_value = SUCCEED;

    /* clear error stack and validate args */
    HEclear();

    /* sanity check */
    if (access_rec==NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    if (( info = (extinfo_t *) access_rec->special_info)==NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* build the customized external file name. */
    if ((fname = HXIbuildfilename(info->extern_file_name, DFACC_OLD))==NULL)
        HGOTO_ERROR(DFE_BADOPEN, FAIL);

    /* Open the external file for the correct access type */
    switch (access_rec->access_type)
      {
          case DFACC_SERIAL:
              file_external = (hdf_file_t)HI_OPEN(fname, DFACC_WRITE);
              if (OPENERR(file_external))
                {
                    file_external = (hdf_file_t)HI_CREATE(fname);
                    if (OPENERR(file_external))
                        HGOTO_ERROR(DFE_BADOPEN, FAIL);
                }
	      HDfree(fname);
              info->file_external = file_external;
              break;
              
          default:
              HGOTO_ERROR(DFE_BADOPEN, FAIL);
      }

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */
        if(fname!=NULL)
            HDfree(fname);

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

/* ----------------------------- HXIstaccess ------------------------------ */
/*
NAME
   HXIstaccess -- set up AID to access an ext elem
USAGE
   int32 HXIstaccess(access_rec, acc_mode)
   access_t * access_rec;   IN: access record to fill in
   int16      acc_mode;     IN: access mode
RETURNS
   The AID of the access record on success FAIL on error.
DESCRIPTION
   Calls to HXIstread and HXIstwrite resolve to this function.
   Given an active AID fill in all of the special information.
   If this information has already been read in for a different
   element use that else we must go out to the HDF file and
   pull in the information ourselves

---------------------------------------------------------------------------*/
PRIVATE int32
HXIstaccess(accrec_t * access_rec, int16 acc_mode)
{
    CONSTR(FUNC, "HXIstaccess");    /* for HERROR */
    extinfo_t  *info = NULL;        /* special element information */
    filerec_t  *file_rec = NULL;    /* file record */
    int32       data_off;		    /* offset of the data we are checking */
    uint8       local_ptbuf[12];    /* working buffer */
    int32       ret_value = SUCCEED;

    /* get file record and validate */
    file_rec = HAatom_object(access_rec->file_id);
    if (BADFREC(file_rec) || !(file_rec->access & acc_mode))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* intialize the access record */
    access_rec->special = SPECIAL_EXT;
    access_rec->posn = 0;
    access_rec->access = (uint32)(acc_mode|DFACC_READ);

    /* Get the data's offset & length */
    if(HTPinquire(access_rec->ddid,NULL,NULL,&data_off,NULL)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* get the special info record */
    access_rec->special_info = HIgetspinfo(access_rec);
    if (access_rec->special_info)
      {   /* found it from other access records */
          info = (extinfo_t *) access_rec->special_info;
          info->attached++;
      }
    else
      {   /* look for information in the file */
          if (HPseek(file_rec, data_off + 2) == FAIL)
              HGOTO_ERROR(DFE_SEEKERROR, FAIL);
          if (HP_read(file_rec, local_ptbuf, 12) == FAIL)
              HGOTO_ERROR(DFE_READERROR, FAIL);

          access_rec->special_info = HDmalloc((uint32) sizeof(extinfo_t));
          info = (extinfo_t *) access_rec->special_info;
          if (info==NULL)
            HGOTO_ERROR(DFE_NOSPACE, FAIL);

          {
              uint8      *p = local_ptbuf;
              INT32DECODE(p, info->length);
              INT32DECODE(p, info->extern_offset);
              INT32DECODE(p, info->length_file_name);
          }
          info->extern_file_name = (char *) HDmalloc((uint32) info->length_file_name + 1);
          if (!info->extern_file_name)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
          if (HP_read(file_rec, info->extern_file_name, info->length_file_name) == FAIL)
              HGOTO_ERROR(DFE_READERROR, FAIL);

          info->extern_file_name[info->length_file_name] = '\0';

          /* delay file opening until needed */
          info->file_open = FALSE;
          info->attached = 1;
      }

    file_rec->attach++;
    ret_value = HAregister_atom(AIDGROUP,access_rec);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */
        if(access_rec!=NULL)
            HIrelease_accrec_node(access_rec);
        if(info !=NULL)
          {   /* free file name first */
              if (info->extern_file_name != NULL)
                  HDfree(info->extern_file_name);
              HDfree(info);
          }
    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}	/* HXIstaccess */

/* ------------------------------ HXPstread ------------------------------- */
/*
NAME
   HXPstread -- open an access record for reading
USAGE
   int32 HXPstread(access_rec)
   access_t * access_rec;   IN: access record to fill in
RETURNS
   The AID of the access record on success FAIL on error.
DESCRIPTION
   Calls to HXIstaccess to fill in the access rec for 
   reading

---------------------------------------------------------------------------*/
int32
HXPstread(accrec_t * rec)
{
  int32 ret_value;

  ret_value = HXIstaccess(rec, DFACC_READ);

  return ret_value;
}   /* HXPstread */

/* ------------------------------ HXPstwrite ------------------------------- */
/*
NAME
   HXPstwrite -- open an access record for reading
USAGE
   int32 HXPstwrite(access_rec)
   access_t * access_rec;   IN: access record to fill in
RETURNS
   The AID of the access record on success FAIL on error.
DESCRIPTION
   Calls to HXIstaccess to fill in the access rec for writing

---------------------------------------------------------------------------*/
int32
HXPstwrite(accrec_t * rec)
{
  int32 ret_value;

  ret_value = HXIstaccess(rec, DFACC_WRITE);

  return ret_value;
}   /* HXPstwrite */

/* ------------------------------ HXPseek ------------------------------- */
/*
NAME
   HXPseek -- set the seek posn
USAGE
   int32 HXPseek(access_rec, offset, origin)
   access_t * access_rec;      IN: access record to mess with
   int32      offset;          IN: seek offset
   int32      origin;          IN: where we should calc the offset from
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Set the seek posn in the given external element

---------------------------------------------------------------------------*/
int32
HXPseek(accrec_t * access_rec, int32 offset, int origin)
{
    int32     ret_value = SUCCEED;
    CONSTR(FUNC, "HXPseek");    /* for HERROR */

    /* Adjust offset according to origin.
       there is no upper bound to posn */
    if (origin == DF_CURRENT)
        offset += access_rec->posn;
    if (origin == DF_END)
        offset += ((extinfo_t *) (access_rec->special_info))->length;
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
}   /* HXPseek */

/* ------------------------------ HXPread ------------------------------- */
/*
NAME
   HXPread -- read some data out of an external file
USAGE
   int32 HXPseek(access_rec, length, data)
   access_t * access_rec;      IN: access record to mess with
   int32      length;          IN: number of bytes to read
   void *      data;            IN: buffer for data
RETURNS
   The number of bytes read or FAIL on error
DESCRIPTION
   Read in some data from an external file.  If length is zero
   read until the end of the element.  It is assumed that the
   data buffer is big enough to store the data.

   BUG:  Need to investigate what happens if length would take
   us off the end of what has been written -- should only read
   until the end.

---------------------------------------------------------------------------*/
int32
HXPread(accrec_t * access_rec, int32 length, void * data)
{
    CONSTR(FUNC, "HXPread");    /* for HERROR */
    extinfo_t  *info =          /* information on the special element */
    (extinfo_t *) access_rec->special_info;
    int32    ret_value = SUCCEED;

    /* validate length */
    if (length < 0)
        HGOTO_ERROR(DFE_RANGE, FAIL);

    /* adjust length if it falls off the end of the element */
    if ((length == 0) || (access_rec->posn + length > info->length))
        length = info->length - access_rec->posn;
    else if (length < 0)
        HGOTO_ERROR(DFE_RANGE, FAIL);

    /* see if the file is open, if not open it */
    if (!info->file_open)
      {
        char	*fname;

        /* build the customized external file name. */
        if ((fname = HXIbuildfilename(info->extern_file_name, DFACC_OLD))==NULL)
            HGOTO_ERROR(DFE_BADOPEN, FAIL);

        info->file_external = (hdf_file_t)HI_OPEN(fname, access_rec->access);
        HDfree(fname);
        if (OPENERR(info->file_external))
          {
            HERROR(DFE_BADOPEN);
            HEreport("Could not find external file %s\n", info->extern_file_name);
            HGOTO_DONE(FAIL);
          }
        info->file_open = TRUE;
      }

    /* read it in from the file */
      {
          if (HI_SEEK(info->file_external, access_rec->posn + info->extern_offset) == FAIL)
              HGOTO_ERROR(DFE_SEEKERROR, FAIL);
          if (HI_READ(info->file_external, data, length) == FAIL)
              HGOTO_ERROR(DFE_READERROR, FAIL);
      }

    /* adjust access position */
    access_rec->posn += length;

    ret_value = length;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}	/* HXPread */

/* ------------------------------ HXPwrite ------------------------------- */
/*
NAME
   HXPwrite -- write some data out to an external file
USAGE
   int32 HXPwrite(access_rec, length, data)
   access_t * access_rec;      IN: access record to mess with
   int32      length;          IN: number of bytes to read
   void *      data;            IN: buffer of data
RETURNS
   The number of bytes written or FAIL on error
DESCRIPTION
   Write out some data to an external file.  

   It looks like this will allow us to write to a file even if we only
   have a read AID for it.   Is that really the behavior that we want?

---------------------------------------------------------------------------*/
int32
HXPwrite(accrec_t * access_rec, int32 length, const void * data)
{
    uint8       local_ptbuf[4]; /* temp buffer */
    CONSTR(FUNC, "HXPwrite");   /* for HERROR */
    extinfo_t  *info =          /* information on the special element */
                    (extinfo_t *) (access_rec->special_info);
    uint8      *p =local_ptbuf; /* temp buffer ptr */
    filerec_t  *file_rec;       /* file record */
    int32      ret_value = SUCCEED;

    /* convert file id to file record */
    file_rec = HAatom_object(access_rec->file_id);

    /* validate length */
    if (length < 0)
        HGOTO_ERROR(DFE_RANGE, FAIL);

    /* see if the file is open, if not open it */
    if (!info->file_open)
    {
        char *fname;

        /* build the customized external file name. */
        if ((fname = HXIbuildfilename(info->extern_file_name, DFACC_OLD))==NULL)
            HGOTO_ERROR(DFE_BADOPEN, FAIL);

        info->file_external = (hdf_file_t)HI_OPEN(fname, access_rec->access);
        HDfree(fname);
        if (OPENERR(info->file_external))
          {
            HERROR(DFE_BADOPEN);
            HEreport("Could not find external file %s\n", info->extern_file_name);
            HGOTO_DONE(FAIL);
          }
        info->file_open = TRUE;
    }

    /* write the data onto file */
      {
          if (HI_SEEK(info->file_external, access_rec->posn + info->extern_offset) == FAIL)
              HGOTO_ERROR(DFE_SEEKERROR, FAIL);
          if (HI_WRITE(info->file_external, data, length) == FAIL)
            {
            /* this external file might not be opened with write permission,
               reopen the file and try again */
                hdf_file_t  f = (hdf_file_t)HI_OPEN(info->extern_file_name, DFACC_WRITE);
                
                if (OPENERR(f) ||
                    HI_SEEK(f, access_rec->posn + info->extern_offset) == FAIL ||
                    HI_WRITE(f, data, length) == FAIL)
                  {
                    HI_CLOSE(f);
                    HGOTO_ERROR(DFE_DENIED, FAIL);
                  }
                HI_CLOSE(info->file_external);

                /* if okay, substitute the file descriptor */
                info->file_external = f;
            }
      }

    /* update access record, and information about special elelemt */
    access_rec->posn += length;
    if (access_rec->posn > info->length)
      {
          int32       data_off;		/* offset of the data we are checking */
          info->length = access_rec->posn;
          INT32ENCODE(p, info->length);

          /* Get the data's offset & length */
          if(HTPinquire(access_rec->ddid,NULL,NULL,&data_off,NULL)==FAIL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);
          if (HPseek(file_rec, data_off + 2) == FAIL)
              HGOTO_ERROR(DFE_SEEKERROR, FAIL);
          if (HP_write(file_rec, local_ptbuf, 4) == FAIL)
              HGOTO_ERROR(DFE_WRITEERROR, FAIL);
      }

    ret_value = length;    /* return length of bytes written */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value; 
}	/* HXPwrite */

/* ------------------------------ HXPinquire ------------------------------ */
/*
NAME
   HXPinquire -- retreive information about an external element
USAGE
   int32 HXPinquire(access_rec, file, tag, ref, len, off, pos, acc, sp)
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
   Return interesting information about an external element.
   NULL can be passed for any of the OUT parameters if their
   value is not needed.
   BUG: The offset returned is not correct.

---------------------------------------------------------------------------*/
int32
HXPinquire(accrec_t * access_rec, int32 *pfile_id, uint16 *ptag,
           uint16 *pref, int32 *plength, int32 *poffset,
           int32 *pposn, int16 *paccess, int16 *pspecial)
{
    CONSTR(FUNC, "HXPinquire");   /* for HERROR */
    extinfo_t  *info =          /* special information record */
    (extinfo_t *) access_rec->special_info;
    uint16 data_tag,data_ref;   /* tag/ref of the data we are checking */
    int32    ret_value = SUCCEED;

    /* Get the data's offset & length */
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
        *poffset = 0;   /* meaningless -- actually not anymore */
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
}	/* HXPinquire */

/* ----------------------------- HXPendaccess ----------------------------- */
/*
NAME
   HXPendacess -- close file, free AID
USAGE
   intn HXPendaccess(access_rec)
   access_t * access_rec;      IN:  access record to close
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Close the file pointed to by the current AID and free the AID

---------------------------------------------------------------------------*/
intn
HXPendaccess(accrec_t * access_rec)
{
    CONSTR(FUNC, "HXPendaccess");   /* for HERROR */
    filerec_t  *file_rec;           /* file record */
    intn     ret_value = SUCCEED;

    /* convert file id to file record */
    file_rec = HAatom_object(access_rec->file_id);

    /* close the file pointed to by this access rec */
    HXPcloseAID(access_rec);

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
      if(access_rec!=NULL)
          HIrelease_accrec_node(access_rec);

    } /* end if */

  /* Normal function cleanup */

  return ret_value; 
}	/* HXPendaccess */

/* ----------------------------- HXPcloseAID ------------------------------ */
/*
NAME
   HXPcloseAID -- close file but keep AID active
USAGE
   int32 HXPcloseAID(access_rec)
   access_t * access_rec;      IN:  access record of file to close
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   close the file currently being pointed to by this AID but 
   do *NOT* free the AID.

   This is called by Hnextread() which reuses an AID to point to
   the 'next' object as requested.  If the current object was an
   external object, the external file needs to be closed before all
   reference to it is lost.

---------------------------------------------------------------------------*/
int32
HXPcloseAID(accrec_t * access_rec)
{
#ifdef LATER
    CONSTR(FUNC, "HXPcloseAID");    /* for HERROR */
#endif /* LATER */
    extinfo_t  *info =          /* special information record */
    (extinfo_t *) access_rec->special_info;
    int32      ret_value = SUCCEED;

    /* detach the special information record.
       If no more references to that, free the record */

    if (--(info->attached) == 0)
      {
          if (info->file_open)
              HI_CLOSE(info->file_external);
          HDfree(info->extern_file_name);
          HDfree(info);
          access_rec->special_info=NULL;
      }

#ifdef LATER
done:
#endif /* LATER */
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

    return ret_value;
}   /* HXPcloseAID */

/* ------------------------------- HXPinfo -------------------------------- */
/*
NAME
   HXPinfo -- return info about an external element
USAGE
   int32 HXPinfo(access_rec, info_block)
   accrec_t        * access_rec; IN: access record of element
   sp_info_block_t * info_block; OUT: information about the special element 
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Return information about the given external element.  Info_block is
   assumed to be non-NULL.  Do not make a copy of the path, just have
   the info_block copy point to our local copy.

   --------------------------------------------------------------------------- */
int32
HXPinfo(accrec_t * access_rec, sp_info_block_t * info_block)
{
    CONSTR(FUNC, "HXPinfo");    /* for HERROR */
    extinfo_t  *info =          /* special information record */
    (extinfo_t *) access_rec->special_info;
    int32      ret_value = SUCCEED;

    /* validate access record */
    if (access_rec->special != SPECIAL_EXT)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* fill in the info_block */
    info_block->key = SPECIAL_EXT;

    info_block->offset = info->extern_offset;
    info_block->path = info->extern_file_name;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value; 
}   /* HXPinfo */

/* ------------------------------- HXPreset ------------------------------- */
/*
NAME
   HXPreset -- replace the current external info with new info
USAGE
   int32 HXPreset(access_rec, info_block)
   accrec_t        * access_rec;   IN: access record of element
   sp_info_block_t * info_block;   IN: information about the special element 
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Reset information about the given external element.  Info_block is
   assumed to be non-NULL.

   Basically, what this routine does is throw out the old file
   information for a special element and replaces it with a new
   file name.  This is useful for when a file has changed places.
   The offset and length are assumed to be the same.

---------------------------------------------------------------------------*/
int32
HXPreset(accrec_t * access_rec, sp_info_block_t * info_block)
{
    CONSTR(FUNC, "HXPreset");    /* for HERROR */
    filerec_t  *file_rec;       /* file record */
    uint8       local_ptbuf[14 + MAX_PATH_LEN];     /* temp buffer */
    extinfo_t  *info =          /* special information record */
    (extinfo_t *) access_rec->special_info;
    int32 new_len, new_off;     /* new length & offset of the special info */
    int32      ret_value = SUCCEED;

    /* validate access record -- make sure is already external element */
    if (access_rec->special != SPECIAL_EXT)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* just replace with other external element info for now  */
    /* (i.e., this can not change the type of special element */
    if (info_block->key != SPECIAL_EXT)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* check validity of file record */
    file_rec = HAatom_object(access_rec->file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* update our internal pointers */
    info->extern_offset = info_block->offset;
    info->extern_file_name = (char *) HDstrdup(info_block->path);
    if (!info->extern_file_name)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);
    info->length_file_name = (int32)HDstrlen(info->extern_file_name);

    /*
     * delete the existing tag / ref object
     * accomplish this by changing the offset and length of the existing
     *  special element DD and writing it in a new place
     */
    new_len=14+info->length_file_name;
    if ((new_off=HPgetdiskblock(file_rec, new_len, TRUE)) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* write the new external file record */
    {
        uint8      *p = local_ptbuf;
        INT16ENCODE(p, SPECIAL_EXT);
        INT32ENCODE(p, info->length);
        INT32ENCODE(p, info->extern_offset);
        INT32ENCODE(p, info->length_file_name);
        HDstrcpy((char *) p, (char *) info->extern_file_name);
    }

    /* write out the new external file record */
    if (HP_write(file_rec, local_ptbuf, new_len) == FAIL)
        HGOTO_ERROR(DFE_WRITEERROR, FAIL);

    /* update the DD block in the file */
    if (HTPupdate(access_rec->ddid, new_off, new_len) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value; 
}	/* HXPreset */

static	char*	extcreatedir = NULL;
static	char*	HDFEXTCREATEDIR = NULL;
static	char*	extdir = NULL;
static	char*	HDFEXTDIR = NULL;

/*------------------------------------------------------------------------ 
NAME
   HXsetcreatedir -- set the directory variable for creating external file
USAGE
   intn HXsetcreatedir(dir)
   const char *dir		IN: directory for creating external file
RETURNS
   SUCCEED if no error, else FAIL
DESCRIPTION
   Set up the directory variable for creating external file.
   The directory content is copied into HXsetcreatedir area.
   If dir is NULL, the directory variable is unset.
   If error encountered during setup, previous value of createdir
   is not changed.

FORTRAN
   hxscdir

--------------------------------------------------------------------------*/
intn
HXsetcreatedir(const char *dir)
{
    CONSTR(FUNC, "HXsetcreatedir");
  char	*pt;
  intn       ret_value = SUCCEED;


  if (dir)
    {
      if (!(pt = HDstrdup(dir)))
        HGOTO_ERROR(DFE_NOSPACE, FAIL);
    }
  else
    pt = NULL;		/* will reset extcreatedir to NULL */

  if (extcreatedir)
    HDfree(extcreatedir);
    
  extcreatedir = pt;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value; 
}	/* HXsetcreatedir */

/*------------------------------------------------------------------------ 
NAME
   HXsetdir -- set the directory variable for locating external file
USAGE
   intn HXsetdir(dir)
   const char *dir		IN: directory for locating external file
RETURNS
   SUCCEED if no error, else FAIL
DESCRIPTION
   Set up the directory variable for locating external file.
   It can contain multiple directories separated by colons.
   The directory content is copied into HXsetdir area.
   If dir is NULL, the directory variable is unset.
   If error encountered during setup, previous value of extdir
   is not changed.

FORTRAN
   hxsdir

--------------------------------------------------------------------------*/
intn
HXsetdir(const char *dir)
{
    CONSTR(FUNC, "HXsetdir");
  char	*pt;
  intn   ret_value = SUCCEED;

  if (dir)
    {
      if (!(pt = HDstrdup(dir)))
        HGOTO_ERROR(DFE_NOSPACE, FAIL);
    }
  else
    pt = NULL;		/* will reset extdir to NULL */

  if (extdir)
    HDfree(extdir);
    
  extdir = pt;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value; 
}	/* HXsetdir */

/* ------------------------------- HXIbuildfilename ------------------------------- */
/*
NAME
    HXIbuildfilename -- Build the Filename for the External Element
USAGE
    char* HXIbuildfilename(char *ext_fname, const intn acc_mode)
    char            * ext_fname;	IN: external filename as stored
    intn 	      acc_mode;		IN: access mode
RETURNS
    finalpath / NULL
DESCRIPTION
    Compose the external object file name.
    [More detail later.]

---------------------------------------------------------------------------*/
/* the following can be sped up by doing my own copying instead of scanning */
/* for end-of-line two extra times, or even use memcpy since the string lengths */
/* are calculated already.  For now, it works. */
#define HDstrcpy3(s1, s2, s3, s4)	(HDstrcat(HDstrcat(HDstrcpy(s1, s2),s3),s4))
#define HDstrcpy4(s1, s2, s3, s4, s5)	(HDstrcat(HDstrcat(HDstrcat(HDstrcpy(s1, s2),s3),s4),s5))

PRIVATE
char *
HXIbuildfilename(const char *ext_fname, const intn acc_mode)
{
    CONSTR(FUNC, "HXIbuildfilename");
    int	        fname_len;		/* string length of the ext_fname */
    int	        path_len;		/* string length of prepend pathname */
    static int	firstinvoked = 1;	/* true if invoked the first time */

    char	*finalpath = NULL;	/* Final pathname to return */
    const char	*fname = NULL;
#if !(defined (MAC) || defined (macintosh) || defined (SYMANTEC_C))
    struct	stat filestat;	/* for checking pathname existence */
#endif
    char        *ret_value = NULL; /* FAIL */

    /* initialize HDFEXTDIR and HDFCREATEDIR if invoked the first time */
    if (firstinvoked){
        firstinvoked = 0;
        HDFEXTCREATEDIR = HDgetenv("HDFEXTCREATEDIR");
        HDFEXTDIR = HDgetenv("HDFEXTDIR");
    }

    if (!ext_fname)
        HGOTO_ERROR(DFE_ARGS, NULL);
    fname = ext_fname;

    /* get the space for the final pathname */
    if (!(finalpath=HDmalloc(MAX_PATH_LEN)))
        HGOTO_ERROR(DFE_NOSPACE, NULL);

    fname_len = (int)HDstrlen(fname);
    
    switch (acc_mode){
    case DFACC_CREATE: {			/* Creating a new external element */
#ifndef macintosh
        if ( *fname == DIR_SEPC ) {	/* Absolute Pathname */
            ret_value = (HDstrcpy(finalpath, fname));
            goto done;
        }
        else {				/* Relative Pathname */

            /* try function variable */
            if (extcreatedir) {
                path_len = (int)HDstrlen(extcreatedir);

                if (fname_len + 1 + path_len + 1 > MAX_PATH_LEN )
                    HGOTO_ERROR(DFE_NOSPACE, NULL);
                ret_value = (HDstrcpy3(finalpath, extcreatedir, DIR_SEPS, fname));
                goto done;
            }

            /* try Envrironment Variable */
            if (HDFEXTCREATEDIR) {
                path_len = (int)HDstrlen(HDFEXTCREATEDIR);

                if (fname_len + 1 + path_len + 1 > MAX_PATH_LEN )
                    HGOTO_ERROR(DFE_NOSPACE, NULL);

                ret_value = (HDstrcpy3(finalpath, HDFEXTCREATEDIR, DIR_SEPS, fname));
                goto done;
            }

            /* try Head File Directory */
            /* Don't have Head File information now.  Continue */

            /* Just return the ext_fname */
            ret_value = (HDstrcpy(finalpath, fname));
            goto done;
        }
#else		/* macintosh */

		/* Absolute Pathname */
		char* dirptr;
        if ( ( *fname != DIR_SEPC ) && ((dirptr = strchr(fname, DIR_SEPC)) != NULL) )	 {	
        	/* This may seem a bit of overkill, but without it we can't determine if the mac path   */
        	/* (testdir:test.hdf) is absolute or relative. For example, myHD:testdir:test.hdf would */
        	/* be absolute. How do you tell without checking it out?                                */
        	FSSpec tmpSpec;
        	Str255 VolName;
        	memset(VolName, 0, sizeof(Str255));
        	strncpy((char*)VolName, fname, dirptr-fname+1);
        	c2pstr((char*)VolName);
        	/* verify that the first item is a good volume name*/
        	if (FSMakeFSSpec(0,0,VolName, &tmpSpec) == noErr)		{
	            ret_value = (HDstrcpy(finalpath, fname));
	            goto done;
	        }
	    }
        
        /* Relative Pathname */

        /* try function variable */
        if (extcreatedir) {
            path_len = (int)HDstrlen(extcreatedir);

            if (1 + fname_len + 1 + path_len + 1 > MAX_PATH_LEN )
                HGOTO_ERROR(DFE_NOSPACE, NULL);

            ret_value = (HDstrcpy4(finalpath, DIR_SEPS, extcreatedir, DIR_SEPS, fname));

            goto done;
        }

        /* try Envrironment Variable */
        if (HDFEXTCREATEDIR) {
            path_len = (int)HDstrlen(HDFEXTCREATEDIR);

            if (1 + fname_len + 1 + path_len + 1 > MAX_PATH_LEN )
                HGOTO_ERROR(DFE_NOSPACE, NULL);

            ret_value = (HDstrcpy4(finalpath, DIR_SEPS, HDFEXTCREATEDIR, DIR_SEPS, fname));
             goto done;
        }

        /* try Head File Directory */
        /* Don't have Head File information now.  Continue */

        /* Just return the ext_fname */
        ret_value = (HDstrcpy(finalpath, fname));

        goto done;

#endif		/* macintosh */
        /* break; */
    } /*DFACC_CREATE */
    case DFACC_OLD:{			/* Locating an old external element */
#ifndef macintosh
        if ( *fname == DIR_SEPC ) {	/* Absolute Pathname */
#else
		char* dirptr;
        if ( ( *fname != DIR_SEPC ) && ((dirptr = strchr(fname, DIR_SEPC)) != NULL) )	 {	
#endif
            if (HDstat(fname, &filestat) == 0){
                ret_value = (HDstrcpy(finalpath, fname));
                goto done;
            }
#ifndef macintosh
            else if (!extdir && !HDFEXTDIR) {
                HGOTO_ERROR(DFE_FNF, NULL);
            }
            /* strip the pathname component */
            fname = HDstrrchr(fname, DIR_SEPC) + 1;
            fname_len = (int)HDstrlen(fname);
#else
			/* don't do this on the mac... it could be a relative path. */
#endif
            /* continue to Relative Pathname */
        }


        /* Relative Pathname */
        {
            char   *dir_pt, *path_pt;	/* temporary pointers */

            /* try function variable */
            if (extdir) {
                dir_pt = extdir;
                while (*dir_pt){
                    /* extract one extdir component to finalpath */
                    path_len = 0;
                    path_pt = finalpath;
                    while (*dir_pt && *dir_pt != DIR_PATH_SEPC){
                        if (path_len >= MAX_PATH_LEN)
                            HGOTO_ERROR(DFE_NOSPACE, NULL);

                        *path_pt++ = *dir_pt++;
                        path_len++;
                    }
                    if (*dir_pt == DIR_PATH_SEPC) dir_pt++;
                    *path_pt++ = DIR_SEPC;
                    path_len++;

                    if (fname_len + path_len + 1 > MAX_PATH_LEN )
                        HGOTO_ERROR(DFE_NOSPACE, NULL);

                    HDstrcpy(path_pt, fname);
                    if (HDstat(finalpath, &filestat) == 0 ){
                        ret_value = finalpath;
                        goto done;
                    }
                }
            }

            /* try Envrironment Variable */
            if (HDFEXTDIR) {
                dir_pt = HDFEXTDIR;
                while (*dir_pt){
                    /* extract one HDFEXTDIR component to finalpath */
                    path_len = 0;
                    path_pt = finalpath;
                    while (*dir_pt && *dir_pt != DIR_PATH_SEPC){
                        if (path_len >= MAX_PATH_LEN)
                            HGOTO_ERROR(DFE_NOSPACE, NULL);

                        *path_pt++ = *dir_pt++;
                        path_len++;
                    }
                    if (*dir_pt == DIR_PATH_SEPC) dir_pt++;
                    *path_pt++ = DIR_SEPC;
                    path_len++;

                    if (fname_len + path_len + 1 > MAX_PATH_LEN )
                        HGOTO_ERROR(DFE_NOSPACE, NULL);

                    HDstrcpy(path_pt, fname);
                    if (HDstat(finalpath, &filestat) == 0 ){
                        ret_value = finalpath;
                        goto done;
                    }
                }
            }

            /* try Head File Directory */
            /* Don't have Head File information now.  Continue */

            /* See if the file exists */
            if (HDstat(fname, &filestat) == 0 )
              {
                  ret_value = (HDstrcpy(finalpath, fname));
                  goto done;
              }

            /* All have failed */
            ret_value = NULL;
            goto done;
        }
        /* break; */
    } /* DFACC_OLD */
    default:
        HDfree(finalpath);
        HGOTO_ERROR(DFE_ARGS, NULL);
    }

  done:
    if(ret_value == NULL)   
      { /* Error condition cleanup */
          if (finalpath != NULL)
              HDfree(finalpath); /* free this */
      } /* end if */

    /* Normal function cleanup */

    return ret_value; 
}	/* HXIbuildfilename */

/*------------------------------------------------------------------------ 
NAME
   HXPshutdown -- free any memory buffers we've allocated
USAGE
   intn HXPshutdown()
RETURNS
   SUCCEED/FAIL
DESCRIPTION
    Free buffers we've allocated during the execution of the program.

--------------------------------------------------------------------------*/
intn
HXPshutdown(void)
{
    if(extcreatedir!=NULL)
      {
          HDfree(extcreatedir);
          extcreatedir=NULL;
      } /* end if */
    if(HDFEXTCREATEDIR!=NULL)
      {
          HDFEXTCREATEDIR=NULL;
      } /* end if */
    if(extdir!=NULL)
      {
          HDfree(extdir);
          extdir=NULL;
      } /* end if */
    if(HDFEXTDIR!=NULL)
      {
          HDFEXTDIR=NULL;
      } /* end if */
    return(SUCCEED);
} /* end HXPshutdown() */
