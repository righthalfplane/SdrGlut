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

/* $Id: crle.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*
   FILE
   crle.c
   HDF run-length encoding I/O routines

   REMARKS

   DESIGN

   EXPORTED ROUTINES
   None of these routines are designed to be called by other users except
   for the modeling layer of the compression routines.

   AUTHOR
   Quincey Koziol

   MODIFICATION HISTORY
   9/28/93     Starting writing specs & coding prototype.
   10/09/93    Finished testing.  First version done.
 */

/* General HDF includes */
#include "hdf.h"

#define CRLE_MASTER
#define CODER_CLIENT
/* HDF compression includes */
#include "hcompi.h"     /* Internal definitions for compression */

/* internal defines */
#define TMP_BUF_SIZE    8192    /* size of throw-away buffer */
#define RUN_MASK        0x80    /* bit mask for run-length control bytes */
#define COUNT_MASK      0x7f    /* bit mask for count of run or mix */

/* #define TESTING */

/* declaration of the functions provided in this module */
PRIVATE int32 HCIcrle_staccess
            (accrec_t * access_rec, int16 acc_mode);

PRIVATE int32 HCIcrle_init
            (accrec_t * access_rec);

PRIVATE int32 HCIcrle_decode
            (compinfo_t * info, int32 length, uint8 *buf);

PRIVATE int32 HCIcrle_encode
            (compinfo_t * info, int32 length, const uint8 *buf);

PRIVATE int32 HCIcrle_term
            (compinfo_t * info);

/*--------------------------------------------------------------------------
 NAME
    HCIcrle_init -- Initialize a RLE compressed data element.

 USAGE
    int32 HCIcrle_init(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called by HCIcrle_staccess and HCIcrle_seek

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcrle_init(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCIcrle_init");
    compinfo_t *info;           /* special element information */
    comp_coder_rle_info_t *rle_info;    /* ptr to RLE info */

    info = (compinfo_t *) access_rec->special_info;
    if (Hseek(info->aid, 0, DF_START) == FAIL)  /* seek to beginning of element */
        HRETURN_ERROR(DFE_SEEKERROR, FAIL);

    rle_info = &(info->cinfo.coder_info.rle_info);

    /* Initialize RLE state information */
    rle_info->rle_state = RLE_INIT;     /* start in initial state */
    rle_info->buf_pos = 0;  /* start at the beginning of the buffer */
    rle_info->last_byte = (uintn) RLE_NIL;  /* start with no code in the last byte */
    rle_info->second_byte = (uintn) RLE_NIL;    /* start with no code here too */
    rle_info->offset = 0;   /* offset into the file */

    return (SUCCEED);
}   /* end HCIcrle_init() */

/*--------------------------------------------------------------------------
 NAME
    HCIcrle_decode -- Decode RLE compressed data into a buffer.

 USAGE
    int32 HCIcrle_decode(info,length,buf)
    compinfo_t *info;   IN: the info about the compressed element
    int32 length;       IN: number of bytes to read into the buffer
    uint8 *buf;         OUT: buffer to store the bytes read

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called to decode RLE data from the file.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcrle_decode(compinfo_t * info, int32 length, uint8 *buf)
{
    CONSTR(FUNC, "HCIcrle_decode");
    comp_coder_rle_info_t *rle_info;    /* ptr to RLE info */
    int32       orig_length;    /* original length to write */
    uintn       dec_len;        /* length to decode */
    intn        c;              /* character to hold a byte read in */

    rle_info = &(info->cinfo.coder_info.rle_info);

    orig_length = length;   /* save this for later */
    while (length > 0)
      {     /* decode until we have all the bytes we need */
          if (rle_info->rle_state == RLE_INIT)
            {   /* need to figure out RUN or MIX state */
                if ((c = HDgetc(info->aid)) == FAIL)
                    HRETURN_ERROR(DFE_READERROR, FAIL);
                if (c & RUN_MASK)
                  {     /* run byte */
                      rle_info->rle_state = RLE_RUN;    /* set to run state */
                      rle_info->buf_length = (c & COUNT_MASK) + RLE_MIN_RUN;    /* run length */
                      if ((rle_info->last_byte = (uintn)HDgetc(info->aid)) == (uintn)FAIL)
                          HRETURN_ERROR(DFE_READERROR, FAIL);
                  }     /* end if */
                else
                  {     /* mix byte */
                      rle_info->rle_state = RLE_MIX;    /* set to mix state */
                      rle_info->buf_length = (c & COUNT_MASK) + RLE_MIN_MIX;    /* mix length */
                      if (Hread(info->aid, rle_info->buf_length, rle_info->buffer) == FAIL)
                          HRETURN_ERROR(DFE_READERROR, FAIL);
                      rle_info->buf_pos = 0;
                  }     /* end else */
            }   /* end if */

            /* RUN or MIX states */
            if (length > rle_info->buf_length)  /* still need more data */
                dec_len = (uintn)rle_info->buf_length;
            else    /* only grab "length" bytes */
                dec_len = (uintn) length;

            if (rle_info->rle_state == RLE_RUN)
                HDmemset(buf, rle_info->last_byte, dec_len);    /* copy the run */
            else
              {
                  HDmemcpy(buf, &(rle_info->buffer[rle_info->buf_pos]), dec_len);
                  rle_info->buf_pos += (intn)dec_len;
              }     /* end else */

            rle_info->buf_length -= (intn)dec_len;
            if (rle_info->buf_length <= 0)  /* check for running out of bytes */
                rle_info->rle_state = RLE_INIT;     /* get the next status byte */
            length -= (int32)dec_len;  /* decrement the bytes to get */
            buf += dec_len;     /* in case we need more bytes */
      }     /* end while */

    rle_info->offset += orig_length;    /* incr. abs. offset into the file */
    return (SUCCEED);
}   /* end HCIcrle_decode() */

