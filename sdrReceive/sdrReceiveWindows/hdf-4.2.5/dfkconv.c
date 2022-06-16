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

/* $Id: dfkconv.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*------------------------------------------------------------------
 File:  dfkconv.c

 Purpose:
    Routines to support Convex-Native conversion to and from HDF format

 Invokes:

 PRIVATE conversion functions:
    DFKci4f -  Convex routine for importing 32 bit floats
    DFKco4f -  Convex routine for exporting 32 bit floats
    DFKci8f -  Convex routine for importing 64 bit floats
    DFKco8f -  Convex routine for exporting 64 bit floats
    DFKlci4f-  Convex routine for importing little-endian 32 bit floats
    DFKlco4f-  Convex routine for exporting little-endian 32 bit floats
    DFKlci8f-  Convex routine for importing little-endian 64 bit floats
    DFKlco8f-  Convex routine for exporting little-endian 64 bit floats

 Remarks:
    These files used to be in dfconv.c, but it got a little too huge,
    so I broke them out into seperate files. - Q

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

#ifdef CONVEXNATIVE

/****************************************************************
 * NUMBER CONVERSION ROUTINES FOR CONVEX ARCHITECTURES          *
 * Most of the CONVEX number types can be handled with the      *
 * generic routines.  Only the floating point                   *
 * conversion routines need to be customized.                   *
 * Routines written by J.W. de Bruijn,                          *
 * DELPHI project, Laboratory of Seismics and Acoustics,        *
 * Delft University of Technology.                              *
 * Based on the VAX <-> IEEE routines from NCSA.                *
 ****************************************************************/

/************************************************************/
/* DFKci4f()                                                */
/* --> Import routine for 4 byte CONVEX floats              */
/************************************************************/
int
DFKci4f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    int         in_place = 0;   /* Inplace must be detected */
    uint32 i;
    uint8       buf[4];         /* Inplace processing buffer */
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    char       *FUNC = "DFKci4f";
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

          /* extract exponent */
          exp = (uint8) (source[0] << 1) | (uint8) (source[1] >> 7);
          if (exp)
            {
                /*
                 * non-zero exponent
                 */

                /* copy mantissa, last bit of exponent */
                dest[1] = source[1];
                dest[3] = source[3];
                dest[2] = source[2];
                if (exp < 254)
                    /* normal value */
                    dest[0] = source[0] + (uint8) 1;    /* actually adds two to exp */
                else
                  {
                      /* infinity or NaN */
                      if (exp == 254)   /* unrepresentable - OFL */
                          /* set mant = 0 for overflow */
                          dest[0] = dest[1] = dest[2] = dest[3] = 0;
                      dest[1] &= 0x7f;  /* set last bit of exp to 0 */
                      dest[0] = 0x80;   /* sign=1 exp=0 -> OFL or NaN */
                  }
            }
          else if (source[1] & 0x60)
            {   /* denormalized value */
                int shft;

                shft = (source[1] & 0x40) ? 1 : 2;  /* shift needed to normalize */
                /* shift mantissa */
                /* note last bit of exp set to 1 implicitly */
                dest[1] = (uint8) (source[1] << shft) | (uint8) (source[2] >> (8 - shft));
                dest[2] = (uint8) (source[2] << shft) | (uint8) (source[3] >> (8 - shft));
                dest[3] = (uint8) (source[3] << shft);
                dest[0] = (uint8) (source[0] & 0x80);   /* sign */
                if (shft == 1)
                  {     /* set exp to 2 */
                      dest[0] |= 0x01;
                      dest[1] &= 0x7f;  /* set LSB of exp to 0 */
                  }
            }
          else
              dest[0] = dest[1] = dest[2] = dest[3] = 0;

          source += source_stride;
          dest += dest_stride;
      }

    return 0;

}

