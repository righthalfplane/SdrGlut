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
static char RcsId[] = "@(#)$Revision: 5199 $";
#endif

/* $Id: hcomp.c 5199 2009-06-17 04:02:00Z bmribler $ */

/*
FILE
   hcomp.c
   HDF compressed data I/O routines

REMARKS
   These functions read and write compressed data to HDF data objects.
   The compressed data objects are implemented as "special tags"
   in the HDF file and the "H" layer I/O routines break out to the
   functions in this module to deal with them.

DESIGN
   The compression I/O functions are designed as state machines.
   There are two seperate state machines implemented, as layers
   on top of one another. 
      The top layer is the modeling layer, 
   whose purpose is to send/receive uncompressed bytes between the higher 
   layer (the "H" layer routines) and the lower layer, the coding layer.
   The modeling layer is constrained to look like Standard C
   I/O functions to the upper layer, while sending data in
   potentially unusual orders to the coding layer.  [An unusual
   order of bytes would be a Peano or Hilbert curve instead
   of the raster order more normally used].
      The lower layer is the coding layer, whose purpose is to
   send/receive bytes of data to the higher layer (the modeling
   layer) and to send/receive bits/bytes of data to the bit I/O
   functions after encoding them with some compression scheme.
   Both of these layers are designed as independent state
   machines whose state contains all the information to restart
   at a given point.  The purpose of this is to "cache" the
   state of each layer at certain [convenient] times in order
   to increase performance during random I/O.

BUGS/LIMITATIONS
   Currently the following design limitations are still in place:
   1 - Cannot compress an existing data element (will be fixed
       before release)  [ I think this is done, but it needs
       testing]

   2 - Statistic gathering from several types of compression
       is not implemented (should be fixed before release)
   3 - "State caching" for improved performance in not implemented,
       although some data-structures allow for it. (should be
       fixed before release)
   4 - Random writing in compressed data is not supported (unlikely
       to _ever_ be fixed)

EXPORTED ROUTINES
   HCcreate - create or modify an existing data element to be compressed
LOCAL ROUTINES

AUTHOR
   Quincey Koziol

MODIFICATION HISTORY
   9/21/93  - Starting writing specs & coding prototype
   10/09/93 - Finished initial testing.  First version with only stdio
              modeling and RLE coding done.
 */

/* General HDF includes */
#define COMPRESS_MASTER
#include "hdf.h"

#ifdef H4_HAVE_LIBSZ
#include "szlib.h"                                                                                 
#endif 

/* HDF compression includes */
#include "hcompi.h"     /* Internal definitions for compression */

/* Local defines */
#define COMP_HEADER_VERSION 0
#define COMP_HEADER_LENGTH  14
#ifdef OLD_WAY
#define COMP_START_BLOCK    1
#else /* OLD_WAY */
#define COMP_START_BLOCK    0
#endif /* OLD_WAY */

/* declaration of the functions provided in this module */
PRIVATE int32 HCIstaccess
            (accrec_t * access_rec, int16 acc_mode);

PRIVATE int32 HCIinit_coder
            (int16 acc_mode, comp_coder_info_t * cinfo, comp_coder_t coder_type,
             comp_info * coder_info);

PRIVATE int32 HCIread_header
            (accrec_t * access_rec, compinfo_t * info,
                comp_info * c_info, model_info * m_info);

PRIVATE int32 HCIwrite_header
            (atom_t file_id, compinfo_t * info, uint16 special_tag, uint16 ref,
            comp_info *c_info, model_info *m_info);

PRIVATE int32 HCIinit_model
            (int16 acc_mode, comp_model_info_t * minfo, comp_model_t model_type,
             model_info * m_info);

/* comp_funcs -- struct of accessing functions for the compressed
   data element function modules.  The position of each function in
   the table is standard */

funclist_t  comp_funcs =
{
    HCPstread,
    HCPstwrite,
    HCPseek,
    HCPinquire,
    HCPread,
    HCPwrite,
    HCPendaccess,
    HCPinfo,
    NULL         /* no routine registerd */
};

/* #define TESTING */

/*--------------------------------------------------------------------------
 NAME
    HCIinit_coder -- Set the coder function pointers
 USAGE
    int32 HCIinit_coder(cinfo,coder_type,coder_info)
    comp_coder_info_t *cinfo;   IN/OUT: pointer to coder information to modify
    comp_coder_t coder_type;    IN: the type of encoding to use
    comp_info *coder_info;      IN: setup information for some encoding types

 RETURNS
    Return SUCCEED or FAIL
 DESCRIPTION
    Sets the encoder function pointers and the encoder type for a given
    coder type.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIinit_coder(int16 acc_mode, comp_coder_info_t * cinfo, comp_coder_t coder_type,
              comp_info * c_info)
{
    uint32 comp_info;
    CONSTR(FUNC, "HCIinit_coder");  /* for HERROR */

    HCget_config_info(coder_type, &comp_info);
    if ((comp_info & COMP_DECODER_ENABLED|COMP_ENCODER_ENABLED) == 0) {
	/* coder not present?? */
              HRETURN_ERROR(DFE_BADCODER, FAIL)
    }

    switch (coder_type)
      {     /* determin the type of encoding */
          case COMP_CODE_NONE:      /* "none" (i.e. no) encoding */
              cinfo->coder_type = COMP_CODE_NONE;   /* set coding type */
              cinfo->coder_funcs = cnone_funcs;     /* set the "none" func. ptrs */
              break;

          case COMP_CODE_RLE:   /* Run-length encoding */
              cinfo->coder_type = COMP_CODE_RLE;    /* set coding type */
              cinfo->coder_funcs = crle_funcs;  /* set the RLE func. ptrs */
              break;

          case COMP_CODE_NBIT:      /* N-bit encoding */
              cinfo->coder_type = COMP_CODE_NBIT;   /* set the coding type */
              cinfo->coder_funcs = cnbit_funcs;     /* set the N-bit func. ptrs */

              /* copy encoding info */
              cinfo->coder_info.nbit_info.nt = c_info->nbit.nt;
              cinfo->coder_info.nbit_info.sign_ext = c_info->nbit.sign_ext;
              cinfo->coder_info.nbit_info.fill_one = c_info->nbit.fill_one;
              cinfo->coder_info.nbit_info.mask_off = c_info->nbit.start_bit;
              cinfo->coder_info.nbit_info.mask_len = c_info->nbit.bit_len;
              if ((cinfo->coder_info.nbit_info.nt_size
                   = DFKNTsize(cinfo->coder_info.nbit_info.nt)) == FAIL)
                  HRETURN_ERROR(DFE_BADNUMTYPE, FAIL);
              break;

          case COMP_CODE_SKPHUFF:   /* Skipping Huffman encoding */
              if(c_info->skphuff.skp_size<1)
                  HRETURN_ERROR(DFE_BADCODER, FAIL)

              /* set the coding type and the skipping huffman func. ptrs */
              cinfo->coder_type = COMP_CODE_SKPHUFF;
              cinfo->coder_funcs = cskphuff_funcs;

              /* copy encoding info */
              cinfo->coder_info.skphuff_info.skip_size = c_info->skphuff.skp_size;
              break;

          case COMP_CODE_DEFLATE:   /* gzip 'deflate' encoding */
	      /* valid deflate levels are from 0 to 9, this error checking
		 caused the problem in HDF4r1.2 , fixed by Apu Kapadia    
	      if(c_info->deflate.level<1 || c_info->deflate.level>9)
	      */
              if(c_info->deflate.level<0 || c_info->deflate.level>9)
                  HRETURN_ERROR(DFE_BADCODER, FAIL)

              /* set the coding type and the gzip 'deflate' func. ptrs */
              cinfo->coder_type = COMP_CODE_DEFLATE;
              cinfo->coder_funcs = cdeflate_funcs;

              /* copy encoding info */
              if(acc_mode&DFACC_WRITE)
                  cinfo->coder_info.deflate_info.deflate_level = c_info->deflate.level;
              break;

           case COMP_CODE_SZIP:
              /* set the coding type */
              cinfo->coder_type = COMP_CODE_SZIP;

	      /* when libsz presents, initialize other info - BMR, 08/25/2007
		 (changed from eliminating this case completely) */
	      /* completely removed the libsz limitation, we shouldn't need 
		 szip library to initialize here - BMR, 10/21/2008 */

              /* set the szip func. ptrs */
              cinfo->coder_funcs = cszip_funcs;

              /* copy encoding info */
              cinfo->coder_info.szip_info.pixels = c_info->szip.pixels;
              cinfo->coder_info.szip_info.bits_per_pixel = c_info->szip.bits_per_pixel;
              cinfo->coder_info.szip_info.pixels_per_block = c_info->szip.pixels_per_block;
              cinfo->coder_info.szip_info.pixels_per_scanline = c_info->szip.pixels_per_scanline;
              cinfo->coder_info.szip_info.options_mask = c_info->szip.options_mask;
              cinfo->coder_info.szip_info.buffer = NULL;
              cinfo->coder_info.szip_info.buffer_size = 0;
              cinfo->coder_info.szip_info.offset = 0;
              cinfo->coder_info.szip_info.szip_state = SZIP_INIT;
              cinfo->coder_info.szip_info.szip_dirty = SZIP_CLEAN;
              break;

          default:
              HRETURN_ERROR(DFE_BADCODER, FAIL)
      }     /* end switch */
    return (SUCCEED);
}   /* end HCIinit_coder() */

