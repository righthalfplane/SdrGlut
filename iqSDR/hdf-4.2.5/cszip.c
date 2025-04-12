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
static char RcsId[] = "@(#)$Revision: 5108 $";
#endif

/* $Id: cszip.c 5108 2008-10-22 18:49:22Z bmribler $ */

/* General HDF includes */

#include "hdf.h"
#include <assert.h>

#ifdef H4_HAVE_LIBSZ
#include "szlib.h"
#endif

#define CSZIP_MASTER
#define CODER_CLIENT
/* HDF compression includes */
#include "hcompi.h"     /* Internal definitions for compression */

/* internal defines */
#define TMP_BUF_SIZE    8192    /* size of throw-away buffer */

/* declaration of the functions provided in this module */
PRIVATE int32 HCIcszip_staccess
            (accrec_t * access_rec, int16 acc_mode);

PRIVATE int32 HCIcszip_init
            (accrec_t * access_rec);

PRIVATE int32 HCIcszip_decode
            (compinfo_t * info, int32 length, uint8 *buf);

PRIVATE int32 HCIcszip_encode
            (compinfo_t * info, int32 length, const uint8 *buf);

PRIVATE int32 HCIcszip_term
            (compinfo_t * info);

/*--------------------------------------------------------------------------
 NAME
    HCIcszip_init -- Initialize a SZIP compressed data element.

 USAGE
    int32 HCIcszip_init(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called by HCIcszip_staccess and HCIcszip_seek

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcszip_init(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCIcszip_init");
    compinfo_t *info;           /* special element information */
    comp_coder_szip_info_t *szip_info;    /* ptr to SZIP info */
    intn       ret_value = SUCCEED;

#ifdef H4_HAVE_LIBSZ
    /* Sanity check to make certain that we haven't drifted out of date with
     * the mask options from the SZIP ricehdf.h header */
    assert(H4_SZ_ALLOW_K13_OPTION_MASK==SZ_ALLOW_K13_OPTION_MASK);
    assert(H4_SZ_CHIP_OPTION_MASK==SZ_CHIP_OPTION_MASK);
    assert(H4_SZ_EC_OPTION_MASK==SZ_EC_OPTION_MASK);
    assert(H4_SZ_LSB_OPTION_MASK==SZ_LSB_OPTION_MASK);
    assert(H4_SZ_MSB_OPTION_MASK==SZ_MSB_OPTION_MASK);
    assert(H4_SZ_NN_OPTION_MASK==SZ_NN_OPTION_MASK);
    assert(H4_SZ_RAW_OPTION_MASK==SZ_RAW_OPTION_MASK);
#endif

    info = (compinfo_t *) access_rec->special_info;
    if (Hseek(info->aid, 0, DF_START) == FAIL)  /* seek to beginning of element */
        HRETURN_ERROR(DFE_SEEKERROR, FAIL);

    szip_info = &(info->cinfo.coder_info.szip_info);

    /* Initialize SZIP state information */
    szip_info->szip_state = SZIP_INIT;     /* start in initial state */
    if (szip_info->buffer_size != 0) {
        szip_info->buffer_size = 0;   /* offset into the file */
        if (szip_info->buffer != NULL) {
		HDfree(szip_info->buffer);
		szip_info->buffer = NULL;
        }
    }
    szip_info->offset = 0;   /* offset into the file */
    szip_info->szip_dirty=SZIP_CLEAN;


    return(ret_value);
}   /* end HCIcszip_init() */

