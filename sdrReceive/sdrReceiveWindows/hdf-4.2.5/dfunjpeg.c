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

/* $Id: dfunjpeg.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*-----------------------------------------------------------------------------
 * File:    dfunjpeg.c
 * Purpose: JPEG image decompression algorithm
 * Invokes: JPEG library functions
 * Contents:
 *  DFCIunjpeg: decompress image using JPEG compression
 * Remarks: DFCIunjpeg() decompresses JPEG encoded images using the JPEG
 *      library functions.  The dfjpeg.c file and this file (dfunjpeg.c) should
 *      remain the only HDF files that has to know about how to use the JPEG
 *      routines.
 *---------------------------------------------------------------------------*/

#include "hdf.h"
#include "jpeglib.h"
#include "jerror.h"

/* Expanded data destination object for HDF input */

typedef struct {
    struct jpeg_source_mgr pub; /* public fields */

    int32 aid;              /* target AID for input */
    int32 file_id;          /* HDF file ID */
    uint16 tag, ref;        /* tag & ref of image to input */

    /* HDF backward compatibility flags */
    intn old_jpeg_image;    /* whether the image is an JPEG4-style HDF image */
    intn old_header_read;   /* if the header has been read from the old image */

    JOCTET *buffer;         /* buffer for JPEG library to fill */
} hdf_source_mgr;

typedef hdf_source_mgr * hdf_src_ptr;

#define INPUT_BUF_SIZE     4096    /* size of JPEG input buffer */

/* Prototypes */
extern void    hdf_init_source(struct jpeg_decompress_struct *cinfo_ptr);
extern boolean hdf_fill_input_buffer(struct jpeg_decompress_struct *cinfo_ptr);
extern void    hdf_skip_input_data(struct jpeg_decompress_struct *cinfo_ptr,long num_bytes);
extern void    hdf_term_source(struct jpeg_decompress_struct *cinfo_ptr);
extern intn    jpeg_HDF_src(struct jpeg_decompress_struct *cinfo_ptr, int32 file_id, uint16 tag,
                            uint16 ref, VOIDP image, int32 xdim, int32 ydim, int16 scheme);
extern intn    jpeg_HDF_src_term(struct jpeg_decompress_struct *cinfo_ptr);

/*-----------------------------------------------------------------------------
 * Name:    hdf_init_source
 * Purpose: Initialize the source mgr for the JPEG image
 * Inputs:
 *      cinfo_ptr - JPEG decompression structure pointer
 * Returns: none.
 * Users:   JPEG library
 * Invokes: HDF low-level I/O functions
 * Remarks: Initializes the JPEG source mgr for further output.
 *---------------------------------------------------------------------------*/
void
hdf_init_source(struct jpeg_decompress_struct *cinfo_ptr)
{
    hdf_src_ptr src=(hdf_src_ptr)cinfo_ptr->src;

    if((src->buffer=HDmalloc(sizeof(JOCTET)*INPUT_BUF_SIZE))==NULL)
        ERREXIT1(cinfo_ptr, JERR_OUT_OF_MEMORY, (int)1);

    if((src->aid=Hstartaccess(src->file_id,src->tag,src->ref,DFACC_READ))==FAIL)
        ERREXIT(cinfo_ptr, JERR_FILE_WRITE);
} /* end hdf_init_source() */

/*-----------------------------------------------------------------------------
 * Name:    hdf_fill_input_buffer
 * Purpose: Read in a chunk of compressed data to feed to the JPEG routines
 * Inputs:
 *      cinfo_ptr - JPEG decompression structure pointer
 * Returns: none.
 * Users:   JPEG library
 * Invokes: HDF low-level I/O functions
 * Remarks: Reads in a chunk of data from either an old-style JPEG image or
 *              a new-style one.
 *---------------------------------------------------------------------------*/
boolean
hdf_fill_input_buffer(struct jpeg_decompress_struct *cinfo_ptr)
{
    hdf_src_ptr src=(hdf_src_ptr)cinfo_ptr->src;
    int32 num_read;     /* number of bytes read */

    if(src->old_jpeg_image==TRUE) /* reading old-style JPEG image */
      {
        if(src->old_header_read==TRUE)  /* done with header just grab data now */
          {
            if((num_read=Hread(src->aid,INPUT_BUF_SIZE,src->buffer))==FAIL)
                ERREXIT(cinfo_ptr, JERR_FILE_READ);

            src->pub.bytes_in_buffer = (size_t)num_read;
          } /* end if */
        else 
          {
            int32 num_read2=0;     /* number of bytes read */
                
            if((num_read=Hread(src->aid,INPUT_BUF_SIZE,src->buffer))==FAIL)
                ERREXIT(cinfo_ptr, JERR_FILE_READ);
            if(num_read<INPUT_BUF_SIZE)
              { /* finished with header, move into the data portion of the JPEG image */
                Hendaccess(src->aid);   /* close the header AID */

                src->old_header_read=TRUE; /* done with the header */

                /* start reading the image data */
                if((src->aid=Hstartaccess(src->file_id,DFTAG_CI,src->ref,DFACC_READ))==FAIL)
                    ERREXIT(cinfo_ptr, JERR_FILE_READ);

                /* finish filling the buffer */
                if((num_read2=Hread(src->aid,(INPUT_BUF_SIZE-num_read),src->buffer+num_read))==FAIL)
                    ERREXIT(cinfo_ptr, JERR_FILE_READ);

              } /* end if */
            src->pub.bytes_in_buffer = (size_t)(num_read+num_read2);
          } /* end else */
      } /* end if */
    else
      {
        if((num_read=Hread(src->aid,INPUT_BUF_SIZE,src->buffer))==FAIL)
            ERREXIT(cinfo_ptr, JERR_FILE_READ);

        src->pub.bytes_in_buffer = (size_t)num_read;
      } /* end else */

    /* check if we are at the end of the input stream */
    if(src->pub.bytes_in_buffer==0)
      {
          src->buffer[0] = (JOCTET) 0xFF;
          src->buffer[1] = (JOCTET) JPEG_EOI;
          src->pub.bytes_in_buffer = 2;
      } /* end if */
    src->pub.next_input_byte = src->buffer;
    return TRUE;
} /* end hdf_fill_input_buffer() */

