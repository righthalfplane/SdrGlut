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

/* $Id: cnbit.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*
   FILE
   cnbit.c
   HDF N-bit encoding I/O routines

   REMARKS

   DESIGN

   EXPORTED ROUTINES
   None of these routines are designed to be called by other users except
   for the modeling layer of the compression routines.

   AUTHOR
   Quincey Koziol

   MODIFICATION HISTORY
   10/10/93     Starting writing specs & coding prototype
 */

/* General HDF includes */
#include "hdf.h"

#define CNBIT_MASTER
#define CODER_CLIENT
/* HDF compression includes */
#include "hcompi.h"     /* Internal definitions for compression */

/* Internal Defines */
/* #define TESTING */

/* Local Variables */
static const uint8 mask_arr8[9] =
{                               /* array of values with [n] bits set */
    0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF
};

static const uint32 mask_arr32[33] =
{                               /* array of values with [n] bits set */
    0x00000000,
    0x00000001, 0x00000003, 0x00000007, 0x0000000F,
    0x0000001F, 0x0000003F, 0x0000007F, 0x000000FF,
    0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF,
    0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
    0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF,
    0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
    0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF,
    0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFFUL
};

/* declaration of the functions provided in this module */
PRIVATE int32 HCIcnbit_staccess
            (accrec_t * access_rec, int16 acc_mode);

PRIVATE int32 HCIcnbit_init
            (accrec_t * access_rec);

PRIVATE int32 HCIcnbit_decode
            (compinfo_t * info, int32 length, uint8 *buf);

PRIVATE int32 HCIcnbit_encode
            (compinfo_t * info, int32 length, const uint8 *buf);

PRIVATE int32 HCIcnbit_term
            (compinfo_t * info);

/*--------------------------------------------------------------------------
 NAME
    HCIcnbit_init -- Initialize a N-bit compressed data element.

 USAGE
    int32 HCIcnbit_init(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called by HCIcnbit_staccess and HCIcnbit_seek

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcnbit_init(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCIcnbit_init");
    compinfo_t *info;           /* special element information */
    comp_coder_nbit_info_t *nbit_info;  /* ptr to N-bit info */
    intn        bits;           /* number of bits in number type */
    intn        top_bit, bot_bit;   /* bits around a range of bytes */
    intn        mask_top, mask_bot;     /* top and bottom bits in mask */
    intn        i;              /* local counting variable */

    info = (compinfo_t *) access_rec->special_info;
#ifdef TESTING
    printf("HCIcnbit_init(): info=%p\n", info);
#endif
    if (Hbitseek(info->aid, 0, 0) == FAIL)  /* seek to beginning of element */
        HRETURN_ERROR(DFE_SEEKERROR, FAIL);
#ifdef TESTING
    printf("HCIcnbit_init(): after seek call\n");
    printf("HCIcnbit_init(): 0.1 - coder_func.write=%p\n", info->cinfo.coder_funcs.write);
#endif

    nbit_info = &(info->cinfo.coder_info.nbit_info);

    /* Initialize N-bit state information */
    nbit_info->buf_pos = NBIT_BUF_SIZE;     /* start at the beginning of the buffer */
    nbit_info->nt_pos = 0;  /* start at beginning of the NT info */
    nbit_info->offset = 0;  /* offset into the file */
#ifdef TESTING
    printf("HCIcnbit_init(): 0.4 - coder_func.write=%p\n", info->cinfo.coder_funcs.write);
#endif
    HDmemset(nbit_info->mask_buf, (nbit_info->fill_one == TRUE ? 0xff : 0),
             nbit_info->nt_size);
#ifdef TESTING
    printf("HCIcnbit_init(): 0.5 - fill_one=%d, sign_ext=%d\n", (int) nbit_info->fill_one, (int) nbit_info->sign_ext);
#endif

    bits = nbit_info->nt_size * 8;  /* compute # of bits */
    mask_top = nbit_info->mask_off;     /* compute top and bottom ends of mask */
    mask_bot = nbit_info->mask_off - (nbit_info->mask_len - 1);

    top_bit = bits - 1;     /* set the initial top and bottom bits */
    bot_bit = bits - 8;
#ifdef TESTING
    printf("HCIcnbit_init(): 0.6 - mask_top=%d, mask_bot=%d, bits=%d, top_bit=%d, bot_bit=%d\n", mask_top, mask_bot, bits, top_bit, bot_bit);