/*--------------------------------------------------------------------------
 NAME
    HCIcszip_decode -- Decode SZIP compressed data into a buffer.

 USAGE
    int32 HCIcszip_decode(info,length,buf)
    compinfo_t *info;   IN: the info about the compressed element
    int32 length;       IN: number of bytes to read into the buffer
    uint8 *buf;         OUT: buffer to store the bytes read

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called to decode SZIP data from the file.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcszip_decode(compinfo_t * info, int32 length, uint8 *buf)
{
    CONSTR(FUNC, "HCIcszip_decode");
 #ifdef H4_HAVE_LIBSZ
   accrec_t *access_rec;
    comp_coder_szip_info_t *szip_info;
    uint8 *in_buffer;
    uint8 *out_buffer;
    int32 in_length;
    int32 out_length;
    int bytes_per_pixel;
    int32 rbytes;
    uint16 tag,ref;
    int32 len1;
   int32 aid;
    int32 status;
    size_t size_out;
    uint8 *cp;
    int32 good_bytes;
    int32 old_way;
    SZ_com_t sz_param;
#endif

#ifdef H4_HAVE_LIBSZ

    szip_info = &(info->cinfo.coder_info.szip_info);
    if (szip_info->szip_state == SZIP_INIT) {
	/*  Load from disk, decode the data */

	if ((access_rec = HAatom_object(info->aid)) == NULL)    /* get the access_rec pointer */
	    HRETURN_ERROR(DFE_ARGS, FAIL);

	/* Discover how much data must be read */
	if(HTPinquire(access_rec->ddid,&tag,&ref,NULL,&in_length)==FAIL)
		HRETURN_ERROR(DFE_INTERNAL, FAIL);

        if (in_length == -1)
		HRETURN_ERROR(DFE_INTERNAL, FAIL);

        if (tag & 0x4000) {
	    /* this is linked list -- get the length of the data */
            aid = Hstartread(access_rec->file_id, tag, ref);
            if (HDinqblockinfo(aid, &len1, NULL, NULL, NULL) == FAIL) {
	       Hendaccess(aid);
	       HRETURN_ERROR(DFE_INTERNAL, FAIL);
            }
            in_length = len1; 
	    Hendaccess(aid);
        }

	old_way = (int)(szip_info->options_mask & SZ_H4_REV_2);
	if (old_way == 0) {
		/* special case: read data encoded in V4.2r0 */
		old_way = 1;
		good_bytes = in_length;
                in_length = in_length+5;
	        if ((in_buffer = (uint8 *) HDmalloc(in_length)) == NULL)
    	           HRETURN_ERROR(DFE_NOSPACE, FAIL);
		cp = in_buffer;
		*cp = 0;
		cp++;
		INT32ENCODE(cp, good_bytes);
	} else {
		/*  V4.2r1: in_length is correct */
		old_way = 0;
	        if ((in_buffer = (uint8 *) HDmalloc(in_length)) == NULL)
    	           HRETURN_ERROR(DFE_NOSPACE, FAIL);
	}

        /* Allocate memory for the uncompressed data */
	bytes_per_pixel = (szip_info->bits_per_pixel + 7) >> 3;
	if (bytes_per_pixel == 3)
		bytes_per_pixel++;

        out_length = szip_info->pixels * bytes_per_pixel;
	if ((out_buffer = (uint8 *) HDmalloc(out_length)) == NULL)
		HRETURN_ERROR(DFE_NOSPACE, FAIL);

	/* Read the unompressed data */
	if (old_way == 1) {
		/* this is encoded in V4.2r0 */
		/* the preamble isn't in the file, so read only the data */
		if ((rbytes = Hread(info->aid, in_length-5, in_buffer+5)) == FAIL)
		{
			HDfree(out_buffer);
			HDfree(in_buffer);
			HRETURN_ERROR(DFE_READERROR, FAIL);
		}
		if (rbytes == 0 || rbytes != (in_length - 5)) {
			/* is this possible? */
			HDfree(out_buffer);
			HDfree(in_buffer);
			HRETURN_ERROR(DFE_READERROR, FAIL);
		}
	} else {
		/* HDF4.2R1: read the data plus preamble */
		if ((rbytes = Hread(info->aid, in_length, in_buffer)) == FAIL)
		{
			HDfree(out_buffer);
			HDfree(in_buffer);
			HRETURN_ERROR(DFE_READERROR, FAIL);
		}
		if (rbytes == 0 || rbytes != in_length) {
			/* is this possible? */
			HDfree(out_buffer);
			HDfree(in_buffer);
			HRETURN_ERROR(DFE_READERROR, FAIL);
		}
	}
        cp = in_buffer;
        cp++;
        INT32DECODE(cp, good_bytes);
	if (in_buffer[0] == 1) {
           /* This byte means the data was not compressed -- just copy out */
	    szip_info->szip_state = SZIP_RUN;
	    HDmemcpy(out_buffer, in_buffer+5, good_bytes);
	    szip_info->buffer = out_buffer;
	    szip_info->buffer_pos = 0;
	    szip_info->buffer_size = good_bytes;
	    szip_info->offset = 0;
	    if (good_bytes > length) {
		/* partial read */
		HDmemcpy(buf, in_buffer+5, length);
	        szip_info->buffer_pos += length;
	        szip_info->buffer_size -= length;
	    } else {
		/* read the whole data block to the user buffer */
		HDmemcpy(buf, in_buffer+5, good_bytes);
	        szip_info->buffer_pos += good_bytes;
	        szip_info->buffer_size -= good_bytes;
	    }
	    szip_info->offset = szip_info->buffer_pos;
	    HDfree(in_buffer);
	    if (szip_info->buffer_size == 0) {
		if (szip_info->buffer != NULL) {
		   HDfree(szip_info->buffer);
		   szip_info->buffer = NULL;
		}
	    }
	    return (SUCCEED);
        }

	/* Decompress the data */
     
        /* set up the parameters */
	sz_param.options_mask = (szip_info->options_mask & ~SZ_H4_REV_2);
	sz_param.bits_per_pixel = szip_info->bits_per_pixel;
	sz_param.pixels_per_block = szip_info->pixels_per_block;
	sz_param.pixels_per_scanline = szip_info->pixels_per_scanline;
	size_out = out_length;
	if(SZ_OK!= (status = SZ_BufftoBuffDecompress(out_buffer, &size_out, (in_buffer+5), good_bytes, &sz_param)))
         {
		HDfree(out_buffer);
		HDfree(in_buffer);
		HRETURN_ERROR(DFE_CDECODE, FAIL);
        }

	if ((int32)size_out != out_length) {
	   /* This should never happen?? */
	   printf("status: %d ??bytes != out_length %d != %d\n",status,size_out,out_length);
	}

        /* The data is successfully decompressed. Put into the szip struct */
	 HDfree(in_buffer);
	 szip_info->szip_state = SZIP_RUN;
	 szip_info->buffer = out_buffer;
	 szip_info->buffer_pos = 0;
	 szip_info->buffer_size = out_length;
	 szip_info->offset = 0;
    }

   /* copy the data into the return buffer */
    if (length > szip_info->buffer_size)
    {	
        /*  can't happen?? panic?? */
	if (szip_info->buffer != NULL) {
            HDfree(szip_info->buffer);
	    szip_info->buffer = NULL;
        } 
        return (FAIL);
    }

    HDmemcpy(buf, szip_info->buffer + szip_info->buffer_pos, length);
    szip_info->buffer_pos += length;
    szip_info->buffer_size -= length;
    szip_info->offset = szip_info->buffer_pos;

    if (szip_info->buffer_size == 0) {
       if (szip_info->buffer != NULL) {
  	    HDfree(szip_info->buffer);
	    szip_info->buffer = NULL;
       }
    }

    return (SUCCEED);