/*-----------------------------------------------------------------------------
 * Name:    hdf_skip_input_data
 * Purpose: Skip past a chunk of compressed data
 * Inputs:
 *      cinfo_ptr - JPEG decompression structure pointer
 * Returns: none.
 * Users:   JPEG library
 * Invokes: HDF low-level I/O functions
 * Remarks: Skips past a chunk of data from either an old-style JPEG image or
 *              a new-style one.
 *---------------------------------------------------------------------------*/
void
hdf_skip_input_data(struct jpeg_decompress_struct *cinfo_ptr,long num_bytes)
{
    hdf_src_ptr src=(hdf_src_ptr)cinfo_ptr->src;

    /* Just a dumb implementation for now.  Not clear that being smart 
    * is worth any trouble anyway --- large skips are infrequent.
    */
    if (num_bytes > 0) {
        while (num_bytes > (long) src->pub.bytes_in_buffer) {
            num_bytes -= (long) src->pub.bytes_in_buffer;
            (void) hdf_fill_input_buffer(cinfo_ptr);
            /* note we assume that fill_input_buffer will never return FALSE,
            * so suspension need not be handled.
            */
        }
        src->pub.next_input_byte += (size_t) num_bytes;
        src->pub.bytes_in_buffer -= (size_t) num_bytes;
    }
} /* end hdf_skip_input_data() */

/*-----------------------------------------------------------------------------
 * Name:    hdf_term_source
 * Purpose: Terminate the source mgr for the JPEG image
 * Inputs:
 *      cinfo_ptr - JPEG compression structure pointer
 * Returns: none.
 * Users:   JPEG library
 * Invokes: HDF low-level I/O functions
 * Remarks: Terminate the JPEG source mgr for further input.
 *---------------------------------------------------------------------------*/
void
hdf_term_source(struct jpeg_decompress_struct *cinfo_ptr)
{
    hdf_src_ptr src=(hdf_src_ptr)cinfo_ptr->src;

    /* close the HDF object */
    Hendaccess(src->aid);

    /* Free the input buffer */
    HDfree(src->buffer);
} /* end hdf_term_source() */

/*-----------------------------------------------------------------------------
 * Name:    jpeg_HDF_src
 * Purpose: Setup an HDF I/O source manager for the IJG library.
 * Inputs:
 *      cinfo_ptr - JPEG decompression structure pointer
 *      file_id - HDF file ID for file we are opening
 *      tag - HDF tag for image we are writing
 *      ref - HDF ref for image we are writing
 *      image - pointer to the image data to output
 *      xdim,ydim - X & Y dimensions of image
 *      scheme - the type of image to output
 * Returns: 0 on success, -1 on failure
 * Users:   DFCIunjpeg
 * Invokes: JPEG library routines (lots of them...)
 * Remarks: Sets up the source manager functions for the JPEG library.
 *          These routines will be called by the JPEG routines to input
 *---------------------------------------------------------------------------*/
