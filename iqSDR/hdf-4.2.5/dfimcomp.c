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

/* $Id: dfimcomp.c 4932 2007-09-07 17:17:23Z bmribler $ */

/************************************************************************/
/*  Module Name : imcomp                                                */
/*  Exports     : DFCimcomp(), DFCunimcomp()                            */
/*  Purpose     : Compresses color images                               */
/*  Author  : Eng-Kiat Koh                                              */
/*  Date    : June 30th, 1988                                           */
/*  Functions   : DFCimcomp(), compress(), init_global(), cnt_color()   */
/*        set_palette(), fillin_color(), map(), nearest_color()         */
/*        DFCunimcomp(), sqr()                                          */
/************************************************************************/

#include "hdf.h"

#define PALSIZE 256
#define BIT8 0
#define BIT24 1

#if (!defined MAC && !defined macintosh && !defined SYMANTEC_C )
#define MAXCOLOR 32768
#else  /*MAC */
#define MAXCOLOR 768
#endif /*MAC */

#ifndef NULL
#   define NULL 0
#endif

#define RED 0
#define GREEN 1
#define BLUE 2
#define EPSILON 0.5
#define LO 1
#define HI 0

struct rgb
  {
      uint8       c[3];
  };

struct box
  {
      float32     bnd[3][2];
      int        *pts;
      int         nmbr_pts;
      int         nmbr_distinct;
      struct box *left;
      struct box *right;
  };

uint8      *new_pal;            /* pointer to new palette           */

static int *hist = (int *) NULL;    /* histogram for distinct colors    */
static struct box *frontier = (struct box *) NULL;  /* pointer to the */
/* list of boxes */
static struct rgb *distinct_pt = (struct rgb *) NULL;   /* contains all */
/* distinct rgb points */

static struct rgb *color_pt = (struct rgb *) NULL;  /*contains the hi-lo */
/*colors for each block */
static uint8 *image;            /* contains the compressed image            */
static int  trans[MAXCOLOR];    /* color translation table                  */

PRIVATE VOID compress(unsigned char raster[], int block);
PRIVATE VOID init_global(int32 xdim, int32 ydim, VOIDP out, VOIDP out_pal);
PRIVATE int cnt_color(int blocks);
PRIVATE VOID set_palette(int blocks);
PRIVATE VOID fillin_color(int blocks);
PRIVATE int indx(unsigned char r, unsigned char g, unsigned char b);
PRIVATE VOID map(int blocks);
PRIVATE int nearest_color(uint8 r, uint8 g, uint8 b);
PRIVATE uint32 sqr(int16 x);
PRIVATE VOID sel_palette(int blocks, int distinct, struct rgb *my_color_pt);
PRIVATE VOID init(int blocks, int distinct, struct rgb *my_color_pt);
PRIVATE VOID sort(int l, int r, int dim, int rank[]);
PRIVATE int partition(int l, int r, int dim, int rank[]);
PRIVATE struct box *find_box(void);
PRIVATE VOID split_box(struct box *ptr);
PRIVATE VOID assign_color(void);
PRIVATE int select_dim(struct box *ptr);
PRIVATE float find_med(struct box *ptr, int dim);
PRIVATE VOID classify(struct box *ptr, struct box *child);
PRIVATE int next_pt(int dim, int i, int rank[], int distinct);

/************************************************************************/
/*  Function: DFCIimcomp                                                */
/*  Purpose : Performs Imcomp Compression                               */
/*  Parameters  :                                                       */
/*    xdim, ydim - dimensions of image                                  */
/*                 IT IS ASSUMED THAT THE DIMENSIONS ARE A MULTIPLE OF 4 */
/*    in, out    - input image array and output image buffer size of in */
/*                 is xdim*ydim bytes for 8 bit per pixel mode. It is 3 */
/*                 times that for 24 bits per pixel mode. The output    */
/*                 buffer is always (xdim*ydim)/4.                      */
/*    in_pal     - input palette. Consist of rgb triples unlike seq-type */
/*                 palette. This is a NULL pointer if operating at the  */
/*                 24 bit per pixel mode.                               */
/*    out_pal    - output palette. Consist of PALSIZE color entries.    */
/*                 each entry is an rgb triple.                         */
/*    mode       - Either BIT8 or BIT24                                 */
/*  Returns     : none                                                  */
/*  Called by   : External routines                                     */
/*  Calls       : init_global(), compress(), cnt_color(), set_palette(), */
/*        sel_palette(), map()                                          */
/************************************************************************/