/*--------------------------------------------------------------------------
 NAME
    HCIcrle_encode -- Encode data from a buffer into RLE compressed data

 USAGE
    int32 HCIcrle_encode(info,length,buf)
    compinfo_t *info;   IN: the info about the compressed element
    int32 length;       IN: number of bytes to store from the buffer
    const uint8 *buf;         OUT: buffer to get the bytes from

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called to encode RLE data into a file.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcrle_encode(compinfo_t * info, int32 length, const uint8 *buf)
{
    CONSTR(FUNC, "HCIcrle_encode");
    comp_coder_rle_info_t *rle_info;    /* ptr to RLE info */
    int32       orig_length;    /* original length to write */
    intn        c;              /* character to hold a byte read in */

    rle_info = &(info->cinfo.coder_info.rle_info);

    orig_length = length;   /* save this for later */
    while (length > 0)
      {     /* encode until we stored all the bytes */
          switch (rle_info->rle_state)
            {
                case RLE_INIT:      /* initial encoding state */
                    rle_info->rle_state = RLE_MIX;  /* shift to MIX state */
                    rle_info->last_byte = (uintn)(rle_info->buffer[0] = *buf);
                    rle_info->buf_length = 1;
                    rle_info->buf_pos = 1;
                    buf++;
                    length--;
                    break;

                case RLE_RUN:
                    /* check for end of run */
                    if ((uintn)*buf != rle_info->last_byte)
                      {
                          rle_info->rle_state = RLE_MIX;
                          c = RUN_MASK | (rle_info->buf_length - RLE_MIN_RUN);
                          if (HDputc((uint8) c, info->aid) == FAIL)
                              HRETURN_ERROR(DFE_WRITEERROR, FAIL);
                          if (HDputc((uint8) rle_info->last_byte, info->aid) == FAIL)
                              HRETURN_ERROR(DFE_WRITEERROR, FAIL);
                          rle_info->last_byte = (uintn)(rle_info->buffer[0] = *buf);
                          rle_info->buf_length = 1;
                          rle_info->buf_pos = 1;
                      }     /* end if */
                    else
                      {     /* run is continuing */
                          rle_info->buf_length++;
                          if (rle_info->buf_length >= RLE_MAX_RUN)
                            {   /* check for too long */
                                c = RUN_MASK | (rle_info->buf_length - RLE_MIN_RUN);
                                if (HDputc((uint8) c, info->aid) == FAIL)
                                    HRETURN_ERROR(DFE_WRITEERROR, FAIL);
                                if (HDputc((uint8) rle_info->last_byte, info->aid) == FAIL)
                                    HRETURN_ERROR(DFE_WRITEERROR, FAIL);
                                rle_info->rle_state = RLE_INIT;
                                rle_info->second_byte = rle_info->last_byte = (uintn) RLE_NIL;
                            }   /* end if */
                      }     /* end else */
                    buf++;
                    length--;
                    break;

                case RLE_MIX:   /* mixed bunch of bytes */
                    /* check for run */
                    if ((uintn)*buf == rle_info->last_byte && (uintn)*buf == rle_info->second_byte)
                      {
                          rle_info->rle_state = RLE_RUN;    /* shift to RUN state */
                          if (rle_info->buf_length > (RLE_MIN_RUN - 1))
                            {   /* check for mixed data to write */
                                if (HDputc((uint8) ((rle_info->buf_length - RLE_MIN_MIX) - (RLE_MIN_RUN - 1)), info->aid) == FAIL)
                                    HRETURN_ERROR(DFE_WRITEERROR, FAIL);
                                if (Hwrite(info->aid, (rle_info->buf_length - (RLE_MIN_RUN - 1)), rle_info->buffer) == FAIL)
                                    HRETURN_ERROR(DFE_WRITEERROR, FAIL);
                            }   /* end if */
                          rle_info->buf_length = RLE_MIN_RUN;
                      }     /* end if */
                    else
                      {     /* continue MIX */
                          rle_info->second_byte = rle_info->last_byte;
                          rle_info->last_byte = (uintn)(rle_info->buffer[rle_info->buf_pos] = *buf);
                          rle_info->buf_length++;
                          rle_info->buf_pos++;
                          if (rle_info->buf_length >= RLE_BUF_SIZE)
                            {   /* check for too long */
                                if (HDputc((uint8) (rle_info->buf_length - RLE_MIN_MIX), info->aid) == FAIL)
                                    HRETURN_ERROR(DFE_WRITEERROR, FAIL);
                                if (Hwrite(info->aid, rle_info->buf_length, rle_info->buffer) == FAIL)
                                    HRETURN_ERROR(DFE_WRITEERROR, FAIL);
                                rle_info->rle_state = RLE_INIT;
                                rle_info->second_byte = rle_info->last_byte = (uintn) RLE_NIL;
                            }   /* end if */
                      }     /* end else */
                    buf++;
                    length--;
                    break;

                default:
                    HRETURN_ERROR(DFE_INTERNAL, FAIL)
            }   /* end switch */
      }     /* end while */

    rle_info->offset += orig_length;    /* incr. abs. offset into the file */
    return (SUCCEED);
}   /* end HCIcrle_encode() */