#else /* ifdef H4_HAVE_LIBSZ */

    HRETURN_ERROR(DFE_CANTCOMP, FAIL);

#endif /* H4_HAVE_LIBSZ */

}   /* end HCIcszip_decode() */

/*--------------------------------------------------------------------------
 NAME
    HCIcszip_encode -- Encode data from a buffer into SZIP compressed data

 USAGE
    int32 HCIcszip_encode(info,length,buf)
    compinfo_t *info;   IN: the info about the compressed element
    int32 length;       IN: number of bytes to store from the buffer
    const uint8 *buf;         OUT: buffer to get the bytes from

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called to encode SZIP data into a file.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcszip_encode(compinfo_t * info, int32 length, const uint8 *buf)
{
    CONSTR(FUNC, "HCIcszip_encode");
 #ifdef H4_HAVE_SZIP_ENCODER
   int bytes_per_pixel;
    comp_coder_szip_info_t *szip_info;    /* ptr to SZIP info */
    int32 buffer_size;

    if (SZ_encoder_enabled() == 0) 
        HRETURN_ERROR(DFE_NOENCODER, FAIL);

    szip_info = &(info->cinfo.coder_info.szip_info);
    if (szip_info->szip_state == SZIP_INIT) {
	/* Need to initialize */
	bytes_per_pixel = (szip_info->bits_per_pixel + 7) >> 3;
	if (bytes_per_pixel == 3)
		bytes_per_pixel = 4;

	buffer_size = szip_info->pixels * bytes_per_pixel;
	if ((szip_info->buffer = HDmalloc(buffer_size)) == NULL)
			HRETURN_ERROR(DFE_NOSPACE, FAIL);
	
	szip_info->buffer_size = buffer_size;
	szip_info->buffer_pos = 0;
	szip_info->szip_state = SZIP_RUN;
    }

    /* copy the data into the buffer.  This wil be written in 'term' function */
    HDmemcpy(szip_info->buffer + szip_info->buffer_pos, buf, length);
    szip_info->buffer_pos += length;
    szip_info->buffer_size -= length;
    szip_info->offset = szip_info->buffer_pos; 
    szip_info->szip_dirty=SZIP_DIRTY;

    return (SUCCEED);

