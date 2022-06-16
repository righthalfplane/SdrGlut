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

/* $Id: dfkfuji.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*------------------------------------------------------------------
 File:  dfkfuji.c

 Purpose:
    Routines to support Fujitsu-Native (VP) conversion to and from HDF format

 Invokes:

 PRIVATE conversion functions:
    DFKpi4f -  VP routine for importing 32 bit floats
    DFKpo4f -  VP routine for exporting 32 bit floats
    DFKpi8f -  VP routine for importing 64 bit floats
    DFKpo8f -  VP routine for exporting 64 bit floats
    DFKlpi4f - VP routine for importing little-endian 32 bit floats
    DFKlpo4f - VP routine for exporting little-endian 32 bit floats
    DFKlpi8f - VP routine for importing little-endian 64 bit floats
    DFKlpo8f - VP routine for exporting little-endian 64 bit floats

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

#if defined(VP)

/*
 * NUMBER CONVERSION ROUTINES FOR Fujitsu VP series
 * (IBM floating point, BIG-ENDIAN integer types, ASCII
 *  chars...)
 *
 * Drew Whitehouse,
 * Australian National University Supercomputer Facility.
 * Dec 92.
 *
 * Routines used were adapted from routines provided for
 * anon. ftp by the Cornell National Supercomputer
 * Facility. The following copyright was attached.....
 *
 */

/*---------------------------------------------------------------------*
 * Val I. Garger, Technology Integration                               *
 * Group, CNSF, Cornell University                                     *
 * vig@eagle.cnsf.cornell.edu                                          *
 *-------------------------------------------------------------------- *
 *                                                                     *
 *  COPYRIGHT -  VAL GARGER, CORNELL NATIONAL SUPERCOMPUTER FACILITY,  *
 *               (JUNE 1990) CORNELL UNIVERSITY, ITHACA, NY.           *
 *               CONTAINS RESTRICTED MATERIALS OF CORNELL UNIVERSITY,  *
 *               (C) COPYRIGHT CORNELL UNIVERSITY 1990                 *
 *---------------------------------------------------------------------*/

/************************************************************/
/* DFKpi4f()                                                */
/* --> Import routine for 4 byte IBM floats                 */
/* ie IEEE32->IBM32                                         */
/************************************************************/

#define pi4f_last 0x000000ff
#define pi4f_impl 0x00800000
#define pi4f_sign 0x80000000
#define pi4f_tiss 0x007fffff

int
DFKpi4f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
  intn i,k;
  int32 ibs, ibe, ibt;

  uint8 * source = (uint8*)s;
  uint8 * dest = (uint8*)d;
  uint32 *buf;
  char *FUNC="DFKpi4f";

  HEclear();

  if(num_elm == 0)   /* No elements is an error */
    HRETURN_ERROR(DFE_BADCONV,FAIL);

  if(source_stride == 0 && dest_stride == 0)
      source_stride = dest_stride = 4;

  for(i = 0; i < num_elm; i++) {

    buf = (uint32*)source;
    ibt =   *buf;
    ibs =   *buf          &  pi4f_sign;
    ibe  = ( *buf >> 23 )  &  pi4f_last;

    if( ibe != 0 )  {
      if( ibe == 255 ) { 
	ibe = 378;
	ibt = pi4f_tiss;
      } 
      ibe = ibe - 127 + 256 +1;
      k = ibe%4;
      ibe = ibe/4;
      if( k != 0 ) 
	ibe = ibe + 1;
      ibe = ibe << 24 ;
      ibt = (ibt & pi4f_tiss) | pi4f_impl ;
      if (k != 0)
	ibt = ( ibt + (1 << (3-k) )  ) >> (4-k);
    }

    buf = (uint32*)dest;
    *buf = ibs | ibe | ibt;                            
    source += source_stride;
    dest   += dest_stride;
  }
  return 0;
}

/************************************************************/
/* DFKpo4f()                                                */
/* --> Export routine for 4 byte IBM floats                 */
/* IBM->IEEE                                                */
/************************************************************/

