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
static char RcsId[] = "@(#)1.13";
#endif

/* dfkvms.c,v 1.13 1994/10/19 20:55:46 koziol Exp */

/*------------------------------------------------------------------
 File:  dfkvms.c

 Purpose:
    Routines to support Vax-native conversion to and from HDF format

 Invokes:

 PRIVATE conversion functions:
    DFKvi4f -  VMS routine for importing 32 bit floats
    DFKvo4f -  VMS routine for exporting 32 bit floats
    DFKvi8f -  VMS routine for importing 64 bit floats
    DFKvo8f -  VMS routine for exporting 64 bit floats
    DFKlvi4f-  VMS routine for importing little-endian 32 bit floats
    DFKlvo4f-  VMS routine for exporting little-endian 32 bit floats
    DFKlvi8f-  VMS routine for importing little-endian 64 bit floats
    DFKlvo8f-  VMS routine for exporting little-endian 64 bit floats

 Remarks:
    These files used to be in dfconv.c, but it got a little too huge,
    so I broke them out into seperate files. - Q

 Modifications:
    Lawrence Fisher, Digital Equipment Corp, October 29, 1997
    Added routines to support OpenVMS Alpha.  The default 64 bit floating point
    type for OpenVMS Alpha is Vax G-Float, not Vax D-float, as a result the
    VAX VMS designed conversion routines were in error.  Substituted
    the OpenVMS routine CVT$FTOF (convert float to float) to optimize
    throughput.

    Lawrence Fisher, Digital Eqipment Corp, January 30th, 1998
    Rewrote conversion routines to use OpenVMS routine CVT$CONVERT_FLOAT
    to provide compatibility back to version 6.2 of OpenVMS Alpha.

 *------------------------------------------------------------------*/

/*****************************************************************************/
/*                                                                           */
/*    All the routines in this file marked as PRIVATE have been marked so    */
/*  for a reason.  *ANY* of these routines may or may nor be supported in    */
/*  the next version of HDF (4.00).  Furthurmore, the names, paramters, or   */
/*  functionality is *NOT* guaranteed to remain the same.                    */
/*    The *ONLY* guarantee possible is that DFKnumin(), and DFKnumout()      */
/*  will not change.  They are *NOT* guaranteed to be implemented in the     */
/*  next version of HDF as function pointers.  They are guaranteed to take   */
/*  the same arguments and produce the same results.                         */
/*    If your programs call any routines in this file except for             */
/*  DFKnumin(), DFKnumout, and/or DFKsetntype(), your code may not work      */
/*  with future versions of HDF and your code will *NOT* be portable.        */
/*                                                                           */
/*****************************************************************************/

#include "hdf.h"
#include "hconv.h"

#if defined(VMS)
#ifdef __ALPHA
#include <cvt$routines.h>
#include <cvtdef.h>

#endif /* __ALPHA */

/************************************************************/
/* NUMBER CONVERSION ROUTINES FOR VAX ARCHITECTURES         */
/* Most of the VAX number types can be handled with the     */
/* generic byte swapping routines.  Only the floating point */
/* conversion routines need to be customized.               */
/************************************************************/