#endif
    HDmemset(nbit_info->mask_info, 0, sizeof(nbit_info->mask_info));    /* set to 0 */
#ifdef TESTING
    printf("HCIcnbit_init(): before init'ing mask, nt_size=%d\n", nbit_info->nt_size);
    printf("HCIcnbit_init(): 1 - coder_func.write=%p\n", info->cinfo.coder_funcs.write);
#endif
    for (i = 0; i < nbit_info->nt_size; i++)
      {     /* initialize the bitmask info */
#ifdef TESTING
          printf("HCIcnbit_init(): i=%d, mask_top=%d, mask_bot=%d, top_bit=%d, bot_bit=%d\n", i, mask_top, mask_bot, top_bit, bot_bit);
#endif
          if (mask_top >= top_bit)
            {   /* mask offset is above current top bit */
                if (mask_bot <= bot_bit)
                  {     /* entire byte is in mask */
                      nbit_info->mask_info[i].offset = 7;
                      nbit_info->mask_info[i].length = 8;
                      nbit_info->mask_info[i].mask = mask_arr8[8];
                  }     /* end if */
                else
                  {     /* only top part of byte is in mask */
                      nbit_info->mask_info[i].offset = 7;
                      nbit_info->mask_info[i].length = (top_bit - mask_bot) + 1;
                      nbit_info->mask_info[i].mask = (uint8) (mask_arr8[(top_bit - mask_bot) + 1] << (8 - ((top_bit - mask_bot) + 1)));
                      break;    /* we've found the bottom of the mask, we're done */
                  }     /* end else */
            }   /* end if */
          else
            {   /* mask top is below current top bit */
                if (mask_top >= bot_bit)
                  {     /* mask top is inside current byte */
                      if (mask_bot < bot_bit)
                        {   /* mask top to bottom of byte is in mask */
                            nbit_info->mask_info[i].offset = mask_top - bot_bit;
                            nbit_info->mask_info[i].length = (mask_top - bot_bit) + 1;
                            nbit_info->mask_info[i].mask = mask_arr8[(mask_top - bot_bit) + 1];
                        }   /* end if */
                      else
                        {   /* entire bit-field is inside of this byte */
                            nbit_info->mask_info[i].offset = mask_top - bot_bit;
                            nbit_info->mask_info[i].length = (mask_top - mask_bot) + 1;
                            nbit_info->mask_info[i].mask = (uint8) (mask_arr8[(mask_top - mask_bot) + 1] << (mask_bot - bot_bit));
                            break;  /* we've found the bottom of the mask, we're done */
                        }   /* end else */
                  }     /* end if */
                else
                    /* mask top is below bottom bit, just continue */
                  {
                  }
            }   /* end else */
          top_bit -= 8;     /* decrement location in fill mask */
          bot_bit -= 8;
      }     /* end for */
#ifdef TESTING
    printf("HCIcnbit_init(): after init'ing mask\n");
    printf("HCIcnbit_init(): 2 - coder_func.write=%p\n", info->cinfo.coder_funcs.write);
#endif

    /* mask to 0 the bits where the bit-field will go */
    if (nbit_info->fill_one == TRUE)
      {
          for (i = 0; i < nbit_info->nt_size; i++)
              nbit_info->mask_buf[i] &= ~(nbit_info->mask_info[i].mask);
      }     /* end if */

#ifdef TESTING
    printf("HCIcnbit_init(): successful\n");
    printf("HCIcnbit_init(): 3 - coder_func.write=%p\n", info->cinfo.coder_funcs.write);
#endif
    return (SUCCEED);
}   /* end HCIcnbit_init() */