VOID
DFCIimcomp(int32 xdim, int32 ydim, const uint8 *in, uint8 out[],
           uint8 in_pal[], uint8 out_pal[], int mode)
{
    unsigned char raster[48];
    int         blocks, nmbr;
    int32       i, j, k, l, x, y;

    init_global(xdim, ydim, (VOIDP) out, (VOIDP) out_pal);

    /* compress pixel blocks */
    blocks = 0;
    for (i = 0; i < (ydim / 4); i++)
        for (j = 0; j < (xdim / 4); j++)
          {
              switch (mode)
                {
                    case BIT8:      /* 8 bit per pixel format */
                        k = 0;
                        for (y = (i * 4); y < (i * 4 + 4); y++)
                            for (x = (j * 4); x < (j * 4 + 4); x++)
                              {
                                  l = y * xdim + x;
                                  raster[k++] = (unsigned char)
                                      in_pal[3 * (unsigned char) in[l]];
                                  raster[k++] = (unsigned char)
                                      in_pal[3 * (unsigned char) in[l] + 1];
                                  raster[k++] = (unsigned char)
                                      in_pal[3 * (unsigned char) in[l] + 2];
                              }     /* end of for x */
                        compress(raster, blocks);
                        break;

                    case BIT24: /* 24 bit per pixel format */
                        k = 0;
                        for (y = (i * 4); y < (i * 4 + 4); y++)
                            for (x = (j * 4); x < (j * 4 + 4); x++)
                              {
                                  l = 3 * (y * xdim + x);
                                  raster[k++] = (unsigned char) in[l];
                                  raster[k++] = (unsigned char) in[l + 1];
                                  raster[k++] = (unsigned char) in[l + 2];
                              }     /* end of for x */
                        compress(raster, blocks);
                        break;

                    default:    /* unsupported format */
                        break;
                }   /* end of switch */

              blocks++;
          }     /* end of for j */

    /* set palette */
    nmbr = cnt_color(blocks);
    /*
       printf("Number of colors %d \n", nmbr);
     */
    if (nmbr <= PALSIZE)
        set_palette(blocks);
    else
      {
          sel_palette(blocks, nmbr, color_pt);
          map(blocks);
      }

    fillin_color(blocks);
    if (color_pt)
      {
          HDfree((VOIDP) color_pt);
          color_pt = NULL;
      }     /* end if */

}   /* end of DFCIimcomp */

/************************************************************************/
/*  Function    : compress                                              */
/*  Purpose : Given a block of 16 pixels, sets up a 16 bit bitmap       */
/*                and assigns a lo and hi color for the block. For block */
/*                i, hi color is stored in color_pt[2i] and lo in       */
/*                color_pt[2i+1]. Each color is then reduced to 15 bits */
/*                by truncating the lower order 3 bits of each component */
/*  Parameter   :                                                       */
/*    raster     - contains the 16 pixels of a block. Each pixel is 3   */
/*         bytes, 1 byte for each color component                       */
/*    block  - pixel block number                                       */
/*  Returns     : none                                                  */
/*  Called by   : DFCimcomp()                                           */
/*  Calls       : none                                                  */
/************************************************************************/

PRIVATE     VOID
compress(unsigned char raster[], int block)
{
    float32     y[16], y_av;
    int         i, j, k, l;
    uint8       bit;
    int         high, hi, lo;
    int         c_hi[3], c_lo[3];

    /* calculate luminance */
    y_av = (float32) 0.0;
    for (i = 0; i < 16; i++)
      {
          j = 3 * i;
          y[i] = (float32) 0.3 *(float32) raster[j] +
                      (float32) 0.59 *(float32) raster[j + 1] +
                      (float32) 0.11 *(float32) raster[j + 2];
          /*    printf("compress: y[%d] is %f\n",i,y[i]); */
          y_av = y_av + y[i];
      }
    y_av /= (float32) 16.0;
    /*  printf("y_av is %f\n",y_av); */

    /* initialize c_hi and c_lo */
    for (i = RED; i <= BLUE; i++)
      {
          c_hi[i] = 0;
          c_lo[i] = 0;
      }

    /* build bit map */
    k = 4 * block;
    high = 0;
    hi = 2 * block;
    lo = hi + 1;
    for (i = 0; i < 2; i++)
      {
          bit = 128;
          for (j = (i * 8); j < (i * 8 + 8); j++)
            {
                if (y[j] > y_av)
                  {
                      image[k] |= bit;
                      high++;
                      for (l = RED; l <= BLUE; l++)
                          c_hi[l] = c_hi[l] + (int) raster[3 * j + l];
                  }
                else
                  {
                      for (l = RED; l <= BLUE; l++)
                          c_lo[l] = c_lo[l] + (int) raster[3 * j + l];
                  }     /* end of if */

                bit = (uint8)(bit>>1);
            }   /* end of for j */

          k++;
      }     /* end of for i */

    /* calculate hi lo color */
    for (i = RED; i <= BLUE; i++)
      {
          if (high != 0)
              color_pt[hi].c[i] = (uint8) ((float) c_hi[i] / (float) high);
          if (high != 16)
              color_pt[lo].c[i] = (uint8) ((float) c_lo[i] / (float) (16 - high));
          color_pt[hi].c[i] = (uint8)(color_pt[hi].c[i] >> 3);
          color_pt[lo].c[i] = (uint8)(color_pt[lo].c[i] >> 3);

      }
}   /* end of compress */