/*--------------------------------------------------------------------------
 NAME
    HCIinit_model -- Set the model function pointers
 USAGE
    int32 HCIinit_model(minfo,model_type,m_info)
    comp_model_info_t *minfo;   IN/OUT: pointer to model information to modify
    comp_model_t model_type;    IN: the type of encoding to use
    model_info *m_info;         IN: modeling information

 RETURNS
    Return SUCCEED or FAIL
 DESCRIPTION
    Sets the modeling function pointers and the model type for a given
    model type.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIinit_model(int16 acc_mode, comp_model_info_t * minfo, comp_model_t model_type,
              model_info * m_info)
{
    CONSTR(FUNC, "HCIinit_model");  /* for HERROR */

    /* shut compiler up */
    acc_mode = acc_mode;
    m_info = m_info;

    switch (model_type)
      {     /* determine the type of modeling */
          case COMP_MODEL_STDIO:    /* standard C stdio modeling */
              minfo->model_type = COMP_MODEL_STDIO;     /* set model type */
              minfo->model_funcs = mstdio_funcs;    /* set the stdio func. ptrs */
              break;

          default:
              HRETURN_ERROR(DFE_BADMODEL, FAIL)
      }     /* end switch */

    return (SUCCEED);
}   /* end HCIinit_model() */

/*--------------------------------------------------------------------------
 NAME
    HCPquery_encode_header -- Query the length of compression header for a memory buffer
 USAGE
    int32 HCPquery_encode_header(model_type, model_info, coder_type, coder_info)
    comp_model_t model_type; IN: the type of modeling to use
    model_info *m_info;      IN: Information needed for the modeling type chosen
    comp_coder_t coder_type; IN: the type of encoding to use
    coder_info *c_info;      IN: Information needed for the encoding type chosen

 RETURNS
    Return the length of the buffer needed to store the compression header on
    success (>0) or FAIL (-1)
 DESCRIPTION
    Determine the compression information length in a memory block.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPquery_encode_header(comp_model_t model_type, model_info * m_info,
         comp_coder_t coder_type, comp_info * c_info)
{
    CONSTR(FUNC, "HCPquery_encode_header");    /* for HERROR */
    int32 coder_len=2;  /* # of bytes to encode coder information (2 minimum) */
    int32 model_len=2;  /* # of bytes to encode model information (2 minimum) */
    int32 ret_value=SUCCEED;
    
    /* clear error stack and validate args */
    HEclear();
    if (m_info==NULL || c_info==NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* add any additional information needed for modeling type */
    switch (model_type)
      {
          default:      /* no additional information needed */
              break;
      }     /* end switch */

    /* add any additional information needed for coding type */
    switch (coder_type)
      {
          case COMP_CODE_NBIT:    /* N-bit coding needs 16 bytes of info */
              coder_len+=16;
              break;

          case COMP_CODE_SKPHUFF: /* Skipping Huffman coding needs 8 bytes of info */
              coder_len+=8;
              break;

          case COMP_CODE_DEFLATE: /* Deflation coding stores deflation level */
              coder_len+=2;
              break;

          case COMP_CODE_SZIP: /* Szip coding stores various szip parameters */
	      coder_len += 14;
	      break;

          default:      /* no additional information needed */
              break;
      }     /* end switch */

    ret_value=model_len+coder_len;

done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
  
      } /* end if */

    /* Normal function cleanup */
    return ret_value; 
} /* end HCPquery_encode_header() */

/*--------------------------------------------------------------------------
 NAME
    HCPencode_header -- Encode the compression header info to a memory buffer
 USAGE
    intn HCPencode_header(model_type, model_info, coder_type, coder_info)
    void * buf;               OUT: encoded compression info header
    comp_model_t model_type; IN: the type of modeling to use
    model_info *m_info;      IN: Information needed for the modeling type chosen
    comp_coder_t coder_type; IN: the type of encoding to use
    coder_info *c_info;      IN: Information needed for the encoding type chosen

 RETURNS
    Return SUCCEED or FAIL
 DESCRIPTION
    Encodes the compression information to a block in memory.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
HCPencode_header(uint8 *p, comp_model_t model_type, model_info * m_info,
         comp_coder_t coder_type, comp_info * c_info)
{
    CONSTR(FUNC, "HCPencode_header");    /* for HERROR */
    int32 ret_value=SUCCEED;
    
    /* clear error stack and validate args */
    HEclear();
    if (p==NULL || m_info==NULL || c_info==NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    UINT16ENCODE(p, (uint16) model_type);   /* specify model type */
    UINT16ENCODE(p, (uint16) coder_type);   /* specify coder type */

    /* add any additional information needed for modeling type */
    switch (model_type)
      {
          default:      /* no additional information needed */
              break;
      }     /* end switch */

    /* add any additional information needed for coding type */
    switch (coder_type)
      {
          case COMP_CODE_NBIT:      /* N-bit coding needs info */
              /* specify number-type of N-bit data */
              INT32ENCODE(p, c_info->nbit.nt);
              /* next is the flag to indicate whether to sign extend */
              UINT16ENCODE(p, (uint16) c_info->nbit.sign_ext);
              /* flag to fill with 1's or 0's */
              UINT16ENCODE(p, (uint16) c_info->nbit.fill_one);
              /* the offset of the bits extracted */
              INT32ENCODE(p, (int32) c_info->nbit.start_bit);
              /* the number of bits extracted */
              INT32ENCODE(p, (int32) c_info->nbit.bit_len);
              break;

          case COMP_CODE_SKPHUFF:   /* Skipping Huffman coding needs info */
              if(c_info->skphuff.skp_size<1)
                  HRETURN_ERROR(DFE_BADCODER, FAIL)

              /* specify skipping unit size */
              UINT32ENCODE(p, (uint32) c_info->skphuff.skp_size);
              /* specify # of bytes compressed (not used currently) */
              UINT32ENCODE(p, (uint32) c_info->skphuff.skp_size);
              break;

          case COMP_CODE_DEFLATE:   /* Deflation coding stores deflation level */
              /* valid deflate levels are from 0 to 9
              if(c_info->deflate.level<1 || c_info->deflate.level>9)
              */
              if(c_info->deflate.level<0 || c_info->deflate.level>9)
                  HRETURN_ERROR(DFE_BADCODER, FAIL)

              /* specify deflation level */
              UINT16ENCODE(p, (uint16) c_info->deflate.level);
              break;

          case COMP_CODE_SZIP: /* Szip coding stores various szip parameters */
              UINT32ENCODE(p, (uint32) c_info->szip.pixels);
              UINT32ENCODE(p, (uint32) c_info->szip.pixels_per_scanline);
              UINT32ENCODE(p, (uint32) (c_info->szip.options_mask | SZ_H4_REV_2));
              *p++ = (uint8) c_info->szip.bits_per_pixel;
              *p++ = (uint8) c_info->szip.pixels_per_block;
              break;

          default:      /* no additional information needed */
              break;
      }     /* end switch */

done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
  
      } /* end if */

    /* Normal function cleanup */
    return ret_value; 
} /* end HCPencode_header() */