#define   po4f_exp   0x7F000000
#define   po4f_sign  0x80000000
#define   po4f_tis   0x00FFFFFF
#define   po4f_etis  0x007FFFFF

int
DFKpo4f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
  intn i, k;
  int32 ibs, ibe, ibt, it;

  uint8 * source = (uint8*)s;
  uint8 * dest = (uint8*)d;
  char *FUNC="DFKpi4f";
  uint32 *buf;
  uint8 exp;

  HEclear();

  if(num_elm == 0 )		/* No elements is an error */
    HRETURN_ERROR(DFE_BADCONV,FAIL);

  if(source_stride == 0 && dest_stride == 0)
    source_stride = dest_stride = 4;

  for(i = 0; i < num_elm; i++) {

    buf = (uint32*)source;
    ibs = *buf  & po4f_sign;
    ibe = *buf  & po4f_exp;
    ibt = *buf  & po4f_tis;
    it = ibt << 8;

    for(k = 0;  (k < 5) && (it >= 0);  k++ ) 
      it = it << 1;

    if( k < 4 ) {
      ibt = (it >> 8) & po4f_etis;
      ibe = (ibe >> 22) - 256 + 127 - k - 1;
      if( ibe < 0 )
	ibe = ibt = 0;
      if( ibe >= 255 ) {
	ibe = 255;                                                
	ibt = 0;                                                 
      }                                                              
      ibe = ibe << 23;
    }

    /* put result into dest */
    buf = (uint32*)dest;
    *buf = ibs | ibe | ibt;                            
    source += source_stride;
    dest   += dest_stride;
  }
  return 0;

}

/************************************************************/
/* DFKpi8f()                                                */
/* --> Import routine for 8 byte VP floats                  */
/* ie IEEE64->IBM64                                         */
/************************************************************/

#define pi8f_impl 0x00100000
#define pi8f_sign 0x80000000
#define pi8f_maxl 0x00ffffff
#define pi8f_expn 0x7ff00000
#define pi8f_tiss 0x000fffff
#define pi8f_maxr 0xffffffff

int
DFKpi8f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
  uint8 * source = (uint8*)s;
  uint8 * dest   = (uint8*)d;
  char *FUNC="DFKpi8f"; 
  intn i,k;
  uint32 ibs,ibe,ibt1,ibt2,isht;
  uint32 *buf;

  HEclear();

  if(num_elm == 0) {		/* No elements is an error */ 
    HERROR(DFE_BADCONV);
    return FAIL;
  }

  if(source_stride == 0 && dest_stride == 0)
    source_stride = dest_stride = 8;

  for ( i = 0; i < num_elm ; i++) {

    buf  = (uint32*)source;
    ibs  = *buf & pi8f_sign;
    ibe  = *buf & pi8f_expn;
    ibt1 = *buf & pi8f_tiss;
    ibt2 = *(buf+1);

    if (ibe != 0) {

      ibe = (ibe >> 20) - 1023 + 256 + 1;
      k = 0;                                                                    
      if (ibe > 508) k = 2;                                                     
      if (ibe <   0) k = 1; 

      switch (k) {

      case 1:
	ibe = ibt1 = ibt2 = 0;
	break;

      case 2:
	ibe = 127;
	ibt1 = pi8f_maxl;
	ibt2 = pi8f_maxr;
	break;

      default:
	isht = ibe % 4 -  1;
	ibe = ibe >> 2 ; 
	if ( isht != -1)
	  ibe += 1;
	else
	  isht = 3;

	ibt1 |=  pi8f_impl;
	if (isht != 0 ) {
	  ibt1 = ( ibt1 << isht ) |  (ibt2 >> 32-isht);
	  ibt2 = ibt2 << isht;
	}
	break;
      }
      ibe = ibe << 24;

    } /* ibe != 0 */

    /* put number into destination array */
    buf  = (uint32*)dest;

    *buf = ibs | ibe | ibt1;
    *(buf+1) = ibt2;

    source += source_stride;
    dest   += dest_stride;
  }
  return 0;
}