/************************************************************/
/* DFKco4f()                                                */
/* --> Export routine for 4 byte CONVEX floats              */
/************************************************************/
int
DFKco4f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    int         in_place = 0;   /* Inplace must be detected */
    uint32 i;
    uint8       buf[4];         /* Inplace processing buffer */
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    char       *FUNC = "DFKco4f";
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

          /* extract exponent */
          exp = (source[0] << 1) | (source[1] >> 7);

          if (!exp && !source[0])
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

                dest[0] = source[0] - (uint8) 1;    /* subtracts 2 from exponent */
                /* copy mantissa, LSB of exponent */
                dest[1] = source[1];
                dest[2] = source[2];
                dest[3] = source[3];

            }
          else if (exp)
            {
                intn shft;
                /*
                 * denormalized number
                 */

                /* keep sign, zero exponent */
                dest[0] = source[0] & 0x80;

                shft = 3 - exp;

                /* shift original mant by 1 or 2 to get denormalized mant */
                /* prefix mantissa with '1'b or '01'b as appropriate */
                dest[1] = (uint8) ((source[1] & 0x7f) >> shft) | (uint8) (0x10 << exp);
                dest[2] = (uint8) (source[1] << (8 - shft)) | (uint8) (source[2] >> shft);
                dest[3] = (uint8) (source[2] << (8 - shft)) | (uint8) (source[3] >> shft);
            }
          else
            {
                /*
                 * sign=1 -> infinity or NaN
                 */

                dest[0] = 0xff;     /* set exp to 255 */
                /* copy mantissa */
                dest[1] = source[1] | (uint8) 0x80;     /* LSB of exp = 1 */
                dest[2] = source[2];
                dest[3] = source[3];
            }

          source += source_stride;
          dest += dest_stride;
      }

    return 0;

}

/************************************************************/
/* DFKci8f()                                                */
/* --> Import routine for 8 byte CONVEX floats              */
/************************************************************/
int
DFKci8f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
  int in_place = 0;                     /* Inplace must be detected */
  uint32 i;            
  uint8 buf[8];                          /* Inplace processing buffer */
  uint8 * source = (uint8*)s;
  uint8 * dest = (uint8*)d;
  char *FUNC="DFKci8f";
  intn exp;

  HEclear();

  if(source == dest)
      in_place = TRUE;

  if(num_elm == 0 || in_place){   /* No elements is an error as is in place. */
    HERROR(DFE_BADCONV);
    return FAIL;
  }

  if(source_stride == 0 && dest_stride == 0)
      source_stride = dest_stride = 8;

  for(i = 0; i < num_elm; i++) {

      /* extract exponent */
      exp = (source[0] << 1) | (source[1] >> 4); 
      if (exp) {
          /* 
           * non-zero exponent 
           */

          /* copy mantissa, sign and first bits of exponent */
          dest[2] = source[2];
          dest[3] = source[3];
          dest[4] = source[4];
          dest[5] = source[5];
          dest[6] = source[6];
          dest[7] = source[7];
          dest[0] = source[0];
          if (exp < 2046) {
              /* normal value */
	      dest[1] = source[1] + 0x20;	/* add two to exp */
	      if (dest[1] < 0x20) dest[0] += 1;	/* carry */
	  }
          else {                              
              /* infinity or NaN */
              if (exp == 2046)                      /* unrepresentable - OFL */
                  /* set mant = 0 for overflow */
                  dest[0] = dest[1] = dest[2] = dest[3] = 0;
              dest[0] = 0x80;               /* sign=1 exp=0 -> OFL or NaN */
              dest[1] &= 0x0f;              /* set last bit of exp to 0 */
          }
      }
      else if (source[1] & 0x0C) {               /* denormalized value */
          int shft;
          
          shft = (source[1] & 0x08) ? 1 : 2;  /* shift needed to normalize */
          /* shift mantissa */
          /* note last bit of exp set to 1 implicitly */
          dest[1] = (uint8)(source[1] << shft) | (uint8)(source[2] >> (8-shft));
          dest[2] = (uint8)(source[2] << shft) | (uint8)(source[3] >> (8-shft));
          dest[3] = (uint8)(source[3] << shft) | (uint8)(source[4] >> (8-shft));
          dest[4] = (uint8)(source[4] << shft) | (uint8)(source[5] >> (8-shft));
          dest[5] = (uint8)(source[5] << shft) | (uint8)(source[6] >> (8-shft));
          dest[6] = (uint8)(source[6] << shft) | (uint8)(source[7] >> (8-shft));
          dest[7] = (uint8)(source[7] << shft);
          dest[0] = (uint8)(source[0] & 0x80);    /* sign */
          if (shft==1) {                          /* set exp to 2 */
            dest[1] |= 0x20;                  /* set LSB of exp to 0 */
          }
      }
      else {
      	dest[0] = dest[1] = dest[2] = dest[3] = 0;
      	dest[4] = dest[5] = dest[6] = dest[7] = 0;
      }
      
      source += source_stride;
      dest   += dest_stride;
  }

    return 0;
}