/*--------------------------------------------------------------------------
 NAME
    HCPdecode_header -- Decode the compression header info from a memory buffer
 USAGE
    intn HCPdecode_header(model_type, model_info, coder_type, coder_info)
    void * buf;                  IN: encoded compression info header
    comp_model_t *model_type;   OUT: the type of modeling to use
    model_info *m_info;         OUT: Information needed for the modeling type chosen
    comp_coder_t *coder_type;   OUT: the type of encoding to use
    coder_info *c_info;         OUT: Information needed for the encoding type chosen

 RETURNS
    Return SUCCEED or FAIL
 DESCRIPTION
    Decodes the compression information from a block in memory.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
HCPdecode_header(uint8 *p, comp_model_t *model_type, model_info * m_info,
         comp_coder_t *coder_type, comp_info * c_info)
{
    CONSTR(FUNC, "HCPdecode_header");    /* for HERROR */
    uint16 m_type, c_type;
    int32 ret_value=SUCCEED;

    /* clear error stack and validate args */
    HEclear();
    if (p==NULL || model_type==NULL || m_info==NULL || coder_type==NULL || c_info==NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    UINT16DECODE(p, m_type);     /* get model type */
    *model_type=(comp_model_t)m_type;
    UINT16DECODE(p, c_type);     /* get encoding type */
    *coder_type=(comp_coder_t)c_type;

    /* read any additional information needed for modeling type */
    switch (*model_type)
      {
          default:      /* no additional information needed */
              break;
      }     /* end switch */

    /* read any additional information needed for coding type */
    switch (*coder_type)
      {
          case COMP_CODE_NBIT:      /* Obtain info for N-bit coding */
              {
                  uint16      s_ext;    /* temp. var for sign extend */
                  uint16      f_one;    /* temp. var for fill one */
                  int32       m_off, m_len;     /* temp. var for mask offset and len */

                  /* number-type of N-bit data */
                  INT32DECODE(p, c_info->nbit.nt);
                  /* next is the flag to indicate whether to sign extend */
                  UINT16DECODE(p, s_ext);
                  c_info->nbit.sign_ext = (intn) s_ext;
                  /* flag to indicate whether to fill with 1's or 0's */
                  UINT16DECODE(p, f_one);
                  c_info->nbit.fill_one = (intn) f_one;
                  /* offset of the bits extracted */
                  INT32DECODE(p, m_off);
                  c_info->nbit.start_bit = (intn) m_off;
                  /* number of bits extracted */
                  INT32DECODE(p, m_len);
                  c_info->nbit.bit_len = (intn) m_len;
              }     /* end case */
              break;

          case COMP_CODE_SKPHUFF: /* Obtain info for Skipping Huffman coding */
              {
                  uint32      skp_size,     /* size of skipping unit */
                              comp_size;    /* # of bytes to compress */

                  /* specify skipping unit size */
                  UINT32DECODE(p, skp_size);
                  /* specify # of bytes of skipping data to compress */
                  UINT32DECODE(p, comp_size);   /* ignored for now */
                  c_info->skphuff.skp_size = (intn) skp_size;
              }     /* end case */
              break;

          case COMP_CODE_DEFLATE: /* Obtains deflation level for Deflation coding */
              {
                  uint16      level;    /* deflation level */

                  /* specify deflation level */
                  UINT16DECODE(p, level);
                  c_info->deflate.level = (intn) level;
              }     /* end case */
              break;

          case COMP_CODE_SZIP: /* Obtains szip parameters for Szip coding */
	      {
                  UINT32DECODE(p, c_info->szip.pixels);
                  UINT32DECODE(p, c_info->szip.pixels_per_scanline);
                  UINT32DECODE(p, c_info->szip.options_mask);
                  c_info->szip.bits_per_pixel = *p++;
                  c_info->szip.pixels_per_block = *p++;
	      }
              break;

          default:      /* no additional information needed */
              break;
      }     /* end switch */

done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
  
      } /* end if */

    /* Normal function cleanup */
    return ret_value; 
} /* end HCPdecode_header() */

/*--------------------------------------------------------------------------
 NAME
    HCIwrite_header -- Write the compression header info to a file
 USAGE
    int32 HCIwrite_header(access_rec,info,special_tag,ref)
    atom_t file_id;         IN: File ID of the file to write the header to
    compinfo_t *info;       IN: ptr the compression information
    uint16 special_tag,ref; IN: the tag/ref of the compressed element
    coder_info *c_info;     IN: Information needed for the encoding type chosen
    model_info *m_info;     IN: Information needed for the modeling type chosen

 RETURNS
    Return SUCCEED or FAIL
 DESCRIPTION
    Writes the compression information to a new block in the HDF file.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIwrite_header(atom_t file_id, compinfo_t * info, uint16 special_tag, uint16 ref, comp_info *c_info, model_info *m_info)
{
    CONSTR(FUNC, "HCIwrite_header");    /* for HERROR */
    int32       dd_aid;         /* AID for writing the special info */
    uint8      *p;              /* pointer to the temporary buffer */
    uint8       local_ptbuf[32];
    int32       header_len;     /* how many bytes the header is */
    int32       ret_value=SUCCEED;

    /* write special element info to the file */
    p = local_ptbuf;
    INT16ENCODE(p, SPECIAL_COMP);   /* specify special tag type */
    UINT16ENCODE(p, COMP_HEADER_VERSION);   /* specify header version */
    INT32ENCODE(p, info->length);   /* write length of un-comp. data */
    UINT16ENCODE(p, (uint16) info->comp_ref);   /* specify ref # of comp. data */
#ifdef OLD_WAY
    UINT16ENCODE(p, (uint16) info->minfo.model_type);   /* specify model type */
    UINT16ENCODE(p, (uint16) info->cinfo.coder_type);   /* specify coder type */

    /* write any additional information needed for modeling type */
    switch (info->minfo.model_type)
      {
          default:      /* no additional information needed */
              break;
      }     /* end switch */

    /* write any additional information needed for coding type */
    switch (info->cinfo.coder_type)
      {
          case COMP_CODE_NBIT:      /* N-bit coding needs info */
              /* specify number-type of N-bit data */
              INT32ENCODE(p, info->cinfo.coder_info.nbit_info.nt);
              /* next is the flag to indicate whether to sign extend */
              UINT16ENCODE(p, (uint16) info->cinfo.coder_info.nbit_info.sign_ext);
              /* flag to fill with 1's or 0's */
              UINT16ENCODE(p, (uint16) info->cinfo.coder_info.nbit_info.fill_one);
              /* the offset of the bits extracted */
              INT32ENCODE(p, (int32) info->cinfo.coder_info.nbit_info.mask_off);
              /* the number of bits extracted */
              INT32ENCODE(p, (int32) info->cinfo.coder_info.nbit_info.mask_len);
              break;

          case COMP_CODE_SKPHUFF:   /* Skipping Huffman coding needs info */
              /* specify skipping unit size */
              UINT32ENCODE(p, (uint32) info->cinfo.coder_info.skphuff_info.skip_size);
              /* specify # of bytes compressed (not used currently) */
              UINT32ENCODE(p, (uint32) info->cinfo.coder_info.skphuff_info.skip_size);
              break;

          case COMP_CODE_SZIP:
              INT32ENCODE(p, (int32) c_info->szip.pixels);
              INT32ENCODE(p, (int32) c_info->szip.pixels_per_scanline);
              INT32ENCODE(p, (int32) c_info->szip.options_mask);
              INT32ENCODE(p, (int32) c_info->szip.bits_per_pixel);
              INT32ENCODE(p, (int32) c_info->szip.pixels_per_block);
              break;

          default:      /* no additional information needed */
              break;
      }     /* end switch */
#else /* OLD_WAY */
    if((header_len=HCPquery_encode_header(info->minfo.model_type,
            m_info,info->cinfo.coder_type,c_info))==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);
    if(HCPencode_header(p,info->minfo.model_type,m_info,
            info->cinfo.coder_type,c_info)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);
    p+=header_len;
#endif /* OLD_WAY */
    
    /* write the special info structure to fill */
    if((dd_aid=Hstartaccess(file_id,special_tag,ref,DFACC_ALL))==FAIL)
        HGOTO_ERROR(DFE_CANTACCESS, FAIL);
    if (Hwrite(dd_aid, (int32)(p-local_ptbuf), local_ptbuf) == FAIL)
        HGOTO_ERROR(DFE_WRITEERROR, FAIL);
    if(Hendaccess(dd_aid)==FAIL)
        HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
  
      } /* end if */

    /* Normal function cleanup */
    return ret_value; 
}   /* end HCIwrite_header() */