/************************************************************/
/* DFKpo8f()                                                */
/* --> Export routine for 8 byte VP floats                  */
/* IBM64->IEEE64                                            */
/************************************************************/
#define  po8f_exp   0x7f000000
#define  po8f_sign  0x80000000
#define  po8f_tiss  0x00FFFFFF
#define  po8f_nrm   0x00f00000
static uint32 take[4] = {0x007fffff, 0x003fffff, 0x001fffff, 0x000fffff};
static uint32 look[4] = {0x00800000, 0x00400000, 0x00200000, 0x00100000};
int
DFKpo8f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
  uint8 * source = (uint8*)s;
  uint8 * dest = (uint8*)d;
  uint32 ibs, ibe, ibt1, ibt2, it, isht;
  intn i,k;
  uint32 *buf;
  double *tmp;
  char *FUNC="DFKpo8f";
  union {
    struct { 
      unsigned long i1;
      unsigned long i2;
    } sti;
    struct { 
      double r;
    } str;
  } un;

  HEclear();

  if(num_elm == 0)		/* No elements is an error */
    HRETURN_ERROR(DFE_BADCONV,FAIL);

  if(source_stride == 0 && dest_stride == 0)
    source_stride = dest_stride = 8;

  for (i = 0; i < num_elm; i++) {

    buf = (uint32*)source;
    ibs  = *buf & po8f_sign;
    ibe  = *buf & po8f_exp;
    ibt1 = *buf & po8f_tiss;
    ibt2 = *(buf+1);

    if  ( (ibt1 == 0) && (ibt2 == 0) )                                          
      ibe = 0 ;                                                                
    else {

      if ( (ibe != 0) && ( (ibt1 & po8f_nrm) == 0 ) ) {
	un.sti.i1 = *buf;
	un.sti.i2 = *(buf+1) ;
	un.str.r  = un.str.r + 0e0 ;
	ibe  = un.sti.i1 & po8f_exp ;
	ibt1 = un.sti.i1 & po8f_tiss ;
	ibt2 = un.sti.i2 ;
      }

      if      ((ibt1 & look[0]) != 0 )
	{ k = 3 ; ibt1 =  ibt1 & take[0] ; }
      else if ((ibt1 & look[1]) != 0 )
	{ k = 2 ; ibt1 =  ibt1 & take[1] ; }
      else if ((ibt1 & look[2]) != 0 )
	{ k = 1 ; ibt1 =  ibt1 & take[2] ; }
      else if ((ibt1 & look[3]) != 0 )
	{ k = 0 ;  ibt1 =  ibt1 & take[3] ; }
      else k = 4;

      /* mantissa */
      if  (k < 4) {
	if (k != 0) {
	  /* shift with rounding */
	    it   = (ibt2 & 65535) + (1 << (k-1)) ;
	    ibt2 = ( (ibt2 >> 16) & 65535 )  +  (it >> 16) ;
	    ibt1 = ibt1 + (ibt2 >> 16) ;
	    /* rounded */
	    ibt2 = ( (ibt2 & 65535) << (16-k) ) | ( (it & 65535) >> k) ;
	    it   = ibt1 << (32 - k) ;
	    ibt1 = ibt1 >> k ;
	    ibt2 = ibt2 | it;
	  }                                                                      

	/*  exponent */                                                       
	ibe = ( (ibe >> 22)  - 256 + 1023 + k - 4 ) << 20 ;                    
      } 
    }
    /* put number into destination array */
    buf = (uint32*)dest;
    *buf     = ibs | ibe | ibt1;
    *(buf+1) = ibt2;
    source += source_stride;
    dest   += dest_stride;
  }
  return 0;
}

/************************************************************/
/* DFKlpi4f()                                               */
/* --> Import routine for 4 byte IBM floats                 */
/* ie little-endian IEEE32->IBM32                           */
/************************************************************/

#define lpi4f_last  0x000000ff
#define lpi4f_impl  0x00800000
#define lpi4f_sign  0x80000000
#define lpi4f_tiss  0x007fffff
#define LPI4F_MASKA 0xff00ff00
#define LPI4F_MASKB 0x00ff00ff
#define LPI4F_MASKC 0xffff0000
#define LPI4F_MASKD 0x0000ffff