/*--------------------------------------------------------------------------
 NAME
    HCIcnbit_decode -- Decode n-bit data into a buffer.

 USAGE
    int32 HCIcnbit_decode(info,length,buf)
    compinfo_t *info;   IN: the info about the compressed element
    int32 length;       IN: number of bytes to read into the buffer
    uint8 *buf;         OUT: buffer to store the bytes read

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called to decode n-bit data from the file.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcnbit_decode(compinfo_t * info, int32 length, uint8 *buf)
{
    CONSTR(FUNC, "HCIcnbit_decode");
    comp_coder_nbit_info_t *nbit_info;  /* ptr to n-bit info */
    int32       orig_length;    /* original length to write */
    uint32      input_bits;     /* bits read from the file */
    uint32      sign_mask,      /* mask to get the sign bit */
                sign_ext_mask;  /* mask for sign extension */
    intn        sign_byte,      /* byte which contains the sign bit */
                sign_bit = 0;   /* the sign bit from the n_bit data */
    nbit_mask_info_t *mask_info;    /* ptr to the mask info */
    intn        copy_length;    /* number of bytes to copy */
    intn        buf_size,       /* size of the expansion buffer to use */
                buf_items;      /* number of items which will fit into expansion buffer */
    uint8      *rbuf, *rbuf2;   /* pointer into the n-bit read buffer */
    intn        i, j;           /* local counting variable */

    /* get a local ptr to the nbit info for convenience */
    nbit_info = &(info->cinfo.coder_info.nbit_info);

#ifdef TESTING
    printf("HCIcnbit_decode(): nbit_info=%p\n", nbit_info);
#endif
    /* calculate sign extension information */
    sign_ext_mask = ~mask_arr32[nbit_info->mask_off % 8];   /* sign mask has all 1's in upper bits */
    sign_byte = nbit_info->nt_size - ((nbit_info->mask_off / 8) + 1);
    sign_mask = mask_arr32[(nbit_info->mask_off % 8) + 1] ^
        mask_arr32[nbit_info->mask_off % 8];
#ifdef TESTING
    printf("HCInbit_decode(): sign_ext=%d,sign_ext_mask=%lx, sign_byte=%d, sign_mask=%lx\n", nbit_info->sign_ext, (unsigned long) sign_ext_mask, (int) sign_byte, (unsigned long) sign_mask);
    printf("HCInbit_decode(): file_one=%d\n", nbit_info->fill_one);
    for (j = 0; j < nbit_info->nt_size; j++)
        printf("HCInbit_decode(): j=%d, mask=%x, offset=%d, length=%d\n", j, (unsigned) nbit_info->mask_info[j].mask, (int) nbit_info->mask_info[j].offset, (int) nbit_info->mask_info[j].length);