/************************************************************************/
/*  Function    : init_global                       */
/*  Purpose : Allocates memory for global variables                 */
/*  Parameter   :                           */
/*    xdim, ydim - x and y dimension of image               */
/*    out        - pointer to output buffer                             */
/*    out_pal    - pointer to output palette                            */
/*  Returns     : none                              */
/*  Called by   : DFCimcomp()                       */
/*  Calls       : none                          */
/************************************************************************/

PRIVATE     VOID
init_global(int32 xdim, int32 ydim, VOIDP out, VOIDP out_pal)
{
    int32       i, j;

    /* allocate memory */
    image = (unsigned char *) out;
    new_pal = (unsigned char *) out_pal;
    if (color_pt)
        HDfree((VOIDP) color_pt);
    color_pt = (struct rgb *) HDmalloc((unsigned) ((xdim * ydim) / 8) *
                                         sizeof(struct rgb));

    if (image == NULL || color_pt == NULL || new_pal == NULL)
      {
          return; /* punt! */
      }

    /* initialize */
    for (i = 0; i < (xdim * ydim / 4); i++)
        image[i] = 0;

    for (i = 0; i < (xdim * ydim / 8); i++)
        for (j = RED; j <= BLUE; j++)
            color_pt[i].c[j] = 0;

    for (i = 0; i < MAXCOLOR; i++)
        trans[i] = -1;
}   /* end of init_global */

/************************************************************************/
/*  Function    : cnt_color                                 */
/*  Purpose : Counts the number of distinct colors compressd image  */
/*  Parameter   :                           */
/*    blocks     - total number of pixel blocks             */
/*  Returns     : Number of distinct colors                             */
/*  Called by   : DFCimcomp()                                           */
/*  Calls       : indx()                        */
/************************************************************************/

PRIVATE int
cnt_color(int blocks)
{
    int         temp[MAXCOLOR];
    int         i, k, count;

    for (i = 0; i < MAXCOLOR; i++)
        temp[i] = -1;

    for (i = 0; i < (2 * blocks); i++)
      {
          k = indx(color_pt[i].c[RED], color_pt[i].c[GREEN], color_pt[i].c[BLUE]);
          /*    printf("cnt_color: k is %d\n",k); */
          if (k < MAXCOLOR) /* Fortner Fix: supplied by Peter Lawton */
              temp[k] = 0;
      }

    count = 0;
    for (i = 0; i < MAXCOLOR; i++)
        if (temp[i] == 0)
            count++;

    return count;
}   /* end of cnt_color */

/************************************************************************/
/*  Function    : set_palette                       */
/*  Purpose : The number of distinct colors is less than the desired */
/*                output palette size. Therefore each distinct color can */
/*        be a palette entry. Function enters each distinct     */
/*                color as a palette entry and sets up the translation  */
/*                table. It also shifts each color component left 3 bits */
/*                so that each color component is again 8 bits wide     */
/*  Parameter   :                           */
/*    blocks     - total number of pixel blocks                         */
/*  Returns     : none                          */
/*  Called by   : DFCimcomp()                       */
/*  Calls       : indx()                        */
/************************************************************************/

PRIVATE     VOID
set_palette(int blocks)
{
    int         ent, i, k;

    ent = 0;
    for (i = 0; i < (2 * blocks); i++)
      {
          k = indx(color_pt[i].c[RED], color_pt[i].c[GREEN], color_pt[i].c[BLUE]);
          if (trans[k] == -1)
            {
                new_pal[3 * ent] = (uint8) (color_pt[i].c[RED] << 3);
                new_pal[3 * ent + 1] = (uint8) (color_pt[i].c[GREEN] << 3);
                new_pal[3 * ent + 2] = (uint8) (color_pt[i].c[BLUE] << 3);
                trans[k] = ent;
                ent++;
            }
      }
}   /* end of set_palette */