int
DFKlpi4f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    intn i,k;
    int32 ibs, ibe, ibt;
    uint8 * source = (uint8*)s;
    uint8 * dest = (uint8*)d;
    uint32 *buf;
    uint32 temp;
    char *FUNC="DFKpi4f";

    HEclear();

    if(num_elm == 0)   /* No elements is an error */
        HRETURN_ERROR(DFE_BADCONV,FAIL);

    if (source_stride == 0 && dest_stride == 0)
        source_stride = dest_stride = 4;

    for (i = 0; i < num_elm; i++)
      {

          buf = (uint32 *) source;
          temp = ((*buf & LPI4F_MASKA) >> 8) | ((*buf & LPI4F_MASKB) << 8);
          temp = ((temp & LPI4F_MASKC) >> 16) | ((temp & LPI4F_MASKD) << 16);

          ibt = temp;
          ibs = temp & lpi4f_sign;
          ibe = (temp >> 23) & lpi4f_last;

          if (ibe != 0)
            {
                if (ibe == 255)
                  {
                      ibe = 378;
                      ibt = lpi4f_tiss;
                  }     /* end if */
                ibe = ibe - 127 + 256 + 1;
                k = ibe % 4;
                ibe = ibe / 4;
                if (k != 0)
                    ibe = ibe + 1;
                ibe = ibe << 24;
                ibt = (ibt & lpi4f_tiss) | lpi4f_impl;
                if (k != 0)
                    ibt = (ibt + (1 << (3 - k))) >> (4 - k);
            }   /* end if */

          buf = (uint32 *) dest;
          *buf = ibs | ibe | ibt;
          source += source_stride;
          dest += dest_stride;
      }     /* end for */
    return 0;
}

/************************************************************/
/* DFKlpo4f()                                               */
/* --> Export routine for 4 byte IBM floats                 */
/* IBM->little-endian IEEE                                  */
/************************************************************/

#define lpo4f_exp   0x7F000000
#define lpo4f_sign  0x80000000
#define lpo4f_tis   0x00FFFFFF
#define lpo4f_etis  0x007FFFFF
#define LPO4F_MASKA 0xff00ff00
#define LPO4F_MASKB 0x00ff00ff
#define LPO4F_MASKC 0xffff0000
#define LPO4F_MASKD 0x0000ffff

int
DFKlpo4f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    intn i, k;
    int32 ibs, ibe, ibt, it;
    uint8 * source = (uint8*)s;
    uint8 * dest = (uint8*)d;
    char *FUNC="DFKpi4f";
    uint32 *buf;
    uint32 temp;
    uint8 exp;

    HEclear();

    if(num_elm == 0 )        /* No elements is an error */
        HRETURN_ERROR(DFE_BADCONV,FAIL);

    if (source_stride == 0 && dest_stride == 0)
        source_stride = dest_stride = 4;

    for (i = 0; i < num_elm; i++)
      {

          buf = (uint32 *) source;
          ibs = *buf & lpo4f_sign;
          ibe = *buf & lpo4f_exp;
          ibt = *buf & lpo4f_tis;
          it = ibt << 8;

          for (k = 0; (k < 5) && (it >= 0); k++)
              it = it << 1;

          if (k < 4)
            {
                ibt = (it >> 8) & lpo4f_etis;
                ibe = (ibe >> 22) - 256 + 127 - k - 1;
                if (ibe < 0)
                    ibe = ibt = 0;
                if (ibe >= 255)
                  {
                      ibe = 255;
                      ibt = 0;
                  }     /* end if */
                ibe = ibe << 23;
            }   /* end if */

          /* put result into dest */
          buf = (uint32 *) dest;
          temp = ibs | ibe | ibt;
          temp = ((temp & LPO4F_MASKA) >> 8) | ((temp & LPO4F_MASKB) << 8);
          temp = ((temp & LPO4F_MASKC) >> 16) | ((temp & LPO4F_MASKD) << 16);
          *buf = temp;
          source += source_stride;
          dest += dest_stride;
      }     /* end for */
    return 0;
}