/************************************************************/
/* DFKvi4f()                                                */
/* --> Import routine for 4 byte VAX floats                 */
/************************************************************/
int
DFKvi4f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    int         in_place = 0;   /* Inplace must be detected */
    uint32 i;
    uint8       buf[4];         /* Inplace processing buffer */
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    char       *FUNC = "DFKvi4f";
    uint8       exp;

    HEclear();

    if (source == dest)
        in_place = TRUE;

    if (num_elm == 0 || in_place)
      {     /* No elements is an error as is in place. */
          HERROR(DFE_BADCONV);
          return FAIL;
      }

    if (source_stride == 0 && dest_stride == 0)
        source_stride = dest_stride = 4;

    for (i = 0; i < num_elm; i++)
      {
#ifdef __ALPHA

        {
	uint32 *s1 = (uint32 *)source;
        uint32 *d1 = (uint32 *)dest;
        uint32 input = *s1;
	uint32 output= *d1;
/*          (void)cvt$ftof(&input,CVT$K_BIG_ENDIAN_IEEE_S,&output,CVT$K_VAX_F,0);
*/
	(void)CVT$CONVERT_FLOAT(&input,CVT$K_IEEE_S,&output,CVT$K_VAX_F,
				CVT$M_ROUND_TO_NEAREST+CVT$M_BIG_ENDIAN);
	*d1 = output;
        }

#else

          /* extract exponent */
          exp = (source[0] << 1) | (source[1] >> 7);
          if (exp)
            {
                /*
                 * non-zero exponent
                 */

                /* copy mantissa, last bit of exponent */
                dest[0] = source[1];
                dest[2] = source[3];
                dest[3] = source[2];
                if (exp < 254)
                    /* normal value */
                    dest[1] = source[0] + (uint8) 1;    /* actually adds two to exp */
                else
                  {
                      /* infinity or NaN */
                      if (exp == 254)   /* unrepresentable - OFL */
                          /* set mant = 0 for overflow */
                          dest[0] = dest[1] = dest[2] = dest[3] = 0;
                      dest[0] &= 0x7f;  /* set last bit of exp to 0 */
                      dest[1] = 0x80;   /* sign=1 exp=0 -> OFL or NaN */
                  }
            }
          else if (source[1] & 0x60)
            {   /* denormalized value */
                int shft;

                shft = (source[1] & 0x40) ? 1 : 2;  /* shift needed to normalize */
                /* shift mantissa */
                /* note last bit of exp set to 1 implicitly */
                dest[0] = (uint8) (source[1] << shft)
                    | (uint8) (source[2] >> (8 - shft));
                dest[3] = (uint8) (source[2] << shft)
                    | (uint8) (source[3] >> (8 - shft));
                dest[2] = (uint8) (source[3] << shft);
                dest[1] = (uint8) (source[0] & 0x80);   /* sign */
                if (shft == 1)
                  {     /* set exp to 2 */
                      dest[1] |= 0x01;
                      dest[0] &= 0x7f;  /* set LSB of exp to 0 */
                  }
            }
          else
              dest[0] = dest[1] = dest[2] = dest[3] = 0;

#endif /* __ALPHA */
          source += source_stride;
          dest += dest_stride;
      }

    return 0;

#ifdef DFKIT
    for (i = 0; i < size; i++)
      {
          /* extract exponent */
          exp = (uint8) (in[i].c[0] << 1) | (uint8) (in[i].c[1] >> 7);
          if (exp)
            {   /* non-zero exponent */
                /* copy mantissa, last bit of exponent */
                out[i].c[0] = in[i].c[1];
                out[i].c[2] = in[i].c[3];
                out[i].c[3] = in[i].c[2];
                if (exp < 254)  /* normal value */
                    out[i].c[1] = in[i].c[0] + (uint8) 1;   /* adds two to exp */
                else
                  {     /* infinity or NaN */
                      if (exp == 254)   /* unrepresentable - OFL */
                          out[i].i = 0;     /* set mant=0 for overflow */
                      out[i].c[0] &= 0x7f;  /* set last bit of exp to 0 */
                      out[i].c[1] = 0x80;   /* sign=1 exp=0 -> OFL or NaN */
                  }
            }
          else if (in[i].c[1] & 0x60)
            {   /* denormalized value */
                int shft;

                /* shift needed to normalize */
                shft = (in[i].c[1] & 0x40) ? 1 : 2;

                /* shift mantissa */
                /* note last bit of exp set to 1 implicitly */
                out[i].c[0] = (uint8) (in[i].c[1] << shft)
                    | (uint8) (in[i].c[2] >> (8 - shft));
                out[i].c[3] = (uint8) (in[i].c[2] << shft)
                    | (uint8) (in[i].c[3] >> (8 - shft));
                out[i].c[2] = (uint8) (in[i].c[3] << shft);
                out[i].c[1] = (uint8) (in[i].c[0] & 0x80);  /* sign */
                if (shft == 1)
                  {     /* set exp to 2 */
                      out[i].c[1] |= 0x01;
                      out[i].c[0] &= 0x7f;  /* set LSB of exp to 0 */
                  }
            }
          else
              out[i].i = 0;     /* zero */
      }
    return (0);
#endif /* DFKIT */

}