/************************************************************************/
/*  Function    : fillin_color                      */
/*  Purpose : For each pixel block, fills in the pointers into the  */
/*                palette.                                              */
/*  Parameter   :                           */
/*    blocks     - total number of pixel blocks             */
/*  Returns     : none                          */
/*  Called by   : DFCimcomp()                       */
/*  Calls       : none                          */
/************************************************************************/

PRIVATE     VOID
fillin_color(int blocks)
{
    int         i, j, k;

    for (i = 0; i < blocks; i++)
        for (j = HI; j <= LO; j++)
          {
              k = indx(color_pt[2 * i + j].c[RED], color_pt[2 * i + j].c[GREEN],
                       color_pt[2 * i + j].c[BLUE]);
              image[i * 4 + 2 + j] = (uint8) trans[k];
          }
}   /* end of fillin_color */

/************************************************************************/
/*  Function    : indx                          */
/*  Purpose : Maps an rgb triple (5 bits each) to an integer array  */
/*        index                         */
/*  Parameter   :                           */
/*    r, g, b    - color components                 */
/*  Returns     : returns an array index                */
/*  Called by   : set_palette(), fillin_color(), map()                  */
/*  Calls       : none                          */
/************************************************************************/

PRIVATE int
indx(unsigned char r, unsigned char g, unsigned char b)
{
    int         temp;

    temp = 0;
    temp = ((r & 0x1f) << 10) | ((g & 0x1f) << 5) | (b & 0x1f);
    return temp;
}   /* end of indx */

/************************************************************************/
/*  Function    : map                           */
/*  Purpose : Maps a color_pt to the closest representative color   */
/*        Sets up translation table             */
/*  Parameter   :                           */
/*    blocks     - total number of pixel blocks             */
/*  Returns     : none                          */
/*  Called by   : DFCimcomp()                       */
/*  Calls       : nearest_color()                   */
/************************************************************************/

PRIVATE     VOID
map(int blocks)
{
    int         i, k;
    uint8       r, g, b;

    for (i = 0; i < (2 * blocks); i++)
      {
          k = indx(color_pt[i].c[RED], color_pt[i].c[GREEN], color_pt[i].c[BLUE]);

          if (trans[k] == -1)
            {
                r = (uint8) (color_pt[i].c[RED] << 3);
                g = (uint8) (color_pt[i].c[GREEN] << 3);
                b = (uint8) (color_pt[i].c[BLUE] << 3);
                trans[k] = nearest_color(r, g, b);
                /*
                   printf("map: %d %d %d mapped to %d %d %d\n", r, g, b, new_pal[tran
                   s[k]*3
                   ],
                   new_pal[trans[k]*3+1], new_pal[trans[k]*3+2]);
                 */
            }
      }
}   /* end of map */

/************************************************************************/
/*  Function    : nearest_color                     */
/*  Purpose : Finds the nearest palette color           */
/*  Parameter   :                           */
/*    r, g, b    - color component                  */
/*  Returns     : Entry number of the closest color in the palette      */
/*  Called by   : map()                         */
/*  Calls       : sqr()                         */
/************************************************************************/

PRIVATE int
nearest_color(uint8 r, uint8 g, uint8 b)
{
    int         i, nearest;
    long int    min, error;

    min = (long)(sqr((int16) (r - new_pal[0])) + sqr((int16) (g - new_pal[1])) +
        sqr((int16) (b - new_pal[2])));
    nearest = 0;
    for (i = 1; i < PALSIZE; i++)
      {
          error = (long)(sqr((int16) (r - new_pal[3 * i])) + sqr((int16) (g - new_pal[3 * i + 1])) +
              sqr((int16) (b - new_pal[3 * i + 2])));
          if (error < min)
            {
                min = error;
                nearest = i;
            }
      }

    return nearest;
}   /* end of nearest_color */

/************************************************************************/
/*  Function    : sqr                           */
/*  Purpose : Computes the square of an integer         */
/*  Parameter   :                           */
/*    x      - an integer                       */
/*  Returns     : The square of x                   */
/*  Called by   : nearest_color()                   */
/*  Calls       : none                          */
/************************************************************************/

PRIVATE uint32
sqr(int16 x)
{
    return ((uint32) x * (uint32) x);
}

/************************************************************************/
/*  Function    : DFCIunimcomp                       */
/*  Purpose : 'Decompresses' the compressed image           */
/*  Parameter   :                           */
/*    xdim, ydim - dimensions of image                  */
/*    in, out    - Input buffer and output buffer. Size of input buffer */
/*         is (xdim*ydim)/4. Size of output buffer is 4 times   */
/*         that. It 'restores' images into seq-type files       */
/*  Returns     : none                          */
/*  Called by   : External routines                 */
/*  Calls       : none                          */
/************************************************************************/