/************************************************************/
/* DFKco8f()                                                */
/* --> Export routine for 8 byte CONVEX floats              */
/************************************************************/
int
DFKco8f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
  int in_place = 0;                     /* Inplace must be detected */
  uint32 i;            
  uint8 buf[8];                          /* Inplace processing buffer */
  uint8 * source = (uint8*)s;
  uint8 * dest = (uint8*)d;
  char *FUNC="DFKco8f";
  intn  exp;

  HEclear();

  if(source == dest) 
      in_place = TRUE;

  if(num_elm == 0 || in_place){  /* No elements is an error as is in place*/
    HERROR(DFE_BADCONV);
    return FAIL;
  }

  if(source_stride == 0 && dest_stride == 0)
      source_stride = dest_stride = 8;

  for(i = 0; i < num_elm; i++) {
      
      /* extract exponent */
      exp = (source[0] << 1) | (source[1] >> 4); 

      if(!exp && !source[0]) {
          /* 
           * zero value 
           */
          dest[0] = dest[1] = dest[2] = dest[3] = 0;
          dest[4] = dest[5] = dest[6] = dest[7] = 0;
      }
      else if(exp > 2) {
          /*
           * Normal value
           */
          /* copy sign, MSBs of exponent */
          dest[0] = source[0];
	  dest[1] = source[1] - 0x20;	/* subtracts 2 from exponent */
	  if(dest[1]>=0xe0) dest[0]--;  /* borrow from next exp. byte */
          /* copy mantissa */
          dest[2] = source[2];
          dest[3] = source[3];
          dest[4] = source[4];
          dest[5] = source[5];
          dest[6] = source[6];
          dest[7] = source[7];
      }
      else if(exp) {
          intn shft;
          /* 
           * denormalized number 
           */

          /* keep sign, zero exponent */
          dest[0] = source[0] & 0x80;

          shft = 3 - exp;

          /* shift original mant by 1 or 2 to get denormalized mant */
          /* prefix mantissa with '1'b or '01'b as appropriate */
          dest[1] = (uint8)((source[1] & 0x0f) >> shft) | (uint8)(0x02 << exp);
          dest[2] = (uint8)(source[1] << (8-shft)) | (uint8)(source[2] >> shft);
          dest[3] = (uint8)(source[2] << (8-shft)) | (uint8)(source[3] >> shft);  
          dest[4] = (uint8)(source[3] << (8-shft)) | (uint8)(source[4] >> shft);  
          dest[5] = (uint8)(source[4] << (8-shft)) | (uint8)(source[5] >> shft);  
          dest[6] = (uint8)(source[5] << (8-shft)) | (uint8)(source[6] >> shft);  
          dest[7] = (uint8)(source[6] << (8-shft)) | (uint8)(source[7] >> shft);  
      }
      else {
          /* 
           * sign=1 -> infinity or NaN 
           */
          
          dest[0] = 0xff;                /* set exp to 255 */
          /* copy mantissa */
          dest[1] = source[1] | (uint8)0xF0;  /* LSBs of exp = 1 */
          dest[2] = source[2];
          dest[3] = source[3];
          dest[4] = source[4];
          dest[5] = source[5];
          dest[6] = source[6];
          dest[7] = source[7];
      }
      
      source += source_stride;
      dest   += dest_stride;
  }
  
  return 0;

}