/************************************************************/
/* DFKvo4f()                                                */
/* --> Export routine for 4 byte VAX floats                 */
/************************************************************/
int
DFKvo4f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    int         in_place = 0;   /* Inplace must be detected */
    uint32 i;
    uint8       buf[4];         /* Inplace processing buffer */
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    char       *FUNC = "DFKvo4f";
    intn        exp;

    HEclear();

    if (source == dest)
        in_place = TRUE;

    if (num_elm == 0 || in_place)
      {     /* No elements is an error as is in place */
          HERROR(DFE_BADCONV);
          return FAIL;
      }

    if (source_stride == 0 && dest_stride == 0)
        source_stride = dest_stride = 4;

    for (i = 0; i < num_elm; i++)
      {
#ifdef __ALPHA

	 {
	 uint32 *s1 = (uint32 *)source;
	 uint32 *d1 = (uint32 *)dest;
	 uint32 input = *s1;
	 uint32 output = *d1;
/*        
(void)cvt$ftof(&input,CVT$K_VAX_F,&output,CVT$K_BIG_ENDIAN_IEEE_S,0); */
	(void)CVT$CONVERT_FLOAT(&input,CVT$K_VAX_F,&output,CVT$K_IEEE_S,
				CVT$M_ROUND_TO_NEAREST+CVT$M_BIG_ENDIAN);
	 *d1 = output;
	 }

#else

          /* extract exponent */
          exp = (source[1] << 1) | (source[0] >> 7);

          if (!exp && !source[1])
            {
                /*
                 * zero value
                 */
                dest[0] = dest[1] = dest[2] = dest[3] = 0;
            }
          else if (exp > 2)
            {
                /*
                 * Normal value
                 */

                dest[0] = source[1] - (uint8) 1;    /* subtracts 2 from exponent */
                /* copy mantissa, LSB of exponent */
                dest[1] = source[0];
                dest[2] = source[3];
                dest[3] = source[2];

            }
          else if (exp)
            {
                intn shft;
                /*
                 * denormalized number
                 */

                /* keep sign, zero exponent */
                dest[0] = source[1] & 0x80;

                shft = 3 - exp;

                /* shift original mant by 1 or 2 to get denormalized mant */
                /* prefix mantissa with '1'b or '01'b as appropriate */
                dest[1] = (uint8) ((source[0] & 0x7f) >> shft)
                    | (uint8) (0x10 << exp);
                dest[2] = (uint8) (source[0] << (8 - shft))
                    | (uint8) (source[3] >> shft);
                dest[3] = (uint8) (source[3] << (8 - shft))
                    | (uint8) (source[2] >> shft);
            }
          else
            {
                /*
                 * sign=1 -> infinity or NaN
                 */

                dest[0] = 0xff;     /* set exp to 255 */
                /* copy mantissa */
                dest[1] = source[0] | (uint8) 0x80;     /* LSB of exp = 1 */
                dest[2] = source[3];
                dest[3] = source[2];
            }

#endif /* __ALPHA */

          source += source_stride;
          dest += dest_stride;
      }

    return 0;

#ifdef DFKIT
    uint8       exp;
    int         i;

    for (i = 0; i < size; i++)
      {
          /* extract exponent */
          exp = (uint8) (in[i].c[1] << 1) | (uint8) (in[i].c[0] >> 7);
          if (!exp && !in[i].c[1])
              out[i].i = 0;     /* zero value */
          else if (exp > 2)
            {   /* normal value */
                out[i].c[0] = in[i].c[1] - (uint8) 1;   /* subtracts 2 from expent */
                /* copy mantissa, LSB of exponent */
                out[i].c[1] = in[i].c[0];
                out[i].c[2] = in[i].c[3];
                out[i].c[3] = in[i].c[2];
            }
          else if (exp)
            {   /* denormalized number */
                int shft;

                /* keep sign, zero exponent */
                out[i].c[0] = in[i].c[1] & (uint8) 0x80;
                shft = 3 - exp;
                /* shift original mant by 1 or 2 to get denormalized mant */
                /* prefix mantissa with '1'b or '01'b as appropriate */
                out[i].c[1] = (uint8) ((in[i].c[0] & 0x7f) >> shft)
                    | (uint8) (0x10 << exp);
                out[i].c[2] = (uint8) (in[i].c[0] << (8 - shft))
                    | (uint8) (in[i].c[3] >> shft);
                out[i].c[3] = (uint8) (in[i].c[3] << (8 - shft))
                    | (uint8) (in[i].c[2] >> shft);
            }
          else
            {   /* sign=1 -> infinity or NaN */
                out[i].c[0] = 0xff;     /* set exp to 255 */
                /* copy mantissa */
                out[i].c[1] = in[i].c[0] | (uint8) 0x80;    /* LSB of exp = 1 */
                out[i].c[2] = in[i].c[3];
                out[i].c[3] = in[i].c[2];
            }
      }
    return (0);

#endif /* DFKIT */

}

/*
 * Define structures to encode and decode Vax numbers
 * The following code is based on the methods of reading / writing
 *  doubles from Vaxen developed by Sun Microsystems as part of
 *  the XDR library.
 */