/*--------------------------------------------------------------------------
 NAME
    HCIread_header -- Read the compression header info from a file
 USAGE
    int32 HCIread_header(file_rec,access_rec,info,comp_info,model_info)
    accrec_t *access_rec;   IN: ptr to the access element record
    compinfo_t *info;       IN: ptr the compression information
    comp_info *comp_info;   IN/OUT: ptr to encoding info
    model_info *model_info; IN/OUT: ptr to modeling info
 RETURNS
    Return SUCCEED or FAIL
 DESCRIPTION
    Parses the compression header from a data element in an HDF file.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIread_header(accrec_t * access_rec,
               compinfo_t * info, comp_info * c_info, model_info * m_info)
{
    CONSTR(FUNC, "HCIread_header");     /* for HERROR */
    uint16      header_version; /* version of the compression header */
    uint16      ctag, cref;     /* tag/ref for the object */
    int32       temp_aid;       /* temporary AID for header info */
    int32       data_len;		/* offset of the data we are checking */
    uint8      *p;              /* pointer to the temporary buffer */
    uint8      *local_ptbuf;
    int32       ret_value=SUCCEED;

    /* shut compiler up */
    m_info = m_info;

    /* get the info for the dataset */
    if(HTPinquire(access_rec->ddid,&ctag,&cref,NULL,&data_len)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);
    if((local_ptbuf=(uint8 *)HDmalloc(data_len))==NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* Get the compression header */
    if ((temp_aid=Hstartaccess(access_rec->file_id,MKSPECIALTAG(ctag),cref,DFACC_READ)) == FAIL)
        HGOTO_ERROR(DFE_BADAID, FAIL);
    if (Hread(temp_aid,0,local_ptbuf) == FAIL)
        HGOTO_ERROR(DFE_READERROR, FAIL);
    if(Hendaccess(temp_aid)==FAIL)
        HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

    p = local_ptbuf+2;
    UINT16DECODE(p, header_version);    /* get compression version */
    INT32DECODE(p, info->length);   /* get _uncompressed_ data length */
    UINT16DECODE(p, info->comp_ref);    /* get ref # of comp. data */

    /* Decode the compression header */
    if(HCPdecode_header(p,&(info->minfo.model_type),m_info,&(info->cinfo.coder_type),c_info)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);
    HDfree(local_ptbuf);

done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
  
      } /* end if */

    /* Normal function cleanup */
    return ret_value; 
}   /* end HCIread_header() */

/*--------------------------------------------------------------------------
 NAME
    HCcreate -- Create a compressed data element
 USAGE
    int32 HCcreate(id,tag,ref,model_type,coder_type)
    int32 id;                IN: the file id to create the data in
    uint16 tag,ref;          IN: the tag/ref pair which is to be compressed
    comp_model_t model_type; IN: the type of modeling to use
    model_info *m_info;      IN: Information needed for the modeling type chosen
    comp_coder_t coder_type; IN: the type of encoding to use
    coder_info *c_info;      IN: Information needed for the encoding type chosen
 RETURNS
    Return an AID to the newly created compressed element, FAIL on error.
 DESCRIPTION
    Create a compressed data element.  If that data element already
    exists, we will compress that data element if it is currently un-compresed,
    or return FAIL if it is already compressed.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCcreate(int32 file_id, uint16 tag, uint16 ref, comp_model_t model_type,
         model_info * m_info, comp_coder_t coder_type,
         comp_info * c_info)
{
    CONSTR(FUNC, "HCcreate");   /* for HERROR */
    filerec_t  *file_rec;       /* file record */
    accrec_t   *access_rec=NULL;/* access element record */
    compinfo_t *info=NULL;      /* special element information */
    atom_t      data_id=FAIL;   /* dd ID of existing regular element */
    int32       data_len;		/* length of the data we are checking */
    uint16      special_tag;    /* special version of tag */
    void *       buf = NULL;      /* temporary buffer */
    int32       ret_value=SUCCEED;

    /* clear error stack and validate args */
    HEclear();
    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec) || SPECIALTAG(tag)
            || (special_tag = MKSPECIALTAG(tag)) == DFTAG_NULL)
        HRETURN_ERROR(DFE_ARGS, FAIL);

    /* chech for access permission */
    if (!(file_rec->access & DFACC_WRITE))
        HRETURN_ERROR(DFE_DENIED, FAIL);

    /* get a slot in the access records table */
    if (NULL == (access_rec = HIget_access_rec()))
        HRETURN_ERROR(DFE_TOOMANY, FAIL);

    /* search for identical dd */
    if ((data_id=HTPselect(file_rec,tag,ref))!=FAIL)
      {
          /* Check if the element is already special */
          if (HTPis_special(data_id)==TRUE)
            {
                if (HTPendaccess(data_id) == FAIL)
                    HGOTO_ERROR(DFE_CANTFLUSH, FAIL);
                HGOTO_ERROR(DFE_CANTMOD, FAIL);
            }   /* end if */
          
          /* get the info for the dataset */
          if(HTPinquire(data_id,NULL,NULL,NULL,&data_len)==FAIL)
            {
                if (HTPendaccess(data_id) == FAIL)
                    HGOTO_ERROR(DFE_CANTFLUSH, FAIL);
                HGOTO_ERROR(DFE_INTERNAL, FAIL);
            } /* end if */

          if ((buf = HDmalloc((uint32) data_len)) == NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
          if (Hgetelement(file_id, tag, ref, buf) == FAIL)
              HGOTO_ERROR(DFE_READERROR, FAIL);
          /* Delete the old DD from the file and memory hash table */
          if (FAIL == HTPdelete(data_id))
              HGOTO_ERROR(DFE_CANTDELDD, FAIL);

      } /* end if */

    /* set up the special element information and write it to file */
    info = (compinfo_t *) HDmalloc(sizeof(compinfo_t));
    access_rec->special_info = info;
    if (info == NULL)
          HGOTO_ERROR(DFE_NOSPACE, FAIL);

    info->length = (data_id!=FAIL) ? data_len : COMP_START_BLOCK;

    /* set up compressed special info structure */
    info->attached = 1;
    info->comp_ref = Htagnewref(file_id,DFTAG_COMPRESSED);  /* get the new reference # */
    if(HCIinit_model(DFACC_RDWR, &(info->minfo), model_type, m_info)==FAIL)
        HGOTO_ERROR(DFE_MINIT,FAIL);
    if(HCIinit_coder(DFACC_RDWR, &(info->cinfo), coder_type, c_info)==FAIL)
        HGOTO_ERROR(DFE_CINIT,FAIL);

    if (HCIwrite_header(file_id, info, special_tag, ref, c_info, m_info) == FAIL)
          HGOTO_ERROR(DFE_WRITEERROR, FAIL);

    /* update access record and file record */
    if((access_rec->ddid=HTPselect(file_rec,tag,ref))==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);
    access_rec->special_func = &comp_funcs;
    access_rec->special = SPECIAL_COMP;
    access_rec->posn = 0;
    access_rec->access = DFACC_RDWR;
    access_rec->file_id = file_id;
    access_rec->appendable = FALSE;     /* start data as non-appendable */
    file_rec->attach++;

    /* propagate the initialization down to the modeling layer */
    if ((*(info->minfo.model_funcs.stwrite))(access_rec) == FAIL)
          HGOTO_ERROR(DFE_MODEL, FAIL);

    /* compress the old DD and get rid of it, if there was one */
    if (data_id != FAIL)
      {
          /* write the data through to the compression layer */
          if (HCPwrite(access_rec, data_len, buf) == FAIL)
              HGOTO_ERROR(DFE_MODEL, FAIL);

          /* seek back to the beginning of the data through to the compression layer */
          if (HCPseek(access_rec, 0, DF_START) == FAIL)
              HGOTO_ERROR(DFE_MODEL, FAIL);
      }     /* end if */

    ret_value=HAregister_atom(AIDGROUP,access_rec);

done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
        if(access_rec!=NULL)
            HIrelease_accrec_node(access_rec);
        if(info!=NULL)
            HDfree(info);
      } /* end if */

    /* Normal function cleanup */
    if (buf != NULL)
        HDfree(buf);

    return ret_value; 
}   /* end HCcreate() */

