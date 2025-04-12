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

/* $Id: cskphuff.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*
   FILE
   cskphuff.c
   HDF "skipping" huffman encoding I/O routines

   REMARKS

   DESIGN

   EXPORTED ROUTINES
   None of these routines are designed to be called by other users except
   for the modeling layer of the compression routines.

   AUTHOR
   Quincey Koziol

   MODIFICATION HISTORY
   4/25/94     Starting writing specs & coding prototype
 */

/* General HDF includes */
#include "hdf.h"

#define CSKPHUFF_MASTER
#define CODER_CLIENT
/* HDF compression includes */
#include "hcompi.h"     /* Internal definitions for compression */

/* Internal Defines */
/* #define TESTING */
#define TMP_BUF_SIZE    8192    /* size of throw-away buffer */

/*
   *   This piece of code uses Semi-Splay trees to Huffman encode a raster
   *   image file.
 */

/* declaration of the functions provided in this module */
PRIVATE int32 HCIcskphuff_init(accrec_t * access_rec, uintn alloc_buf);

/*--------------------------------------------------------------------------
 NAME
    HCIcskphuff_splay -- Splay the tree around the source code passed

 USAGE
    void HCIcskphuff_splay(skphuff_info,plain)
    comp_coder_skphuff_info_t *skphuff_info;    IN:ptr to skphuff info
    uint8 plain;            IN: the source code to splay the tree around

 RETURNS
    None.

 DESCRIPTION
    Common code called by HCIcskphuff_encode and HCIcskphuff_decode

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
static void
HCIcskphuff_splay(comp_coder_skphuff_info_t * skphuff_info, uint8 plain)
{
    uintn       a, b;           /* children of nodes to semi-rotate */
    uint8       c, d;           /* pair of nodes to semi-rotate */
    intn        skip_num;       /* the tree we are splaying */
    uintn       *lleft,        /* local copy of the left pointer */
                *lright;       /* local copy of the right pointer */
    uint8       *lup;          /* local copy of the up pointer */

    skip_num = skphuff_info->skip_pos;  /* get the tree number to splay */

    /* Get the tree pointers */
    lleft=skphuff_info->left[skip_num];
    lright=skphuff_info->right[skip_num];
    lup=skphuff_info->up[skip_num];

    a = (uintn)plain + SUCCMAX;    /* get the index for this source code in the up array */
    do
      {     /* walk up the tree, semi-rotating pairs */
          c = lup[a];    /* find the parent of the node to semi-rotate around */
          if (c != ROOT)
            {   /* a pair remain above this node */
                d = lup[(int)c];  /* get the grand-parent of the node to semi-rotate around */
                b = lleft[(int)d];

/* Exchange the children of the pair */
                if ((uintn)c == b)
                  {
                      b = lright[(int)d];
                      lright[(int)d] = a;
                  }     /* end if */
                else
                    lleft[(int)d] = a;

                if (a == lleft[(int)c])
                    lleft[(int)c] = b;
                else
                    lright[(int)c] = b;

                lup[a] = d;
                lup[b] = c;
                a = (uintn)d;
            }   /* end if */
          else
            {   /* handle odd node at end */
                a = (uintn)c;
            }   /* end else */
      }
    while (a != ROOT);
}   /* end HCIcskphuff_splay() */

/*--------------------------------------------------------------------------
 NAME
    HCIcskphuff_init -- Initialize a skipping huffman compressed data element.

 USAGE
    int32 HCIcskphuff_init(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element
    uintn alloc_buf;        IN: whether to allocate the buffers or not

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called by HCIcskphuff_staccess and HCIcskphuff_seek

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcskphuff_init(accrec_t * access_rec, uintn alloc_buf)
{
    CONSTR(FUNC, "HCIcskphuff_init");
    compinfo_t *info;           /* special element information */
    comp_coder_skphuff_info_t *skphuff_info;    /* ptr to skphuff info */
    intn        i, j, k;        /* local counting var */

    info = (compinfo_t *) access_rec->special_info;

#ifdef TESTING
    printf("HCIcskphuff_init(): before Hbitseek() call\n");