/*--------------------------------------------------------------------------
 NAME
    HCIcrle_term -- Flush encoded data from internal buffer to RLE compressed data

 USAGE
    int32 HCIcrle_term(info)
    compinfo_t *info;   IN: the info about the compressed element

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called to flush RLE data into a file.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcrle_term(compinfo_t * info)
{
    CONSTR(FUNC, "HCIcrle_term");
    comp_coder_rle_info_t *rle_info;    /* ptr to RLE info */
    intn        c;              /* character to hold a byte read in */

    rle_info = &(info->cinfo.coder_info.rle_info);

    switch (rle_info->rle_state)
      {
          case RLE_RUN:
              c = RUN_MASK | (rle_info->buf_length - RLE_MIN_RUN);
              if (HDputc((uint8) c, info->aid) == FAIL)
                  HRETURN_ERROR(DFE_WRITEERROR, FAIL);
              if (HDputc((uint8) rle_info->last_byte, info->aid) == FAIL)
                  HRETURN_ERROR(DFE_WRITEERROR, FAIL);
              break;

          case RLE_MIX: /* mixed bunch of bytes */
              if (HDputc((uint8) ((rle_info->buf_length - RLE_MIN_MIX)), info->aid) == FAIL)
                  HRETURN_ERROR(DFE_WRITEERROR, FAIL);
              if (Hwrite(info->aid, rle_info->buf_length, rle_info->buffer) == FAIL)
                  HRETURN_ERROR(DFE_WRITEERROR, FAIL);
              break;

          default:
              HRETURN_ERROR(DFE_INTERNAL, FAIL)
      }     /* end switch */
    rle_info->rle_state = RLE_INIT;
    rle_info->second_byte = rle_info->last_byte = (uintn) RLE_NIL;

    return (SUCCEED);
}   /* end HCIcrle_term() */

/*--------------------------------------------------------------------------
 NAME
    HCIcrle_staccess -- Start accessing a RLE compressed data element.

 USAGE
    int32 HCIcrle_staccess(access_rec, access)
    accrec_t *access_rec;   IN: the access record of the data element
    int16 access;           IN: the type of access wanted

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called by HCIcrle_stread and HCIcrle_stwrite

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcrle_staccess(accrec_t * access_rec, int16 acc_mode)
{
    CONSTR(FUNC, "HCIcrle_staccess");
    compinfo_t *info;           /* special element information */

    info = (compinfo_t *) access_rec->special_info;

#ifdef OLD_WAY
    if (acc_mode == DFACC_READ)
        info->aid = Hstartread(access_rec->file_id, DFTAG_COMPRESSED,
                               info->comp_ref);
    else
        info->aid = Hstartwrite(access_rec->file_id, DFTAG_COMPRESSED,
                                info->comp_ref, info->length);

    if (info->aid == FAIL)
        HRETURN_ERROR(DFE_DENIED, FAIL);
    if ((acc_mode&DFACC_WRITE) && Happendable(info->aid) == FAIL)
        HRETURN_ERROR(DFE_DENIED, FAIL);