/*--------------------------------------------------------------------------
 NAME
    HCPgetcompress -- Retrieves compression information of an element
 USAGE
    intn HCPgetcompress(aid, coder_type, c_info)
    int32 aid;                  IN: access record ID
    comp_coder_t* coder_type;   OUT: the type of compression
    comp_info* c_info;          OUT: ptr to compression information
                                structure for storing the retrieved info
 RETURNS
    SUCCEED/FAIL
 DESCRIPTION
    This routine retrieves the compression type and the compression
    information of the element, identified by 'aid'.  The routine is 
    used by GRgetcompress and SDgetcompress at this time.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
    July 2001: Added to fix bug #307 - BMR
    Dec. 2004: Changed name to HCPgetcompress, to be consistent with other
               practice.  REM
--------------------------------------------------------------------------*/
intn
HCPgetcompress(int32 file_id,
              uint16 data_tag, uint16 data_ref,
              comp_coder_t* comp_type,  /* OUT: compression type */
              comp_info* c_info)        /* OUT: retrieved compression info */
{
    CONSTR(FUNC, "HCPgetcompress");   /* for HGOTO_ERROR */
    int32   aid=0, status;
    accrec_t*    access_rec=NULL;/* access element record */
    compinfo_t*  info=NULL;  /* compressed element information */
    model_info  m_info;         /* modeling information - dummy */
    int32       ret_value=SUCCEED;

    /* clear error stack */
    HEclear();

    /* start read access on the access record of the data element, which
       is being inquired for its compression information */
    aid = Hstartread(file_id, data_tag, data_ref);

    /* get the access_rec pointer */
    access_rec = HAatom_object(aid);
    if (access_rec == NULL) HGOTO_ERROR(DFE_ARGS, FAIL);

    /* if the element is compressed, get the compression info as requested*/
    if (access_rec->special == SPECIAL_COMP)
    {
        info = (compinfo_t *) access_rec->special_info;
        if (info == NULL) HGOTO_ERROR(DFE_COMPINFO, FAIL);

        status = HCIread_header(access_rec, info, c_info, &m_info);
        if (status == FAIL) HGOTO_ERROR(DFE_COMPINFO, FAIL);

        /* get the compression type */
        *comp_type = info->cinfo.coder_type;

    }  /* end if element is compressed */

    /* if the element is chunked, call HMCgetcompress to get the 
	compression info as appropriate */
    else if (access_rec->special == SPECIAL_CHUNKED)
    {
	status = HMCgetcompress(access_rec, comp_type, c_info);
        if (status == FAIL) HGOTO_ERROR(DFE_COMPINFO, FAIL);
    }

    /* flag the error when attempting to get compression info on a
       non-compressed element */
    else 
    /* EIP 9/16/03  Fail but return compression type COMP_CODE_NONE
       instead of junk in this case.
    */
     {
        /*Mac OSX screams here (comp_coder_t)*comp_type = COMP_CODE_NONE; */
        *comp_type = COMP_CODE_NONE; 
        HGOTO_ERROR(DFE_ARGS, FAIL);
     }
    /* end access to the aid appropriately */
    if (Hendaccess(aid)== FAIL)
        HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

done:
  if(ret_value == FAIL)
    { /* Error condition cleanup */
       /* end access to the aid if it's been accessed */
        if (aid != 0)
            if (Hendaccess(aid)== FAIL)
                HERROR(DFE_CANTENDACCESS);
    } /* end if */

  /* Normal function cleanup */
  return ret_value;
} /* HCPgetcompress */


/*--------------------------------------------------------------------------
 NAME
    HCPgetcompinfo -- Retrieves compression information of an element
 USAGE
    intn HCPgetcompinfo(aid, coder_type, c_info)
    int32 aid;                  IN: access record ID
    comp_coder_t* coder_type;   OUT: the type of compression
    comp_info* c_info;          OUT: ptr to compression information
                                structure for storing the retrieved info
 RETURNS
    SUCCEED/FAIL
 DESCRIPTION
    This routine retrieves the compression type and the compression
    information of the element, identified by 'aid'.  The routine is 
    used by GRgetcompinfo and SDgetcompinfo at this time.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
    July 2001: Added to fix bug #307 - BMR
    Dec. 2004: Changed name to HCPgetcompress, to be consistent with other
               practice.  REM
    Apr. 2005:	HCPgetcompinfo was added to fix bugzilla #130 and may replace 
		HCPgetcompress in the future because HCPgetcompress did not 
		behave correctly.  The revision logs above are carried over 
		from HCPgetcompress for the records.
--------------------------------------------------------------------------*/
intn
HCPgetcompinfo(int32 file_id,
              uint16 data_tag, uint16 data_ref,
              comp_coder_t* comp_type,  /* OUT: compression type */
              comp_info* c_info)        /* OUT: retrieved compression info */
{
    CONSTR(FUNC, "HCPgetcompinfo");	/* for HGOTO_ERROR */
    int32   aid=0, status;
    accrec_t*    access_rec=NULL;	/* access element record */
    compinfo_t*  info=NULL;		/* compressed element information */
    model_info  m_info;			/* modeling information - dummy */
    intn       ret_value=SUCCEED;

    /* clear error stack */
    HEclear();

    /* start read access on the access record of the data element, which
       is being inquired for its compression information */
    aid = Hstartread(file_id, data_tag, data_ref);

    /* get the access_rec pointer */
    access_rec = HAatom_object(aid);
    if (access_rec == NULL) HGOTO_ERROR(DFE_ARGS, FAIL);

    /* if the element is compressed, get the compression info as requested */
    if (access_rec->special == SPECIAL_COMP)
    {
        info = (compinfo_t *) access_rec->special_info;
        if (info == NULL) HGOTO_ERROR(DFE_COMPINFO, FAIL);

        status = HCIread_header(access_rec, info, c_info, &m_info);
        if (status == FAIL) HGOTO_ERROR(DFE_COMPINFO, FAIL);

        /* get the compression type */
        *comp_type = info->cinfo.coder_type;

    }  /* end if element is compressed */

    /* if the element is chunked, call HMCgetcompress to get the 
	compression info as appropriate */
    else if (access_rec->special == SPECIAL_CHUNKED)
    {
	status = HMCgetcompress(access_rec, comp_type, c_info);
        if (status == FAIL) HGOTO_ERROR(DFE_COMPINFO, FAIL);
    }

    /* return COMP_CODE_NONE for a non-compressed element */
    /* Note: SPECIAL_COMPRAS may need special handling */
    else if (access_rec->special == SPECIAL_LINKED ||
             access_rec->special == SPECIAL_EXT ||
             access_rec->special == SPECIAL_VLINKED ||
             access_rec->special == SPECIAL_BUFFERED ||
             access_rec->special == SPECIAL_COMPRAS ||
             access_rec->special == 0)
    {
        *comp_type = COMP_CODE_NONE;
    }

    /* flag the error when access_rec->special is not something valid */
    else 
    {
	*comp_type = COMP_CODE_INVALID; 
        HGOTO_ERROR(DFE_ARGS, FAIL);
    }
    /* end access to the aid appropriately */
    if (Hendaccess(aid)== FAIL)
        HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

done:
  if(ret_value == FAIL)
    { /* Error condition cleanup */
       /* end access to the aid if it's been accessed */
        if (aid != 0)
            if (Hendaccess(aid)== FAIL)
                HERROR(DFE_CANTENDACCESS);
    } /* end if */

  /* Normal function cleanup */
  return ret_value;
} /* HCPgetcompinfo */

/*--------------------------------------------------------------------------
 NAME
    HCIstaccess -- Start accessing a compressed data element.
 USAGE
    int32 HCIstaccess(access_rec, access)
    accrec_t *access_rec;   IN: the access record of the data element
    int16 access;           IN: the type of access wanted
 RETURNS
    Returns an AID or FAIL
 DESCRIPTION
    Common code called by HCIstread and HCIstwrite

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIstaccess(accrec_t * access_rec, int16 acc_mode)
{
    CONSTR(FUNC, "HCIstaccess");    /* for HERROR */
    compinfo_t *info=NULL;      /* special element information */
    filerec_t  *file_rec;       /* file record */
    comp_info   c_info;         /* encoding information from the header */
    model_info  m_info;         /* modeling information from the header */
    int32       ret_value=SUCCEED;

    /* get file record and validate */
    file_rec = HAatom_object(access_rec->file_id);
    if (BADFREC(file_rec) || !(file_rec->access & acc_mode))
        HRETURN_ERROR(DFE_ARGS, FAIL);

    /* intialize the access record */
    access_rec->special = SPECIAL_COMP;
    access_rec->posn = 0;
    access_rec->access = (uint32)(acc_mode|DFACC_READ);

    /* get the special info record */
    access_rec->special_info = HDmalloc(sizeof(compinfo_t));
    info = (compinfo_t *) access_rec->special_info;
    if (info == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    if (HCIread_header(access_rec, info, &c_info, &m_info) == FAIL)
          HGOTO_ERROR(DFE_COMPINFO, FAIL);
    info->attached = 1;
    if (HCIinit_model(acc_mode,&(info->minfo), info->minfo.model_type, &m_info) == FAIL)
          HRETURN_ERROR(DFE_MINIT, FAIL);
    if (HCIinit_coder(acc_mode,&(info->cinfo), info->cinfo.coder_type, &c_info) == FAIL)
          HRETURN_ERROR(DFE_CINIT, FAIL);

    file_rec->attach++;

    ret_value=HAregister_atom(AIDGROUP,access_rec);

done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
        if(info!=NULL)
            HDfree(info);
      } /* end if */

    /* Normal function cleanup */
    return ret_value; 
}   /* end HCIstaccess() */