VOID
DFCIunimcomp(int32 xdim, int32 ydim, uint8 in[], uint8 out[])
{
    int         bitmap, temp;
    int32       i, j, k, x, y;
    uint8       hi_color, lo_color;

    for (y = 0; y < (ydim / 4); y++)
        for (x = 0; x < xdim; x = x + 4)
          {
              k = y * xdim + x;
              hi_color = (unsigned char) in[k + 2];
              lo_color = (unsigned char) in[k + 3];

              bitmap = ((unsigned char) in[k] << 8) | (unsigned char) in[k + 1];

              for (i = (y * 4); i < (y * 4 + 4); i++)
                {
                    temp = bitmap >> (3 + y * 4 - i) * 4;
                    for (j = x; j < (x + 4); j++)
                      {
                          if ((temp & 8) == 8)
                              out[i * xdim + j] = (char) hi_color;
                          else
                              out[i * xdim + j] = (char) lo_color;
                          temp = temp << 1;
                      }
                }
          }     /* end of for x */
}   /* end of DFCIunimcomp */

/************************************************************************/
/*  Module Name : color                         */
/*  Exports     : sel_palette(); new_pal, pointer to a new color palette */
/*  Purpose     : Quantizes colors                  */
/*  Author  : Eng-Kiat Koh                      */
/*  Date    : June 30th, 1988                   */
/*  Functions   : sel_palette(), init(), sort(), partition(), find_box() */
/*        split_box(), assign_color(), select_dim(), find_med() */
/*                classify(), next_pt()                                 */
/************************************************************************/

/************************************************************************/
/*  Function    : sel_palette                       */
/*  Purpose : Selects PALSIZE palette colors out of a list of colors */
/*        in color_pt                       */
/*  Parameter   :                           */
/*    blocks     - number of pixel blocks               */
/*    distinct   - number of distinct colors                */
/*    color_pt   - contains the lo hi colors for each pixel block       */
/*  Returns     : none                          */
/*  Called by   : DFCimcomp()                       */
/*  Calls       : init(), split_box(), find_box(), assign_color()   */
/************************************************************************/

PRIVATE     VOID
sel_palette(int blocks, int distinct, struct rgb *my_color_pt)
{
    int         boxes;
    /*  int i, j; */
    struct box *ptr;

    init(blocks, distinct, my_color_pt);

    /* split box into smaller boxes with about equal number of points */
    for (boxes = 1; boxes < PALSIZE; boxes++)
      {
          /*
             ptr=frontier->right;
             j = 0;
             while (ptr != NULL)
             {
             printf("Box %d, distinct %d, total %d\n",j,ptr->nmbr_distinct,
             ptr->nmbr_pts);
             for (i=0; i<ptr->nmbr_distinct; i++)
             printf("pt %d: %d %d %d",i,distinct_pt[ptr->pts[i]].c[RED],
             distinct_pt[ptr->pts[i]].c[GREEN],
             distinct_pt[ptr->pts[i]].c[BLUE]);
             j++;
             ptr = ptr->right;
             }
           */

          ptr = find_box();
          split_box(ptr);
      }

    assign_color();
}

/************************************************************************/
/*  Function    : init                          */
/*  Purpose : Initializes the global variables, sets up the first   */
/*        box. It will contain all the color points     */
/*  Parameter   :                           */
/*    blocks     - number of pixel blocks               */
/*    distinct   - number of distinct colors                */
/*    my_color_pt   - contains the lo hi colors for each pixel block       */
/*  Returns     : none                          */
/*  Called by   : sel_palette()                     */
/*  Calls       : none                          */
/************************************************************************/