/* How an IEEE double looks */
struct ieee_double
{
    unsigned int mantissa1:20;
    unsigned int exp:11;
    unsigned int sign:1;
    unsigned int mantissa2:32;
};

/* How a Vax double looks */
struct vax_double
  {
      unsigned int mantissa1:7;
      unsigned int exp:8;
      unsigned int sign:1;
      unsigned int mantissa2:16;
      unsigned int mantissa3:16;
      unsigned int mantissa4:16;
  };

#define VAX_DBL_BIAS   0x81
#define IEEE_DBL_BIAS  0x3ff
#define MASK(nbits)    ((1 << nbits) - 1)

struct dbl_limits
  {
      struct vax_double vaxx;
      struct ieee_double ieee;
  };

static struct dbl_limits dbl_lim[2] =
{
    {
        {0x7f, 0xff, 0x0, 0xffff, 0xffff, 0xfff},   /* Max Vax */
        {0x0, 0x7ff, 0x0, 0x0}},    /* Max IEEE */
    {
        {0x0, 0x0, 0x0, 0x0, 0x0, 0x0},     /* Min Vax */
        {0x0, 0x0, 0x0, 0x0}}   /* Min IEEE */
};

/************************************************************/
/* DFKvi8f()                                                */
/* --> Import routine for 8 byte VAX floats                 */
/************************************************************/
int
DFKvi8f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    int         in_place = 0;   /* Inplace must be detected */
    uint32 i;
    uint8       buf[4];         /* Inplace processing buffer */
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    char       *FUNC = "DFKvi8f";
    intn        exp;

    struct dbl_limits *lim;
    struct ieee_double id;
    struct vax_double *vd;
    intn        found, j;

    HEclear();

    if (source == dest)
        in_place = TRUE;

    if (num_elm == 0 || in_place)
      {     /* No elements is an error as is in place. */
          HERROR(DFE_BADCONV);
          return FAIL;
      }

    if (source_stride == 0 && dest_stride == 0)
        source_stride = dest_stride = 8;

    for (i = 0; i < num_elm; i++)
      {
#ifdef __ALPHA

/*        
(void)cvt$ftof(&source[0],CVT$K_BIG_ENDIAN_IEEE_T,&dest[0],CVT$K_VAX_G,0); */
	(void)CVT$CONVERT_FLOAT(&source[0],CVT$K_IEEE_T,&dest[0],CVT$K_VAX_G,
				CVT$M_ROUND_TO_NEAREST+CVT$M_BIG_ENDIAN);

#else
        shipit: /* In VAX, bytes in a word are counted from right to left */
          {
              int j, k;
              uint8      *bufi, *i3e;

              i3e = source;
              bufi = (uint8 *) &(id);
              for (j = 0; j < 2; j++)
                {
                    for (k = 0; k < 4; k++)
                        bufi[k] = i3e[3 - k];
                    bufi += 4;
                    i3e += 4;
                }
          }     /* shipit   */

          vd = (struct vax_double *) dest;

          found = FALSE;

          for (j = 0, lim = dbl_lim;
               j < sizeof(dbl_lim) / sizeof(struct dbl_limits);
               j++, lim++)
            {
                if ((id.mantissa2 == lim->ieee.mantissa2) &&
                    (id.mantissa1 == lim->ieee.mantissa1) &&
                    (id.exp == lim->vaxx.exp))
                  {
                      *vd = lim->vaxx;
                      found = TRUE;
                      break;
                  }
            }

          if (!found)
            {
                vd->exp = id.exp - IEEE_DBL_BIAS + VAX_DBL_BIAS;
                vd->mantissa1 = id.mantissa1 >> 13;
                vd->mantissa2 = ((id.mantissa1 & MASK(13)) << 3) |
                    (id.mantissa2 >> 29);
                vd->mantissa3 = id.mantissa2 >> 13;
                vd->mantissa4 = id.mantissa2 << 3;
            }

          vd->sign = id.sign;

#endif /* __ALPHA */

          source += source_stride;
          dest += dest_stride;
      }

    return 0;
}