#endif /* TESTING */
    if (Hbitseek(info->aid, 0, 0) == FAIL)  /* seek to beginning of element */
        HRETURN_ERROR(DFE_SEEKERROR, FAIL);

#ifdef TESTING
    printf("HCIcskphuff_init(): after Hbitseek() call\n");
#endif /* TESTING */
    skphuff_info = &(info->cinfo.coder_info.skphuff_info);

    /* Initialize RLE state information */
    skphuff_info->skip_pos = 0;     /* start in first byte */
    skphuff_info->offset = 0;   /* start at the beginning of the data */

    if(alloc_buf==TRUE)
      {
        /* allocate pointers to the compression buffers */
        if ((skphuff_info->left = (uintn **) HDmalloc(sizeof(uintn *) * (uintn)skphuff_info->skip_size)) == NULL)
                        HRETURN_ERROR(DFE_NOSPACE, FAIL);
        if ((skphuff_info->right = (uintn **) HDmalloc(sizeof(uintn *) * (uintn)skphuff_info->skip_size)) == NULL)
                        HRETURN_ERROR(DFE_NOSPACE, FAIL);
        if ((skphuff_info->up = (uint8 **) HDmalloc(sizeof(uint8 *) * (uintn)skphuff_info->skip_size)) == NULL)
                        HRETURN_ERROR(DFE_NOSPACE, FAIL);

#ifdef TESTING
        printf("HCIcskphuff_init(): halfway through allocating space\n");
#endif /* TESTING */
        /* allocate compression buffer for each skipping byte */
        for (i = 0; i < skphuff_info->skip_size; i++)
          {
              if ((skphuff_info->left[i] = (uintn *) HDmalloc(sizeof(uintn) * SUCCMAX)) == NULL)
                              HRETURN_ERROR(DFE_NOSPACE, FAIL);
              if ((skphuff_info->right[i] = (uintn *) HDmalloc(sizeof(uintn) * SUCCMAX)) == NULL)
                              HRETURN_ERROR(DFE_NOSPACE, FAIL);
              if ((skphuff_info->up[i] = (uint8 *) HDmalloc(sizeof(uint8) * TWICEMAX)) == NULL)
                              HRETURN_ERROR(DFE_NOSPACE, FAIL);
          }     /* end for */
      } /* end if */

#ifdef TESTING
    printf("HCIcskphuff_init(): after allocating space\n");
#endif /* TESTING */
    for (k = 0; k < skphuff_info->skip_size; k++)
      {
#ifdef UNICOS
#pragma novector
#endif /* UNICOS */
          for (i = 0; i < TWICEMAX; i++)    /* initialize the up pointers to point to their parent in the tree */
              skphuff_info->up[k][i] = (uint8)(i >> 1);

          for (j = 0; j < SUCCMAX; j++)
            {   /* initialize the left & right pointers correctly */
                skphuff_info->left[k][j] = (uintn)(j << 1);
                skphuff_info->right[k][j] = (uintn)((j << 1) + 1);
            }   /* end for */
      }     /* end for */

#ifdef TESTING
    printf("HCIcskphuff_init(): after initializing arrays\n");
#endif /* TESTING */
    return (SUCCEED);
}   /* end HCIcskphuff_init() */