/*--------------------------------------------------------------------------
 NAME
    HCPstread -- Start read access on a compressed data element.
 USAGE
    int32 HCPstread(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element
 RETURNS
    Returns an AID or FAIL
 DESCRIPTION
    Start read access on a compressed data element.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPstread(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPstread");  /* for HERROR */
    compinfo_t *info;           /* information on the special element */
    int32       ret_value;      /* AID to return */

    if ((ret_value = HCIstaccess(access_rec, DFACC_READ)) == FAIL)
        HGOTO_ERROR(DFE_DENIED, FAIL);
    info = (compinfo_t *) access_rec->special_info;
    if ((*(info->minfo.model_funcs.stread)) (access_rec) == FAIL)
        HGOTO_ERROR(DFE_MODEL, FAIL);

done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */
    return ret_value; 
}   /* end HCPstread() */

/*--------------------------------------------------------------------------
 NAME
    HCPstwrite -- Start write access on a compressed data element.
 USAGE
    int32 HCPstwrite(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element
 RETURNS
    Returns an AID or FAIL
 DESCRIPTION
    Start write access on a compressed data element.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPstwrite(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPstwrite");     /* for HERROR */
    compinfo_t *info;               /* information on the special element */
    int32       ret_value;          /* AID to return */

    if ((ret_value = HCIstaccess(access_rec, DFACC_WRITE)) == FAIL)
        HGOTO_ERROR(DFE_DENIED, FAIL);
    info = (compinfo_t *) access_rec->special_info;
    if ((*(info->minfo.model_funcs.stwrite)) (access_rec) == FAIL)
        HGOTO_ERROR(DFE_MODEL, FAIL);

done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */
    return ret_value; 
}   /* end HCPstwrite() */

/*--------------------------------------------------------------------------
 NAME
    HCPseek -- Seek to offset within the data element
 USAGE
    int32 HCPseek(access_rec,offset,origin)
    accrec_t *access_rec;   IN: the access record of the data element
    int32 offset;       IN: the offset in bytes from the origin specified
    intn origin;        IN: the origin to seek from
 RETURNS
    Returns SUCCEED or FAIL
 DESCRIPTION
    Seek to a position with a compressed data element.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPseek(accrec_t * access_rec, int32 offset, intn origin)
{
    CONSTR(FUNC, "HCPseek");    /* for HERROR */
    compinfo_t *info;           /* information on the special element */
    int32       ret_value;

    /* Adjust offset according to origin.  There is no upper bound to posn */
    if (origin == DF_CURRENT)
        offset += access_rec->posn;
    if (origin == DF_END)
        offset += ((compinfo_t *) (access_rec->special_info))->length;
    if (offset < 0)
        HGOTO_ERROR(DFE_RANGE, FAIL);

    info = (compinfo_t *) access_rec->special_info;
    if ((ret_value = (*(info->minfo.model_funcs.seek)) (access_rec, offset, origin)) == FAIL)
        HGOTO_ERROR(DFE_MODEL, FAIL);

    /* set the offset */
    access_rec->posn = offset;

done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */
    return ret_value; 
}   /* end HCPseek() */

/*--------------------------------------------------------------------------
 NAME
    HCPread -- Read in a portion of data from a compressed data element.
 USAGE
    int32 HCPread(access_rec,length,data)
    accrec_t *access_rec;   IN: the access record of the data element
    int32 length;           IN: the number of bytes to read
    void * data;             OUT: the buffer to place the bytes read
 RETURNS
    Returns the number of bytes read or FAIL
 DESCRIPTION
    Read in a number of bytes from a compressed data element.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPread(accrec_t * access_rec, int32 length, void * data)
{
    CONSTR(FUNC, "HCPread");    /* for HERROR */
    compinfo_t *info;           /* information on the special element */
    int32       ret_value;

    /* validate length */
    if (length < 0)
        HGOTO_ERROR(DFE_RANGE, FAIL);

    info = (compinfo_t *) access_rec->special_info;

    /* adjust length if it falls off the end of the element */
    if (length == 0)
        length = info->length - access_rec->posn;
    else if (length < 0 || access_rec->posn + length > info->length)
        HGOTO_ERROR(DFE_RANGE, FAIL);

    if ((*(info->minfo.model_funcs.read))(access_rec, length, data) == FAIL)
        HGOTO_ERROR(DFE_MODEL, FAIL);

    /* adjust access position */
    access_rec->posn += length;

    ret_value=length;

done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */
    return ret_value; 
}   /* end HCPread() */

/*--------------------------------------------------------------------------
 NAME
    HCPwrite -- Write out a portion of data from a compressed data element.
 USAGE
    int32 HCPwrite(access_rec,length,data)
    accrec_t *access_rec;   IN: the access record of the data element
    int32 length;           IN: the number of bytes to write
    void * data;             IN: the buffer to retrieve the bytes written
 RETURNS
    Returns the number of bytes written or FAIL
 DESCRIPTION
    Write out a number of bytes to a compressed data element.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPwrite(accrec_t * access_rec, int32 length, const void * data)
{
    CONSTR(FUNC, "HCPwrite");   /* for HERROR */
    compinfo_t *info;           /* information on the special element */
    uint8       local_ptbuf[4];
    uint8       *p = local_ptbuf;  /* temp buffer ptr */
    filerec_t  *file_rec;           /* file record */
    int32       ret_value;

    /* convert file id to file record */
    file_rec = HAatom_object(access_rec->file_id);

    /* validate length */
    if (length < 0)
        HRETURN_ERROR(DFE_RANGE, FAIL);

    info = (compinfo_t *) access_rec->special_info;
    if ((*(info->minfo.model_funcs.write)) (access_rec, length, data) == FAIL)
        HGOTO_ERROR(DFE_MODEL, FAIL);

    /* update access record, and information about special element */
    access_rec->posn += length;
    if (access_rec->posn > info->length)
      {
          int32       data_off;		/* offset of the data we are checking */

          /* get the info for the dataset */
          if(HTPinquire(access_rec->ddid,NULL,NULL,&data_off,NULL)==FAIL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);

          info->length = access_rec->posn;

          INT32ENCODE(p, info->length);
          if (HPseek(file_rec, data_off + 4) == FAIL)
              HGOTO_ERROR(DFE_SEEKERROR, FAIL);
          /* re-write un-comp. len */
          if (HP_write(file_rec, local_ptbuf, 4) == FAIL)     
              HGOTO_ERROR(DFE_WRITEERROR, FAIL);
      }     /* end if */

    ret_value=length;  /* return length of bytes written */

done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */
    return ret_value; 
}   /* end HCPwrite() */

/*--------------------------------------------------------------------------
 NAME
    HCPinquire -- Inquire information about the access record and data element.
 USAGE
    int32 HCPinquire(access_rec,pfile_id,ptag,pref,plength,poffset,pposn,
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

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPinquire(accrec_t * access_rec, int32 *pfile_id, uint16 *ptag,
 uint16 *pref, int32 *plength, int32 *poffset, int32 *pposn, int16 *paccess,
           int16 *pspecial)
{
    CONSTR(FUNC, "HCPinquire");   /* for HERROR */
    compinfo_t *info =          /* special information record */
        (compinfo_t *) access_rec->special_info;
    uint16 data_tag,data_ref;   /* tag/ref of the data we are checking */
    int32       data_off;		/* offset of the data we are checking */

    /* get the info for the dataset */
    if(HTPinquire(access_rec->ddid,&data_tag,&data_ref,&data_off,NULL)==FAIL)
        HRETURN_ERROR(DFE_INTERNAL, FAIL);

    /* fill in the variables if they are present */
    if (pfile_id != NULL)
        *pfile_id = access_rec->file_id;
    if (ptag != NULL)
        *ptag = data_tag;
    if (pref != NULL)
        *pref = data_ref;
    if (plength != NULL)
        *plength = info->length;
    if (poffset != NULL)
        *poffset = data_off;
    if (pposn != NULL)
        *pposn = access_rec->posn;
    if (paccess != NULL)
        *paccess = (int16)access_rec->access;
    if (pspecial != NULL)
        *pspecial = (int16)access_rec->special;

    return (SUCCEED);
}   /* end HCPinquire() */