/************************************************************/
/* DFKlpi8f()                                               */
/* --> Import routine for 8 byte VP floats                  */
/* ie little-endian IEEE64->IBM64                           */
/************************************************************/

#define lpi8f_impl 0x00100000
#define lpi8f_sign 0x80000000
#define lpi8f_maxl 0x00ffffff
#define lpi8f_expn 0x7ff00000
#define lpi8f_tiss 0x000fffff
#define lpi8f_maxr 0xffffffff
#define LPI8F_MASKA 0xff00ff00
#define LPI8F_MASKB 0x00ff00ff
#define LPI8F_MASKC 0xffff0000
#define LPI8F_MASKD 0x0000ffff

int
DFKlpi8f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    uint8 * source = (uint8*)s;
    uint8 * dest   = (uint8*)d;
    char *FUNC="DFKpi8f";
    intn i,k;
    uint32 ibs,ibe,ibt1,ibt2,isht;
    uint32 *buf;
    uint32 temp;

    HEclear();

    if(num_elm == 0)         /* No elements is an error */
        HRETURN_ERROR(DFE_BADCONV,FAIL);

    if (source_stride == 0 && dest_stride == 0)
        source_stride = dest_stride = 8;

    for (i = 0; i < num_elm; i++)
      {
        buf  = (uint32*)source; /* be careful of 32-bit word swapping below */
        temp = ((*(buf+1) & LPI8F_MASKA)>>8) | ((*(buf+1) & LPI8F_MASKB)<<8);
        temp = ((temp & LPI8F_MASKC)>>16) | ((temp & LPI8F_MASKD)<<16);
        ibs  = temp & lpi8f_sign;
        ibe  = temp & lpi8f_expn;
        ibt1 = temp & lpi8f_tiss;

        temp = ((*buf & LPI8F_MASKA)>>8) | ((*buf & LPI8F_MASKB)<<8);
        temp = ((temp & LPI8F_MASKC)>>16) | ((temp & LPI8F_MASKD)<<16);
        ibt2 = temp;

          if (ibe != 0)
            {

                ibe = (ibe >> 20) - 1023 + 256 + 1;
                k = 0;
                if (ibe > 508)
                    k = 2;
                if (ibe < 0)
                    k = 1;

                switch (k)
                  {

                      case 1:
                          ibe = ibt1 = ibt2 = 0;
                          break;

                      case 2:
                          ibe = 127;
                          ibt1 = lpi8f_maxl;
                          ibt2 = lpi8f_maxr;
                          break;

                      default:
                          isht = ibe % 4 - 1;
                          ibe = ibe >> 2;
                          if (isht != -1)
                              ibe += 1;
                          else
                              isht = 3;

                          ibt1 |= lpi8f_impl;
                          if (isht != 0)
                            {
                                ibt1 = (ibt1 << isht) | (ibt2 >> 32 - isht);
                                ibt2 = ibt2 << isht;
                            }   /* end if */
                          break;
                  }     /* end switch */
                ibe = ibe << 24;
            }   /* ibe != 0 */

          /* put number into destination array */
          buf = (uint32 *) dest;
          *buf = ibs | ibe | ibt1;
          *(buf + 1) = ibt2;

          source += source_stride;
          dest += dest_stride;
      }     /* end for */
    return 0;
}

/************************************************************/
/* DFKlpo8f()                                               */
/* --> Export routine for 8 byte VP floats                  */
/* IBM64->little-endian IEEE64                              */
/************************************************************/

#define lpo8f_exp   0x7f000000
#define lpo8f_sign  0x80000000
#define lpo8f_tiss  0x00FFFFFF
#define lpo8f_nrm   0x00f00000
#define LPO8F_MASKA 0xff00ff00
#define LPO8F_MASKB 0x00ff00ff
#define LPO8F_MASKC 0xffff0000
#define LPO8F_MASKD 0x0000ffff