PRIVATE     VOID
init(int blocks, int distinct, struct rgb *my_color_pt)
{
    int         i, j, k, l;
    int         temp[MAXCOLOR];
    struct box *first;
    struct box *dummy;

    /* alloc memory */
    if (hist)
        HDfree((VOIDP) hist);
    if (distinct_pt)
        HDfree((VOIDP) distinct_pt);
    hist = (int *) HDmalloc((unsigned) distinct * sizeof(int));
    distinct_pt = (struct rgb *) HDmalloc((unsigned) distinct *
                                            sizeof(struct rgb));

    for (i = 0; i < distinct; i++)
        hist[i] = 0;

    /* select distinct pts and set up histogram */
    for (i = 0; i < MAXCOLOR; i++)
        temp[i] = -1;

    k = 0;
    for (i = 0; i < (2 * blocks); i++)
      {
          j = ((int) my_color_pt[i].c[RED] << 10) | (my_color_pt[i].c[GREEN] << 5) |
              my_color_pt[i].c[BLUE];

          if (temp[j] == -1)
            {
                /* new pt */
                temp[j] = k;
                for (l = RED; l <= BLUE; l++)
                    distinct_pt[k].c[l] = my_color_pt[i].c[l];
                k++;
            }

          hist[temp[j]]++;
      }

    /* set up first box */
    first = (struct box *) HDmalloc(sizeof(struct box));
    for (i = RED; i <= BLUE; i++)
      {
          first->bnd[i][LO] = (float32) 999.9;
          first->bnd[i][HI] = (float32) -999.9;

          for (j = 0; j < distinct; j++)
            {
                if (first->bnd[i][LO] > (float) distinct_pt[j].c[i])
                    first->bnd[i][LO] = (float) distinct_pt[j].c[i];

                if (first->bnd[i][HI] < (float) distinct_pt[j].c[i])
                    first->bnd[i][HI] = (float) distinct_pt[j].c[i];
            }   /* end of for j */

          first->bnd[i][LO] = first->bnd[i][LO] - (float32) EPSILON;
          first->bnd[i][HI] = first->bnd[i][HI] + (float32) EPSILON;
      }     /* end of for i */

    first->pts = (int *) HDmalloc((unsigned) distinct * sizeof(int));
    for (i = 0; i < distinct; i++)
        first->pts[i] = i;
    first->nmbr_pts = 2 * blocks;
    first->nmbr_distinct = distinct;

    dummy = (struct box *) HDmalloc(sizeof(struct box));
    frontier = dummy;
    dummy->right = first;
    first->left = dummy;
    first->right = NULL;
    dummy->nmbr_pts = 0;

    HDfree((VOIDP) first);
    HDfree((VOIDP) dummy);
}   /* end of init */

/************************************************************************/
/*  Function    : sort                          */
/*  Purpose : Performs quick sort on the points in a box along a    */
/*        given dimension                   */
/*  Parameter   :                           */
/*    l, r   - index of leftmost and rightmost element      */
/*    dim    - dimension along which sorting is done        */
/*    rank   - an array which carries the index of the points to be */
/*         sorted                       */
/*  Returns     : none                          */
/*  Called by   : find_med()                        */
/*  Calls       : partition()                       */
/************************************************************************/

PRIVATE     VOID
sort(int l, int r, int dim, int rank[])
{
    int         i;

    if (r > l)
      {
          i = partition(l, r, dim, rank);
          sort(l, i - 1, dim, rank);
          sort(i + 1, r, dim, rank);
      }
}

/************************************************************************
*  Function    : partition
*  Purpose : Partitions the list into 2 parts as in the quick sort
*        algorithm
*  Parameter   :
*    l, r   - index of leftmost and rightmost element
*    dim    - dimension along which sorting is done
*    rank   - an array which carries the index of the points to be
*  Returns     : index where list is partitioned
*  Called by   : sort()
*  Calls       : none
************************************************************************/

PRIVATE int
partition(int l, int r, int dim, int rank[])
{
    int         i, j, temp;
    uint8       v;

    v = distinct_pt[rank[r]].c[dim];
    i = l - 1;
    j = r;

    /* repeat until i and j crosses */
    do
      {
          /* repeat until an element >= v is found */
          do
              i++;
          while (distinct_pt[rank[i]].c[dim] < v);

          /* repeat until an element <= v is found */
          do
              j--;
          while ((j > 0) && (distinct_pt[rank[j]].c[dim] > v));

          /* swap pointers */
          temp = rank[i];
          rank[i] = rank[j];
          rank[j] = temp;
      }
    while (i < j);

    /* position partitioning element at location i */
    temp = rank[j];
    rank[j] = rank[i];
    rank[i] = rank[r];
    rank[r] = temp;

    return i;
}

/************************************************************************/
/*  Function    : find_box                      */
/*  Purpose : Finds the box with the largest number of color points */
/*        The points need not necessarily be distinct. But in   */
/*        order to partition the box, there must be at least  2 */
/*        distinct points                   */
/*  Parameter   : none                          */
/*  Returns     : pointer to box selected for splitting         */
/*  Called by   : sel_palette()                     */
/*  Calls       : none                          */
/************************************************************************/

PRIVATE struct box *
find_box(void)
{
    struct box *temp;
    struct box *max;
    int         max_pts;

    max_pts = 1;
    max = NULL;
    temp = frontier->right;
    while (temp != NULL)
        if ((temp->nmbr_distinct > 1) && (max_pts < temp->nmbr_pts))
          {
              max_pts = temp->nmbr_pts;
              max = temp;
              temp = temp->right;
          }
        else
            temp = temp->right;

    if (max == NULL)
      {
          return(NULL); /* punt! */
      }

    return max;
}