#else /* OLD_WAY */
    if (acc_mode == DFACC_READ)
        info->aid = Hstartread(access_rec->file_id, DFTAG_COMPRESSED,
                               info->comp_ref);
    else
        info->aid = Hstartaccess(access_rec->file_id, DFTAG_COMPRESSED,
                                info->comp_ref, DFACC_RDWR|DFACC_APPENDABLE);

    if (info->aid == FAIL)
        HRETURN_ERROR(DFE_DENIED, FAIL);
#endif /* OLD_WAY */
    return (HCIcrle_init(access_rec));  /* initialize the RLE info */
}   /* end HCIcrle_staccess() */

/*--------------------------------------------------------------------------
 NAME
    HCPcrle_stread -- start read access for compressed file

 USAGE
    int32 HCPcrle_stread(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Start read access on a compressed data element using a simple RLE scheme.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcrle_stread(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPcrle_stread");
    int32       ret;

    if ((ret = HCIcrle_staccess(access_rec, DFACC_READ)) == FAIL)
        HRETURN_ERROR(DFE_CINIT, FAIL);
    return (ret);
}   /* HCPcrle_stread() */

/*--------------------------------------------------------------------------
 NAME
    HCPcrle_stwrite -- start write access for compressed file

 USAGE
    int32 HCPcrle_stwrite(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Start write access on a compressed data element using a simple RLE scheme.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcrle_stwrite(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPcrle_stwrite");
    int32       ret;

    if ((ret = HCIcrle_staccess(access_rec, DFACC_WRITE)) == FAIL)
        HRETURN_ERROR(DFE_CINIT, FAIL);
    return (ret);
}   /* HCPcrle_stwrite() */

/*--------------------------------------------------------------------------
 NAME
    HCPcrle_seek -- Seek to offset within the data element

 USAGE
    int32 HCPcrle_seek(access_rec,offset,origin)
    accrec_t *access_rec;   IN: the access record of the data element
    int32 offset;       IN: the offset in bytes from the origin specified
    intn origin;        IN: the origin to seek from [UNUSED!]

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Seek to a position with a compressed data element.  The 'origin'
    calculations have been taken care of at a higher level, it is an
    un-used parameter.  The 'offset' is used as an absolute offset
    because of this.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcrle_seek(accrec_t * access_rec, int32 offset, int origin)
{
    CONSTR(FUNC, "HCPcrle_seek");
    compinfo_t *info;           /* special element information */
    comp_coder_rle_info_t *rle_info;    /* ptr to RLE info */
    uint8      *tmp_buf;        /* pointer to throw-away buffer */

    /* shut compiler up */
    origin = origin;

    info = (compinfo_t *) access_rec->special_info;
    rle_info = &(info->cinfo.coder_info.rle_info);

    if (offset < rle_info->offset)
      {     /* need to seek from the beginning */
          if ((access_rec->access&DFACC_WRITE) && rle_info->rle_state != RLE_INIT)
              if (HCIcrle_term(info) == FAIL)
                  HRETURN_ERROR(DFE_CTERM, FAIL);
          if (HCIcrle_init(access_rec) == FAIL)
              HRETURN_ERROR(DFE_CINIT, FAIL);
      }     /* end if */

    if ((tmp_buf = (uint8 *) HDmalloc(TMP_BUF_SIZE)) == NULL)     /* get tmp buffer */
        HRETURN_ERROR(DFE_NOSPACE, FAIL);

    while (rle_info->offset + TMP_BUF_SIZE < offset)    /* grab chunks */
        if (HCIcrle_decode(info, TMP_BUF_SIZE, tmp_buf) == FAIL)
          {
              HDfree(tmp_buf);
              HRETURN_ERROR(DFE_CDECODE, FAIL)
          }     /* end if */
    if (rle_info->offset < offset)  /* grab the last chunk */
        if (HCIcrle_decode(info, offset - rle_info->offset, tmp_buf) == FAIL)
          {
              HDfree(tmp_buf);
              HRETURN_ERROR(DFE_CDECODE, FAIL)
          }     /* end if */

    HDfree(tmp_buf);
    return (SUCCEED);
}   /* HCPcrle_seek() */