/************************************************************/
/* DFKvo8f()                                                */
/* --> Export routine for 8 byte VAX floats                 */
/************************************************************/
int
DFKvo8f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    int         in_place = 0;   /* Inplace must be detected */
    intn i;
    uint8       buf[4];         /* Inplace processing buffer */
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    char       *FUNC = "DFKvo8f";
    intn        exp;

    struct dbl_limits *lim;
    struct ieee_double id;
    struct vax_double vd;
    intn        found, j;

    HEclear();

    if (source == dest)
        in_place = TRUE;

    if (num_elm == 0 || in_place)
      {     /* No elements is an error as is in place. */
          HERROR(DFE_BADCONV);
          return FAIL;
      }

    if (source_stride == 0 && dest_stride == 0)
        source_stride = dest_stride = 8;

    for (i = 0; i < num_elm; i++)
      {

#ifdef __ALPHA

/*        
(void)cvt$ftof(&source[0],CVT$K_VAX_G,&dest[0],CVT$K_BIG_ENDIAN_IEEE_T,0); */
	(void)CVT$CONVERT_FLOAT(&source[0],CVT$K_VAX_G,&dest[0],CVT$K_IEEE_T,
				CVT$M_ROUND_TO_NEAREST+CVT$M_BIG_ENDIAN);

#else
          vd = *((struct vax_double *) source);

          found = FALSE;

          for (j = 0, lim = dbl_lim;
               j < sizeof(dbl_lim) / sizeof(struct dbl_limits);
               j++, lim++)
            {
                if ((vd.mantissa4 == lim->vaxx.mantissa4) &&
                    (vd.mantissa3 == lim->vaxx.mantissa3) &&
                    (vd.mantissa2 == lim->vaxx.mantissa2) &&
                    (vd.mantissa1 == lim->vaxx.mantissa1) &&
                    (vd.exp == lim->vaxx.exp))
                  {
                      id = lim->ieee;
                      found = TRUE;
                      break;
                  }
            }

          if (!found)
            {
                id.exp = vd.exp - VAX_DBL_BIAS + IEEE_DBL_BIAS;
                id.mantissa1 = (vd.mantissa1 << 13) | (vd.mantissa2 >> 3);
                id.mantissa2 = ((vd.mantissa2 & MASK(3)) << 29) |
                    (vd.mantissa3 << 13) |
                    ((vd.mantissa4 >> 3) & MASK(13));
            }

          id.sign = vd.sign;

        shipit: /* In VAX the bytes in a word are counted from right to left */
          {
              int j, k;
              uint8      *i3e, *bufo;

              i3e = (uint8 *) &(id);
              bufo = dest;
              for (j = 0; j < 2; j++)
                {
                    for (k = 0; k < 4; k++)
                        bufo[k] = i3e[3 - k];
                    bufo += 4;
                    i3e += 4;
                }
          }

#endif /* __ALPHA */

          source += source_stride;
          dest += dest_stride;
      }

    return 0;
}

/************************************************************/
/* DFKlvi4f()                                                */
/* --> Import routine for 4 byte VAX floats                 */
/************************************************************/
int
DFKlvi4f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    int         in_place = 0;   /* Inplace must be detected */
    uint32 i;
    uint8       buf[4];         /* Inplace processing buffer */
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    char       *FUNC = "DFKvi4f";
    uint8       exp;

    HEclear();

    if (source == dest)
        in_place = TRUE;

    if (num_elm == 0 || in_place)
      {     /* No elements is an error as is in place. */
          HERROR(DFE_BADCONV);
          return FAIL;
      }

    if (source_stride == 0 && dest_stride == 0)
        source_stride = dest_stride = 4;

    for (i = 0; i < num_elm; i++)
      {

#ifdef __ALPHA

        {
	uint32 *s1 = (uint32 *)source;
        uint32 *d1 = (uint32 *)dest;
        uint32 input = *s1;
	uint32 output= *d1;
/*          (void)cvt$ftof(&input,CVT$K_IEEE_S,&output,CVT$K_VAX_F,0); */
	(void)CVT$CONVERT_FLOAT(&input,CVT$K_IEEE_S,&output,CVT$K_VAX_F,
				CVT$M_ROUND_TO_NEAREST);
	*d1 = output;
        }

#else

          /* extract exponent */
          exp = (source[3] << 1) | (source[2] >> 7);
          if (exp)
            {
                /*
                 * non-zero exponent
                 */

                /* copy mantissa, last bit of exponent */
                dest[0] = source[2];
                dest[2] = source[0];
                dest[3] = source[1];
                if (exp < 254)
                    /* normal value */
                    dest[1] = source[3] + (uint8) 1;    /* actually adds two to exp */
                else
                  {
                      /* infinity or NaN */
                      if (exp == 254)   /* unrepresentable - OFL */
                          /* set mant = 0 for overflow */
                          dest[0] = dest[1] = dest[2] = dest[3] = 0;
                      dest[0] &= 0x7f;  /* set last bit of exp to 0 */
                      dest[1] = 0x80;   /* sign=1 exp=0 -> OFL or NaN */
                  }
            }
          else if (source[2] & 0x60)
            {   /* denormalized value */
                int shft;

                shft = (source[2] & 0x40) ? 1 : 2;  /* shift needed to normalize */
                /* shift mantissa */
                /* note last bit of exp set to 1 implicitly */
                dest[0] = (uint8) (source[2] << shft) | (uint8) (source[1] >> (8 - shft));
                dest[3] = (uint8) (source[1] << shft) | (uint8) (source[0] >> (8 - shft));
                dest[2] = (uint8) (source[0] << shft);
                dest[1] = (uint8) (source[3] & 0x80);   /* sign */
                if (shft == 1)
                  {     /* set exp to 2 */
                      dest[1] |= 0x01;
                      dest[0] &= 0x7f;  /* set LSB of exp to 0 */
                  }
            }
          else
            {
                dest[0] = dest[1] = dest[2] = dest[3] = 0;
            }

#endif /* __ALPHA */

          source += source_stride;
          dest += dest_stride;
      }

    return 0;