/************************************************************************/
/*  Function    : split_box                     */
/*  Purpose : Splits a selected box into 2 and reinserts the 2 sub- */
/*        boxes into the frontier list              */
/*  Parameter   :                           */
/*    ptr    - pointer to box to be split               */
/*  Returns     : none                          */
/*  Called by   : sel_palette()                     */
/*  Calls       : find_med(), select_dim(), classify()          */
/************************************************************************/

PRIVATE     VOID
split_box(struct box * ptr)
{
    int         dim, j, i;
    float       median;
    struct box *l_child, *r_child;

    dim = select_dim(ptr);
    median = find_med(ptr, dim);

    /* create 2 child */
    l_child = (struct box *) HDmalloc(sizeof(struct box));
    r_child = (struct box *) HDmalloc(sizeof(struct box));

    for (i = RED; i <= BLUE; i++)
        for (j = HI; j <= LO; j++)
          {
              l_child->bnd[i][j] = ptr->bnd[i][j];
              r_child->bnd[i][j] = ptr->bnd[i][j];
          }
    l_child->bnd[dim][HI] = median;
    r_child->bnd[dim][LO] = median;

    classify(ptr, l_child);
    classify(ptr, r_child);

    r_child->right = ptr->right;
    r_child->left = l_child;
    l_child->right = r_child;
    l_child->left = ptr->left;
    (ptr->left)->right = l_child;
    if (ptr->right != NULL)
        (ptr->right)->left = r_child;
}   /* end of split_box */

/************************************************************************/
/*  Function    : assign_color                      */
/*  Purpose : Assigns a color to each box. It computes the average  */
/*        color of all the points in the box            */
/*        Sets up the new_pal buffer. Each color component is   */
/*        shifted left 3 bits because of the truncation when    */
/*        color_pt was set up                   */
/*  Parameter   : none                          */
/*  Returns     : none                          */
/*  Called by   : sel_palette()                     */
/*  Calls       : none                          */
/************************************************************************/

PRIVATE     VOID
assign_color(void)
{
    struct box *temp;
    int         ent, k, j;
    int         c[3];

    temp = frontier->right;
    for (ent = 0; ent < PALSIZE; ent++)
      {
          for (k = RED; k <= BLUE; k++)
              c[k] = 0;

          /*
             printf("Box %d: number of pts %d\n", ent, temp->nmbr_pts);
           */

          for (j = 0; j < temp->nmbr_distinct; j++)
            {
                /*
                   printf("pt %d:", j);
                 */
                for (k = RED; k <= BLUE; k++)
                  {
                      /*
                         printf("%d ",distinct_pt[temp->pts[j]].c[k]);
                       */
                      c[k] = c[k] +
                          distinct_pt[temp->pts[j]].c[k] * hist[temp->pts[j]];
                  }
                /*
                   printf("\n");
                 */
            }

          for (k = RED; k <= BLUE; k++)
            {
                c[k] = c[k] / temp->nmbr_pts;
                new_pal[3 * ent + k] = (uint8) (c[k] << 3);
            }

          temp = temp->right;
      }     /* end of for entry */
}

/************************************************************************/
/*  Function    : select_dim                        */
/*  Purpose : Selects the dimension with the largest spread         */
/*  Parameter   :                           */
/*    ptr    - pointer to desired box               */
/*  Returns     : dimension where the box is to be split        */
/*  Called by   : split_box()                       */
/*  Calls       : none                          */
/************************************************************************/
PRIVATE int
select_dim(struct box *ptr)
{
    int         i, j;
    uint8       low[3], high[3];
    uint8       max;

    for (j = RED; j <= BLUE; j++)
      {
          low[j] = distinct_pt[ptr->pts[0]].c[j];
          high[j] = distinct_pt[ptr->pts[0]].c[j];
      }

    for (i = 1; i < ptr->nmbr_distinct; i++)
        for (j = RED; j <= BLUE; j++)
          {
              if (low[j] > distinct_pt[ptr->pts[i]].c[j])
                  low[j] = distinct_pt[ptr->pts[i]].c[j];
              if (high[j] < distinct_pt[ptr->pts[i]].c[j])
                  high[j] = distinct_pt[ptr->pts[i]].c[j];
          }

    max = (uint8) (high[RED] - low[RED]);
    i = RED;
    for (j = GREEN; j <= BLUE; j++)
        if (max < (uint8) (high[j] - low[j]))
          {
              max = (uint8) (high[j] - low[j]);
              i = j;
          }

    return i;
}   /* end of select_dim */