#else /* ifdef H4_HAVE_SZIP_ENCODER */

    HRETURN_ERROR(DFE_CANTDECOMP, FAIL);

#endif /* H4_HAVE_SZIP_ENCODER */

}   /* end HCIcszip_encode() */

/*--------------------------------------------------------------------------
 NAME
    HCIcszip_term -- Flush encoded data from internal buffer to SZIP compressed data

 USAGE
    int32 HCIcszip_term(info)
    compinfo_t *info;   IN: the info about the compressed element

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called to flush SZIP data into a file.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcszip_term(compinfo_t * info)
{
    CONSTR(FUNC, "HCIcszip_term");
#ifdef H4_HAVE_SZIP_ENCODER
    comp_coder_szip_info_t *szip_info;
    uint8 *out_buffer;
    uint8 *ob;
    int32 out_buffer_size;
    int bytes_per_pixel;
    int32 current_size;
    accrec_t *access_rec;
    uint16 tag,ref;
    int32 len1;
    int32 aid;
    SZ_com_t sz_param;
    size_t size_out;
    int32 status;
    uint8 *cp;
#endif

#ifdef H4_HAVE_SZIP_ENCODER

    szip_info = &(info->cinfo.coder_info.szip_info);
    if (szip_info->szip_state != SZIP_RUN)
	return (SUCCEED); /* nothing to do */

    if (szip_info->szip_dirty != SZIP_DIRTY) /* Should never happen?? */
    {
	    if (szip_info->buffer_size == 0) {
                if (szip_info->buffer != NULL) {
		    HDfree(szip_info->buffer);
	            szip_info->buffer = NULL;
                }
            }
	return (SUCCEED);
    }

    szip_info->szip_state = SZIP_TERM;

    current_size = 0;
    if ((access_rec = HAatom_object(info->aid)) == NULL)    /* get the access_rec pointer */
		HRETURN_ERROR(DFE_INTERNAL, FAIL);

    /* Discover how much data must be read */
    if(HTPinquire(access_rec->ddid,&tag,&ref,NULL, &current_size)==FAIL)
	HRETURN_ERROR(DFE_INTERNAL, FAIL);
    if (tag & 0x4000) {
    /* this is linked list -- get the length of the data */
        aid = Hstartread(access_rec->file_id, tag, ref);
        if (HDinqblockinfo(aid, &len1, NULL, NULL, NULL) == FAIL) 
        {
            Hendaccess(aid);
	    HRETURN_ERROR(DFE_INTERNAL, FAIL);
        }
        current_size = len1; 
	Hendaccess(aid);
    }

   /* Compute how much memory is needed to hold compressed data */
    bytes_per_pixel = (szip_info->bits_per_pixel + 7) >> 3;
    if (bytes_per_pixel == 3)
	bytes_per_pixel = 4;

    /* temporary buffer for compression -- leave extra space in case of
          overflow in the SZIP algorithm. (This number corresponds to
          the current internal buffer of szip lib.)  Sigh. */
    /* allocate one byte to indicate when data is written uncompressed */
    /* allocate 4 bytes to store the amount of data written:  
        after compression may be less than the previous size  */
    out_buffer_size = (szip_info->pixels * 2 * bytes_per_pixel) + 5;

    /* heuristic for tiny data -- really shouldn't compress stuff this small,
       but there isn't any way to prevent it from getting here */
    if (out_buffer_size < 1024) out_buffer_size = 1024;
    if ((out_buffer = HDmalloc(out_buffer_size)) == NULL)
		HRETURN_ERROR(DFE_NOSPACE, FAIL);

    /* set parms */
    sz_param.options_mask = szip_info->options_mask;
    sz_param.bits_per_pixel = szip_info->bits_per_pixel;
    sz_param.pixels_per_block = szip_info->pixels_per_block;
    sz_param.pixels_per_scanline = szip_info->pixels_per_scanline;
    size_out = out_buffer_size - 5;

    *out_buffer=0;
    if(SZ_OK!= (status =SZ_BufftoBuffCompress((out_buffer+5), &size_out, szip_info->buffer, szip_info->buffer_pos, &sz_param)))
    {
       /* Compression Failed.  Analyse several cases, and clean up the mess */
	if ((int32)size_out > out_buffer_size) {
                /* Should never happen */
		printf("PANIC: overwrote memory\n");fflush(stdout);
	}
	if (status == 2/*SZ_OUTBUF_FULL*/) {
            /* SZIP internal overflow: data not successfully compressed */
            /* Write out the uncompressed data */
	    *out_buffer=1;  /* 1 = don't decompress */
            cp = out_buffer;
            cp++;
            INT32ENCODE(cp, szip_info->buffer_pos);
	    HDmemcpy((out_buffer+5), szip_info->buffer, szip_info->buffer_pos);
	    HDfree(out_buffer);
	    szip_info->szip_dirty=SZIP_CLEAN;
			
	    if (szip_info->buffer_size == 0) {
	        if (szip_info->buffer != NULL)  {
		    HDfree(szip_info->buffer);
	            szip_info->buffer = NULL;
                }
            }
	    return (SUCCEED);
	 } 

	  /* compress failed, return error */
          szip_info->szip_dirty=SZIP_CLEAN;
          HDfree(out_buffer);
	  if (szip_info->buffer_size == 0) {
	        if (szip_info->buffer != NULL)  {
		    HDfree(szip_info->buffer);
	            szip_info->buffer = NULL;
                }
          }
	  HRETURN_ERROR(DFE_CENCODE, FAIL);
	}

        /* Compression Succeeded, write out the compressed data */

	if ((int32)size_out >= out_buffer_size) 
		printf("PANIC: overwrote memory but returned OK?\n");fflush(stdout);
	if ((int32)size_out > (szip_info->pixels * bytes_per_pixel)) {
            /* The compression succeeded, but is larger than the original data! */
	    /*  Write the original data, discard the output  */
	    *out_buffer=1;  /* 1 = don't decompress */
            cp = out_buffer;
            cp++;
            INT32ENCODE(cp, szip_info->buffer_pos);
	    HDmemcpy((out_buffer+5), szip_info->buffer, szip_info->buffer_pos);
	    Hwrite(info->aid, (szip_info->buffer_pos+5), out_buffer);
	    szip_info->szip_dirty=SZIP_CLEAN;
	    HDfree(out_buffer);
	    if (szip_info->buffer_size == 0) {
	        if (szip_info->buffer != NULL) {
		    HDfree(szip_info->buffer);
	            szip_info->buffer = NULL;
                }
            }
	    return (SUCCEED);
	}

      if ((current_size > 0) && (((int32)size_out+5) < current_size)) {
            /* SZIP freaks out if there is junk at the end of the good data */
            /* need to have enough data to overwrite the existing data */
            /* allocate a buffer, fill in the good data. The rest must be
                zeroes */
            if ((ob = HDmalloc(current_size)) == NULL)
		HRETURN_ERROR(DFE_NOSPACE, FAIL);
	    *ob=0;  /* data needs to be decompressed */
            cp = ob;
            cp++;
            INT32ENCODE(cp, size_out); /* how much to decompress  (< total size)*/
	    HDmemcpy((ob+5), out_buffer+5, size_out);
	    Hwrite(info->aid, current_size, ob);  /* write out at least 'current_size' bytes */
	    szip_info->szip_dirty=SZIP_CLEAN;
	    HDfree(out_buffer);
	    HDfree(ob);
	    if (szip_info->buffer_size == 0) {
	        if (szip_info->buffer != NULL)  {
		    HDfree(szip_info->buffer);
	            szip_info->buffer = NULL;
                }
            }
	    return (SUCCEED);
     } 

      /* Finally!  Write the compressed data. Byte 0 is '0' */ 
    *out_buffer=0;   /* data needs to be decompressed */
    cp = out_buffer;
    cp++;
    INT32ENCODE(cp, size_out);   /* whole bufer needs to be decompressed */
    status = Hwrite(info->aid, size_out+5, out_buffer);

    szip_info->szip_dirty=SZIP_CLEAN;
    if (szip_info->buffer_size == 0) {
	if (szip_info->buffer != NULL) {
	    HDfree(szip_info->buffer);
	    szip_info->buffer = NULL;
        }
    }
    HDfree(out_buffer);

    return (SUCCEED);