#ifdef DFKIT
    for (i = 0; i < size; i++)
      {
          /* extract exponent */
          exp = (uint8) (in[i].c[0] << 1) | (uint8) (in[i].c[1] >> 7);
          if (exp)
            {   /* non-zero exponent */
                /* copy mantissa, last bit of exponent */
                out[i].c[0] = in[i].c[1];
                out[i].c[2] = in[i].c[3];
                out[i].c[3] = in[i].c[2];
                if (exp < 254)  /* normal value */
                    out[i].c[1] = in[i].c[0] + (uint8) 1;   /* adds two to exp */
                else
                  {     /* infinity or NaN */
                      if (exp == 254)   /* unrepresentable - OFL */
                          out[i].i = 0;     /* set mant=0 for overflow */
                      out[i].c[0] &= 0x7f;  /* set last bit of exp to 0 */
                      out[i].c[1] = 0x80;   /* sign=1 exp=0 -> OFL or NaN */
                  }
            }
          else if (in[i].c[1] & 0x60)
            {   /* denormalized value */
                int shft;

                /* shift needed to normalize */
                shft = (in[i].c[1] & 0x40) ? 1 : 2;

                /* shift mantissa */
                /* note last bit of exp set to 1 implicitly */
                out[i].c[0] = (uint8) (in[i].c[1] << shft)
                    | (uint8) (in[i].c[2] >> (8 - shft));
                out[i].c[3] = (uint8) (in[i].c[2] << shft)
                    | (uint8) (in[i].c[3] >> (8 - shft));
                out[i].c[2] = (uint8) (in[i].c[3] << shft);
                out[i].c[1] = (uint8) (in[i].c[0] & 0x80);  /* sign */
                if (shft == 1)
                  {     /* set exp to 2 */
                      out[i].c[1] |= 0x01;
                      out[i].c[0] &= 0x7f;  /* set LSB of exp to 0 */
                  }
            }
          else
              out[i].i = 0;     /* zero */
      }
    return (0);
#endif /* DFKIT */

}