/************************************************************/
/* DFKlci4f()                                                */
/* --> Import routine for 4 byte CONVEX floats              */
/************************************************************/
int
DFKlci4f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
  int in_place = 0;                     /* Inplace must be detected */
  uint32 i;            
  uint8 buf[4];                          /* Inplace processing buffer */
  uint8 * source = (uint8*)s;
  uint8 * dest = (uint8*)d;
  char *FUNC="DFKci4f";
  uint8 exp;

  HEclear();

  if(source == dest)
      in_place = TRUE;

  if(num_elm == 0 || in_place){   /* No elements is an error as is in place. */
    HERROR(DFE_BADCONV);
    return FAIL;
  }

  if(source_stride == 0 && dest_stride == 0)
      source_stride = dest_stride = 4;

  for(i = 0; i < num_elm; i++) {

      /* extract exponent */
      exp = (uint8)(source[3] << 1) | (uint8)(source[2] >> 7);
      if (exp) {       
          /* 
           * non-zero exponent 
           */

          /* copy mantissa, last bit of exponent */
          dest[1] = source[2];
          dest[3] = source[0];
          dest[2] = source[1];
          if (exp < 254) 
              /* normal value */
              dest[0] = source[3] + (uint8)1;   /* actually adds two to exp */
          else {                              
              /* infinity or NaN */
              if (exp == 254)                      /* unrepresentable - OFL */
                  /* set mant = 0 for overflow */
                  dest[0] = dest[1] = dest[2] = dest[3] = 0;
              dest[1] &= 0x7f;              /* set last bit of exp to 0 */
              dest[0] = 0x80;               /* sign=1 exp=0 -> OFL or NaN */
          }
      }
      else if (source[2] & 0x60) {               /* denormalized value */
          int shft;
          
          shft = (source[2] & 0x40) ? 1 : 2;  /* shift needed to normalize */
          /* shift mantissa */
          /* note last bit of exp set to 1 implicitly */
          dest[1] = (uint8)(source[2] << shft) | (uint8)(source[1] >> (8-shft));
          dest[2] = (uint8)(source[1] << shft) | (uint8)(source[0] >> (8-shft));
          dest[3] = (uint8)(source[0] << shft);
          dest[0] = (uint8)(source[3] & 0x80);    /* sign */
          if (shft==1) {                          /* set exp to 2 */
            dest[0] |= 0x01;
            dest[1] &= 0x7f;                  /* set LSB of exp to 0 */
          }
      }
      else dest[0] = dest[1] = dest[2] = dest[3] = 0;
      
      source += source_stride;
      dest   += dest_stride;
  }

  return 0;
}