#else /* H4_HAVE_SZIP_ENCODER */

    HRETURN_ERROR(DFE_CANTCOMP, FAIL);

#endif /* H4_HAVE_SZIP_ENCODER */

}   /* end HCIcszip_term() */

/*--------------------------------------------------------------------------
 NAME
    HCIcszip_staccess -- Start accessing a SZIP compressed data element.

 USAGE
    int32 HCIcszip_staccess(access_rec, access)
    accrec_t *access_rec;   IN: the access record of the data element
    int16 access;           IN: the type of access wanted

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called by HCIcszip_stread and HCIcszip_stwrite

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcszip_staccess(accrec_t * access_rec, int16 acc_mode)
{
    CONSTR(FUNC, "HCIcszip_staccess");
    compinfo_t *info;           /* special element information */

    info = (compinfo_t *) access_rec->special_info;
    if (acc_mode == DFACC_READ)
        info->aid = Hstartread(access_rec->file_id, DFTAG_COMPRESSED,
                               info->comp_ref);
    else
#ifdef H4_HAVE_SZIP_ENCODER
	{
	    if (SZ_encoder_enabled() == 0) 
		HRETURN_ERROR(DFE_NOENCODER, FAIL);
	    info->aid = Hstartaccess(access_rec->file_id, DFTAG_COMPRESSED,
                       info->comp_ref, DFACC_RDWR|DFACC_APPENDABLE);
	}