/*--------------------------------------------------------------------------
 NAME
    HCPendaccess -- Close the compressed data element and free the AID
 USAGE
    intn HCPendaccess(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element
 RETURNS
    Returns SUCCEED or FAIL
 DESCRIPTION
    Close the compressed data element and free the AID.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
HCPendaccess(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPendaccess");   /* for HERROR */
    filerec_t  *file_rec;           /* file record */
    intn      ret_value = SUCCEED;

    /* convert file id to file record */
    file_rec = HAatom_object(access_rec->file_id);

    /* validate file record */
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* close the file pointed to by this access rec */
    HCPcloseAID(access_rec);

    /* update file and access records */
    if (HTPendaccess(access_rec->ddid) == FAIL)
      HGOTO_ERROR(DFE_CANTFLUSH, FAIL);

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
}   /* end HCPendaccess() */

/*--------------------------------------------------------------------------
 NAME
    HCPcloseAID -- Get rid of the compressed data element data structures
 USAGE
    int32 HCPcloseAID(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element
 RETURNS
    Returns SUCCEED or FAIL
 DESCRIPTION
    Get rid of the compressed data element internal data structures

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcloseAID(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPcloseAID");    /* for HERROR */
    compinfo_t *info;           /* special information record */
    int32       ret=SUCCEED;

    info = (compinfo_t *) access_rec->special_info;
    if ((ret = (*(info->minfo.model_funcs.endaccess)) (access_rec)) == FAIL)
        HRETURN_ERROR(DFE_MODEL, FAIL);

    /* Free the compression information */
    /* BMR - reset special_info to NULL after memory is freed; problem shown
       by the failure when running hdp list with a large file on PC - 12/6/98 */
    if (--(info->attached) == 0)
    {
       HDfree(info);
       access_rec->special_info = NULL;
    }
    return (ret);
}   /* end HCPcloseAID() */

/* ------------------------------- HCPinfo -------------------------------- */
/*
NAME
   HCPinfo -- return info about a compressed element
USAGE
   int32 HCPinfo(access_rec, info_block)
   accrec_t        *  access_rec;   IN: access record of access element
   sp_info_block_t * info_block;   OUT: information about the special element
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Return information about the given compressed element.  Info_block is
   assumed to be non-NULL.

---------------------------------------------------------------------------*/
int32
HCPinfo(accrec_t * access_rec, sp_info_block_t * info_block)
{
    CONSTR(FUNC, "HCPinfo");
    compinfo_t *info =          /* special information record */
    (compinfo_t *) access_rec->special_info;

    /* validate access record */
    if (access_rec->special != SPECIAL_COMP)
        HRETURN_ERROR(DFE_INTERNAL, FAIL);

    /* fill in the info_block */
    info_block->key = SPECIAL_COMP;

    info_block->comp_type = (int32)info->cinfo.coder_type;
    info_block->model_type = (int32)info->minfo.model_type;
    info_block->comp_size = Hlength(access_rec->file_id, DFTAG_COMPRESSED, info->comp_ref);

    return SUCCEED;
}   /* HCPinfo */

/* ------------------------------- HCPgetinfo ----------------------------- */
/*
NAME
   HCget_config_info -- return info about configuration of a compression method

USAGE
     intn HCget_config_info( comp_coder_t coder_type,  
	uint32* compression_config_info)
        comp_coder_t coder_type;  IN: the compression type queried  
	compression_config_info;  OUT: flags to indiat compression status
               
                0 -- not enabled
		COMP_DECODER_ENABLED - decoding enabled
                COMP_ENCODER_ENABLED - encoding enabled

RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Return information about the given compresion method. 

   Currently, reports if encoding and/or decoding are available. SZIP
   is the only method that varies in the current versions.


---------------------------------------------------------------------------*/
intn
HCget_config_info( comp_coder_t coder_type,  /* IN: compression type */
		uint32* compression_config_info)
{
    CONSTR(FUNC, "HCget_config_info");

    *compression_config_info = 0;
    switch (coder_type)
      {     
          case COMP_CODE_NONE:      /* "none" (i.e. no) encoding */
		*compression_config_info = 0;
          case COMP_CODE_RLE:   /* Run-length encoding */
          case COMP_CODE_NBIT:      /* N-bit encoding */
          case COMP_CODE_SKPHUFF:   /* Skipping Huffman encoding */
		*compression_config_info = COMP_DECODER_ENABLED|COMP_ENCODER_ENABLED;
              break;

          case COMP_CODE_JPEG:  /* jpeg may be optional */
		*compression_config_info = COMP_DECODER_ENABLED|COMP_ENCODER_ENABLED;
              break;
          case COMP_CODE_DEFLATE:   /* gzip 'deflate' encoding, maybe optional */
		*compression_config_info = COMP_DECODER_ENABLED|COMP_ENCODER_ENABLED;
              break;

           case COMP_CODE_SZIP:
#ifdef H4_HAVE_LIBSZ
	      if (SZ_encoder_enabled()) {
		*compression_config_info = COMP_DECODER_ENABLED|COMP_ENCODER_ENABLED;
		} else {
		*compression_config_info = COMP_DECODER_ENABLED;
		}
#else
		*compression_config_info = 0;
#endif /* H4_HAVE_LIBSZ */
              break;
          default:
		*compression_config_info = 0;
              HRETURN_ERROR(DFE_BADCODER, FAIL)
	}
    return SUCCEED;
}

/*--------------------------------------------------------------------------
 NAME
    HCPgetcomptype -- Retrieves compression type of an element
 USAGE
    intn HCPgetcomptype(aid, coder_type)
    int32 aid;                  IN: access record ID
    comp_coder_t* coder_type;   OUT: the type of compression
 RETURNS
    SUCCEED/FAIL
 DESCRIPTION
    This routine retrieves the compression type of the element, identified 
    by 'aid'.  It is very similar to HCPgetcompinfo except that it only
    retrieves the compression type and not the compression information.  The 
    routine is used by GRgetcomptype and SDgetcomptype.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
    Dec. 2007: Added so that applications can get the compression method only.
		The immediate intention is to avoid the need for external 
		libraries to be present when only compression type is desired
		and not compression information. -BMR
--------------------------------------------------------------------------*/
intn
HCPgetcomptype(int32 file_id,
              uint16 data_tag, uint16 data_ref, /* IN: tag/ref of element */
              comp_coder_t* comp_type)  /* OUT: compression type */
{
    CONSTR(FUNC, "HCPgetcomptype");	/* for HGOTO_ERROR */
    uint16      ctag, cref;	/* tag/ref for the special info header object */
    int32       data_id=0;	/* temporary AID for header info */
    int32	temp_aid=0;	/* temporary AID for header info */
    int32	data_len;	/* offset of the data we are checking */
    uint8      *p;		/* pointers to the temporary buffer */
    uint8      *local_ptbuf=NULL;	/* temporary buffer */
    uint16	sp_tag;		/* special tag */
    uint16	c_type;		/* compression type */
    filerec_t  *file_rec;	/* file record */
    intn        ret_value=SUCCEED;

    /* clear error stack */
    HEclear();

    /* convert file id to file rec and check for validity */
    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get access element from dataset's tag/ref */
    if ((data_id=HTPselect(file_rec, data_tag, data_ref))!=FAIL)
    {
	/* get the info for the dataset */
	if(HTPinquire(data_id,&ctag,&cref,NULL,&data_len)==FAIL)
	    HGOTO_ERROR(DFE_INTERNAL, FAIL);

	/* if element is not special, return COMP_CODE_NONE */
	if (!SPECIALTAG(ctag))	
	{
	    *comp_type = COMP_CODE_NONE;
	    HGOTO_DONE(SUCCEED);
	}

	/* element is special, proceed with reading special info header */
	if((local_ptbuf=(uint8 *)HDmalloc(data_len))==NULL)
	    HGOTO_ERROR(DFE_NOSPACE, FAIL);

	/* Get the special info header */
	if ((temp_aid=Hstartaccess(file_id,MKSPECIALTAG(ctag),cref,DFACC_READ)) == FAIL)
	    HGOTO_ERROR(DFE_BADAID, FAIL);
	if (Hread(temp_aid,2,local_ptbuf) == FAIL)
	    HGOTO_ERROR(DFE_READERROR, FAIL);

	/* Get special tag */
	p = local_ptbuf;
	INT16DECODE(p, sp_tag);

	/* If it is a compressed element, move forward until compression
	   coder and get it */
	switch (sp_tag)
	{
	  case SPECIAL_COMP:
	      if (Hread(temp_aid,0,local_ptbuf) == FAIL)
		  HGOTO_ERROR(DFE_READERROR, FAIL);

	      /* Skip comp version, length, ref#, and model type */
	      p = local_ptbuf + 2 + 4 + 2 + 2;
	      UINT16DECODE(p, c_type);     /* get encoding type */
	      *comp_type=(comp_coder_t)c_type;
	      break;

	  /* If element is chunked, hand over to the chunk interface to check 
	     if it is compressed and get the type */
	  case SPECIAL_CHUNKED:
	      if (HMCgetcomptype(temp_aid, comp_type)==FAIL)
		  HGOTO_ERROR(DFE_INTERNAL, FAIL);
	      break;

	  /* return COMP_CODE_NONE for a non-compressed element */
	  /* Developer's Note: SPECIAL_COMPRAS may need special handling */
	  case SPECIAL_LINKED:
	  case SPECIAL_EXT:
	  case SPECIAL_VLINKED:
	  case SPECIAL_BUFFERED:
	  case SPECIAL_COMPRAS:
	  case 0:
	      *comp_type = COMP_CODE_NONE;
	      break;

	  /* flag the error when special tag is not something valid */
	  default:
	      *comp_type = COMP_CODE_INVALID;
	      HGOTO_ERROR(DFE_ARGS, FAIL);
	}

	/* release allocated memory */
	if (local_ptbuf != NULL)
	    HDfree(local_ptbuf);

	if(Hendaccess(temp_aid)==FAIL)
	    HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

	/* end access to the aid appropriately */
	if (HTPendaccess(data_id)== FAIL)
	    HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);
    }
    else /* no special element */
    {
        *comp_type = COMP_CODE_NONE;
    }