/************************************************************/
/* DFKlco4f()                                                */
/* --> Export routine for 4 byte CONVEX floats              */
/************************************************************/
int
DFKlco4f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
  int in_place = 0;                     /* Inplace must be detected */
  uint32 i;            
  uint8 buf[4];                          /* Inplace processing buffer */
  uint8 * source = (uint8*)s;
  uint8 * dest = (uint8*)d;
  char *FUNC="DFKco4f";
  intn  exp;

  HEclear();

  if(source == dest) 
      in_place = TRUE;

  if(num_elm == 0 || in_place){  /* No elements is an error as is in place*/
    HERROR(DFE_BADCONV);
    return FAIL;
  }

  if(source_stride == 0 && dest_stride == 0)
      source_stride = dest_stride = 4;

  for(i = 0; i < num_elm; i++) {
      
      /* extract exponent */
      exp = (source[0] << 1) | (source[1] >> 7);

      if(!exp && !source[3]) {
          /* 
           * zero value 
           */
          dest[0] = dest[1] = dest[2] = dest[3] = 0;
      }
      else if(exp > 2) {
          /*
           * Normal value
           */

          /* copy mantissa, LSB of exponent */
          dest[0] = source[3];
          dest[1] = source[2];
          dest[2] = source[1];
          dest[3] = source[0] - (uint8)1; /* subtracts 2 from exponent */

      }
      else if(exp) {
          intn shft;
          /* 
           * denormalized number 
           */

          /* keep sign, zero exponent */
          dest[0] = source[3] & 0x80;

          shft = 3 - exp;

          /* shift original mant by 1 or 2 to get denormalized mant */
          /* prefix mantissa with '1'b or '01'b as appropriate */
          dest[1] = (uint8)((source[2] & 0x7f) >> shft) | (uint8)(0x10 << exp);
          dest[2] = (uint8)(source[2] << (8-shft)) | (uint8)(source[1] >> shft);
          dest[3] = (uint8)(source[1] << (8-shft)) | (uint8)(source[0] >> shft);
      }
      else {
          /* 
           * sign=1 -> infinity or NaN 
           */
          
          dest[0] = 0xff;                /* set exp to 255 */
          /* copy mantissa */
          dest[1] = source[2] | (uint8)0x80;  /* LSB of exp = 1 */
          dest[2] = source[1];
          dest[3] = source[0];
      }
      
      source += source_stride;
      dest   += dest_stride;
  }
  
  return 0;

}

/************************************************************/
/* DFKlci8f()                                                */
/* --> Import routine for 8 byte CONVEX floats              */
/************************************************************/
int
DFKlci8f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    int         in_place = 0;   /* Inplace must be detected */
    uint32 i;
    uint8       buf[8];         /* Inplace processing buffer */
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    char       *FUNC = "DFKci8f";
    intn        exp;

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

          /* extract exponent */
          exp = (source[7] << 1) | (source[6] >> 4);
          if (exp)
            {
                /*
                 * non-zero exponent
                 */

                /* copy mantissa, sign and first bits of exponent */
                dest[2] = source[5];
                dest[3] = source[4];
                dest[4] = source[3];
                dest[5] = source[2];
                dest[6] = source[1];
                dest[7] = source[0];
                dest[0] = source[7];
                if (exp < 2046)
                  {
                      /* normal value */
                      dest[1] = source[6] + 0x20;   /* add two to exp */
                      if (dest[1] < 0x20)
                          dest[0] += 1;     /* carry */
                  }
                else
                  {
                      /* infinity or NaN */
                      if (exp == 2046)  /* unrepresentable - OFL */
                          /* set mant = 0 for overflow */
                          dest[0] = dest[1] = dest[2] = dest[3] = 0;
                      dest[0] = 0x80;   /* sign=1 exp=0 -> OFL or NaN */
                      dest[1] &= 0x0f;  /* set last bit of exp to 0 */
                  }
            }
          else if (source[6] & 0x0C)
            {   /* denormalized value */
                int shft;

                shft = (source[6] & 0x08) ? 1 : 2;  /* shift needed to normalize */
                /* shift mantissa */
                /* note last bit of exp set to 1 implicitly */
                dest[1] = (uint8) (source[6] << shft) | (uint8) (source[5] >> (8 - shft));
                dest[2] = (uint8) (source[5] << shft) | (uint8) (source[4] >> (8 - shft));
                dest[3] = (uint8) (source[4] << shft) | (uint8) (source[3] >> (8 - shft));
                dest[4] = (uint8) (source[3] << shft) | (uint8) (source[2] >> (8 - shft));
                dest[5] = (uint8) (source[2] << shft) | (uint8) (source[1] >> (8 - shft));
                dest[6] = (uint8) (source[1] << shft) | (uint8) (source[0] >> (8 - shft));
                dest[7] = (uint8) (source[0] << shft);
                dest[0] = (uint8) (source[7] & 0x80);   /* sign */
                if (shft == 1)
                  {     /* set exp to 2 */
                      dest[1] |= 0x20;  /* set LSB of exp to 0 */
                  }
            }
          else
            {
                dest[0] = dest[1] = dest[2] = dest[3] = 0;
                dest[4] = dest[5] = dest[6] = dest[7] = 0;
            }

          source += source_stride;
          dest += dest_stride;
      }

    return 0;

}