/*--------------------------------------------------------------------------
 NAME
    HCIcskphuff_decode -- Decode skipping Huffman compressed data into a buffer.

 USAGE
    int32 HCIcskphuff_decode(info,length,buf)
    compinfo_t *info;   IN: the info about the compressed element
    int32 length;       IN: number of bytes to read into the buffer
    uint8 *buf;         OUT: buffer to store the bytes read

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called to decode skipping Huffman data from the file.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcskphuff_decode(compinfo_t * info, int32 length, uint8 *buf)
{
    CONSTR(FUNC, "HCIcskphuff_decode");
    comp_coder_skphuff_info_t *skphuff_info;    /* ptr to skipping Huffman info */
    int32       orig_length;    /* original length to read */
    uint32      bit;            /* bit from the file */
    uintn       a;
    uint8       plain;          /* the source code expanded from the file */

    skphuff_info = &(info->cinfo.coder_info.skphuff_info);

    orig_length = length;   /* save this for later */
    while (length > 0)
      {     /* decode until we have all the bytes we need */
#ifdef TESTING
printf("length=%ld\n",(long)length);
#endif /* TESTING */
          a = ROOT;     /* start at the root of the tree and find the leaf we need */

          do
            {   /* walk down once for each bit on the path */
#ifdef TESTING
intn bitcount=0;
printf("bitcount=%d\n",++bitcount);
#endif /* TESTING */
                if(Hbitread(info->aid,1,&bit)==FAIL)
                    HRETURN_ERROR(DFE_CDECODE, FAIL);
                a=((bit==0) ? ( skphuff_info->left[skphuff_info->skip_pos][a]) \
                    : (skphuff_info->right[skphuff_info->skip_pos][a]));
            }
          while (a <= SKPHUFF_MAX_CHAR);

          plain = (uint8)(a - SUCCMAX);
          HCIcskphuff_splay(skphuff_info, plain);
          skphuff_info->skip_pos = (skphuff_info->skip_pos + 1) % skphuff_info->skip_size;
          *buf++ = plain;
          length--;
      }     /* end while */
    skphuff_info->offset += orig_length;    /* incr. abs. offset into the file */
    return (SUCCEED);
}   /* end HCIcskphuff_decode() */

/*--------------------------------------------------------------------------
 NAME
    HCIcskphuff_encode -- Encode data from a buffer into skipping Huffman
                            compressed data

 USAGE
    int32 HCIcskphuff_encode(info,length,buf)
    compinfo_t *info;   IN: the info about the compressed element
    int32 length;       IN: number of bytes to store from the buffer
    const uint8 *buf;         OUT: buffer to get the bytes from

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called to encode skipping Huffman data into a file.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcskphuff_encode(compinfo_t * info, int32 length, const uint8 *buf)
{
    CONSTR(FUNC, "HCIcskphuff_encode");
    comp_coder_skphuff_info_t *skphuff_info;    /* ptr to skipping Huffman info */
    int32       orig_length;    /* original length to write */
    intn        stack_ptr;      /* pointer to the position on the stack */
#ifdef OLD_WAY
    intn        stack[SKPHUFF_MAX_CHAR]; /* stack to store the bits generated */
    uintn       a;              /* variable to record the position in the tree */
#else /* OLD_WAY */
    uintn       a, last_node;   /* variables to record the current & last position in the tree */
    uint32      output_bits[(SKPHUFF_MAX_CHAR/4)+1],    /* bits to write out */
                bit_count[(SKPHUFF_MAX_CHAR/4)+1],      /* # of bits stored in each stack location */
                bit_mask;       /* bit-mask for accumulating bits to output */