done:
  if(ret_value == FAIL)
    { /* Error condition cleanup */
       /* end access to the aid's if they've been accessed */
        if (temp_aid != 0)
            if (Hendaccess(temp_aid)== FAIL)
                HERROR(DFE_CANTENDACCESS);
        if (data_id != 0)
            if (HTPendaccess(data_id)== FAIL)
                HERROR(DFE_CANTENDACCESS);
	if (local_ptbuf != NULL)
	    HDfree(local_ptbuf);
    } /* end if */

  /* Normal function cleanup */
  return ret_value;
} /* HCPgetcomptype */

/*--------------------------------------------------------------------------
 NAME
    HCPgetdatasize -- Retrieves the sizes of original and compressed data.
 USAGE
    int32 HCPgetdatasize(file_id, data_tag, data_ref, comp_size, orig_size)	
		int32 file_id;		IN: file id
		uint16 data_tag;	IN: tag of the element
		uint16 data_ref;	IN: ref of element
		int32* comp_size;	OUT: size of compressed data
		int32* orig_size;	OUT: size of non-compressed data
 RETURNS
    SUCCEED/FAIL
 DESCRIPTION
    This routine gets access to the element pointed to by the dataset's 
    tag/ref pair, then proceeds as followed:
    - If the element is not special, HCPgetdatasize will use Hlength to get 
      the length of the data then return.
    - If the element is compressed, HCPgetdatasize will read the element's
      special header and decode it for the uncompressed data length and the 
      compressed data ref#, then use Hlength to get the length of the 
      compressed data.
    - If the element is chunked, HCPgetdatasize will let the chunking layer
      retrieve the sizes (HMCgetdatasize.)
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
HCPgetdatasize(int32 file_id,
              uint16 data_tag, uint16 data_ref, /* IN: tag/ref of element */
              int32* comp_size,	/* OUT  - size of compressed data */
              int32* orig_size)	/* OUT  - size of non-compressed data */
{
    CONSTR(FUNC, "HCPgetdatasize");	/* for HGOTO_ERROR */
    int32	data_len;	/* offset of the data we are checking */
    uint8      *local_ptbuf=NULL, *p;
    uint16	sp_tag;		/* special tag */
    uint16 comp_ref = 0;
    uint16	drec_tag, drec_ref;	/* description record tag/ref */
    int32	drec_aid;	/* description record access id */
    atom_t      data_id = FAIL;	/* dd ID of existing regular element */
    int32 len = 0;
    filerec_t  *file_rec;	/* file record */
    intn        ret_value=SUCCEED;

    /* clear error stack */
    HEclear();

    /* convert file id to file rec and check for validity */
    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
	HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get access element from dataset's tag/ref */
    if ((data_id=HTPselect(file_rec, data_tag, data_ref))!=FAIL)
    {
	/* if the element is not special, that means dataset's tag/ref 
	   specifies the actual data that was written to the dataset, so
	   we don't need to check further */
	if (HTPis_special(data_id)==FALSE)
        {
	    if ((len = Hlength(file_id, data_tag, data_ref)) == FAIL)
		HGOTO_ERROR(DFE_BADLEN, FAIL);
	    *orig_size = *comp_size = len;
        }

	/* if the element is special, get the special info header and decode
	   for the uncompressed data length and the compressed data ref#, which
	   will be used with DFTAG_COMPRESSED to get the compressed data len */
	else
	{
	    /* get the info for the dataset (description record) */
	    if (HTPinquire(data_id,&drec_tag,&drec_ref,NULL,&data_len) == FAIL)
		HGOTO_ERROR(DFE_INTERNAL, FAIL);
	    if ((local_ptbuf = (uint8 *)HDmalloc(data_len)) == NULL)
		HGOTO_ERROR(DFE_NOSPACE, FAIL);

	    /* get the special info header */
	    drec_aid = Hstartaccess(file_id,MKSPECIALTAG(drec_tag),drec_ref,DFACC_READ);
	    if (drec_aid == FAIL)
		HGOTO_ERROR(DFE_BADAID, FAIL);
	    if (Hread(drec_aid,0,local_ptbuf) == FAIL)
		HGOTO_ERROR(DFE_READERROR, FAIL);
	    if(Hendaccess(drec_aid)==FAIL)
		HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

	    /* get special tag */
	    p = local_ptbuf;
	    INT16DECODE(p, sp_tag);

	    /* verify that it is a compressed element, then get the data len */
	    if (sp_tag == SPECIAL_COMP)
	    {
		/* skip 2byte header_version */
		p = p + 2;
		INT32DECODE(p, len);	/* get _uncompressed_ data length */
		*orig_size = len;	/* set original data size */

		/* if no data written, set compressed data size too */
		if (len == 0)
		{
		    *comp_size = len;
		}
		/* Data has been written, get compressed data size */
		else
		{
		    /* get ref# of compressed data */
		    UINT16DECODE(p, comp_ref);
		    if ((len = Hlength(file_id, DFTAG_COMPRESSED, comp_ref)) == FAIL)
			HGOTO_ERROR(DFE_BADLEN, FAIL);
		    *comp_size = len;	/* set compressed data size */
		} /* data written */
	    } /* element is compressed */

	    /* if it is a chunked element, hand the task over to the chunking
		layer. */
	    else if (sp_tag == SPECIAL_CHUNKED)
	    {
	     if (HMCgetdatasize(file_id, p, comp_size, orig_size)==FAIL)
		  HGOTO_ERROR(DFE_INTERNAL, FAIL);
	    }

	    /* unlimited dimension falls in here */
	    else if (sp_tag == SPECIAL_LINKED)
	    {
		INT32DECODE(p, len);	/* get total data length */
		*orig_size = *comp_size = len;	/* set data sizes */
	    }
	} /* else, data_id is special */

	/* end access to the aid */
	if (HTPendaccess(data_id) == FAIL)
	    HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);
    }  /* end if data_id != FAIL */

    else /* HTPselect failed */
        HGOTO_ERROR(DFE_CANTACCESS, FAIL);
 
done:
    if(ret_value == FAIL)   
    { /* Error condition cleanup */
    } /* end if */

    /* Normal function cleanup */
    if (local_ptbuf != NULL) 
	HDfree(local_ptbuf);

    return ret_value;
} /* HCPgetdatasize */