/************************************************************/
/* DFKlco8f()                                                */
/* --> Export routine for 8 byte CONVEX floats              */
/************************************************************/
int
DFKlco8f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
  int in_place = 0;                     /* Inplace must be detected */
  uint32 i;            
  uint8 buf[8];                          /* Inplace processing buffer */
  uint8 * source = (uint8*)s;
  uint8 * dest = (uint8*)d;
  char *FUNC="DFKco8f";
  intn  exp;

  HEclear();

  if(source == dest) 
      in_place = TRUE;

  if(num_elm == 0 || in_place){  /* No elements is an error as is in place*/
    HERROR(DFE_BADCONV);
    return FAIL;
  }

  if(source_stride == 0 && dest_stride == 0)
      source_stride = dest_stride = 8;

  for(i = 0; i < num_elm; i++) {
      
      /* extract exponent */
      exp = (source[0] << 1) | (source[1] >> 4);

      if(!exp && !source[7]) {
          /* 
           * zero value 
           */
          dest[0] = dest[1] = dest[2] = dest[3] = 0;
          dest[4] = dest[5] = dest[6] = dest[7] = 0;
      }
      else if(exp > 2) {
          /*
           * Normal value
           */
          /* copy mantissa */
          dest[0] = source[7];
          dest[1] = source[6];
          dest[2] = source[5];
          dest[3] = source[4];
          dest[4] = source[3];
          dest[5] = source[2];
          /* copy sign, MSBs of exponent */
          dest[6] = source[1] - 0x20;   /* subtracts 2 from exponent */
          dest[7] = source[0];
	  if(dest[6]>=0xe0) dest[7]--;  /* borrow from next exp. byte */
      }
      else if(exp) {
          intn shft;
          /* 
           * denormalized number 
           */

          /* keep sign, zero exponent */
          dest[0] = source[7] & 0x80;

          shft = 3 - exp;

          /* shift original mant by 1 or 2 to get denormalized mant */
          /* prefix mantissa with '1'b or '01'b as appropriate */
          dest[1] = (uint8)((source[6] & 0x0f) >> shft) | (uint8)(0x02 << exp);
          dest[2] = (uint8)(source[6] << (8-shft)) | (uint8)(source[5] >> shft);
          dest[3] = (uint8)(source[5] << (8-shft)) | (uint8)(source[4] >> shft);
          dest[4] = (uint8)(source[4] << (8-shft)) | (uint8)(source[3] >> shft);
          dest[5] = (uint8)(source[3] << (8-shft)) | (uint8)(source[2] >> shft);
          dest[6] = (uint8)(source[2] << (8-shft)) | (uint8)(source[1] >> shft);
          dest[7] = (uint8)(source[1] << (8-shft)) | (uint8)(source[0] >> shft);
      }
      else {
          /* 
           * sign=1 -> infinity or NaN 
           */
          
          dest[0] = 0xff;                /* set exp to 255 */
          /* copy mantissa */
          dest[1] = source[6] | (uint8)0xF0;  /* LSBs of exp = 1 */
          dest[2] = source[5];
          dest[3] = source[4];
          dest[4] = source[3];
          dest[5] = source[2];
          dest[6] = source[1];
          dest[7] = source[0];
      }
      
      source += source_stride;
      dest   += dest_stride;
  }
  
  return 0;

}

#else

int         convex_dummy;       /* prevent empty symbol table messages */

#endif /* CONVEXNATIVE */