/************************************************************************/
/*  Function    : find_med                      */
/*  Purpose : Finds the point where the box is to be split. It finds */
/*        a point such that the 2 new boxes have about the same */
/*        number of color points.               */
/*  Parameter   :                           */
/*    ptr    - pointer to box to be split               */
/*    dim    - dimension to split box               */
/*  Returns     : point where the box is to be cut          */
/*  Called by   : split_box()                       */
/*  Calls       : next_pt()                     */
/************************************************************************/

PRIVATE float
find_med(struct box *ptr, int dim)
{
    int         i, j, count, next, prev;
    int        *rank;
    float32     median;

    rank = (int *) HDmalloc((unsigned) ptr->nmbr_distinct * sizeof(int));
    for (i = 0; i < ptr->nmbr_distinct; i++)
        rank[i] = ptr->pts[i];

    sort(0, ptr->nmbr_distinct - 1, dim, rank);
    /*
       for (i=0; i<ptr->nmbr_distinct; i++)
       printf("find_med: sorted list is %d\n",distinct_pt[rank[i]].c[dim]);
     */

    count = 0;
    prev = i = 0;
    while ((i < ptr->nmbr_distinct) && (count < ptr->nmbr_pts / 2))
      {
          next = next_pt(dim, i, rank, ptr->nmbr_distinct);
          for (j = i; j < next; j++)
              count = count + hist[rank[j]];

          prev = i;
          i = next;
      }

    if (prev == 0)
      {
          /* the first distinct point overshot the median */
          median = (float32) distinct_pt[rank[prev]].c[dim] + (float32) EPSILON;
      }
    else
        median = (float32) distinct_pt[rank[prev - 1]].c[dim] + (float32) EPSILON;

    HDfree((VOIDP) rank);
    return median;
}   /* end of find_med */

/************************************************************************/
/*  Function    : classify                      */
/*  Purpose : Looks at the color points in the parent and selects   */
/*        the points that belong to the child           */
/*  Parameter   :                           */
/*    ptr    - pointer to parent                    */
/*    child  - pointer to child box                 */
/*  Returns     : none                          */
/*  Called by   : split_box()                       */
/*  Calls       : none                          */
/************************************************************************/

PRIVATE     VOID
classify(struct box * ptr, struct box * child)
{
    int         i, j;
    int        *temp;
    int         distinct, total;

    temp = (int *) HDmalloc((unsigned) ptr->nmbr_distinct * sizeof(int));

    distinct = 0;
    total = 0;
    for (i = 0; i < ptr->nmbr_distinct; i++)
      {
          j = ptr->pts[i];
          if ((((float) distinct_pt[j].c[RED] >= child->bnd[RED][LO]) &&
               ((float) distinct_pt[j].c[RED] <= child->bnd[RED][HI])) &&
              (((float) distinct_pt[j].c[GREEN] >= child->bnd[GREEN][LO]) &&
               ((float) distinct_pt[j].c[GREEN] <= child->bnd[GREEN][HI])) &&
              (((float) distinct_pt[j].c[BLUE] >= child->bnd[BLUE][LO]) &&
               ((float) distinct_pt[j].c[BLUE] <= child->bnd[BLUE][HI])))
            {
                /* pt is in new box */
                temp[distinct] = j;
                distinct++;
                total = total + hist[j];
            }   /* end of if */
      }     /* end of for i */

    /* assign points */
    child->nmbr_pts = total;
    child->nmbr_distinct = distinct;
    child->pts = (int *) HDmalloc((unsigned) distinct * sizeof(int));
    for (i = 0; i < distinct; i++)
        child->pts[i] = temp[i];

    HDfree((VOIDP) temp);

}   /* end of classify */

/************************************************************************/
/*  Function    : next_pt                       */
/*  Purpose : Determines the next point that has a different value  */
/*        from the current point along  a dimension     */
/*  Parameter   :                           */
/*    dim    - dimension where box is to be split           */
/*    i      - index to current point               */
/*    rank       - sorted list of points to be searched starting from i */
/*    distinct   - length of sorted list                                */
/*  Returns     : index of point that has a different value     */
/*  Called by   : find_med                      */
/*  Calls       : none                          */
/************************************************************************/

PRIVATE int
next_pt(int dim, int i, int rank[], int distinct)
{
    int         j;
    uint8       old;

    old = distinct_pt[rank[i]].c[dim];
    for (j = (i + 1); j < distinct; j++)
        if (distinct_pt[rank[j]].c[dim] != old)
            break;

    return j;
}   /* end of next_pt */