#else /* H4_HAVE_SZIP_ENCODER */
    HRETURN_ERROR(DFE_DENIED, FAIL);
#endif /* H4_HAVE_SZIP_ENCODER */

    if (info->aid == FAIL)
        HRETURN_ERROR(DFE_DENIED, FAIL);

    return (HCIcszip_init(access_rec));  /* initialize the SZIP info */
}   /* end HCIcszip_staccess() */

/*--------------------------------------------------------------------------
 NAME
    HCPcszip_stread -- start read access for compressed file

 USAGE
    int32 HCPcszip_stread(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Start read access on a compressed data element using a simple SZIP scheme.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcszip_stread(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPcszip_stread");
    int32       ret;

    if ((ret = HCIcszip_staccess(access_rec, DFACC_READ)) == FAIL)
        HRETURN_ERROR(DFE_CINIT, FAIL);
    return (ret);
}   /* HCPcszip_stread() */

/*--------------------------------------------------------------------------
 NAME
    HCPcszip_stwrite -- start write access for compressed file

 USAGE
    int32 HCPcszip_stwrite(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Start write access on a compressed data element using a simple SZIP scheme.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcszip_stwrite(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPcszip_stwrite");
    int32       ret;

    if ((ret = HCIcszip_staccess(access_rec, DFACC_WRITE)) == FAIL)
        HRETURN_ERROR(DFE_CINIT, FAIL);
    return (ret);
}   /* HCPcszip_stwrite() */