#endif /* OLD_WAY */

    skphuff_info = &(info->cinfo.coder_info.skphuff_info);

    orig_length = length;   /* save this for later */
    while (length > 0)
      {     /* encode until we stored all the bytes */
          a = (uintn)*buf + SUCCMAX;   /* find position in the up array */
#ifdef OLD_WAY
          do
            {   /* walk up the tree, pushing bits */
                stack[stack_ptr] = (skphuff_info->right[skphuff_info->skip_pos][skphuff_info->up[skphuff_info->skip_pos][a]] == a);     /* push a 1 is this is the right node */
                stack_ptr++;
                a = skphuff_info->up[skphuff_info->skip_pos][a];
            }
          while (a != ROOT);

          do
            {   /* output the bits we have */
                stack_ptr--;
                if (Hputbit(info->aid, stack[stack_ptr]) == FAIL)
                    HRETURN_ERROR(DFE_CENCODE, FAIL);
            }
          while (stack_ptr != 0);
#else /* OLD_WAY */
/* This way is _much_ faster... */
          stack_ptr=0;
          bit_mask=1;   /* initialize to the lowest bit */
          output_bits[0]=0;
          bit_count[0]=0;
          do
            {   /* walk up the tree, pushing bits */
                last_node=a; /* keep track of the current node */
                a = (uintn)skphuff_info->up[skphuff_info->skip_pos][a]; /* move the current node up one */
                if(skphuff_info->right[skphuff_info->skip_pos][a] == last_node)
                    output_bits[stack_ptr]|=bit_mask; /* push a 1 if this is the right node */
                bit_mask<<=1;   /* rotate bit mask over */
                bit_count[stack_ptr]++;    /* increment # of bits stored */
                if(bit_count[stack_ptr]>=32)
                  {
                    stack_ptr++;    /* increment stack position */
                    bit_mask=1;     /* reset bit mask to lowest bit position */
                    output_bits[stack_ptr]=0;   /* initialize stack location */
                    bit_count[stack_ptr]=0;
                  } /* end if */
            }
          while (a != ROOT);

          do {   /* output the bits we have */
                if(bit_count[stack_ptr]>0)
                  {
                    if(Hbitwrite(info->aid,(intn)bit_count[stack_ptr],output_bits[stack_ptr]) !=(intn)bit_count[stack_ptr])
                        HRETURN_ERROR(DFE_CENCODE, FAIL);
                  } /* end if */
                stack_ptr--;
            } while (stack_ptr >= 0);
#endif /* OLD_WAY */
          HCIcskphuff_splay(skphuff_info, *buf);    /* semi-splay the tree around this node */
          skphuff_info->skip_pos = (skphuff_info->skip_pos + 1) % skphuff_info->skip_size;
          buf++;
          length--;
      }     /* end while */

    skphuff_info->offset += orig_length;    /* incr. abs. offset into the file */
    return (SUCCEED);
}   /* end HCIcskphuff_encode() */

/*--------------------------------------------------------------------------
 NAME
    HCIcskphuff_term -- Flush encoded data from internal buffer to skipping
                    Huffman compressed data

 USAGE
    int32 HCIcskphuff_term(info)
    compinfo_t *info;   IN: the info about the compressed element

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called to flush skipping Huffman data into a file.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcskphuff_term(compinfo_t * info)
{
#ifdef LATER
    CONSTR(FUNC, "HCIcskphuff_term");
#endif /* endif LATER */
    comp_coder_skphuff_info_t *skphuff_info;    /* ptr to skipping Huffman info */
    intn i;                 /* local counting variable */

    skphuff_info = &(info->cinfo.coder_info.skphuff_info);

    skphuff_info->skip_pos = 0;

    /* Free the buffers we allocated */
    for (i = 0; i < skphuff_info->skip_size; i++)
      {
          HDfree(skphuff_info->left[i]);
          HDfree(skphuff_info->right[i]);
          HDfree(skphuff_info->up[i]);
      }     /* end for */

    /* Free the buffer arrays */
    HDfree(skphuff_info->left);
    HDfree(skphuff_info->right);
    HDfree(skphuff_info->up);

    return (SUCCEED);
}   /* end HCIcskphuff_term() */

/*--------------------------------------------------------------------------
 NAME
    HCIcskphuff_staccess -- Start accessing a skipping Huffman compressed data element.

 USAGE
    int32 HCIcskphuff_staccess(access_rec, access)
    accrec_t *access_rec;   IN: the access record of the data element
    int16 access;           IN: the type of access wanted

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called by HCIcskphuff_stread and HCIcskphuff_stwrite

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcskphuff_staccess(accrec_t * access_rec, int16 acc_mode)
{
    CONSTR(FUNC, "HCIcskphuff_staccess");
    compinfo_t *info;           /* special element information */

    info = (compinfo_t *) access_rec->special_info;

#ifdef TESTING
    printf("HCIcskphuff_staccess(): before bitio calls\n");
#endif /* TESTING */
    /* need to check for not writing, as opposed to read access */
    /* because of the way the access works */
    if (!(acc_mode&DFACC_WRITE))
        info->aid = Hstartbitread(access_rec->file_id, DFTAG_COMPRESSED,
                                  info->comp_ref);
    else