static int32 ltake[4] =
{0x007fffff, 0x003fffff, 0x001fffff, 0x000fffff};
static int32 llook[4] =
{0x00800000, 0x00400000, 0x00200000, 0x00100000};

int
DFKlpo8f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    uint8 * source = (uint8*)s;
    uint8 * dest = (uint8*)d;
    uint32 ibs, ibe, ibt1, ibt2, it, isht;
    intn i,k;
    uint32 *buf;
    uint32 temp;
    double *tmp;
    char *FUNC="DFKpo8f";
    union {
        struct {
          unsigned long i1;
          unsigned long i2;
        } sti;
        struct {
          double r;
        } str;
    } un;

    HEclear();

    if(num_elm == 0)      /* No elements is an error */
        HRETURN_ERROR(DFE_BADCONV,FAIL);

    if (source_stride == 0 && dest_stride == 0)
        source_stride = dest_stride = 8;

    for (i = 0; i < num_elm; i++)
      {

        buf = (uint32*)source;
        ibs  = *buf & lpo8f_sign;
        ibe  = *buf & lpo8f_exp;
        ibt1 = *buf & lpo8f_tiss;
        ibt2 = *(buf+1);

        if  ( (ibt1 == 0) && (ibt2 == 0) )
            ibe = 0 ;
        else {

            if ( (ibe != 0) && ( (ibt1 & lpo8f_nrm) == 0 ) ) {
                un.sti.i1 = *buf;
                un.sti.i2 = *(buf+1) ;
                un.str.r  = un.str.r + 0e0 ;
                ibe  = un.sti.i1 & lpo8f_exp ;
                ibt1 = un.sti.i1 & lpo8f_tiss ;
                ibt2 = un.sti.i2 ;
              } /* end if */

            if((ibt1 & llook[0]) != 0 ) {
                k = 3 ;
                ibt1 =  ibt1 & ltake[0] ;
              } /* end if */
            else if((ibt1 & llook[1]) != 0 ) {
                k = 2 ;
                ibt1 =  ibt1 & ltake[1] ;
              } /* end if */
            else if((ibt1 & llook[2]) != 0 ) {
                k = 1 ;
                ibt1 =  ibt1 & ltake[2] ;
              } /* end if */
            else if((ibt1 & llook[3]) != 0 ) {
                k = 0 ;
                ibt1 =  ibt1 & ltake[3] ;
              } /* end if */
            else
                k = 4;

            /* mantissa */
            if  (k < 4) {
                if (k != 0) {   /* shift with rounding */
                    it   = (ibt2 & 65535) + (1 << (k-1)) ;
                    ibt2 = ( (ibt2 >> 16) & 65535 )  +  (it >> 16) ;
                    ibt1 = ibt1 + (ibt2 >> 16) ;
                    /* rounded */
                    ibt2 = ( (ibt2 & 65535) << (16-k) ) | ( (it & 65535) >> k) ;
                    it   = ibt1 << (32 - k) ;
                    ibt1 = ibt1 >> k ;
                    ibt2 = ibt2 | it;
                  } /* end if */

                /*  exponent */
                ibe = ( (ibe >> 22)  - 256 + 1023 + k - 4 ) << 20 ;
              } /* end if */
          } /* end else */
        /* put number into destination array */
        buf = (uint32*)dest;
          temp = ibs | ibe | ibt1;
          temp = ((temp & LPO8F_MASKA) >> 8) | ((temp & LPO8F_MASKB) << 8);
          temp = ((temp & LPO8F_MASKC) >> 16) | ((temp & LPO8F_MASKD) << 16);
          *(buf + 1) = temp;    /* make certain to swap this also! */
          temp = ibt2;
          temp = ((temp & LPO8F_MASKA) >> 8) | ((temp & LPO8F_MASKB) << 8);
          temp = ((temp & LPO8F_MASKC) >> 16) | ((temp & LPO8F_MASKD) << 16);
          *buf = temp;
          source += source_stride;
          dest += dest_stride;
      }     /* end for */
    return 0;
}

#else

int         fuji_dummy;         /* prevent empty symbol table messages */

#endif /* VP */