/*--------------------------------------------------------------------------
 NAME
    HCPcszip_seek -- Seek to offset within the data element

 USAGE
    int32 HCPcszip_seek(access_rec,offset,origin)
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

    COMMENT:

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcszip_seek(accrec_t * access_rec, int32 offset, int origin)
{
    CONSTR(FUNC, "HCPcszip_seek");
    compinfo_t *info;           /* special element information */
    comp_coder_szip_info_t *szip_info;    /* ptr to SZIP info */
    uint8      *tmp_buf;        /* pointer to throw-away buffer */

    /* shut compiler up */
    origin = origin;

    info = (compinfo_t *) access_rec->special_info;
    szip_info = &(info->cinfo.coder_info.szip_info);

    if (offset < szip_info->offset)
      {     /* need to seek from the beginning */
          if (szip_info->szip_dirty == SZIP_DIRTY && szip_info->szip_state != SZIP_INIT)
{
              if (HCIcszip_term(info) == FAIL)
                  HRETURN_ERROR(DFE_CTERM, FAIL);
}
          if (HCIcszip_init(access_rec) == FAIL)
              HRETURN_ERROR(DFE_CINIT, FAIL);
      }     /* end if */

    if ((tmp_buf = (uint8 *) HDmalloc(TMP_BUF_SIZE)) == NULL)     /* get tmp buffer */
        HRETURN_ERROR(DFE_NOSPACE, FAIL);

    while (szip_info->offset + TMP_BUF_SIZE < offset)    /* grab chunks */
    {
        if (HCIcszip_decode(info, TMP_BUF_SIZE, tmp_buf) == FAIL)
          {
              HDfree(tmp_buf);
              HRETURN_ERROR(DFE_CDECODE, FAIL)
          }     /* end if */
    }
    if (szip_info->offset < offset)  /* grab the last chunk */
    {
        if (HCIcszip_decode(info, offset - szip_info->offset, tmp_buf) == FAIL)
          {
              HDfree(tmp_buf);
              HRETURN_ERROR(DFE_CDECODE, FAIL)
          }     /* end if */
    }

    HDfree(tmp_buf);
    return (SUCCEED);
}   /* HCPcszip_seek() */

/*--------------------------------------------------------------------------
 NAME
    HCPcszip_read -- Read in a portion of data from a compressed data element.

 USAGE
    int32 HCPcszip_read(access_rec,length,data)
    accrec_t *access_rec;   IN: the access record of the data element
    int32 length;           IN: the number of bytes to read
    void * data;             OUT: the buffer to place the bytes read

 RETURNS
    Returns the number of bytes read or FAIL

 DESCRIPTION
    Read in a number of bytes from a SZIP compressed data element.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcszip_read(accrec_t * access_rec, int32 length, void * data)
{
    CONSTR(FUNC, "HCPcszip_read");
    compinfo_t *info;           /* special element information */

    info = (compinfo_t *) access_rec->special_info;

    if (HCIcszip_decode(info, length, data) == FAIL)
        HRETURN_ERROR(DFE_CDECODE, FAIL);

    return (length);
}   /* HCPcszip_read() */

/*--------------------------------------------------------------------------
 NAME
    HCPcszip_write -- Write out a portion of data from a compressed data element.

 USAGE
    int32 HCPcszip_write(access_rec,length,data)
    accrec_t *access_rec;   IN: the access record of the data element
    int32 length;           IN: the number of bytes to write
    void * data;             IN: the buffer to retrieve the bytes written

 RETURNS
    Returns the number of bytes written or FAIL

 DESCRIPTION
    Write out a number of bytes to a SZIP compressed data element.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcszip_write(accrec_t * access_rec, int32 length, const void * data)
{
    CONSTR(FUNC, "HCPcszip_write");
#ifdef H4_HAVE_SZIP_ENCODER
    compinfo_t *info;           /* special element information */
    comp_coder_szip_info_t *szip_info;    /* ptr to SZIP info */

    if (SZ_encoder_enabled() == 0) 
	HRETURN_ERROR(DFE_NOENCODER, FAIL);
    info = (compinfo_t *) access_rec->special_info;
    szip_info = &(info->cinfo.coder_info.szip_info);

    /* Don't allow random write in a dataset unless: */
    /*  1 - append onto the end */
    /*  2 - start at the beginning and rewrite (at least) the whole dataset */
    if ((info->length != szip_info->offset)
        && (szip_info->offset != 0 || length < info->length))
        HRETURN_ERROR(DFE_UNSUPPORTED, FAIL);

    if (HCIcszip_encode(info, length, data) == FAIL)
        HRETURN_ERROR(DFE_CENCODE, FAIL);

    return (length);
#else /* ifdef H4_HAVE_SZIP_ENCODER */

    HRETURN_ERROR(DFE_CANTDECOMP, FAIL);

#endif /* H4_HAVE_SZIP_ENCODER */
}   /* HCPcszip_write() */

/*--------------------------------------------------------------------------
 NAME
    HCPcszip_inquire -- Inquire information about the access record and data element.

 USAGE
    int32 HCPcszip_inquire(access_rec,pfile_id,ptag,pref,plength,poffset,pposn,
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
HCPcszip_inquire(accrec_t * access_rec, int32 *pfile_id, uint16 *ptag,
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
}   /* HCPcszip_inquire() */