#ifdef OLD_WAY
        info->aid = Hstartbitwrite(access_rec->file_id, DFTAG_COMPRESSED,
                                   info->comp_ref, info->length);
#else /* OLD_WAY */
      {
        info->aid = Hstartbitwrite(access_rec->file_id, DFTAG_COMPRESSED,
                                   info->comp_ref, 0);
        Hbitappendable(info->aid);
      } /* end else */
#endif /* OLD_WAY */

#ifdef TESTING
    printf("HCIcskphuff_staccess(): after bitio calls\n");
#endif /* TESTING */
    if (info->aid == FAIL)
        HRETURN_ERROR(DFE_DENIED, FAIL);
    if ((acc_mode&DFACC_WRITE) && Hbitappendable(info->aid) == FAIL)
        HRETURN_ERROR(DFE_DENIED, FAIL);
    return (HCIcskphuff_init(access_rec, TRUE));  /* initialize the skip-Huffman info */
}   /* end HCIcskphuff_staccess() */

/*--------------------------------------------------------------------------
 NAME
    HCPcskphuff_stread -- start read access for compressed file

 USAGE
    int32 HCPcskphuff_stread(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Start read access on a compressed data element using a "skipping"
    Huffman scheme.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcskphuff_stread(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPcskphuff_stread");
    int32       ret;

    if ((ret = HCIcskphuff_staccess(access_rec, DFACC_READ)) == FAIL)
        HRETURN_ERROR(DFE_CINIT, FAIL);
    return (ret);
}   /* HCPcskphuff_stread() */

/*--------------------------------------------------------------------------
 NAME
    HCPcskphuff_stwrite -- start write access for compressed file

 USAGE
    int32 HCPcskphuff_stwrite(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Start write access on a compressed data element using a "skipping"
    Huffman scheme.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcskphuff_stwrite(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPcskphuff_stwrite");
    int32       ret;

#ifdef TESTING
    printf("HCPcskphuff_stwrite(): before call to HCIcskphuff_staccess()\n");
#endif
    if ((ret = HCIcskphuff_staccess(access_rec, DFACC_WRITE)) == FAIL)
        HRETURN_ERROR(DFE_CINIT, FAIL);
#ifdef TESTING
    printf("HCPcskphuff_stwrite(): after call to HCIcskphuff_staccess(), ret=%d\n", (int) ret);
#endif
    return (ret);
}   /* HCPcskphuff_stwrite() */

/*--------------------------------------------------------------------------
 NAME
    HCPcskphuff_seek -- Seek to offset within the data element

 USAGE
    int32 HCPcskphuff_seek(access_rec,offset,origin)
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
HCPcskphuff_seek(accrec_t * access_rec, int32 offset, int origin)
{
    CONSTR(FUNC, "HCPcskphuff_seek");
    compinfo_t *info;           /* special element information */
    comp_coder_skphuff_info_t *skphuff_info;    /* ptr to skipping Huffman info */
    uint8      *tmp_buf;        /* pointer to throw-away buffer */

    /* shut compiler up */
    origin = origin;

    info = (compinfo_t *) access_rec->special_info;
    skphuff_info = &(info->cinfo.coder_info.skphuff_info);

    if (offset < skphuff_info->offset)
      {     /* need to seek from the beginning */
          if (HCIcskphuff_init(access_rec, FALSE) == FAIL)
              HRETURN_ERROR(DFE_CINIT, FAIL);
      }     /* end if */

    if ((tmp_buf = (uint8 *) HDmalloc(TMP_BUF_SIZE)) == NULL)     /* get tmp buffer */
        HRETURN_ERROR(DFE_NOSPACE, FAIL);

    while (skphuff_info->offset + TMP_BUF_SIZE < offset)    /* grab chunks */
        if (HCIcskphuff_decode(info, TMP_BUF_SIZE, tmp_buf) == FAIL)
          {
              HDfree(tmp_buf);
              HRETURN_ERROR(DFE_CDECODE, FAIL)
          }     /* end if */
    if (skphuff_info->offset < offset)  /* grab the last chunk */
        if (HCIcskphuff_decode(info, offset - skphuff_info->offset, tmp_buf) == FAIL)
          {
              HDfree(tmp_buf);
              HRETURN_ERROR(DFE_CDECODE, FAIL)
          }     /* end if */

    HDfree(tmp_buf);
    return (SUCCEED);
}   /* HCPcskphuff_seek() */