#endif

    buf_size = MIN(NBIT_BUF_SIZE, length);
    buf_items = buf_size / nbit_info->nt_size;  /* compute # of items in buffer */
    orig_length = length;   /* save this for later */
    while (length > 0)
      {     /* decode until we have all the bytes */
#ifdef TESTING
          printf("HCInbit_decode(): length=%d, buf=%p, buf_items=%d\n", length, buf, buf_items);
#endif
          if (nbit_info->buf_pos >= buf_size)
            {   /* re-fill buffer */
                rbuf = (uint8 *) nbit_info->buffer;     /* get a ptr to the buffer */

                /* get initial copy of the mask */
                HDmemfill(rbuf, nbit_info->mask_buf, (uint32)nbit_info->nt_size, (uint32)buf_items);

                for (i = 0; i < buf_items; i++)
                  {
                      /* get a ptr to the mask info for convenience also */
                      mask_info = &(nbit_info->mask_info[0]);

                      if (nbit_info->sign_ext)
                        {   /* special code for expanding sign extended data */
#ifdef TESTING
                            printf("HCInbit_decode(): sign extending\n");
#endif
                            rbuf2 = rbuf;   /* set temporary pointer into buffer */
                            for (j = 0; j < nbit_info->nt_size; j++, mask_info++, rbuf2++)
                              {
                                  if (mask_info->length > 0)
                                    {   /* check if we need to read bits */
                                        Hbitread(info->aid, mask_info->length, &input_bits);
                                        input_bits <<= (mask_info->offset - mask_info->length) + 1;
                                        *rbuf2 |= (uint8)(mask_info->mask & (uint8)input_bits);
                                        if (j == sign_byte)     /* check if this is the sign byte */
                                            sign_bit = sign_mask & input_bits ? 1 : 0;
                                    }   /* end if */
                              }     /* end for */

#ifdef TESTING
                            printf("HCInbit_decode(): i=%d, sign_bit=%d, input_bits=%x\n", i, sign_bit, input_bits);
#endif
                            /* we only have to sign extend if the sign is not the same */
                            /* as the bit we are filling the n-bit data with */
                            if (sign_bit != nbit_info->fill_one)
                              {
                                  rbuf2 = rbuf;     /* set temporary pointer into buffer */
                                  if (sign_bit == 1)
                                    {   /* fill with ones */
                                        for (j = 0; j < sign_byte; j++, rbuf2++)
                                            *rbuf2 = 0xff;
                                        *rbuf2 |= (uint8)sign_ext_mask;
                                    }   /* end if */
                                  else
                                    {   /* fill with zeroes */
                                        for (j = 0; j < sign_byte; j++, rbuf2++)
                                            *rbuf2 = 0x00;
                                        *rbuf2 &= (uint8)~sign_ext_mask;
                                    }   /* end else */
                              }     /* end if */
                            rbuf += nbit_info->nt_size;     /* increment buffer ptr */
                        }   /* end if */
                      else
                        {   /* no sign extension */
#ifdef TESTING
                            printf("HCInbit_decode(): NO sign extention\n");
#endif
                            for (j = 0; j < nbit_info->nt_size; j++, mask_info++, rbuf++)
                              {
                                  if (mask_info->length > 0)
                                    {   /* check if we need to read bits */
                                        if (Hbitread(info->aid, mask_info->length, &input_bits) != mask_info->length)
                                            HRETURN_ERROR(DFE_CDECODE, FAIL);
#ifdef TESTING
                                        printf("HCInbit_decode(): input_bits=%d\n", (int) input_bits);
#endif
                                        *rbuf |= (uint8)(mask_info->mask & (uint8)(input_bits <<
                                                                    ((mask_info->offset - mask_info->length) + 1)));
#ifdef TESTING
                                        printf("HCInbit_decode(): j=%d, length=%d, *rbuf=%x\n", j, mask_info->length, (unsigned) *rbuf);
#endif
                                    }   /* end if */
                              }     /* end for */
                        }   /* end else */
                  }     /* end for */

                nbit_info->buf_pos = 0;     /* reset buffer position */
            }   /* end if */

          copy_length = (intn) ((length > (buf_size - nbit_info->buf_pos)) ?
                                (buf_size - nbit_info->buf_pos) : length);

          HDmemcpy(buf, &(nbit_info->buffer[nbit_info->buf_pos]), copy_length);

          buf += copy_length;
          length -= copy_length;
          nbit_info->buf_pos += copy_length;
      }     /* end for */

    nbit_info->offset += orig_length;   /* incr. abs. offset into the file */
    return (SUCCEED);
}   /* end HCIcnbit_decode() */

/*--------------------------------------------------------------------------
 NAME
    HCIcnbit_encode -- Encode data from a buffer into n-bit data

 USAGE
    int32 HCIcnbit_encode(info,length,buf)
    compinfo_t *info;   IN: the info about the compressed element
    int32 length;       IN: number of bytes to store from the buffer
    const uint8 *buf;         OUT: buffer to get the bytes from

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called to encode n-bit data into a file.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcnbit_encode(compinfo_t * info, int32 length, const uint8 *buf)
{
#ifdef LATER
    CONSTR(FUNC, "HCIcnbit_encode");
#endif
    comp_coder_nbit_info_t *nbit_info;  /* ptr to n-bit info */
    int32       orig_length;    /* original length to write */
    uint32      output_bits;    /* bits to write to the file */
    nbit_mask_info_t *mask_info;    /* ptr to the mask info */

    /* get a local ptr to the nbit info for convenience */
    nbit_info = &(info->cinfo.coder_info.nbit_info);

#ifdef TESTING
    printf("HCIcnbit_encode(): nbit_info=%p, length=%d, buf=%p\n", nbit_info, length, buf);
#endif
    /* get a ptr to the mask info for convenience also */
    mask_info = &(nbit_info->mask_info[nbit_info->nt_pos]);

    orig_length = length;   /* save this for later */
    for (; length > 0; length--, buf++)
      {     /* encode until we store all the bytes */
#ifdef TESTING
          printf("HCIcnbit_encode(): length=%d, buf=%p, nt_pos=%d\n", length, buf, nbit_info->nt_pos);
          printf("HCIcnbit_encode(): mask->length=%d, offset=%d, mask=%x\n", mask_info->length, mask_info->offset, mask_info->mask);
#endif
          if (mask_info->length > 0)
            {   /* check if we need to output bits */
                output_bits = (uint32)(((*buf) & (mask_info->mask)) >>
                    ((mask_info->offset - mask_info->length) + 1));
#ifdef TESTING
                printf("HCIcnbit_encode(): output_bits=%x\n", (unsigned) output_bits);
#endif
                Hbitwrite(info->aid, mask_info->length, output_bits);
            }   /* end if */

          /* advance to the next mask position */
          mask_info++;
          /* advance buffer offset and check for wrap */
          if ((++nbit_info->nt_pos) >= nbit_info->nt_size)
            {
                nbit_info->nt_pos = 0;  /* reset to beginning of buffer */
                mask_info = nbit_info->mask_info;   /* reset ptr to masks also */
            }   /* end if */
      }     /* end for */

    nbit_info->offset += orig_length;   /* incr. abs. offset into the file */
    return (SUCCEED);
}   /* end HCIcnbit_encode() */