/************************************************************/
/* DFKlvo4f()                                                */
/* --> Export routine for 4 byte VAX floats                 */
/************************************************************/
int
DFKlvo4f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    int         in_place = 0;   /* Inplace must be detected */
    uint32 i;
    uint8       buf[4];         /* Inplace processing buffer */
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    char       *FUNC = "DFKvo4f";
    intn        exp;

    HEclear();

    if (source == dest)
        in_place = TRUE;

    if (num_elm == 0 || in_place)
      {     /* No elements is an error as is in place */
          HERROR(DFE_BADCONV);
          return FAIL;
      }

    if (source_stride == 0 && dest_stride == 0)
        source_stride = dest_stride = 4;

    for (i = 0; i < num_elm; i++)
      {

#ifdef __ALPHA

	 {
	 uint32 *s1 = (uint32 *)source;
	 uint32 *d1 = (uint32 *)dest;
	 uint32 input = *s1;
	 uint32 output = *d1;
/*         (void)cvt$ftof(&input,CVT$K_VAX_F,&output,CVT$K_IEEE_S,0); */
	(void)CVT$CONVERT_FLOAT(&input,CVT$K_VAX_F,&output,CVT$K_IEEE_S,
				CVT$M_ROUND_TO_NEAREST);
	 *d1 = output;
	 }

#else
          /* extract exponent */
          exp = (source[1] << 1) | (source[0] >> 7);

          if (!exp && !source[1])
            {
                /*
                 * zero value
                 */
                dest[0] = dest[1] = dest[2] = dest[3] = 0;
            }
          else if (exp > 2)
            {
                /*
                 * Normal value
                 */

                dest[3] = source[1] - (uint8) 1;    /* subtracts 2 from exponent */
                /* copy mantissa, LSB of exponent */
                dest[2] = source[0];
                dest[1] = source[3];
                dest[0] = source[2];

            }
          else if (exp)
            {
                intn shft;
                /*
                 * denormalized number
                 */

                /* keep sign, zero exponent */
                dest[3] = source[1] & 0x80;

                shft = 3 - exp;

                /* shift original mant by 1 or 2 to get denormalized mant */
                /* prefix mantissa with '1'b or '01'b as appropriate */
                dest[2] = (uint8) ((source[0] & 0x7f) >> shft)
                    | (uint8) (0x10 << exp);
                dest[1] = (uint8) (source[0] << (8 - shft))
                    | (uint8) (source[3] >> shft);
                dest[0] = (uint8) (source[3] << (8 - shft))
                    | (uint8) (source[2] >> shft);
            }
          else
            {
                /*
                 * sign=1 -> infinity or NaN
                 */

                dest[3] = 0xff;     /* set exp to 255 */
                /* copy mantissa */
                dest[2] = source[0] | (uint8) 0x80;     /* LSB of exp = 1 */
                dest[1] = source[3];
                dest[0] = source[2];
            }

#endif /* __ALPHA */
          source += source_stride;
          dest += dest_stride;
      }

    return 0;

#ifdef DFKIT
    uint8       exp;
    int         i;

    for (i = 0; i < size; i++)
      {
          /* extract exponent */
          exp = (uint8) (in[i].c[1] << 1) | (uint8) (in[i].c[0] >> 7);
          if (!exp && !in[i].c[1])
              out[i].i = 0;     /* zero value */
          else if (exp > 2)
            {   /* normal value */
                out[i].c[0] = in[i].c[1] - (uint8) 1;   /* subtracts 2 from expent */
                /* copy mantissa, LSB of exponent */
                out[i].c[1] = in[i].c[0];
                out[i].c[2] = in[i].c[3];
                out[i].c[3] = in[i].c[2];
            }
          else if (exp)
            {   /* denormalized number */
                int shft;

                /* keep sign, zero exponent */
                out[i].c[0] = in[i].c[1] & (uint8) 0x80;
                shft = 3 - exp;
                /* shift original mant by 1 or 2 to get denormalized mant */
                /* prefix mantissa with '1'b or '01'b as appropriate */
                out[i].c[1] = (uint8) ((in[i].c[0] & 0x7f) >> shft)
                    | (uint8) (0x10 << exp);
                out[i].c[2] = (uint8) (in[i].c[0] << (8 - shft))
                    | (uint8) (in[i].c[3] >> shft);
                out[i].c[3] = (uint8) (in[i].c[3] << (8 - shft))
                    | (uint8) (in[i].c[2] >> shft);
            }
          else
            {   /* sign=1 -> infinity or NaN */
                out[i].c[0] = 0xff;     /* set exp to 255 */
                /* copy mantissa */
                out[i].c[1] = in[i].c[0] | (uint8) 0x80;    /* LSB of exp = 1 */
                out[i].c[2] = in[i].c[3];
                out[i].c[3] = in[i].c[2];
            }
      }
    return (0);

#endif /* DFKIT */

}