intn
jpeg_HDF_src(struct jpeg_decompress_struct *cinfo_ptr, int32 file_id, uint16 tag,
    uint16 ref, VOIDP image, int32 xdim, int32 ydim, int16 scheme)
{
    CONSTR(FUNC, "jpeg_HDF_src");     /* for HERROR */
    hdf_src_ptr src;

    /* shut compiler up */
    image=image; xdim=xdim; ydim=ydim; scheme=scheme;

    if((src=HDmalloc(sizeof(hdf_source_mgr)))==NULL)
        HRETURN_ERROR(DFE_NOSPACE,FAIL);

    cinfo_ptr->src=(struct jpeg_source_mgr *)src;
    src->pub.init_source = hdf_init_source;
    src->pub.fill_input_buffer = hdf_fill_input_buffer;
    src->pub.skip_input_data = hdf_skip_input_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart; /* use JPEG default */
    src->pub.term_source = hdf_term_source ;

    /* Now the HDF specific parameters */
    src->aid = 0;  /* start with no AID */
    src->file_id = file_id;
    src->tag = tag;
    src->ref = ref;

    /* check for old-style HDF JPEG image */
#ifdef OLD_WAY
    if(tag==DFTAG_JPEG || tag==DFTAG_GREYJPEG)
      {
        src->old_jpeg_image=TRUE;   /* indicate an old-style image */
        src->old_header_read=FALSE; /* start with the header */
      } /* end if */
    else
        src->old_jpeg_image=FALSE;   /* indicate an new-style image */
#else /* OLD_WAY */
    if(scheme==DFTAG_JPEG || scheme==DFTAG_GREYJPEG)
      {
        src->tag = (uint16)scheme;      /* start reading from the JPEG header first */
        src->old_jpeg_image=TRUE;   /* indicate an old-style image */
        src->old_header_read=FALSE; /* start with the header */
      } /* end if */
    else
        src->old_jpeg_image=FALSE;   /* indicate an new-style image */
#endif /* OLD_WAY */

    /* force fill_input_buffer until buffer loaded */
    src->pub.bytes_in_buffer = 0;
    src->pub.next_input_byte = NULL;

    return(SUCCEED);
} /* end jpeg_HDF_src() */

/*-----------------------------------------------------------------------------
 * Name:    jpeg_HDF_src_term
 * Purpose: Terminate an HDF I/O source manager for the IJG library.
 * Inputs:
 *      cinfo_ptr - JPEG decompression structure pointer
 * Returns: 0 on success, -1 on failure
 * Users:   DFCIunjpeg
 * Invokes: JPEG library routines (lots of them...)
 * Remarks: Terminates the source manager functions for the JPEG library.
 *---------------------------------------------------------------------------*/
intn
jpeg_HDF_src_term(struct jpeg_decompress_struct *cinfo_ptr)
{
    /* all we need to do for now is to free up the dest. mgr structure */
    HDfree(cinfo_ptr->src);

    return(SUCCEED);
} /* end jpeg_HDF_src_term() */

/**********************************************************************/
/* HDF callable routine for reading in an image with JPEG compression */
/**********************************************************************/

/*-----------------------------------------------------------------------------
 * Name:    DFCIunjpeg
 * Purpose: decompress an image using the JPEG compression algorithm
 * Inputs:
 * Returns: 0 on success, -1 on failure
 * Users:   HDF programmers, DFputcomp, other routines
 * Invokes: JPEG library routines (lots of them...)
 * Remarks: Uses the JPEG library routines.
 *---------------------------------------------------------------------------*/

intn
DFCIunjpeg(int32 file_id, uint16 tag, uint16 ref, VOIDP image, int32 xdim,
           int32 ydim, int16 scheme)
{
    CONSTR(FUNC, "DFCIunjpeg");     /* for HERROR */
    /* These three structs contain JPEG parameters and working data.
     * They must survive for the duration of parameter setup and one
     * call to jpeg_decompress; typically, making them local data in the
     * calling routine is the best strategy.
     */
    struct jpeg_decompress_struct *cinfo_ptr;
    struct jpeg_error_mgr *jerr_ptr;
    JDIMENSION lines_read, lines_left;
    JSAMPARRAY buffer;

    if((cinfo_ptr=HDcalloc(1,sizeof(struct jpeg_decompress_struct)))==NULL)
        HRETURN_ERROR(DFE_NOSPACE,FAIL);

    if((jerr_ptr=HDmalloc(sizeof(struct jpeg_error_mgr)))==NULL)
        HRETURN_ERROR(DFE_NOSPACE,FAIL);

    /* Initialize the error-handling routines */
    cinfo_ptr->err=jpeg_std_error(jerr_ptr);

    /* Initialize the JPEG compression stuff */
    jpeg_create_decompress(cinfo_ptr);

    /* Set-up HDF destination manager */
    jpeg_HDF_src(cinfo_ptr,file_id,tag,ref,image,xdim,ydim,scheme);

    /* Read the JPEG header from the datastream */
    jpeg_read_header(cinfo_ptr,TRUE);

    /* OK, get things started */
    jpeg_start_decompress(cinfo_ptr);

    /* read the whole image in */
    lines_left=(JDIMENSION)ydim;
    while(cinfo_ptr->output_scanline < cinfo_ptr->output_height)
      {
        buffer=(JSAMPARRAY)&image;
        lines_read=jpeg_read_scanlines(cinfo_ptr,buffer,1);
        lines_left-=lines_read;
        image=(char *)image
            +((size_t)cinfo_ptr->output_width*(size_t)cinfo_ptr->output_components*lines_read);
      } /* end while */

    /* Finish reading stuff in */
    jpeg_finish_decompress(cinfo_ptr);

    /* Get rid of the JPEG information */
    jpeg_destroy_decompress(cinfo_ptr);

    /* Wrap up any HDF specific code */
    jpeg_HDF_src_term(cinfo_ptr);

    /* Free update memory allocated */
    HDfree(jerr_ptr);
    HDfree(cinfo_ptr);

    return (SUCCEED);   /* we must be ok... */
}   /* end DFCIunjpeg() */