/*--------------------------------------------------------------------------
 NAME
    HCIcnbit_term -- Flush any data left to the dataset

 USAGE
    int32 HCIcnbit_term(info)
    compinfo_t *info;   IN: the info about the compressed element

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called to flush n-bit data into a file.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcnbit_term(compinfo_t * info)
{
#ifdef LATER
    CONSTR(FUNC, "HCIcnbit_term");
#endif

    /* shut compiler up */
    info = info;

#ifdef TESTING
    printf("HCPcnbit_term(): func called\n");
#endif
    return (SUCCEED);
}   /* end HCIcnbit_term() */

/*--------------------------------------------------------------------------
 NAME
    HCIcnbit_staccess -- Start accessing a N-bit data element.

 USAGE
    int32 HCIcnbit_staccess(access_rec, access)
    accrec_t *access_rec;   IN: the access record of the data element
    int16 access;           IN: the type of access wanted

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called by HCIcnbit_stread and HCIcnbit_stwrite

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcnbit_staccess(accrec_t * access_rec, int16 acc_mode)
{
    CONSTR(FUNC, "HCIcnbit_staccess");
    compinfo_t *info;           /* special element information */

    info = (compinfo_t *) access_rec->special_info;

#ifdef TESTING
    printf("HCIcnbit_staccess(): info=%p, ref=%d\n", info, (int) info->comp_ref);
#endif
    if (acc_mode == DFACC_READ)
        info->aid = Hstartbitread(access_rec->file_id, DFTAG_COMPRESSED,
                                  info->comp_ref);
    else
        info->aid = Hstartbitwrite(access_rec->file_id, DFTAG_COMPRESSED,
                                   info->comp_ref, info->length);

    if (info->aid == FAIL)
        HRETURN_ERROR(DFE_DENIED, FAIL);
    if ((acc_mode&DFACC_WRITE) && Hbitappendable(info->aid) == FAIL)
        HRETURN_ERROR(DFE_DENIED, FAIL);
#ifdef TESTING
    printf("HCIcnbit_staccess(): info->aid=%d, coder_func.write=%p\n", info->aid, info->cinfo.coder_funcs.write);
#endif
    return (HCIcnbit_init(access_rec));     /* initialize the N-bit info */
}   /* end HCIcnbit_staccess() */

/*--------------------------------------------------------------------------
 NAME
    HCPclre_stread -- start read access for compressed file

 USAGE
    int32 HCPmstdio_stread(access_rec)
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
HCPcnbit_stread(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPcnbit_stread");
    int32       ret;

    if ((ret = HCIcnbit_staccess(access_rec, DFACC_READ)) == FAIL)
        HRETURN_ERROR(DFE_CINIT, FAIL);
    return (ret);
}   /* HCPcnbit_stread() */

/*--------------------------------------------------------------------------
 NAME
    HCPclre_stwrite -- start write access for compressed file

 USAGE
    int32 HCPmstdio_stwrite(access_rec)
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
HCPcnbit_stwrite(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPcnbit_stwrite");
    int32       ret;

#ifdef TESTING
    printf("HCPcnbit_stwrite(): entering\n");
#endif
    if ((ret = HCIcnbit_staccess(access_rec, DFACC_WRITE)) == FAIL)
        HRETURN_ERROR(DFE_CINIT, FAIL);
    return (ret);
}   /* HCPcnbit_stwrite() */