/*--------------------------------------------------------------------------
 NAME
    HCPcskphuff_read -- Read in a portion of data from a compressed data element.

 USAGE
    int32 HCPcskphuff_read(access_rec,length,data)
    accrec_t *access_rec;   IN: the access record of the data element
    int32 length;           IN: the number of bytes to read
    void * data;             OUT: the buffer to place the bytes read

 RETURNS
    Returns the number of bytes read or FAIL

 DESCRIPTION
    Read in a number of bytes from a skipping Huffman compressed data element.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcskphuff_read(accrec_t * access_rec, int32 length, void * data)
{
    CONSTR(FUNC, "HCPcskphuff_read");
    compinfo_t *info;           /* special element information */

    info = (compinfo_t *) access_rec->special_info;

    if (HCIcskphuff_decode(info, length, data) == FAIL)
        HRETURN_ERROR(DFE_CDECODE, FAIL);

    return (length);
}   /* HCPcskphuff_read() */

/*--------------------------------------------------------------------------
 NAME
    HCPcskphuff_write -- Write out a portion of data from a compressed data element.

 USAGE
    int32 HCPcskphuff_write(access_rec,length,data)
    accrec_t *access_rec;   IN: the access record of the data element
    int32 length;           IN: the number of bytes to write
    void * data;             IN: the buffer to retrieve the bytes written

 RETURNS
    Returns the number of bytes written or FAIL

 DESCRIPTION
    Write out a number of bytes to a skipping Huffman compressed data element.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcskphuff_write(accrec_t * access_rec, int32 length, const void * data)
{
    CONSTR(FUNC, "HCPcskphuff_write");
    compinfo_t *info;           /* special element information */
    comp_coder_skphuff_info_t *skphuff_info;    /* ptr to skipping Huffman info */

    info = (compinfo_t *) access_rec->special_info;
    skphuff_info = &(info->cinfo.coder_info.skphuff_info);

    /* Don't allow random write in a dataset unless: */
    /*  1 - append onto the end */
    /*  2 - start at the beginning and rewrite (at least) the whole dataset */
    if ((info->length != skphuff_info->offset)
        && (skphuff_info->offset != 0 && length <= info->length))
        HRETURN_ERROR(DFE_UNSUPPORTED, FAIL);

    if (HCIcskphuff_encode(info, length, data) == FAIL)
        HRETURN_ERROR(DFE_CENCODE, FAIL);

    return (length);
}   /* HCPcskphuff_write() */

/*--------------------------------------------------------------------------
 NAME
    HCPcskphuff_inquire -- Inquire information about the access record and data element.

 USAGE
    int32 HCPcskphuff_inquire(access_rec,pfile_id,ptag,pref,plength,poffset,pposn,
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
HCPcskphuff_inquire(accrec_t * access_rec, int32 *pfile_id, uint16 *ptag,
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
}   /* HCPcskphuff_inquire() */

/*--------------------------------------------------------------------------
 NAME
    HCPcskphuff_endaccess -- Close the compressed data element

 USAGE
    int32 HCPcskphuff_endaccess(access_rec)
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
HCPcskphuff_endaccess(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPcskphuff_endaccess");
    compinfo_t *info;           /* special element information */

    info = (compinfo_t *) access_rec->special_info;

    /* Clean up the skipping huffman data structures */
    if (HCIcskphuff_term(info) == FAIL)
        HRETURN_ERROR(DFE_CTERM, FAIL);

    /* close the compressed data AID */
    if (Hendbitaccess(info->aid, 0) == FAIL)
        HRETURN_ERROR(DFE_CANTCLOSE, FAIL);

    return (SUCCEED);
}   /* HCPcskphuff_endaccess() */