/*--------------------------------------------------------------------------
 NAME
    HCPcszip_endaccess -- Close the compressed data element

 USAGE
    int32 HCPcszip_endaccess(access_rec)
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
HCPcszip_endaccess(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPcszip_endaccess");
    compinfo_t *info;           /* special element information */
    comp_coder_szip_info_t *szip_info;    /* ptr to SZIP info */

    info = (compinfo_t *) access_rec->special_info;
    szip_info = &(info->cinfo.coder_info.szip_info);

    /* flush out SZIP buffer if there is unwritten data */
    if (szip_info->szip_dirty == SZIP_DIRTY && szip_info->szip_state != SZIP_INIT)
{
        if (HCIcszip_term(info) == FAIL)
            HRETURN_ERROR(DFE_CTERM, FAIL);
    }

    /* close the compressed data AID */
    if (Hendaccess(info->aid) == FAIL)
        HRETURN_ERROR(DFE_CANTCLOSE, FAIL);

    return (SUCCEED);
}   /* HCPcszip_endaccess() */


/*--------------------------------------------------------------------------
 NAME
    HCPsetup_szip_parms -- Initialize SZIP parameters

 USAGE
    intn HCPcszip_setup_parms( comp_info *c_info, int32 nt, int32 ndims, int32 *dims, int32 *cdims)
    comp_info *c_info;    IN/OUT: the szip compression params
    int32 nt;             IN: the number type of the data
    int32 ncomp;          IN: components in GR, 1 for SD
    int32 ndims;          IN: The rank of the data
    int32 *dims;          IN: the dimensions 
    int32 *cdims;         IN: the dimensions of a chunk, if chunked, or NULL;

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    
    Computes the SZIP parameters for dataset or chunk:
       pixels -- total elements per compression
       pixels_per_scanline 
       bits_per_pixel
    
    This is called from SDsetcompress, SDsetchunk, GRsetcompress, GRsetchunk

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn 
HCPsetup_szip_parms( comp_info *c_info, int32 nt, int32 ncomp, int32 ndims, int32 *dims, int32 *cdims)
{
 #ifdef H4_HAVE_SZIP_ENCODER
   int32 scanline;
    int32 npoints;
    int32 sz;
    int i;
    intn       ret_value = SUCCEED;

    if (ndims <= 0) {
          ret_value = FAIL;
          goto done;
    }

    /* compute the number of elements in the compressed unit:
        if chunked, compress each chunk. If not, compress whole 
        object
    */
    npoints = ncomp; /* for GR24, treat as 3 D data for szip */
    /* get npoints */
    if (cdims == NULL ) {
	   /* the whole array */
        for (i = 0; i < ndims; i++) {
	  npoints *= dims[i];
        }
    } else {
	   /* elements in a single chunk */
        for (i = 0; i < ndims; i++) {
	  npoints *= cdims[i];
        }
    }
    c_info->szip.pixels = npoints;

    /* compute the pixels per scanline */
	/* start with the n-1th dimension, allow for components of GR */
    if (cdims == NULL ) {
	scanline = dims[ndims - 1] * ncomp;
    } else {
	scanline = cdims[ndims-1] * ncomp;
    }

    /* apply restrictions to find the correct value */
    if (scanline < c_info->szip.pixels_per_block) {
	if (c_info->szip.pixels < c_info->szip.pixels_per_block) {
	    ret_value = FAIL;
	    goto done;
	}
	scanline = MIN((c_info->szip.pixels_per_block * SZ_MAX_BLOCKS_PER_SCANLINE), npoints);
		
    } else {
	if (scanline <= SZ_MAX_PIXELS_PER_SCANLINE) {
		scanline = MIN((c_info->szip.pixels_per_block * SZ_MAX_BLOCKS_PER_SCANLINE), scanline);
	} else {
		scanline = c_info->szip.pixels_per_block * SZ_MAX_BLOCKS_PER_SCANLINE;
	}
    }
    c_info->szip.pixels_per_scanline = scanline;

    /* compute the bits per pixel from the HDF NDT */
    if (FAIL == (sz = DFKNTsize(nt | DFNT_NATIVE))) {
            ret_value = FAIL;
            goto done;
    }
    c_info->szip.bits_per_pixel = sz * 8;

done:
    return(ret_value);
#else
   /* szip not enabled */
   return(FAIL);
#endif
}