/*--------------------------------------------------------------------------
 NAME
    HCPcnbit_seek -- Seek to offset within the data element

 USAGE
    int32 HCPcnbit_seek(access_rec,offset,origin)
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
HCPcnbit_seek(accrec_t * access_rec, int32 offset, int origin)
{
    CONSTR(FUNC, "HCPcnbit_seek");
    compinfo_t *info;           /* special element information */
    comp_coder_nbit_info_t *nbit_info;  /* ptr to n-bit info */
    int32       bit_offset;     /* offset of the bit to seek to */

    /* shut compiler up */
    origin = origin;

    info = (compinfo_t *) access_rec->special_info;
    nbit_info = &(info->cinfo.coder_info.nbit_info);

    /* only seek to an even multiple of the NT-sized elements in the dataset */
    if (offset % nbit_info->nt_size != 0)
        HRETURN_ERROR(DFE_CSEEK, FAIL);

    bit_offset = (offset / nbit_info->nt_size) * nbit_info->mask_len;

    if (Hbitseek(info->aid, bit_offset / 8, (intn) (bit_offset % 8)) == FAIL)
        HRETURN_ERROR(DFE_CSEEK, FAIL);

    nbit_info->buf_pos = NBIT_BUF_SIZE;     /* force re-read if writing */
    nbit_info->nt_pos = 0;  /* start at the first byte of the mask */
    nbit_info->offset = offset;     /* set abs. offset into the file */

    return (SUCCEED);
}   /* HCPcnbit_seek() */

/*--------------------------------------------------------------------------
 NAME
    HCPclre_read -- Read in a portion of data from a compressed data element.

 USAGE
    int32 HCPcnbit_read(access_rec,length,data)
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
HCPcnbit_read(accrec_t * access_rec, int32 length, void * data)
{
    CONSTR(FUNC, "HCPcnbit_read");
    compinfo_t *info;           /* special element information */

    info = (compinfo_t *) access_rec->special_info;

    if (HCIcnbit_decode(info, length, data) == FAIL)
        HRETURN_ERROR(DFE_CDECODE, FAIL);

    return length;
}   /* HCPcnbit_read() */

/*--------------------------------------------------------------------------
 NAME
    HCPcnbit_write -- Write out a portion of data from a compressed data element.

 USAGE
    int32 HCPwrite(access_rec,length,data)
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
HCPcnbit_write(accrec_t * access_rec, int32 length, const void * data)
{
    CONSTR(FUNC, "HCPcnbit_write");
    compinfo_t *info;           /* special element information */

    info = (compinfo_t *) access_rec->special_info;

#ifdef TESTING
    printf("HCPcnbit_write(): before HCIcnbit_encode() call\n");
#endif
    if (HCIcnbit_encode(info, length, data) == FAIL)
        HRETURN_ERROR(DFE_CENCODE, FAIL);
#ifdef TESTING
    printf("HCPcnbit_write(): after HCIcnbit_encode() call\n");
#endif

    return (length);
}   /* HCPcnbit_write() */

/*--------------------------------------------------------------------------
 NAME
    HCPclre_inquire -- Inquire information about the access record and data element.

 USAGE
    int32 HCPcnbit_inquire(access_rec,pfile_id,ptag,pref,plength,poffset,pposn,
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
HCPcnbit_inquire(accrec_t * access_rec, int32 *pfile_id, uint16 *ptag,
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
}   /* HCPcnbit_inquire() */

/*--------------------------------------------------------------------------
 NAME
    HCPcnbit_endaccess -- Close the compressed data element

 USAGE
    intn HCPnbit_endaccess(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Close the compressed data element and free modelling info.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
HCPcnbit_endaccess(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPcnbit_endaccess");
    compinfo_t *info;           /* special element information */

#ifdef TESTING
    printf("HCPcnbit_endaccess(): entering\n");
#endif
    info = (compinfo_t *) access_rec->special_info;

    /* flush out n-bit buffer */
    if (access_rec->access&DFACC_WRITE)
        if (HCIcnbit_term(info) == FAIL)
            HRETURN_ERROR(DFE_CTERM, FAIL);

#ifdef TESTING
    printf("HCPcnbit_endaccess(): before Hendbitaccess call\n");
#endif
    /* close the n-bit data AID */
    if (Hendbitaccess(info->aid, 0) == FAIL)
        HRETURN_ERROR(DFE_CANTCLOSE, FAIL);

#ifdef TESTING
    printf("HCPcnbit_endaccess(): after Hendbitaccess call\n");
#endif
    return (SUCCEED);
}   /* HCPcnbit_endaccess() */