/************************************************************/
/* DFKlvi8f()                                                */
/* --> Import routine for 8 byte VAX floats                 */
/************************************************************/
int
DFKlvi8f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    int         in_place = 0;   /* Inplace must be detected */
    uint32 i;
    uint8       buf[4];         /* Inplace processing buffer */
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    char       *FUNC = "DFKvi8f";
    intn        exp;

    struct dbl_limits *lim;
    struct ieee_double id;
    struct vax_double *vd;
    intn        found, j;

    HEclear();

    if (source == dest)
        in_place = TRUE;

    if (num_elm == 0 || in_place)
      {     /* No elements is an error as is in place. */
          HERROR(DFE_BADCONV);
          return FAIL;
      }

    if (source_stride == 0 && dest_stride == 0)
        source_stride = dest_stride = 8;

    for (i = 0; i < num_elm; i++)
      {
#ifdef __ALPHA

/*         (void)cvt$ftof(&source[0],CVT$K_IEEE_T,&dest[0],CVT$K_VAX_G,0); */
	(void)CVT$CONVERT_FLOAT(&source[0],CVT$K_IEEE_T,&dest[0],CVT$K_VAX_G,
				CVT$M_ROUND_TO_NEAREST);

#else
          HDmemcpy(&(id), &source[4], 4);   /* swap the two 4-byte words */
          HDmemcpy(((uint8 *) &(id)) + 4, source, 4);

          vd = (struct vax_double *) dest;

          found = FALSE;

          for (j = 0, lim = dbl_lim;
               j < sizeof(dbl_lim) / sizeof(struct dbl_limits); j++, lim++)
            {
                if ((id.mantissa2 == lim->ieee.mantissa2) &&
                    (id.mantissa1 == lim->ieee.mantissa1) &&
                    (id.exp == lim->vaxx.exp))
                  {
                      *vd = lim->vaxx;
                      found = TRUE;
                      break;
                  }
            }

          if (!found)
            {
                vd->exp = id.exp - IEEE_DBL_BIAS + VAX_DBL_BIAS;
                vd->mantissa1 = id.mantissa1 >> 13;
                vd->mantissa2 = ((id.mantissa1 & MASK(13)) << 3) |
                    (id.mantissa2 >> 29);
                vd->mantissa3 = id.mantissa2 >> 13;
                vd->mantissa4 = id.mantissa2 << 3;
            }

          vd->sign = id.sign;

#endif /* __ALPHA */
          source += source_stride;
          dest += dest_stride;
      }

    return 0;
}

/************************************************************/
/* DFKlvo8f()                                                */
/* --> Export routine for 8 byte VAX floats                 */
/************************************************************/
int
DFKlvo8f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    int         in_place = 0;   /* Inplace must be detected */
    intn i;
    uint8       buf[4];         /* Inplace processing buffer */
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    char       *FUNC = "DFKvo8f";
    intn        exp;

    struct dbl_limits *lim;
    struct ieee_double id;
    struct vax_double vd;
    intn        found, j;

    HEclear();

    if (source == dest)
        in_place = TRUE;

    if (num_elm == 0 || in_place)
      {     /* No elements is an error as is in place. */
          HERROR(DFE_BADCONV);
          return FAIL;
      }

    if (source_stride == 0 && dest_stride == 0)
        source_stride = dest_stride = 8;

    for (i = 0; i < num_elm; i++)
      {

#ifdef __ALPHA

/*         (void)cvt$ftof(&source[0],CVT$K_VAX_G,&dest[0],CVT$K_IEEE_T,0); */
	(void)CVT$CONVERT_FLOAT(&source[0],CVT$K_VAX_G,&dest[0],CVT$K_IEEE_T,
				CVT$M_ROUND_TO_NEAREST);

#else
          vd = *((struct vax_double *) source);

          found = FALSE;

          for (j = 0, lim = dbl_lim;
               j < sizeof(dbl_lim) / sizeof(struct dbl_limits);
               j++, lim++)
            {
                if ((vd.mantissa4 == lim->vaxx.mantissa4) &&
                    (vd.mantissa3 == lim->vaxx.mantissa3) &&
                    (vd.mantissa2 == lim->vaxx.mantissa2) &&
                    (vd.mantissa1 == lim->vaxx.mantissa1) &&
                    (vd.exp == lim->vaxx.exp))
                  {
                      id = lim->ieee;
                      found = TRUE;
                      break;
                  }
            }

          if (!found)
            {
                id.exp = vd.exp - VAX_DBL_BIAS + IEEE_DBL_BIAS;
                id.mantissa1 = (vd.mantissa1 << 13) | (vd.mantissa2 >> 3);
                id.mantissa2 = ((vd.mantissa2 & MASK(3)) << 29) |
                    (vd.mantissa3 << 13) |
                    ((vd.mantissa4 >> 3) & MASK(13));
            }

          id.sign = vd.sign;

          HDmemcpy(dest, ((uint8 *) &(id)) + 4, 4);     /* swap the two 4-byte words */
          HDmemcpy(&dest[4], &(id), 4);

#endif /* __ALPHA */

          source += source_stride;
          dest += dest_stride;
      }

    return 0;
}

#else

int         vms_dummy;          /* prevent empty symbol table messages */

#endif /* VMS */