/*--------------------------------------------------------------------------
 NAME
    HCPcrle_read -- Read in a portion of data from a compressed data element.

 USAGE
    int32 HCPcrle_read(access_rec,length,data)
    accrec_t *access_rec;   IN: the access record of the data element
    int32 length;           IN: the number of bytes to read
    void * data;             OUT: the buffer to place the bytes read

 RETURNS
    Returns the number of bytes read or FAIL

 DESCRIPTION
    Read in a number of bytes from a RLE compressed data element.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcrle_read(accrec_t * access_rec, int32 length, void * data)
{
    CONSTR(FUNC, "HCPcrle_read");
    compinfo_t *info;           /* special element information */

    info = (compinfo_t *) access_rec->special_info;

    if (HCIcrle_decode(info, length, data) == FAIL)
        HRETURN_ERROR(DFE_CDECODE, FAIL);

    return (length);
}   /* HCPcrle_read() */

/*--------------------------------------------------------------------------
 NAME
    HCPcrle_write -- Write out a portion of data from a compressed data element.

 USAGE
    int32 HCPcrle_write(access_rec,length,data)
    accrec_t *access_rec;   IN: the access record of the data element
    int32 length;           IN: the number of bytes to write
    void * data;             IN: the buffer to retrieve the bytes written

 RETURNS
    Returns the number of bytes written or FAIL

 DESCRIPTION
    Write out a number of bytes to a RLE compressed data element.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcrle_write(accrec_t * access_rec, int32 length, const void * data)
{
    CONSTR(FUNC, "HCPcrle_write");
    compinfo_t *info;           /* special element information */
    comp_coder_rle_info_t *rle_info;    /* ptr to RLE info */

    info = (compinfo_t *) access_rec->special_info;
    rle_info = &(info->cinfo.coder_info.rle_info);

    /* Don't allow random write in a dataset unless: */
    /*  1 - append onto the end */
    /*  2 - start at the beginning and rewrite (at least) the whole dataset */
    if ((info->length != rle_info->offset)
        && (rle_info->offset != 0 && length <= (info->length-rle_info->offset)))
        HRETURN_ERROR(DFE_UNSUPPORTED, FAIL);

    if (HCIcrle_encode(info, length, data) == FAIL)
        HRETURN_ERROR(DFE_CENCODE, FAIL);

    return (length);
}   /* HCPcrle_write() */

/*--------------------------------------------------------------------------
 NAME
    HCPcrle_inquire -- Inquire information about the access record and data element.

 USAGE
    int32 HCPcrle_inquire(access_rec,pfile_id,ptag,pref,plength,poffset,pposn,
            paccess,pspecial)
    accrec_t *access_rec;   IN: the access record of the data element
    int32 *pfile_id;        OUT: ptr to file id
    uint16 *ptag;           OUT: ptr to tag of information
    uint16 *pref;           OUT: ptr to ref of information
    int32 *plength;         OUT: ptr to length of data element
    int32 *poffset;         OUT: ptr to offset of data element
    int32 *pposn;           OUT: ptr to position of access in element
    int16 *paccess;         OUT: ptr to access mode
    int16 *pspecial;        OUT: ptr to special code

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Inquire information about the access record and data element.
    [Currently a NOP].

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcrle_inquire(accrec_t * access_rec, int32 *pfile_id, uint16 *ptag,
                uint16 *pref, int32 *plength, int32 *poffset,
                int32 *pposn, int16 *paccess, int16 *pspecial)
{
    /* shut compiler up */
    access_rec = access_rec;
    pfile_id = pfile_id;
    ptag = ptag;
    pref = pref;
    plength = plength;
    poffset = poffset;
    pposn = pposn;
    paccess = paccess;
    pspecial = pspecial;

    return (SUCCEED);
}   /* HCPcrle_inquire() */

/*--------------------------------------------------------------------------
 NAME
    HCPcrle_endaccess -- Close the compressed data element

 USAGE
    int32 HCPcrle_endaccess(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Close the compressed data element and free encoding info.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
HCPcrle_endaccess(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPcrle_endaccess");
    compinfo_t *info;           /* special element information */
    comp_coder_rle_info_t *rle_info;    /* ptr to RLE info */

    info = (compinfo_t *) access_rec->special_info;
    rle_info = &(info->cinfo.coder_info.rle_info);

    /* flush out RLE buffer */
    if ((access_rec->access&DFACC_WRITE) && rle_info->rle_state != RLE_INIT)
        if (HCIcrle_term(info) == FAIL)
            HRETURN_ERROR(DFE_CTERM, FAIL);

    /* close the compressed data AID */
    if (Hendaccess(info->aid) == FAIL)
        HRETURN_ERROR(DFE_CANTCLOSE, FAIL);

    return (SUCCEED);
}   /* HCPcrle_endaccess() */
