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

/* $Id: dfkcray.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*------------------------------------------------------------------
 File:  dfkcray.c

 Purpose:
    Routines to support Cray conversion to and from HDF format

 Invokes:

 PRIVATE conversion functions:
    Cray UNICOS
    DFKui2i -  Unicos routine for importing 16 bit unsigned integers
    DFKui2s -  Unicos routine for importing 16 bit signed integers
    DFKuo2i -  Unicos routine for exporting 16 bit unsigned integers
    DFKuo2s -  Unicos routine for exporting 16 bit signed integers
    DFKui4i -  Unicos routine for importing 32 bit unsigned integers
    DFKui4s -  Unicos routine for importing 32 bit signed integers
    DFKuo4i -  Unicos routine for exporting 32 bit unsigned integers
    DFKuo4s -  Unicos routine for exporting 32 bit signed integers
    DFKui4f -  Unicos routine for importing 32 bit floats
    DFKuo4f -  Unicos routine for exporting 32 bit floats
    DFKui8f -  Unicos routine for importing 64 bit floats
    DFKuo8f -  Unicos routine for exporting 64 bit floats
    DFKlui2i-  Unicos routine for importing little-endian 16 bit unsigned ints
    DFKlui2s-  Unicos routine for importing little-endian 16 bit signed ints
    DFKluo2i-  Unicos routine for exporting little-endian 16 bit unsigned ints
    DFKluo2s-  Unicos routine for exporting little-endian 16 bit signed ints
    DFKlui4i-  Unicos routine for importing little-endian 32 bit unsigned ints
    DFKlui4s-  Unicos routine for importing little-endian 32 bit signed ints
    DFKluo4i-  Unicos routine for exporting little-endian 32 bit unsigned ints
    DFKluo4s-  Unicos routine for exporting little-endian 32 bit signed ints
    DFKlui4f-  Unicos routine for importing little-endian 32 bit floats
    DFKluo4f-  Unicos routine for exporting little-endian 32 bit floats
    DFKlui8f-  Unicos routine for importing little-endian 64 bit floats
    DFKluo8f-  Unicos routine for exporting little-endian 64 bit floats

    Cray MPP (T3D)
    DFKmi2i -  CRAYMPP routine for importing 16 bit unsigned integers
    DFKmi2s -  CRAYMPP routine for importing 16 bit signed integers
    DFKmo2i -  CRAYMPP routine for exporting 16 bit unsigned integers
    DFKmo2s -  CRAYMPP routine for exporting 16 bit signed integers
    DFKlmi2i-  CRAYMPP routine for importing little-endian 16 bit unsigned ints
    DFKlmi2s-  CRAYMPP routine for importing little-endian 16 bit signed ints
    DFKlmo2i-  CRAYMPP routine for exporting little-endian 16 bit unsigned ints
    DFKlmo2s-  CRAYMPP routine for exporting little-endian 16 bit signed ints

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

/*****************************************************************************/
/* NUMBER CONVERSION ROUTINES FOR THE UNICOS OPERATING SYSTEM                */
/* Parameter dest_stride is used because source buffer and dest buffer will  */
/* be different sizes for all data types except char.                        */
/*****************************************************************************/

#if defined(UNICOS)

#define BITOFF 58
#define NOINTCRAY2IEG
/* #define NOFLOATCRAY2IEG */

/* When on a Cray T90, if _CRAYIEEE is not defined, floating point numbers    */
/* are in CRAY format.  Cray Research supplies the routines IEG2CRAY() and    */
/* CRAY2IEG() to convert numbers from 32-bit IEEE format to CRAY format.      */
/* These routines are used throughout the local conversion routines.          */

/* If _CRAYIEEE is defined, Cray floating point numbers are in IEEE format.   */
/* Cray Research no longer supplies the routines IEG2CRAY/CRAY2IEG.  Instead, */
/* these function calls are replaced by the macro calls to IEG2CRAY/CRAY2IEG2 */
/* defined below.  The macro call replaces the Cray format conversion routine */
/* call with those supplied by Cray Research IEG2CRI/CRI2IEG for the T90-IEEE */
/* format.                                                                    */

/* On the Cray T90 there are man-pages availabe for IEG2CRAY/CRAY2IEG, and on */
/* the Cray T90-IEEE there are man-pages available for IEG2CRI/CRI2IEG.       */

/* Steven G. Johnson(stevenj@alum.mit.edu) of the Joannopoulous Group,        */
/* Condensed Matter Theory, MIT Physics Department gets credit for the effort */
/* spent and ideas enclosed within the #ifdef _CRAYIEEE block of code.        */
/* A few comments were added to futher describe local situations.             */

#ifdef _CRAYIEEE /* IEEE T90 */

#undef DUFF_luo4i  /* there is a weird Heisenbug in this code */

/* Currently, CRI2IEG has a bug when converting 64-bit ints into 16-bit ints. */
#define BUGGY_CRI2IEG

/* This function get_correct_cri_type() is used to translate from parameters  */
/* used in CRAY2IEG/IEG2CRAY conversion routines to parameters used in new    */
/* CRI2IEG/IEG2CRI conversion routines.                                       */

static void get_correct_cri_type(int *type, int *newtype, int *forlen)
{
	switch (*type) {
		case 1:
			*newtype = 2;
			*forlen = 4 * 8;
			break;
		case 2:
			*newtype = 3;
			*forlen = 4 * 8;
			break;
		case 7:
			*newtype = 2;
			*forlen = 2 * 8;
			break;
		case 8:
			*newtype = 3;
			*forlen = 8 * 8;
			break;
		default:
			*newtype = -1;
			*forlen = -1;
			break;
	}
}

/* The macros have a zero value set after the last parenthesis enclosing the  */
/* list of arguments.  Since the macros are taking the place of functions of  */
/* the form ierr = IEG2CRAY(), it is my interpretation that this zero value   */
/* is an initial value for ierr.  ierr is reset to a real value 3 lines later.*/

#define IEG2CRAY(type_p, num_p, for_p, bitoff_p, nat_p) 0; \
{	\
	int cri2ieg_type, natlen = 8*8, forlen, stride = 1; \
	get_correct_cri_type(type_p, &cri2ieg_type, &forlen); \
	ierr = IEG2CRI(&cri2ieg_type, num_p, for_p, bitoff_p, nat_p, &stride, &natlen, &forlen); \
}

#ifndef BUGGY_CRI2IEG

#define CRAY2IEG(type_p, num_p, for_p, bitoff_p, nat_p) 0; \
{	\
	int cri2ieg_type, natlen = 8*8, forlen, stride = 1; \
	get_correct_cri_type(type_p, &cri2ieg_type, &forlen); \
	ierr = CRI2IEG(&cri2ieg_type, num_p, for_p, bitoff_p, nat_p, &stride, &natlen, &forlen); \
}

#else /* BUGGY_CRI2IEG */


/* It was found by Steven G. Johnson that the CRI2IEG() routine has bugs in.  */
/* In particular the part of the routine which converts from 64-bit Cray int  */
/* into 16-bit integer.  So he wrote a routine which would do just that.  It  */
/* is used if the BUGGY_CRI2IEG switch is defined.                            */

static void my_convert_64_to_16(int n, int *source, char *dest)
{
	int i;
	for (i = 0; i < n; ++i) {
		dest[0] = (char) (((*source) >> 8) & 0xFF);
		dest[1] = (char) ((*source) & 0xFF);
		dest += 2;
		++source;
	}
}

#define CRAY2IEG(type_p, num_p, for_p, bitoff_p, nat_p) 0; \
{	\
	int cri2ieg_type, natlen = 8*8, forlen, stride = 1; \
	if (*(type_p) == 7) \
		my_convert_64_to_16(*(num_p),(int*)(nat_p),(char*)(for_p)); \
	else { \
		get_correct_cri_type(type_p, &cri2ieg_type, &forlen); \
		ierr = CRI2IEG(&cri2ieg_type, num_p, for_p, bitoff_p, nat_p, &stride, &natlen, &forlen); \
	} \
}

#endif /* BUGGY_CRI2IEG */

#else /* Not IEEE T90 */

#define NOSTRIDEFLOATCRAY2IEG

#endif /* Not IEEE T90 */


PRIVATE VOID DFKswap
            (VOIDP s, VOIDP d, uintn elem_size, uintn num_elem);

#define SWAP_MASKA  0xffffffff00000000
#define SWAP_MASKB  0x00000000ffffffff
#define SWAP_MASKC  0xffff0000ffff0000
#define SWAP_MASKD  0x0000ffff0000ffff
#define SWAP_MASKE  0xff00ff00ff00ff00
#define SWAP_MASKF  0x00ff00ff00ff00ff

/************************************************************/
/* DFKswap()                                                */
/* -->Swap groups of 'n' bytes                              */
/*  !NOTE!: This routine does not handle overlapping memory */
/*              blocks properly, but that condition should  */
/*              be caught by the converion routines...      */
/*          Also, there in no provision for source or       */
/*          destination strides other than 1.               */
/************************************************************/
PRIVATE     VOID
DFKswap(VOIDP s, VOIDP d, uintn elem_size, uintn num_elem)
{
    uintn i;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    unsigned long *lp_dest, *lp_src;
    intn        odd_man_out;
    uintn       n;
    char       *FUNC = "DFKswap";

    switch (elem_size)
      {
          case 2:   /* 2 byte elements */
              odd_man_out = num_elem % 4;   /* check for odd number of elem. */
              n = num_elem / 4;
              for (i = 0, lp_src = (unsigned long *) source, lp_dest = (unsigned long *) dest; i < n; i++, lp_dest++, lp_src++)
                {
                    *lp_dest = ((*lp_src & SWAP_MASKE) >> 8) |
                        ((*lp_src & SWAP_MASKF) << 8);
                }   /* end for */
              if (odd_man_out)
                {
                    source = (uint8 *) lp_src;
                    dest = (uint8 *) lp_dest;
                    switch (odd_man_out)
                      {
                          case 3:
                              dest[0] = source[1];
                              dest[1] = source[0];
                              dest[2] = source[3];
                              dest[3] = source[2];
                              dest[4] = source[5];
                              dest[5] = source[4];
                              break;

                          case 2:
                              dest[0] = source[1];
                              dest[1] = source[0];
                              dest[2] = source[3];
                              dest[3] = source[2];
                              break;

                          case 1:
                              dest[0] = source[1];
                              dest[1] = source[0];
                              break;

                      }     /* end switch */
                }   /* end if */
              break;

          case 4:   /* 4 byte elements */
              odd_man_out = num_elem % 2;   /* check for odd number of elem. */
              n = num_elem / 2;
              for (i = 0, lp_src = (unsigned long *) source, lp_dest = (unsigned long *) dest; i < n; i++, lp_dest++, lp_src++)
                {
                    *lp_dest = ((*lp_src & SWAP_MASKC) >> 16) |
                        ((*lp_src & SWAP_MASKD) << 16);
                    *lp_dest = ((*lp_dest & SWAP_MASKE) >> 8) |
                        ((*lp_dest & SWAP_MASKF) << 8);
                }   /* end for */
              if (odd_man_out)
                {
                    source = (uint8 *) lp_src;
                    dest = (uint8 *) lp_dest;
                    dest[0] = source[3];
                    dest[1] = source[2];
                    dest[2] = source[1];
                    dest[3] = source[0];
                }   /* end if */
              break;

          case 8:   /* 8 byte elements */
              for (i = 0, lp_src = (unsigned long *) source, lp_dest = (unsigned long *) dest; i < num_elem; i++, lp_dest++, lp_src++)
                {
                    *lp_dest = ((*lp_src & SWAP_MASKA) >> 32) |
                        ((*lp_src & SWAP_MASKB) << 32);
                    *lp_dest = ((*lp_dest & SWAP_MASKC) >> 16) |
                        ((*lp_dest & SWAP_MASKD) << 16);
                    *lp_dest = ((*lp_dest & SWAP_MASKE) >> 8) |
                        ((*lp_dest & SWAP_MASKF) << 8);
                }   /* end for */
              break;
      }     /* end switch */
}   /* end DFKswap() */

#define UI2I_MASKA  0xffff000000000000
#define UI2I_MASKB  0x0000ffff00000000
#define UI2I_MASKC  0x00000000ffff0000
#define UI2I_MASKD  0x000000000000ffff

/************************************************************/
/* DFKui2i()                                                */
/* -->Unicos routine for importing 2 byte data items        */
/* (**) This routine converts two byte IEEE to eight byte   */
/*      Cray big endian integer.                            */
/************************************************************/
int
DFKui2i(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    uint32 i;
    int         fast_processing = 0;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    long       *lptr_dest = (long *) dest;
    long       *lp_dest;
    unsigned long *lp_src;
    char       *FUNC = "DFKui2i";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 7;       /* type of conversion to perform 7=short integer */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    if (source == dest || num_elm == 0)     /* Inplace conversions not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* No elements is an error */

    /* Find out if it is OK to use faster array processing */
    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
#ifdef DOESNT_WORK
          bitoff = ((unsigned int)source) >> BITOFF;
          ierr = IEG2CRAY(&type, &num_elm, source, &bitoff, dest);
          if (ierr != 0)
              HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
#else
#ifndef DUFF_ui2i
#if defined TEST2_ui2i
          int         odd_man_out;  /* By default there are even num_elm */
          intn        n;

          odd_man_out = num_elm % 4;

          n = num_elm / 4;
          lp_dest = (long *) dest;
          lp_src = (unsigned long *) source;
          HDmemset(lp_dest, 0, num_elm * sizeof(long));
          for (i = 0; i < n; i++)
            {
                lp_dest[0] = (lp_src[0] & UI2I_MASKA) >> 48;
                lp_dest[1] = (lp_src[0] & UI2I_MASKB) >> 32;
                lp_dest[2] = (lp_src[0] & UI2I_MASKC) >> 16;
                lp_dest[3] = lp_src[0] & UI2I_MASKD;
                lp_dest += 4;
                lp_src++;
            }   /* end for */
          switch (odd_man_out)
            {
                case 3:
                    lp_dest[0] = (lp_src[0] & UI2I_MASKA) >> 48;
                    lp_dest[1] = (lp_src[0] & UI2I_MASKB) >> 32;
                    lp_dest[2] = (lp_src[0] & UI2I_MASKC) >> 16;
                    break;

                case 2:
                    lp_dest[0] = (lp_src[0] & UI2I_MASKA) >> 48;
                    lp_dest[1] = (lp_src[0] & UI2I_MASKB) >> 32;
                    break;

                case 1:
                    lp_dest[0] = (lp_src[0] & UI2I_MASKA) >> 48;
                    break;

                default:
                    break;
            }   /* end switch */
#elif defined TEST1_ui2i
          int         odd_man_out;  /* By default there are even num_elm */
          intn        n;

          odd_man_out = num_elm % 4;

          n = num_elm / 4;
          lp_dest = (long *) dest;
          lp_src = (unsigned long *) source;
          HDmemset(lp_dest, 0, num_elm * sizeof(long));
          for (i = 0; i < n; i++)
            {
                *lp_dest++ = (lp_src[0] & UI2I_MASKA) >> 48;
                *lp_dest++ = (lp_src[0] & UI2I_MASKB) >> 32;
                *lp_dest++ = (lp_src[0] & UI2I_MASKC) >> 16;
                *lp_dest++ = lp_src[0] & UI2I_MASKD;
                lp_src++;
            }   /* end for */
          switch (odd_man_out)
            {
                case 3:
                    *lp_dest++ = (lp_src[0] & UI2I_MASKA) >> 48;
                    *lp_dest++ = (lp_src[0] & UI2I_MASKB) >> 32;
                    *lp_dest++ = (lp_src[0] & UI2I_MASKC) >> 16;
                    break;

                case 2:
                    *lp_dest++ = (lp_src[0] & UI2I_MASKA) >> 48;
                    *lp_dest++ = (lp_src[0] & UI2I_MASKB) >> 32;
                    break;

                case 1:
                    *lp_dest++ = (lp_src[0] & UI2I_MASKA) >> 48;
                    break;

                default:
                    break;
            }   /* end switch */
#else
          for (i = 0; i < num_elm; i++)
            {
                lptr_dest[0] = 0x0000000000000000;
                dest[6] = source[0];
                dest[7] = source[1];
                source += 2;
                lptr_dest++;
                dest = (uint8 *) lptr_dest;
            }
#endif
#else  /* DUFF_ui2i */
          uintn       n;
          int         odd_man_out;  /* By default there are even num_elm */
          uintn       orig_num_elm = num_elm;

          lp_dest = (long *) dest;
          lp_src = (unsigned long *) source;
          HDmemset(lp_dest, 0, num_elm * sizeof(long));

          odd_man_out = num_elm % 4;

          num_elm /= 4;
          n = (num_elm + 7) / 8;
          if (orig_num_elm > 3)
              switch (num_elm % 8)
                {
                    case 0:
                        do
                          {
                              lp_dest[0] = (lp_src[0] & UI2I_MASKA) >> 48;
                              lp_dest[1] = (lp_src[0] & UI2I_MASKB) >> 32;
                              lp_dest[2] = (lp_src[0] & UI2I_MASKC) >> 16;
                              lp_dest[3] = lp_src[0] & UI2I_MASKD;
                              lp_dest += 4;
                              lp_src++;
                    case 7:
                              lp_dest[0] = (lp_src[0] & UI2I_MASKA) >> 48;
                              lp_dest[1] = (lp_src[0] & UI2I_MASKB) >> 32;
                              lp_dest[2] = (lp_src[0] & UI2I_MASKC) >> 16;
                              lp_dest[3] = lp_src[0] & UI2I_MASKD;
                              lp_dest += 4;
                              lp_src++;
                    case 6:
                              lp_dest[0] = (lp_src[0] & UI2I_MASKA) >> 48;
                              lp_dest[1] = (lp_src[0] & UI2I_MASKB) >> 32;
                              lp_dest[2] = (lp_src[0] & UI2I_MASKC) >> 16;
                              lp_dest[3] = lp_src[0] & UI2I_MASKD;
                              lp_dest += 4;
                              lp_src++;
                    case 5:
                              lp_dest[0] = (lp_src[0] & UI2I_MASKA) >> 48;
                              lp_dest[1] = (lp_src[0] & UI2I_MASKB) >> 32;
                              lp_dest[2] = (lp_src[0] & UI2I_MASKC) >> 16;
                              lp_dest[3] = lp_src[0] & UI2I_MASKD;
                              lp_dest += 4;
                              lp_src++;
                    case 4:
                              lp_dest[0] = (lp_src[0] & UI2I_MASKA) >> 48;
                              lp_dest[1] = (lp_src[0] & UI2I_MASKB) >> 32;
                              lp_dest[2] = (lp_src[0] & UI2I_MASKC) >> 16;
                              lp_dest[3] = lp_src[0] & UI2I_MASKD;
                              lp_dest += 4;
                              lp_src++;
                    case 3:
                              lp_dest[0] = (lp_src[0] & UI2I_MASKA) >> 48;
                              lp_dest[1] = (lp_src[0] & UI2I_MASKB) >> 32;
                              lp_dest[2] = (lp_src[0] & UI2I_MASKC) >> 16;
                              lp_dest[3] = lp_src[0] & UI2I_MASKD;
                              lp_dest += 4;
                              lp_src++;
                    case 2:
                              lp_dest[0] = (lp_src[0] & UI2I_MASKA) >> 48;
                              lp_dest[1] = (lp_src[0] & UI2I_MASKB) >> 32;
                              lp_dest[2] = (lp_src[0] & UI2I_MASKC) >> 16;
                              lp_dest[3] = lp_src[0] & UI2I_MASKD;
                              lp_dest += 4;
                              lp_src++;
                    case 1:
                              lp_dest[0] = (lp_src[0] & UI2I_MASKA) >> 48;
                              lp_dest[1] = (lp_src[0] & UI2I_MASKB) >> 32;
                              lp_dest[2] = (lp_src[0] & UI2I_MASKC) >> 16;
                              lp_dest[3] = lp_src[0] & UI2I_MASKD;
                              lp_dest += 4;
                              lp_src++;
                          }
                        while (--n > 0);
                }
          switch (odd_man_out)
            {
                case 3:
                    lp_dest[0] = (lp_src[0] & UI2I_MASKA) >> 48;
                    lp_dest[1] = (lp_src[0] & UI2I_MASKB) >> 32;
                    lp_dest[2] = (lp_src[0] & UI2I_MASKC) >> 16;
                    break;

                case 2:
                    lp_dest[0] = (lp_src[0] & UI2I_MASKA) >> 48;
                    lp_dest[1] = (lp_src[0] & UI2I_MASKB) >> 32;
                    break;

                case 1:
                    lp_dest[0] = (lp_src[0] & UI2I_MASKA) >> 48;
                    break;

                default:
                    break;
            }   /* end switch */
#endif /* DUFF_ui2i */
#endif
      }     /* end if */
    else
      {     /* Generic stride processing */
          for (i = 0; i < num_elm; i++)
            {
#ifdef NOINTCRAY2IEG
                dest[0] = 0x00;
                dest[1] = 0x00;
                dest[2] = 0x00;
                dest[3] = 0x00;
                dest[4] = 0x00;
                dest[5] = 0x00;
                dest[6] = source[0];
                dest[7] = source[1];
#else
                bitoff = ((unsigned int)source) >> BITOFF;
                ierr = IEG2CRAY(&type, &n_elem, source, &bitoff, dest);
                if (ierr != 0)
                    HRETURN_ERROR(DFE_BADCONV, FAIL);   /* error in Cray conversion */
#endif
                source += source_stride;
                dest += dest_stride;
            }
      }     /* end if */
    return (SUCCEED);
}

#define UI2S_MASKA  0xffff000000000000
#define UI2S_MASKB  0x0000ffff00000000
#define UI2S_MASKC  0x00000000ffff0000
#define UI2S_MASKD  0x000000000000ffff
#define UI2S_MASKE  0x8000000000000000
#define UI2S_MASKF  0x0000800000000000
#define UI2S_MASKG  0x0000000080000000
#define UI2S_MASKH  0x0000000000008000
#define UI2S_MASKI  0xffffffffffff0000

/************************************************************/
/* DFKui2s()                                                */
/* -->Unicos routine for importing 2 byte signed ints       */
/* (**) This routine converts two byte IEEE to eight byte   */
/*      Cray.                                               */
/************************************************************/
int
DFKui2s(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    uint32 i;
    int         fast_processing = 0;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    long       *lptr_dest = (long *) dest;
    long       *lp_dest;
    unsigned long *lp_src;
    char       *FUNC = "DFKui2s";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 7;       /* type of conversion to perform 7=short integer */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    if (source == dest || num_elm == 0)     /* Inplace conversions  not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* No elements to convert is an error */

    /* Find out if it is OK to use faster array processing */
    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
          bitoff = ((unsigned int)source) >> BITOFF;
          ierr = IEG2CRAY(&type, &num_elm, source, &bitoff, dest);
          if (ierr != 0)
              HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
      }     /* end if */
    else
      {     /* Generic stride processing */
          for (i = 0; i < num_elm; i++)
            {
#ifdef NOINTCRAY2IEG
                if ((source[0] & 0x80))
                  {     /* Can't forget to extend sign */
                      dest[0] = 0xff;
                      dest[1] = 0xff;
                      dest[2] = 0xff;
                      dest[3] = 0xff;
                      dest[4] = 0xff;
                      dest[5] = 0xff;
                  }     /* end if */
                else
                  {
                      dest[0] = 0x00;
                      dest[1] = 0x00;
                      dest[2] = 0x00;
                      dest[3] = 0x00;
                      dest[4] = 0x00;
                      dest[5] = 0x00;
                  }     /* end else */
                dest[6] = source[0];
                dest[7] = source[1];
#else
                bitoff = ((unsigned int)source) >> BITOFF;
                ierr = IEG2CRAY(&type, &n_elem, source, &bitoff, dest);
                if (ierr != 0)
                    HRETURN_ERROR(DFE_BADCONV, FAIL);   /* error in Cray conversion */
#endif
                source += source_stride;
                dest += dest_stride;
            }   /* end for */
      }     /* end if */
    return (SUCCEED);
}

#define UO2I_MASK 0x000000000000ffff

/************************************************************/
/* DFKuo2i()                                                */
/* -->Unicos routine for exporting 2 byte data items        */
/************************************************************/
int
DFKuo2i(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    uintn i;
    int         fast_processing = 0;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    long       *lp_dest;
    long       *lp_src;
    char       *FUNC = "DFKuo2i";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 7;       /* type of conversion to perform 7=short integer */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    if (source == dest || num_elm == 0)     /* Inplace conversions  not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* No elements to convert is an error */

    /* Find out if it is OK to use faster array processing */
    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
#ifdef DOESNT_WORK
          bitoff = ((unsigned int)dest) >> BITOFF;
          ierr = CRAY2IEG(&type, &num_elm, dest, &bitoff, source);
          if (ierr != 0)
              HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
#else
#ifndef DUFF_uo2i
#if defined TEST1_uo2i
          int         odd_man_out;  /* By default there are even num_elm */
          intn        n;

          odd_man_out = num_elm % 4;

          n = num_elm / 4;
          lp_dest = (long *) dest;
          lp_src = (long *) source;
          for (i = 0; i < n; i++)
            {
                *lp_dest++ = ((lp_src[0] & UO2I_MASK) << 48) |
                    ((lp_src[1] & UO2I_MASK) << 32) |
                    ((lp_src[2] & UO2I_MASK) << 16) |
                    (lp_src[3] & UO2I_MASK);
                lp_src += 4;
            }
          switch (odd_man_out)
            {   /* clean up leftovers */
                case 3:
                    *lp_dest = ((lp_src[0] & UO2I_MASK) << 48) |
                        ((lp_src[1] & UO2I_MASK) << 32) |
                        ((lp_src[2] & UO2I_MASK) << 16);
                    break;

                case 2:
                    *lp_dest = ((lp_src[0] & UO2I_MASK) << 48) |
                        ((lp_src[1] & UO2I_MASK) << 32);
                    break;

                case 1:
                    *lp_dest = (lp_src[0] & UO2I_MASK) << 48;
                    break;

                default:
                    break;
            }   /* end switch */
#else
          for (i = 0; i < num_elm; i++)
            {
                dest[0] = source[6];
                dest[1] = source[7];
                dest += 2;
                source += 8;
            }
#endif
#else  /* DUFF_uo2i */
          uintn       n;
          int         odd_man_out;  /* By default there are even num_elm */
          uintn       orig_num_elm = num_elm;

          odd_man_out = num_elm % 4;

          num_elm /= 4;
          n = (num_elm + 7) / 8;
          lp_dest = (long *) dest;
          lp_src = (long *) source;
          if (orig_num_elm > 3)
              switch (num_elm % 8)
                {
                    case 0:
                        do
                          {
                              *lp_dest++ = ((lp_src[0] & UO2I_MASK) << 48) |
                                  ((lp_src[1] & UO2I_MASK) << 32) |
                                  ((lp_src[2] & UO2I_MASK) << 16) |
                                  (lp_src[3] & UO2I_MASK);
                              lp_src += 4;
                    case 7:
                              *lp_dest++ = ((lp_src[0] & UO2I_MASK) << 48) |
                                  ((lp_src[1] & UO2I_MASK) << 32) |
                                  ((lp_src[2] & UO2I_MASK) << 16) |
                                  (lp_src[3] & UO2I_MASK);
                              lp_src += 4;
                    case 6:
                              *lp_dest++ = ((lp_src[0] & UO2I_MASK) << 48) |
                                  ((lp_src[1] & UO2I_MASK) << 32) |
                                  ((lp_src[2] & UO2I_MASK) << 16) |
                                  (lp_src[3] & UO2I_MASK);
                              lp_src += 4;
                    case 5:
                              *lp_dest++ = ((lp_src[0] & UO2I_MASK) << 48) |
                                  ((lp_src[1] & UO2I_MASK) << 32) |
                                  ((lp_src[2] & UO2I_MASK) << 16) |
                                  (lp_src[3] & UO2I_MASK);
                              lp_src += 4;
                    case 4:
                              *lp_dest++ = ((lp_src[0] & UO2I_MASK) << 48) |
                                  ((lp_src[1] & UO2I_MASK) << 32) |
                                  ((lp_src[2] & UO2I_MASK) << 16) |
                                  (lp_src[3] & UO2I_MASK);
                              lp_src += 4;
                    case 3:
                              *lp_dest++ = ((lp_src[0] & UO2I_MASK) << 48) |
                                  ((lp_src[1] & UO2I_MASK) << 32) |
                                  ((lp_src[2] & UO2I_MASK) << 16) |
                                  (lp_src[3] & UO2I_MASK);
                              lp_src += 4;
                    case 2:
                              *lp_dest++ = ((lp_src[0] & UO2I_MASK) << 48) |
                                  ((lp_src[1] & UO2I_MASK) << 32) |
                                  ((lp_src[2] & UO2I_MASK) << 16) |
                                  (lp_src[3] & UO2I_MASK);
                              lp_src += 4;
                    case 1:
                              *lp_dest++ = ((lp_src[0] & UO2I_MASK) << 48) |
                                  ((lp_src[1] & UO2I_MASK) << 32) |
                                  ((lp_src[2] & UO2I_MASK) << 16) |
                                  (lp_src[3] & UO2I_MASK);
                              lp_src += 4;
                          }
                        while (--n > 0);
                }

          switch (odd_man_out)
            {   /* clean up leftovers */
                case 3:
                    *lp_dest = ((lp_src[0] & UO2I_MASK) << 48) |
                        ((lp_src[1] & UO2I_MASK) << 32) |
                        ((lp_src[2] & UO2I_MASK) << 16);
                    break;

                case 2:
                    *lp_dest = ((lp_src[0] & UO2I_MASK) << 48) |
                        ((lp_src[1] & UO2I_MASK) << 32);
                    break;

                case 1:
                    *lp_dest = (lp_src[0] & UO2I_MASK) << 48;
                    break;

                default:
                    break;
            }   /* end switch */
#endif /* DUFF_uo2i */
#endif
      }     /* end if */
    else
      {     /* Generic Stride processing */
          for (i = 0; i < num_elm; i++)
            {
#ifdef NOINTCRAY2IEG
                dest[0] = source[6];
                dest[1] = source[7];
#else
                bitoff = ((unsigned int)dest) >> BITOFF;
                ierr = CRAY2IEG(&type, &n_elem, dest, &bitoff, source);
                if (ierr != 0)
                    HRETURN_ERROR(DFE_BADCONV, FAIL);   /* error in Cray conversion */
#endif
                source += source_stride;
                dest += dest_stride;
            }   /* end for */
      }     /* end else */
    return (SUCCEED);
}

#define UO2S_MASK 0x000000000000ffff

/************************************************************/
/* DFKuo2s()                                                */
/* -->Unicos routine for exporting signed 2 byte data items */
/************************************************************/
int
DFKuo2s(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    uintn i;
    int         fast_processing = 0;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    long       *lp_dest;
    long       *lp_src;
    char       *FUNC = "DFKuo2s";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 7;       /* type of conversion to perform 7=short integer */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    if (source == dest || num_elm == 0)     /* Inplace conversions  not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* No elements to convert is an error */

    /* Find out if it is OK to use faster array processing */
    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
          bitoff = ((unsigned int)dest) >> BITOFF;
          ierr = CRAY2IEG(&type, &num_elm, dest, &bitoff, source);
          if (ierr != 0)
              HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
      }     /* end if */
    else
      {     /* Generic Stride processing */
          for (i = 0; i < num_elm; i++)
            {
#ifdef NOINTCRAY2IEG
                dest[0] = source[6];
                dest[1] = source[7];
#else
                bitoff = ((unsigned int)dest) >> BITOFF;
                ierr = CRAY2IEG(&type, &n_elem, dest, &bitoff, source);
                if (ierr != 0)
                    HRETURN_ERROR(DFE_BADCONV, FAIL);   /* error in Cray conversion */
#endif
                source += source_stride;
                dest += dest_stride;
            }   /* end for */
      }     /* end else */
    return (SUCCEED);
}

#define UI4I_MASKA 0xffffffff00000000
#define UI4I_MASKB 0x00000000ffffffff

/************************************************************/
/* DFKui4i()                                                */
/* -->Unicos routine for importing 4 byte unsigned ints     */
/************************************************************/
int
DFKui4i(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    int         fast_processing = 0;
    uint32 i;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    long       *lptr_dest = (long *) dest;
    long       *lp_dest;
    unsigned long *lp_src;
    char       *FUNC = "DFKui4i";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 1;       /* type of conversion to perform 1=short integer */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    if (source == dest || num_elm == 0)     /* Inplace conversions  not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* No elements to convert is an error */

    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
#ifdef DOESNT_WORK
          bitoff = ((unsigned int)source) >> BITOFF;
          ierr = IEG2CRAY(&type, &num_elm, source, &bitoff, dest);
          if (ierr != 0)
              HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
#else
#ifndef DUFF_ui4i
#if defined TEST2_ui4i
          int         odd_man_out;  /* By default there are even num_elm */
          intn        n;

          odd_man_out = num_elm % 2;

          n = num_elm / 2;
          lp_dest = (long *) dest;
          lp_src = (unsigned long *) source;
          HDmemset(lp_dest, 0, num_elm * sizeof(long));     /* initialize to zeros */
          for (i = 0; i < n; i++)
            {
                lp_dest[0] = (lp_src[0] & UI4I_MASKA) >> 32;
                lp_dest[1] = lp_src[0] & UI4I_MASKB;
                lp_dest += 2;
                lp_src++;
            }   /* end for */
          if (odd_man_out)
              *lp_dest = (lp_src[0] & UI4I_MASKA) >> 32;
#elif defined TEST1_ui4i
          int         odd_man_out;  /* By default there are even num_elm */
          intn        n;

          odd_man_out = num_elm % 2;

          n = num_elm / 2;
          lp_dest = (long *) dest;
          lp_src = (unsigned long *) source;
          HDmemset(lp_dest, 0, num_elm * sizeof(long));     /* initialize to zeros */
          for (i = 0; i < n; i++)
            {
                *lp_dest++ = (lp_src[0] & UI4I_MASKA) >> 32;
                *lp_dest++ = lp_src[0] & UI4I_MASKB;
                lp_src++;
            }   /* end for */
          if (odd_man_out)
              *lp_dest++ = (lp_src[0] & UI4I_MASKA) >> 32;
#else
          for (i = 0; i < num_elm; i++)
            {
                lptr_dest[0] = 0;
                dest[4] = source[0];
                dest[5] = source[1];
                dest[6] = source[2];
                dest[7] = source[3];
                source += 4;
                lptr_dest++;
                dest = (uint8 *) lptr_dest;
            }   /* end for */
#endif
#else  /* DUFF_ui4i */
          uintn       n;
          int         odd_man_out;  /* By default there are even num_elm */
          uintn       orig_num_elm = num_elm;

          lp_dest = (long *) dest;
          lp_src = (unsigned long *) source;
          HDmemset(lp_dest, 0, num_elm * sizeof(long));     /* initialize to zeros */

          odd_man_out = num_elm % 2;

          num_elm /= 2;
          n = (num_elm + 7) / 8;
          if (orig_num_elm > 1)
              switch (num_elm % 8)
                {
                    case 0:
                        do
                          {
                              lp_dest[0] = (lp_src[0] & UI4I_MASKA) >> 32;
                              lp_dest[1] = lp_src[0] & UI4I_MASKB;
                              lp_dest += 2;
                              lp_src++;
                    case 7:
                              lp_dest[0] = (lp_src[0] & UI4I_MASKA) >> 32;
                              lp_dest[1] = lp_src[0] & UI4I_MASKB;
                              lp_dest += 2;
                              lp_src++;
                    case 6:
                              lp_dest[0] = (lp_src[0] & UI4I_MASKA) >> 32;
                              lp_dest[1] = lp_src[0] & UI4I_MASKB;
                              lp_dest += 2;
                              lp_src++;
                    case 5:
                              lp_dest[0] = (lp_src[0] & UI4I_MASKA) >> 32;
                              lp_dest[1] = lp_src[0] & UI4I_MASKB;
                              lp_dest += 2;
                              lp_src++;
                    case 4:
                              lp_dest[0] = (lp_src[0] & UI4I_MASKA) >> 32;
                              lp_dest[1] = lp_src[0] & UI4I_MASKB;
                              lp_dest += 2;
                              lp_src++;
                    case 3:
                              lp_dest[0] = (lp_src[0] & UI4I_MASKA) >> 32;
                              lp_dest[1] = lp_src[0] & UI4I_MASKB;
                              lp_dest += 2;
                              lp_src++;
                    case 2:
                              lp_dest[0] = (lp_src[0] & UI4I_MASKA) >> 32;
                              lp_dest[1] = lp_src[0] & UI4I_MASKB;
                              lp_dest += 2;
                              lp_src++;
                    case 1:
                              lp_dest[0] = (lp_src[0] & UI4I_MASKA) >> 32;
                              lp_dest[1] = lp_src[0] & UI4I_MASKB;
                              lp_dest += 2;
                              lp_src++;
                          }
                        while (--n > 0);
                }
          if (odd_man_out)
              *lp_dest = (lp_src[0] & UI4I_MASKA) >> 32;
#endif /* DUFF_ui4i */
#endif
      }     /* end if */
    else
      {
          for (i = 0; i < num_elm; i++)
            {
#ifdef NOINTCRAY2IEG
                dest[0] = 0;
                dest[1] = 0;
                dest[2] = 0;
                dest[3] = 0;
                dest[4] = source[0];
                dest[5] = source[1];
                dest[6] = source[2];
                dest[7] = source[3];
#else
                bitoff = ((unsigned int)source) >> BITOFF;
                ierr = IEG2CRAY(&type, &n_elem, source, &bitoff, dest);
                if (ierr != 0)
                    HRETURN_ERROR(DFE_BADCONV, FAIL);   /* error in Cray conversion */
#endif
                dest += dest_stride;
                source += source_stride;
            }   /* end for */
      }     /* end else */
    return (SUCCEED);
}

#define UI4S_MASKA 0xffffffff00000000
#define UI4S_MASKB 0x00000000ffffffff
#define UI4S_MASKC 0x8000000000000000
#define UI4S_MASKD 0x0000000080000000
#define UI4S_MASKE 0xffffffff00000000

/************************************************************/
/* DFKui4s()                                                */
/* -->Unicos routine for importing 4 signed ints            */
/************************************************************/
int
DFKui4s(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    int         fast_processing = 0;
    uint32 i;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    long       *lptr_dest = (long *) dest;
    long       *lp_dest;
    long       *lp_src;
    char       *FUNC = "DFKui4s";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 1;       /* type of conversion to perform 1=short integer */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    if (source == dest || num_elm == 0)     /* Inplace conversions  not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* No elements to convert is an error */

    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
          bitoff = ((unsigned int)source) >> BITOFF;
          ierr = IEG2CRAY(&type, &num_elm, source, &bitoff, dest);
          if (ierr != 0)
              HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
      }     /* end if */
    else
        for (i = 0; i < num_elm; i++)
          {
#ifdef NOINTCRAY2IEG
              if (source[0] & 0x80)
                {   /* Can't forget to sign extend */
                    dest[0] = 0xff;
                    dest[1] = 0xff;
                    dest[2] = 0xff;
                    dest[3] = 0xff;
                }   /* end if */
              else
                {
                    dest[0] = 0;
                    dest[1] = 0;
                    dest[2] = 0;
                    dest[3] = 0;
                }   /* end else */
              dest[4] = source[0];
              dest[5] = source[1];
              dest[6] = source[2];
              dest[7] = source[3];
#else
              bitoff = ((unsigned int)source) >> BITOFF;
              ierr = IEG2CRAY(&type, &n_elem, source, &bitoff, dest);
              if (ierr != 0)
                  HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
#endif
              dest += dest_stride;
              source += source_stride;
          }     /* end for */
    return (SUCCEED);
}

#define UO4I_MASK 0x00000000ffffffff

/************************************************************/
/* DFKuo4i()                                                */
/* -->Unicos routine for exporting 4 byte data items        */
/************************************************************/

int
DFKuo4i(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    int         fast_processing = 0;
    uintn i;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    long       *lp_dest;
    long       *lp_src;
    char       *FUNC = "DFKuo4i";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 1;       /* type of conversion to perform 7=short integer */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    if (source == dest || num_elm == 0)     /* Inplace conversions  not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* No elements to convert is an error */

    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
#ifdef DOESNT_WORK
          bitoff = ((unsigned int)dest) >> BITOFF;
          ierr = CRAY2IEG(&type, &num_elm, dest, &bitoff, source);
          if (ierr != 0)
              HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
#else
#ifndef DUFF_uo4i
#if defined TEST1_uo4i
          int         odd_man_out;  /* By default there are even num_elm */
          intn        n;

          odd_man_out = num_elm % 2;

          n = num_elm / 2;
          lp_dest = (long *) dest;
          lp_src = (long *) source;
          for (i = 0; i < n; i++)
            {
                *lp_dest++ = ((lp_src[0] & UO4I_MASK) << 32) | (lp_src[1] & UO4I_MASK);
                lp_src += 2;
            }
          if (odd_man_out)
              *lp_dest = (lp_src[0] & UO4I_MASK) << 32;
#else
          for (i = 0; i < num_elm; i++)
            {
                dest[0] = source[4];
                dest[1] = source[5];
                dest[2] = source[6];
                dest[3] = source[7];
                dest += 4;
                source += 8;
            }
#endif
#else  /* DUFF_uo4i */
          uintn       n;
          int         odd_man_out;  /* By default there are even num_elm */
          uintn       orig_num_elm = num_elm;

          odd_man_out = num_elm % 2;

          num_elm /= 2;
          n = (num_elm + 7) / 8;
          lp_dest = (long *) dest;
          lp_src = (long *) source;

          if (orig_num_elm > 1)
              switch (num_elm % 8)
                {
                    case 0:
                        do
                          {
                              *lp_dest++ = ((lp_src[0] & UO4I_MASK) << 32) | (lp_src[1] & UO4I_MASK);
                              lp_src += 2;
                    case 7:
                              *lp_dest++ = ((lp_src[0] & UO4I_MASK) << 32) | (lp_src[1] & UO4I_MASK);
                              lp_src += 2;
                    case 6:
                              *lp_dest++ = ((lp_src[0] & UO4I_MASK) << 32) | (lp_src[1] & UO4I_MASK);
                              lp_src += 2;
                    case 5:
                              *lp_dest++ = ((lp_src[0] & UO4I_MASK) << 32) | (lp_src[1] & UO4I_MASK);
                              lp_src += 2;
                    case 4:
                              *lp_dest++ = ((lp_src[0] & UO4I_MASK) << 32) | (lp_src[1] & UO4I_MASK);
                              lp_src += 2;
                    case 3:
                              *lp_dest++ = ((lp_src[0] & UO4I_MASK) << 32) | (lp_src[1] & UO4I_MASK);
                              lp_src += 2;
                    case 2:
                              *lp_dest++ = ((lp_src[0] & UO4I_MASK) << 32) | (lp_src[1] & UO4I_MASK);
                              lp_src += 2;
                    case 1:
                              *lp_dest++ = ((lp_src[0] & UO4I_MASK) << 32) | (lp_src[1] & UO4I_MASK);
                              lp_src += 2;
                          }
                        while (--n > 0);
                }
          if (odd_man_out)
              *lp_dest++ = (lp_src[0] & UO4I_MASK) << 32;

#endif /* DUFF_uo4i */
#endif
      }     /* end if */
    else
        for (i = 0; i < num_elm; i++)
          {
#ifdef NOINTCRAY2IEG
              dest[0] = source[4];
              dest[1] = source[5];
              dest[2] = source[6];
              dest[3] = source[7];
#else
              bitoff = ((unsigned int)dest) >> BITOFF;
              ierr = CRAY2IEG(&type, &n_elem, dest, &bitoff, source);
              if (ierr != 0)
                  HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
#endif
              dest += dest_stride;
              source += source_stride;
          }     /* end for */
    return (SUCCEED);
}

#define UO4S_MASK 0x00000000ffffffff

/************************************************************/
/* DFKuo4s()                                                */
/* -->Unicos routine for exporting signed 4 byte data items */
/************************************************************/
int
DFKuo4s(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    int         fast_processing = 0;
    uintn i;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    long       *lp_dest;
    long       *lp_src;
    char       *FUNC = "DFKuo4s";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 1;       /* type of conversion to perform 7=short integer */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    if (source == dest || num_elm == 0)     /* Inplace conversions  not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* No elements to convert is an error */

    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
          bitoff = ((unsigned int)dest) >> BITOFF;
          ierr = CRAY2IEG(&type, &num_elm, dest, &bitoff, source);
          if (ierr != 0)
              HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
      }     /* end if */
    else
        for (i = 0; i < num_elm; i++)
          {
#ifdef NOINTCRAY2IEG
              dest[0] = source[4];
              dest[1] = source[5];
              dest[2] = source[6];
              dest[3] = source[7];
#else
              bitoff = ((unsigned int)dest) >> BITOFF;
              ierr = CRAY2IEG(&type, &n_elem, dest, &bitoff, source);
              if (ierr != 0)
                  HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
#endif
              dest += dest_stride;
              source += source_stride;
          }     /* end for */
    return (SUCCEED);
}

#define UI4F_MASKA 0x8000000000000000
#define UI4F_MASKB 0x7f80000000000000
#define UI4F_MASKC 0x007fffff00000000
#define UI4F_MASKD 0x0000800000000000
#define UI4F_MASKE 0x0000000080000000
#define UI4F_MASKF 0x000000007f800000
#define UI4F_MASKG 0x00000000007fffff
#define UI4F_MASKH 0x0000000000008000
#define UI4F_MASKI 0x7fffffff00000000
#define UI4F_MASKJ 0x000000007fffffff

/************************************************************/
/* DFKui4f()                                                */
/* -->Unicos routine for importing 32 bit floats            */
/************************************************************/

/************************************************************

                     <<<< WARNING >>>>

    The nature of converting between 64 bit floating point
  numbers and 32 bit floating point numbers LOSES PRECISION.
  Taking a number in 64bit cray format, converting to IEEE
  (internal HDF format) and back will round the number at
  about the 7th decimal place.

 ************************************************************/

int
DFKui4f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    int         fast_processing = 0;    /* By default not array processed */
    int         odd_man_out = 0;    /* By default there are even num_elm */
    int         i, j, n;
    long        buf1;           /* This is a temporary stride buf */
    long        buf2;           /* This is a temporary stride buf */
    uint8      *dud1 = (uint8 *) &buf1;     /* Dummy pointer to buf1 for strides */
    uint8      *dud2 = (uint8 *) &buf2;     /* Dummy pointer to buf2 for strides */
    uint8      *source = (uint8 *) s;   /* Cray does not like certain   */
    uint8      *dest = (uint8 *) d;     /* void and void* constructions */
    long       *lptr_src = (long *) source;
    long       *lptr_dest = (long *) dest;
    char       *FUNC = "DFKui4f";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 2;       /* type of conversion to perform 2=32-bit float */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    /* Check for conversion errors */
    if (source == dest || num_elm == 0)     /* Inplace conversions not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* under UNICOS */

    /* Find out if it is OK to use faster array processing */
    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
          bitoff = ((unsigned int)source) >> BITOFF;
          ierr = IEG2CRAY(&type, &num_elm, source, &bitoff, dest);
          if (ierr != 0)
              HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
      }     /* end if */
    else
      {     /* We end up here if we are doing stride based processing */
#ifdef NOSTRIDEFLOATCRAY2IEG
          buf1 = 0;
          for (i = 0; i < num_elm; i++)
            {
                dud1[0] = source[0];    /* Loop would be less efficient */
                dud1[1] = source[1];
                dud1[2] = source[2];
                dud1[3] = source[3];

                if ((float) buf1 != 0)
                  {
                      buf2 = (((buf1 & UI4F_MASKA) | ((buf1 & UI4F_MASKB) >> 7) +
                               (16258 << 48)) |
                              (((buf1 & UI4F_MASKC) >> 8) | (UI4F_MASKD)));
                      if ((buf1 << 1) == 0)
                          buf2 = 0;
                  }     /* end if */
                else
                    buf2 = buf1;

                dest[0] = dud2[0];  /* Loop would be less efficient */
                dest[1] = dud2[1];
                dest[2] = dud2[2];
                dest[3] = dud2[3];
                dest[4] = dud2[4];
                dest[5] = dud2[5];
                dest[6] = dud2[6];
                dest[7] = dud2[7];

                source += source_stride;
                dest += dest_stride;
            }   /* end for */
#else
          uint8       tmp_dst[8];
          float32     tmp_src;

          for (i = 0; i < num_elm; i++)
            {
                HDmemcpy(&tmp_src, source, sizeof(float32));
                bitoff = ((unsigned int)&tmp_src) >> BITOFF;
                ierr = IEG2CRAY(&type, &n_elem, &tmp_src, &bitoff, tmp_dst);
                if (ierr != 0)
                    HRETURN_ERROR(DFE_BADCONV, FAIL);   /* error in Cray conversion */
                HDmemcpy(dest, tmp_dst, 8);
                source += source_stride;
                dest += dest_stride;
            }   /* end for */
#endif
      }     /* end else */
    return (SUCCEED);
}

#define UO4F_MASKA 0x8000000000000000
#define UO4F_MASKB 0x7fff000000000000
#define UO4F_MASKC 0x00007fffff000000
#define UO4F_MASKD 0x0000000000800000
#define UO4F_MASKE 0xffffffff00000000

/************************************************************/
/* DFKuo4f()                                                */
/* -->Unicos routine for exporting 32 bit floats            */
/************************************************************/

/************************************************************

                     <<<< WARNING >>>>

    The nature of converting between 64 bit floating point
  numbers and 32 bit floating point numbers LOSES PRECISION.
  Taking a number in 64bit cray format, converting to IEEE
  (internal HDF format) and back will round the number at
  about the 7th decimal place.

 ************************************************************/

int
DFKuo4f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    int         fast_processing = 0;    /* By default not array processed */
    int         odd_man_out = 0;    /* By default there are even num_elm */
    int         i, j, n;
    long        buf1;           /* This is a temporary stride buf */
    long        buf2;           /* This is a temporary stride buf */
    uint8      *dud1 = (uint8 *) &buf1;     /* Dummy pointer to buf1 for strides */
    uint8      *dud2 = (uint8 *) &buf2;     /* Dummy pointer to buf2 for strides */
    uint8      *source = (uint8 *) s;   /* Cray does not like certain   */
    uint8      *dest = (uint8 *) d;     /* void and void* constructions */
    long       *lptr_src = (long *) source;
    long       *lptr_dest = (long *) dest;
    char       *FUNC = "DFKuo4f";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 2;       /* type of conversion to perform 2=32-bit float */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    /* Check for conversion errors */
    if (source == dest || num_elm == 0)     /* Inplace conversions not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* under UNICOS */

    /* Find out if it is OK to use faster array processing */
    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
          bitoff = ((unsigned int)dest) >> BITOFF;
          ierr = CRAY2IEG(&type, &num_elm, dest, &bitoff, source);
          if (ierr != 0)
              HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
      }     /* end if */
    else
      {     /* We end up here if we are doing stride based processing */
#ifdef NOSTRIDEFLOATCRAY2IEG
          buf1 = 0;
          for (i = 0; i < num_elm; i++)
            {
                dud1[0] = source[0];    /* Loop would be less efficient */
                dud1[1] = source[1];
                dud1[2] = source[2];
                dud1[3] = source[3];
                dud1[4] = source[4];
                dud1[5] = source[5];
                dud1[6] = source[6];
                dud1[7] = source[7];

                if ((float) buf1 != 0)
                    buf2 = (((buf1 & UO4F_MASKA) |
                             ((((buf1 & UO4F_MASKB) >> 48) - 16258) << 55)) +
                            (((buf1 & UO4F_MASKC) +
                              ((buf1 & UO4F_MASKD) << 1)) << 8));
                else
                    buf2 = buf1;

                dest[0] = dud2[0];  /* Loop would be less efficient */
                dest[1] = dud2[1];
                dest[2] = dud2[2];
                dest[3] = dud2[3];

                source += source_stride;
                dest += dest_stride;
            }   /* end for */
#else
          uint8       tmp_dst[8];
          float32     tmp_src;

          for (i = 0; i < num_elm; i++)
            {
                HDmemcpy(&tmp_src, source, sizeof(float32));
                bitoff = ((unsigned int)&tmp_src) >> BITOFF;
                ierr = CRAY2IEG(&type, &n_elem, tmp_dst, &bitoff, &tmp_src);
                if (ierr != 0)
                    HRETURN_ERROR(DFE_BADCONV, FAIL);   /* error in Cray conversion */
                HDmemcpy(dest, tmp_dst, 4);
                source += source_stride;
                dest += dest_stride;
            }   /* end for */
#endif
      }     /* end else */
    return (SUCCEED);
}

#define UI8F_MASKA 0x8000000000000000
#define UI8F_MASKB 0x7ff0000000000000
#define UI8F_MASKC 0x000fffffffffffff
#define UI8F_MASKD 0x0000000000000008
#define UI8F_MASKE 0x0000800000000000
#define UI8F_MASKG 0x7fffffffffffffff

/************************************************************/
/* DFKui8f()                                                */
/* -->Unicos routine for importing 64 bit floats            */
/************************************************************/

int
DFKui8f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    int         fast_processing = 0;    /* By default not array processed */
    int         i, j, n;
    long        buf;            /* This is a temporary stride buf */
    uint8      *dud = (uint8 *) &buf;   /* Dummy pointer to buf1 for strides */
    uint8      *source = (uint8 *) s;   /* Cray does not like certain   */
    uint8      *dest = (uint8 *) d;     /* void and void* constructions */
    long       *lptr_src = (long *) source;
    long       *lptr_dest = (long *) dest;
    char       *FUNC = "DFKui8f";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 8;       /* type of conversion to perform 8=64-bit float */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    /* Check for conversion errors */
    if (source == dest || num_elm == 0)     /* Inplace conversions not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* under UNICOS */

    /* Find out if it is OK to use faster array processing */
    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {  
          bitoff = ((unsigned int)source) >> BITOFF;
          ierr = IEG2CRAY(&type, &num_elm, source, &bitoff, dest);
          if (ierr != 0)
              HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
      }     /* end if */
    else
        for (i = 0; i < num_elm; i++)
          {
#ifdef NOSTRIDEFLOATCRAY2IEG
              dud[0] = source[0];
              dud[1] = source[1];
              dud[2] = source[2];
              dud[3] = source[3];
              dud[4] = source[4];
              dud[5] = source[5];
              dud[6] = source[6];
              dud[7] = source[7];

              if (buf != 0)
                {
                    buf = (((buf & UI8F_MASKA) |
                            ((buf & UI8F_MASKB) >> 4) + (15362 << 48)) |
                           ((((buf & UI8F_MASKC) + ((buf & UI8F_MASKD) << 1)) >> 5) |
                            (UI8F_MASKE)));
                    if ((buf << 1) == 0)
                        buf = 0;
                }   /* end if */
              else
                  buf = 0;

              dest[0] = dud[0];
              dest[1] = dud[1];
              dest[2] = dud[2];
              dest[3] = dud[3];
              dest[4] = dud[4];
              dest[5] = dud[5];
              dest[6] = dud[6];
              dest[7] = dud[7];

#else
              bitoff = ((unsigned int)source) >> BITOFF;
              ierr = IEG2CRAY(&type, &n_elem, source, &bitoff, dest);
              if (ierr != 0)
                  HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
#endif
              source += source_stride;
              dest += dest_stride;
          }     /* end for */
    return (SUCCEED);
}

#define UO8F_MASKA 0x8000000000000000
#define UO8F_MASKB 0x7fff000000000000
#define UO8F_MASKC 0x00007fffffffffff

/************************************************************/
/* DFKuo8f()                                                */
/* -->Unicos routine for exporting 64 bit floats            */
/************************************************************/

int
DFKuo8f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    int         fast_processing = 0;    /* By default not array processed */
    int         odd_man_out = 0;    /* By default there are even num_elm */
    int         i, j, n;
    long        buf;            /* This is a temporary stride buf */
    uint8      *dud = (uint8 *) &buf;   /* Dummy pointer to buf1 for strides */
    uint8      *source = (uint8 *) s;   /* Cray does not like certain   */
    uint8      *dest = (uint8 *) d;     /* void and void* constructions */
    long       *lptr_src = (long *) source;
    long       *lptr_dest = (long *) dest;
    char       *FUNC = "DFKuo8f";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 8;       /* type of conversion to perform 8=64-bit float */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    /* Check for conversion errors */
    if (source == dest || num_elm == 0)     /* Inplace conversions not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* under UNICOS */

    /* Find out if it is OK to use faster array processing */
    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
          bitoff = ((unsigned int)dest) >> BITOFF;
          ierr = CRAY2IEG(&type, &num_elm, dest, &bitoff, source);
          if (ierr != 0)
              HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
      }     /* end if */
    else
        for (i = 0; i < num_elm; i++)
          {
#ifdef NOSTRIDEFLOATCRAY2IEG
              dud[0] = source[0];
              dud[1] = source[1];
              dud[2] = source[2];
              dud[3] = source[3];
              dud[4] = source[4];
              dud[5] = source[5];
              dud[6] = source[6];
              dud[7] = source[7];

              if (buf != 0)
                {
                    buf = (((buf & UO8F_MASKA) |    /* sign bit */
                      (((((buf & UO8F_MASKB) >> 48) - 15362) << 53) >> 1)) |    /* exp */
                           ((buf & UO8F_MASKC) << 5));  /* mantissa */
                }   /* end if */
              else
                  buf = 0;

              dest[0] = dud[0];
              dest[1] = dud[1];
              dest[2] = dud[2];
              dest[3] = dud[3];
              dest[4] = dud[4];
              dest[5] = dud[5];
              dest[6] = dud[6];
              dest[7] = dud[7];

#else
              bitoff = ((unsigned int)dest) >> BITOFF;
              ierr = CRAY2IEG(&type, &n_elem, dest, &bitoff, source);
              if (ierr != 0)
                  HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
#endif
              source += source_stride;
              dest += dest_stride;
          }     /* end for */
    return (SUCCEED);
}

#define LUI2I_MASKA1 0xff00000000000000
#define LUI2I_MASKA2 0x00ff000000000000
#define LUI2I_MASKB1 0x0000ff0000000000
#define LUI2I_MASKB2 0x000000ff00000000
#define LUI2I_MASKC1 0x00000000ff000000
#define LUI2I_MASKC2 0x0000000000ff0000
#define LUI2I_MASKD1 0x000000000000ff00
#define LUI2I_MASKD2 0x00000000000000ff

/************************************************************/
/* DFKlui2i()                                               */
/* -->Unicos routine for importing 2 byte data items        */
/* (**) This routine converts two byte little-endian IEEE   */
/*      to eight byte Cray big endian integer.              */
/************************************************************/
int
DFKlui2i(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    uint32 i;
    int         fast_processing = 0;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    long       *lptr_dest = (long *) dest;
    long       *lp_dest;
    long       *lp_src;
    char       *FUNC = "DFKui2i";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 7;       /* type of conversion to perform 7=short integer */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    uint8      *tmp_dst;        /* temporary buffer to hold byte swapped values */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    if (source == dest || num_elm == 0)     /* Inplace conversions not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* No elements is an error */

    /* Find out if it is OK to use faster array processing */
    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
#ifdef DOESNT_WORK
          tmp_dst = (uint8 *) HDmalloc(2 * num_elm);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          DFKswap(source, tmp_dst, 2, num_elm);
          bitoff = ((unsigned int)source) >> BITOFF;
          ierr = IEG2CRAY(&type, &num_elm, tmp_dst, &bitoff, dest);
          HDfree(tmp_dst);
          if (ierr != 0)
              HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
#else
#ifndef DUFF_lui2i
#if defined TEST2_lui2i
          int         odd_man_out;  /* By default there are even num_elm */
          intn        n;

          odd_man_out = num_elm % 4;

          n = num_elm / 4;
          lp_dest = (long *) dest;
          lp_src = (long *) source;
          HDmemset(lp_dest, 0, num_elm * sizeof(long));
          for (i = 0; i < n; i++)
            {
                lp_dest[0] = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                    ((lp_src[0] & LUI2I_MASKA2) >> 40);
                lp_dest[1] = ((lp_src[0] & LUI2I_MASKB1) >> 40) |
                    ((lp_src[0] & LUI2I_MASKB2) >> 24);
                lp_dest[2] = ((lp_src[0] & LUI2I_MASKC1) >> 24) |
                    ((lp_src[0] & LUI2I_MASKC2) >> 8);
                lp_dest[3] = ((lp_src[0] & LUI2I_MASKD1) >> 8) |
                    ((lp_src[0] & LUI2I_MASKD2) << 8);
                lp_dest += 4;
                lp_src++;
            }   /* end for */
          switch (odd_man_out)
            {
                case 3:
                    lp_dest[0] = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                        ((lp_src[0] & LUI2I_MASKA2) >> 40);
                    lp_dest[1] = ((lp_src[0] & LUI2I_MASKB1) >> 40) |
                        ((lp_src[0] & LUI2I_MASKB2) >> 24);
                    lp_dest[2] = ((lp_src[0] & LUI2I_MASKC1) >> 24) |
                        ((lp_src[0] & LUI2I_MASKC2) >> 8);
                    break;

                case 2:
                    lp_dest[0] = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                        ((lp_src[0] & LUI2I_MASKA2) >> 40);
                    lp_dest[1] = ((lp_src[0] & LUI2I_MASKB1) >> 40) |
                        ((lp_src[0] & LUI2I_MASKB2) >> 24);
                    break;

                case 1:
                    lp_dest[0] = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                        ((lp_src[0] & LUI2I_MASKA2) >> 40);
                    break;

                default:
                    break;
            }   /* end switch */
#elif defined TEST1_lui2i
          int         odd_man_out;  /* By default there are even num_elm */
          intn        n;

          odd_man_out = num_elm % 4;

          n = num_elm / 4;
          lp_dest = (long *) dest;
          lp_src = (long *) source;
          HDmemset(lp_dest, 0, num_elm * sizeof(long));
          for (i = 0; i < n; i++)
            {
                *lp_dest++ = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                    ((lp_src[0] & LUI2I_MASKA2) >> 40);
                *lp_dest++ = ((lp_src[0] & LUI2I_MASKB1) >> 40) |
                    ((lp_src[0] & LUI2I_MASKB2) >> 24);
                *lp_dest++ = ((lp_src[0] & LUI2I_MASKC1) >> 24) |
                    ((lp_src[0] & LUI2I_MASKC2) >> 8);
                *lp_dest++ = ((lp_src[0] & LUI2I_MASKD1) >> 8) |
                    ((lp_src[0] & LUI2I_MASKD2) << 8);
                lp_src++;
            }   /* end for */
          switch (odd_man_out)
            {
                case 3:
                    *lp_dest++ = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                        ((lp_src[0] & LUI2I_MASKA2) >> 40);
                    *lp_dest++ = ((lp_src[0] & LUI2I_MASKB1) >> 40) |
                        ((lp_src[0] & LUI2I_MASKB2) >> 24);
                    *lp_dest++ = ((lp_src[0] & LUI2I_MASKC1) >> 24) |
                        ((lp_src[0] & LUI2I_MASKC2) >> 8);
                    break;

                case 2:
                    *lp_dest++ = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                        ((lp_src[0] & LUI2I_MASKA2) >> 40);
                    *lp_dest++ = ((lp_src[0] & LUI2I_MASKB1) >> 40) |
                        ((lp_src[0] & LUI2I_MASKB2) >> 24);
                    break;

                case 1:
                    *lp_dest++ = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                        ((lp_src[0] & LUI2I_MASKA2) >> 40);
                    break;

                default:
                    break;
            }   /* end switch */
#else
          for (i = 0; i < num_elm; i++)
            {
                lptr_dest[0] = 0x0000000000000000;
                dest[6] = source[1];
                dest[7] = source[0];
                source += 2;
                lptr_dest++;
                dest = (uint8 *) lptr_dest;
            }
#endif
#else  /* DUFF_lui2i */
          uintn       n;
          int         odd_man_out;  /* By default there are even num_elm */
          uintn       orig_num_elm = num_elm;

          lp_dest = (long *) dest;
          lp_src = (long *) source;
          HDmemset(lp_dest, 0, num_elm * sizeof(long));

          odd_man_out = num_elm % 4;

          num_elm /= 4;
          n = (num_elm + 7) / 8;
          if (orig_num_elm > 3)
              switch (num_elm % 8)
                {
                    case 0:
                        do
                          {
                              lp_dest[0] = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                                  ((lp_src[0] & LUI2I_MASKA2) >> 40);
                              lp_dest[1] = ((lp_src[0] & LUI2I_MASKB1) >> 40) |
                                  ((lp_src[0] & LUI2I_MASKB2) >> 24);
                              lp_dest[2] = ((lp_src[0] & LUI2I_MASKC1) >> 24) |
                                  ((lp_src[0] & LUI2I_MASKC2) >> 8);
                              lp_dest[3] = ((lp_src[0] & LUI2I_MASKD1) >> 8) |
                                  ((lp_src[0] & LUI2I_MASKD2) << 8);
                              lp_dest += 4;
                              lp_src++;
                    case 7:
                              lp_dest[0] = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                                  ((lp_src[0] & LUI2I_MASKA2) >> 40);
                              lp_dest[1] = ((lp_src[0] & LUI2I_MASKB1) >> 40) |
                                  ((lp_src[0] & LUI2I_MASKB2) >> 24);
                              lp_dest[2] = ((lp_src[0] & LUI2I_MASKC1) >> 24) |
                                  ((lp_src[0] & LUI2I_MASKC2) >> 8);
                              lp_dest[3] = ((lp_src[0] & LUI2I_MASKD1) >> 8) |
                                  ((lp_src[0] & LUI2I_MASKD2) << 8);
                              lp_dest += 4;
                              lp_src++;
                    case 6:
                              lp_dest[0] = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                                  ((lp_src[0] & LUI2I_MASKA2) >> 40);
                              lp_dest[1] = ((lp_src[0] & LUI2I_MASKB1) >> 40) |
                                  ((lp_src[0] & LUI2I_MASKB2) >> 24);
                              lp_dest[2] = ((lp_src[0] & LUI2I_MASKC1) >> 24) |
                                  ((lp_src[0] & LUI2I_MASKC2) >> 8);
                              lp_dest[3] = ((lp_src[0] & LUI2I_MASKD1) >> 8) |
                                  ((lp_src[0] & LUI2I_MASKD2) << 8);
                              lp_dest += 4;
                              lp_src++;
                    case 5:
                              lp_dest[0] = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                                  ((lp_src[0] & LUI2I_MASKA2) >> 40);
                              lp_dest[1] = ((lp_src[0] & LUI2I_MASKB1) >> 40) |
                                  ((lp_src[0] & LUI2I_MASKB2) >> 24);
                              lp_dest[2] = ((lp_src[0] & LUI2I_MASKC1) >> 24) |
                                  ((lp_src[0] & LUI2I_MASKC2) >> 8);
                              lp_dest[3] = ((lp_src[0] & LUI2I_MASKD1) >> 8) |
                                  ((lp_src[0] & LUI2I_MASKD2) << 8);
                              lp_dest += 4;
                              lp_src++;
                    case 4:
                              lp_dest[0] = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                                  ((lp_src[0] & LUI2I_MASKA2) >> 40);
                              lp_dest[1] = ((lp_src[0] & LUI2I_MASKB1) >> 40) |
                                  ((lp_src[0] & LUI2I_MASKB2) >> 24);
                              lp_dest[2] = ((lp_src[0] & LUI2I_MASKC1) >> 24) |
                                  ((lp_src[0] & LUI2I_MASKC2) >> 8);
                              lp_dest[3] = ((lp_src[0] & LUI2I_MASKD1) >> 8) |
                                  ((lp_src[0] & LUI2I_MASKD2) << 8);
                              lp_dest += 4;
                              lp_src++;
                    case 3:
                              lp_dest[0] = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                                  ((lp_src[0] & LUI2I_MASKA2) >> 40);
                              lp_dest[1] = ((lp_src[0] & LUI2I_MASKB1) >> 40) |
                                  ((lp_src[0] & LUI2I_MASKB2) >> 24);
                              lp_dest[2] = ((lp_src[0] & LUI2I_MASKC1) >> 24) |
                                  ((lp_src[0] & LUI2I_MASKC2) >> 8);
                              lp_dest[3] = ((lp_src[0] & LUI2I_MASKD1) >> 8) |
                                  ((lp_src[0] & LUI2I_MASKD2) << 8);
                              lp_dest += 4;
                              lp_src++;
                    case 2:
                              lp_dest[0] = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                                  ((lp_src[0] & LUI2I_MASKA2) >> 40);
                              lp_dest[1] = ((lp_src[0] & LUI2I_MASKB1) >> 40) |
                                  ((lp_src[0] & LUI2I_MASKB2) >> 24);
                              lp_dest[2] = ((lp_src[0] & LUI2I_MASKC1) >> 24) |
                                  ((lp_src[0] & LUI2I_MASKC2) >> 8);
                              lp_dest[3] = ((lp_src[0] & LUI2I_MASKD1) >> 8) |
                                  ((lp_src[0] & LUI2I_MASKD2) << 8);
                              lp_dest += 4;
                              lp_src++;
                    case 1:
                              lp_dest[0] = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                                  ((lp_src[0] & LUI2I_MASKA2) >> 40);
                              lp_dest[1] = ((lp_src[0] & LUI2I_MASKB1) >> 40) |
                                  ((lp_src[0] & LUI2I_MASKB2) >> 24);
                              lp_dest[2] = ((lp_src[0] & LUI2I_MASKC1) >> 24) |
                                  ((lp_src[0] & LUI2I_MASKC2) >> 8);
                              lp_dest[3] = ((lp_src[0] & LUI2I_MASKD1) >> 8) |
                                  ((lp_src[0] & LUI2I_MASKD2) << 8);
                              lp_dest += 4;
                              lp_src++;
                          }
                        while (--n > 0);
                }
          switch (odd_man_out)
            {
                case 3:
                    lp_dest[0] = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                        ((lp_src[0] & LUI2I_MASKA2) >> 40);
                    lp_dest[1] = ((lp_src[0] & LUI2I_MASKB1) >> 40) |
                        ((lp_src[0] & LUI2I_MASKB2) >> 24);
                    lp_dest[2] = ((lp_src[0] & LUI2I_MASKC1) >> 24) |
                        ((lp_src[0] & LUI2I_MASKC2) >> 8);
                    break;

                case 2:
                    lp_dest[0] = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                        ((lp_src[0] & LUI2I_MASKA2) >> 40);
                    lp_dest[1] = ((lp_src[0] & LUI2I_MASKB1) >> 40) |
                        ((lp_src[0] & LUI2I_MASKB2) >> 24);
                    break;

                case 1:
                    lp_dest[0] = ((lp_src[0] & LUI2I_MASKA1) >> 56) |
                        ((lp_src[0] & LUI2I_MASKA2) >> 40);
                    break;

                default:
                    break;
            }   /* end switch */
#endif /* DUFF_lui2i */
#endif
      }     /* end for */
    else
      {     /* Generic stride processing */
#ifdef NOINTCRAY2IEG
          for (i = 0; i < num_elm; i++)
            {
                dest[0] = 0x00;
                dest[1] = 0x00;
                dest[2] = 0x00;
                dest[3] = 0x00;
                dest[4] = 0x00;
                dest[5] = 0x00;
                dest[6] = source[1];
                dest[7] = source[0];
                source += source_stride;
                dest += dest_stride;
            }   /* end for */
#else
          tmp_dst = (uint8 *) HDmalloc(2);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          for (i = 0; i < num_elm; i++)
            {
                DFKswap(source, tmp_dst, 2, 1);
                bitoff = ((unsigned int)source) >> BITOFF;
                ierr = IEG2CRAY(&type, &n_elem, tmp_dst, &bitoff, dest);
                if (ierr != 0)
                  {
                      HDfree(tmp_dst);
                      HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
                  }     /* end if */
                source += source_stride;
                dest += dest_stride;
            }   /* end for */
          HDfree(tmp_dst);
#endif
      }     /* end else */
    return (SUCCEED);
}

#define LUI2S_MASKA1 0xff00000000000000
#define LUI2S_MASKA2 0x00ff000000000000
#define LUI2S_MASKB1 0x0000ff0000000000
#define LUI2S_MASKB2 0x000000ff00000000
#define LUI2S_MASKC1 0x00000000ff000000
#define LUI2S_MASKC2 0x0000000000ff0000
#define LUI2S_MASKD1 0x000000000000ff00
#define LUI2S_MASKD2 0x00000000000000ff
#define LUI2S_MASKE  0x0080000000000000
#define LUI2S_MASKF  0x0000008000000000
#define LUI2S_MASKG  0x0000000000800000
#define LUI2S_MASKH  0x0000000000000080
#define LUI2S_MASKI  0xffffffffffff0000

/************************************************************/
/* DFKlui2s()                                                */
/* -->Unicos routine for importing 2 byte signed ints       */
/* (**) This routine converts two byte IEEE to eight byte   */
/*      Cray.                                               */
/************************************************************/
int
DFKlui2s(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    uint32 i;
    int         fast_processing = 0;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    long       *lptr_dest = (long *) dest;
    long       *lp_dest;
    long       *lp_src;
    char       *FUNC = "DFKui2s";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 7;       /* type of conversion to perform 7=short integer */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    uint8      *tmp_dst;        /* temporary buffer to hold byte swapped values */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    if (source == dest || num_elm == 0)     /* Inplace conversions  not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* No elements to convert is an error */

    /* Find out if it is OK to use faster array processing */
    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
          tmp_dst = (uint8 *) HDmalloc(2 * num_elm);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          DFKswap(source, tmp_dst, 2, num_elm);
          bitoff = ((unsigned int)source) >> BITOFF;
          ierr = IEG2CRAY(&type, &num_elm, tmp_dst, &bitoff, dest);
          HDfree(tmp_dst);
          if (ierr != 0)
              HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
      }     /* end for */
    else
      {     /* Generic stride processing */
#ifdef NOINTCRAY2IEG
          for (i = 0; i < num_elm; i++)
            {
                if ((source[1] & 0x80))
                  {     /* Can't forget to extend sign */
                      dest[0] = 0xff;
                      dest[1] = 0xff;
                      dest[2] = 0xff;
                      dest[3] = 0xff;
                      dest[4] = 0xff;
                      dest[5] = 0xff;
                  }     /* end if */
                else
                  {
                      dest[0] = 0x00;
                      dest[1] = 0x00;
                      dest[2] = 0x00;
                      dest[3] = 0x00;
                      dest[4] = 0x00;
                      dest[5] = 0x00;
                  }     /* end else */
                dest[6] = source[1];
                dest[7] = source[0];
                source += source_stride;
                dest += dest_stride;
            }   /* end for */
#else
          tmp_dst = (uint8 *) HDmalloc(2);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          for (i = 0; i < num_elm; i++)
            {
                DFKswap(source, tmp_dst, 2, 1);
                bitoff = ((unsigned int)source) >> BITOFF;
                ierr = IEG2CRAY(&type, &n_elem, tmp_dst, &bitoff, dest);
                if (ierr != 0)
                  {
                      HDfree(tmp_dst);
                      HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
                  }     /* end if */
                source += source_stride;
                dest += dest_stride;
            }   /* end for */
          HDfree(tmp_dst);
#endif
      }     /* end else */
    return (SUCCEED);
}

#define LUO2I_MASKA 0x00000000000000ff
#define LUO2I_MASKB 0x000000000000ff00

/************************************************************/
/* DFKluo2i()                                               */
/* -->Unicos routine for exporting 2 byte little-endian     */
/*      data items                                          */
/************************************************************/
int
DFKluo2i(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    uint32 i;
    int         fast_processing = 0;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    long       *lp_dest;
    long       *lp_src;
    char       *FUNC = "DFKuo2i";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 7;       /* type of conversion to perform 7=short integer */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    uint8      *tmp_dst;        /* temporary buffer to hold byte swapped values */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    if (source == dest || num_elm == 0)     /* Inplace conversions  not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* No elements to convert is an error */

    /* Find out if it is OK to use faster array processing */
    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
#ifdef DOESNT_WORK
          tmp_dst = (uint8 *) HDmalloc(2 * num_elm);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          bitoff = ((unsigned int)dest) >> BITOFF;
          ierr = CRAY2IEG(&type, &num_elm, tmp_dst, &bitoff, source);
          if (ierr != 0)
            {
                HDfree(tmp_dst);
                HRETURN_ERROR(DFE_BADCONV, FAIL);   /* error in Cray conversion */
            }   /* end if */
          DFKswap(tmp_dst, dest, 2, num_elm);
          HDfree(tmp_dst);
#else
#ifndef DUFF_luo2i
#if defined TEST1_luo2i
          int         odd_man_out;  /* By default there are even num_elm */
          intn        n;

          odd_man_out = num_elm % 4;

          n = num_elm / 4;
          lp_dest = (long *) dest;
          lp_src = (long *) source;
          for (i = 0; i < n; i++)
            {
                *lp_dest++ = ((lp_src[0] & LUO2I_MASKA) << 56) |
                    ((lp_src[0] & LUO2I_MASKB) << 40) |
                    ((lp_src[1] & LUO2I_MASKA) << 40) |
                    ((lp_src[1] & LUO2I_MASKB) << 24) |
                    ((lp_src[2] & LUO2I_MASKA) << 24) |
                    ((lp_src[2] & LUO2I_MASKB) << 8) |
                    ((lp_src[3] & LUO2I_MASKA) << 8) |
                    ((lp_src[3] & LUO2I_MASKB) >> 8);
                lp_src += 4;
            }
          switch (odd_man_out)
            {   /* clean up leftovers */
                case 3:
                    *lp_dest = ((lp_src[0] & LUO2I_MASKA) << 56) |
                        ((lp_src[0] & LUO2I_MASKB) << 40) |
                        ((lp_src[1] & LUO2I_MASKA) << 40) |
                        ((lp_src[1] & LUO2I_MASKB) << 24) |
                        ((lp_src[2] & LUO2I_MASKA) << 24) |
                        ((lp_src[2] & LUO2I_MASKB) << 8);
                    break;

                case 2:
                    *lp_dest = ((lp_src[0] & LUO2I_MASKA) << 56) |
                        ((lp_src[0] & LUO2I_MASKB) << 40) |
                        ((lp_src[1] & LUO2I_MASKA) << 40) |
                        ((lp_src[1] & LUO2I_MASKB) << 24);
                    break;

                case 1:
                    *lp_dest = ((lp_src[0] & LUO2I_MASKA) << 56) |
                        ((lp_src[0] & LUO2I_MASKB) << 40);
                    break;

                case 0:
                    break;
            }   /* end switch */
#else
          for (i = 0; i < num_elm; i++)
            {
                dest[0] = source[7];
                dest[1] = source[6];
                dest += 2;
                source += 8;
            }
#endif
#else  /* DUFF_luo2i */
          uintn       n;
          int         odd_man_out;  /* By default there are even num_elm */
          uintn       orig_num_elm = num_elm;

          odd_man_out = num_elm % 4;

          num_elm /= 4;
          n = (num_elm + 7) / 8;
          lp_dest = (long *) dest;
          lp_src = (long *) source;
          if (orig_num_elm > 3)
              switch (num_elm % 8)
                {
                    case 0:
                        do
                          {
                              *lp_dest++ = ((lp_src[0] & LUO2I_MASKA) << 56) |
                                  ((lp_src[0] & LUO2I_MASKB) << 40) |
                                  ((lp_src[1] & LUO2I_MASKA) << 40) |
                                  ((lp_src[1] & LUO2I_MASKB) << 24) |
                                  ((lp_src[2] & LUO2I_MASKA) << 24) |
                                  ((lp_src[2] & LUO2I_MASKB) << 8) |
                                  ((lp_src[3] & LUO2I_MASKA) << 8) |
                                  ((lp_src[3] & LUO2I_MASKB) >> 8);
                              lp_src += 4;
                    case 7:
                              *lp_dest++ = ((lp_src[0] & LUO2I_MASKA) << 56) |
                                  ((lp_src[0] & LUO2I_MASKB) << 40) |
                                  ((lp_src[1] & LUO2I_MASKA) << 40) |
                                  ((lp_src[1] & LUO2I_MASKB) << 24) |
                                  ((lp_src[2] & LUO2I_MASKA) << 24) |
                                  ((lp_src[2] & LUO2I_MASKB) << 8) |
                                  ((lp_src[3] & LUO2I_MASKA) << 8) |
                                  ((lp_src[3] & LUO2I_MASKB) >> 8);
                              lp_src += 4;
                    case 6:
                              *lp_dest++ = ((lp_src[0] & LUO2I_MASKA) << 56) |
                                  ((lp_src[0] & LUO2I_MASKB) << 40) |
                                  ((lp_src[1] & LUO2I_MASKA) << 40) |
                                  ((lp_src[1] & LUO2I_MASKB) << 24) |
                                  ((lp_src[2] & LUO2I_MASKA) << 24) |
                                  ((lp_src[2] & LUO2I_MASKB) << 8) |
                                  ((lp_src[3] & LUO2I_MASKA) << 8) |
                                  ((lp_src[3] & LUO2I_MASKB) >> 8);
                              lp_src += 4;
                    case 5:
                              *lp_dest++ = ((lp_src[0] & LUO2I_MASKA) << 56) |
                                  ((lp_src[0] & LUO2I_MASKB) << 40) |
                                  ((lp_src[1] & LUO2I_MASKA) << 40) |
                                  ((lp_src[1] & LUO2I_MASKB) << 24) |
                                  ((lp_src[2] & LUO2I_MASKA) << 24) |
                                  ((lp_src[2] & LUO2I_MASKB) << 8) |
                                  ((lp_src[3] & LUO2I_MASKA) << 8) |
                                  ((lp_src[3] & LUO2I_MASKB) >> 8);
                              lp_src += 4;
                    case 4:
                              *lp_dest++ = ((lp_src[0] & LUO2I_MASKA) << 56) |
                                  ((lp_src[0] & LUO2I_MASKB) << 40) |
                                  ((lp_src[1] & LUO2I_MASKA) << 40) |
                                  ((lp_src[1] & LUO2I_MASKB) << 24) |
                                  ((lp_src[2] & LUO2I_MASKA) << 24) |
                                  ((lp_src[2] & LUO2I_MASKB) << 8) |
                                  ((lp_src[3] & LUO2I_MASKA) << 8) |
                                  ((lp_src[3] & LUO2I_MASKB) >> 8);
                              lp_src += 4;
                    case 3:
                              *lp_dest++ = ((lp_src[0] & LUO2I_MASKA) << 56) |
                                  ((lp_src[0] & LUO2I_MASKB) << 40) |
                                  ((lp_src[1] & LUO2I_MASKA) << 40) |
                                  ((lp_src[1] & LUO2I_MASKB) << 24) |
                                  ((lp_src[2] & LUO2I_MASKA) << 24) |
                                  ((lp_src[2] & LUO2I_MASKB) << 8) |
                                  ((lp_src[3] & LUO2I_MASKA) << 8) |
                                  ((lp_src[3] & LUO2I_MASKB) >> 8);
                              lp_src += 4;
                    case 2:
                              *lp_dest++ = ((lp_src[0] & LUO2I_MASKA) << 56) |
                                  ((lp_src[0] & LUO2I_MASKB) << 40) |
                                  ((lp_src[1] & LUO2I_MASKA) << 40) |
                                  ((lp_src[1] & LUO2I_MASKB) << 24) |
                                  ((lp_src[2] & LUO2I_MASKA) << 24) |
                                  ((lp_src[2] & LUO2I_MASKB) << 8) |
                                  ((lp_src[3] & LUO2I_MASKA) << 8) |
                                  ((lp_src[3] & LUO2I_MASKB) >> 8);
                              lp_src += 4;
                    case 1:
                              *lp_dest++ = ((lp_src[0] & LUO2I_MASKA) << 56) |
                                  ((lp_src[0] & LUO2I_MASKB) << 40) |
                                  ((lp_src[1] & LUO2I_MASKA) << 40) |
                                  ((lp_src[1] & LUO2I_MASKB) << 24) |
                                  ((lp_src[2] & LUO2I_MASKA) << 24) |
                                  ((lp_src[2] & LUO2I_MASKB) << 8) |
                                  ((lp_src[3] & LUO2I_MASKA) << 8) |
                                  ((lp_src[3] & LUO2I_MASKB) >> 8);
                              lp_src += 4;
                          }
                        while (--n > 0);
                }
          switch (odd_man_out)
            {   /* clean up leftovers */
                case 3:
                    *lp_dest = ((lp_src[0] & LUO2I_MASKA) << 56) |
                        ((lp_src[0] & LUO2I_MASKB) << 40) |
                        ((lp_src[1] & LUO2I_MASKA) << 40) |
                        ((lp_src[1] & LUO2I_MASKB) << 24) |
                        ((lp_src[2] & LUO2I_MASKA) << 24) |
                        ((lp_src[2] & LUO2I_MASKB) << 8);
                    break;

                case 2:
                    *lp_dest = ((lp_src[0] & LUO2I_MASKA) << 56) |
                        ((lp_src[0] & LUO2I_MASKB) << 40) |
                        ((lp_src[1] & LUO2I_MASKA) << 40) |
                        ((lp_src[1] & LUO2I_MASKB) << 24);
                    break;

                case 1:
                    *lp_dest = ((lp_src[0] & LUO2I_MASKA) << 56) |
                        ((lp_src[0] & LUO2I_MASKB) << 40);
                    break;

                default:
                    break;
            }   /* end switch */
#endif /* DUFF_luo2i */
#endif
      }     /* end if */
    else
      {     /* Generic Stride processing */
#ifdef NOINTCRAY2IEG
          for (i = 0; i < num_elm; i++)
            {
                dest[0] = source[7];
                dest[1] = source[6];
                source += source_stride;
                dest += dest_stride;
            }   /* end for */
#else
          tmp_dst = (uint8 *) HDmalloc(2);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          for (i = 0; i < num_elm; i++)
            {
                bitoff = ((unsigned int)dest) >> BITOFF;
                ierr = CRAY2IEG(&type, &n_elem, tmp_dst, &bitoff, source);
                if (ierr != 0)
                  {
                      HDfree(tmp_dst);
                      HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
                  }     /* end if */
                DFKswap(tmp_dst, dest, 2, 1);
                source += source_stride;
                dest += dest_stride;
            }   /* end for */
          HDfree(tmp_dst);
#endif
      }     /* end else */
    return (SUCCEED);
}

#define LUO2S_MASKA 0x00000000000000ff
#define LUO2S_MASKB 0x000000000000ff00

/************************************************************/
/* DFKluo2s()                                               */
/* -->Unicos routine for exporting signed 2 byte            */
/*      little-endian data items                            */
/************************************************************/
int
DFKluo2s(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    uint32 i;
    int         fast_processing = 0;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    long       *lp_dest;
    long       *lp_src;
    char       *FUNC = "DFKuo2s";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 7;       /* type of conversion to perform 7=short integer */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    uint8      *tmp_dst;        /* temporary buffer to hold byte swapped values */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    if (source == dest || num_elm == 0)     /* Inplace conversions  not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* No elements to convert is an error */

    /* Find out if it is OK to use faster array processing */
    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
          tmp_dst = (uint8 *) HDmalloc(2 * num_elm);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          bitoff = ((unsigned int)dest) >> BITOFF;
          ierr = CRAY2IEG(&type, &num_elm, tmp_dst, &bitoff, source);
          if (ierr != 0)
            {
                HDfree(tmp_dst);
                HRETURN_ERROR(DFE_BADCONV, FAIL);   /* error in Cray conversion */
            }   /* end if */
          DFKswap(tmp_dst, dest, 2, num_elm);
          HDfree(tmp_dst);
      }     /* end if */
    else
      {     /* Generic Stride processing */
#ifdef NOINTCRAY2IEG
          for (i = 0; i < num_elm; i++)
            {
                dest[0] = source[7];
                dest[1] = source[6];
                source += source_stride;
                dest += dest_stride;
            }   /* end for */
#else
          tmp_dst = (uint8 *) HDmalloc(2);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          for (i = 0; i < num_elm; i++)
            {
                bitoff = ((unsigned int)dest) >> BITOFF; 
                ierr = CRAY2IEG(&type, &n_elem, tmp_dst, &bitoff, source);
                if (ierr != 0)
                  {
                      HDfree(tmp_dst);
                      HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
                  }     /* end if */
                DFKswap(tmp_dst, dest, 2, 1);
                source += source_stride;
                dest += dest_stride;
            }   /* end for */
          HDfree(tmp_dst);
#endif
      }     /* end else */
    return (SUCCEED);
}

#define LUI4I_MASKA 0xff00000000000000
#define LUI4I_MASKB 0x00ff000000000000
#define LUI4I_MASKC 0x0000ff0000000000
#define LUI4I_MASKD 0x000000ff00000000
#define LUI4I_MASKE 0x00000000ff000000
#define LUI4I_MASKF 0x0000000000ff0000
#define LUI4I_MASKG 0x000000000000ff00
#define LUI4I_MASKH 0x00000000000000ff

/************************************************************/
/* DFKlui4i()                                               */
/* -->Unicos routine for importing 4 byte little-endian     */
/*      unsigned ints                                       */
/************************************************************/
int
DFKlui4i(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    int         fast_processing = 0;
    uint32 i;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    unsigned long       *lptr_dest = (unsigned long *) dest;
    unsigned long       *lp_dest;
    unsigned long       *lp_src;
    char       *FUNC = "DFKui4i";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 1;       /* type of conversion to perform 1=integer */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    uint8      *tmp_dst;        /* temporary buffer to hold byte swapped values */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    if (source == dest || num_elm == 0)     /* Inplace conversions  not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* No elements to convert is an error */

    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
#ifdef DOESNT_WORK
          tmp_dst = (uint8 *) HDmalloc(4 * num_elm);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          DFKswap(source, tmp_dst, 4, num_elm);
          bitoff = ((unsigned int)source) >> BITOFF;
          ierr = IEG2CRAY(&type, &num_elm, tmp_dst, &bitoff, dest);
          HDfree(tmp_dst);
          if (ierr != 0)
              HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
#else
#ifndef DUFF_lui4i
#if defined TEST2_lui4i
          int         odd_man_out;  /* By default there are even num_elm */
          intn        n;

          odd_man_out = num_elm % 2;

          n = num_elm / 2;
          lp_dest = (unsigned long *) dest;
          lp_src = (unsigned long *) source;
          HDmemset(lp_dest, 0, num_elm * sizeof(long));     /* initialize to zeros */
          for (i = 0; i < n; i++)
            {
                lp_dest[0] = (lp_src[0] & LUI4I_MASKA) >> 56 |
                    (lp_src[0] & LUI4I_MASKB) >> 40 |
                    (lp_src[0] & LUI4I_MASKC) >> 24 |
                    (lp_src[0] & LUI4I_MASKD) >> 8;
                lp_dest[1] = (lp_src[0] & LUI4I_MASKE) >> 24 |
                    (lp_src[0] & LUI4I_MASKF) >> 8 |
                    (lp_src[0] & LUI4I_MASKG) << 8 |
                    (lp_src[0] & LUI4I_MASKH) << 24;
                lp_dest += 2;
                lp_src++;
            }   /* end for */
          if (odd_man_out)
              *lp_dest = (lp_src[0] & LUI4I_MASKA) >> 56 |
                  (lp_src[0] & LUI4I_MASKB) >> 40 |
                  (lp_src[0] & LUI4I_MASKC) >> 24 |
                  (lp_src[0] & LUI4I_MASKD) >> 8;
#elif defined TEST1_lui4i
          int         odd_man_out;  /* By default there are even num_elm */
          intn        n;

          odd_man_out = num_elm % 2;

          n = num_elm / 2;
          lp_dest = (unsigned long *) dest;
          lp_src = (unsigned long *) source;
          HDmemset(lp_dest, 0, num_elm * sizeof(long));     /* initialize to zeros */
          for (i = 0; i < n; i++)
            {
                *lp_dest++ = (lp_src[0] & LUI4I_MASKA) >> 56 |
                    (lp_src[0] & LUI4I_MASKB) >> 40 |
                    (lp_src[0] & LUI4I_MASKC) >> 24 |
                    (lp_src[0] & LUI4I_MASKD) >> 8;
                *lp_dest++ = (lp_src[0] & LUI4I_MASKE) >> 24 |
                    (lp_src[0] & LUI4I_MASKF) >> 8 |
                    (lp_src[0] & LUI4I_MASKG) << 8 |
                    (lp_src[0] & LUI4I_MASKH) << 24;
                lp_src++;
            }   /* end for */
          if (odd_man_out)
              *lp_dest++ = (lp_src[0] & LUI4I_MASKA) >> 56 |
                  (lp_src[0] & LUI4I_MASKB) >> 40 |
                  (lp_src[0] & LUI4I_MASKC) >> 24 |
                  (lp_src[0] & LUI4I_MASKD) >> 8;
#else
          for (i = 0; i < num_elm; i++)
            {
                lptr_dest[0] = 0;
                dest[4] = source[3];
                dest[5] = source[2];
                dest[6] = source[1];
                dest[7] = source[0];
                source += 4;
                lptr_dest++;
                dest = (uint8 *) lptr_dest;
            }
#endif
#else  /* DUFF_lui4i */
          uintn       n;
          int         odd_man_out;  /* By default there are even num_elm */
          uintn       orig_num_elm = num_elm;

          lp_dest = (unsigned long *) dest;
          lp_src = (unsigned long *) source;
          HDmemset(lp_dest, 0, num_elm * sizeof(long));     /* initialize to zeros */

          odd_man_out = num_elm % 2;

          if (orig_num_elm > 1) {
		num_elm /= 2;

		/* The loop below has been unrolled in interations of 8 */
		while (num_elm > 7) {
                              lp_dest[0] = (lp_src[0] & LUI4I_MASKA) >> 56 |
                                  (lp_src[0] & LUI4I_MASKB) >> 40 |
                                  (lp_src[0] & LUI4I_MASKC) >> 24 |
                                  (lp_src[0] & LUI4I_MASKD) >> 8;
                              lp_dest[1] = (lp_src[0] & LUI4I_MASKE) >> 24 |
                                  (lp_src[0] & LUI4I_MASKF) >> 8 |
                                  (lp_src[0] & LUI4I_MASKG) << 8 |
                                  (lp_src[0] & LUI4I_MASKH) << 24;
                              lp_dest += 2;
                              lp_src++;

                              lp_dest[0] = (lp_src[0] & LUI4I_MASKA) >> 56 |
                                  (lp_src[0] & LUI4I_MASKB) >> 40 |
                                  (lp_src[0] & LUI4I_MASKC) >> 24 |
                                  (lp_src[0] & LUI4I_MASKD) >> 8;
                              lp_dest[1] = (lp_src[0] & LUI4I_MASKE) >> 24 |
                                  (lp_src[0] & LUI4I_MASKF) >> 8 |
                                  (lp_src[0] & LUI4I_MASKG) << 8 |
                                  (lp_src[0] & LUI4I_MASKH) << 24;
                              lp_dest += 2;
                              lp_src++;

                              lp_dest[0] = (lp_src[0] & LUI4I_MASKA) >> 56 |
                                  (lp_src[0] & LUI4I_MASKB) >> 40 |
                                  (lp_src[0] & LUI4I_MASKC) >> 24 |
                                  (lp_src[0] & LUI4I_MASKD) >> 8;
                              lp_dest[1] = (lp_src[0] & LUI4I_MASKE) >> 24 |
                                  (lp_src[0] & LUI4I_MASKF) >> 8 |
                                  (lp_src[0] & LUI4I_MASKG) << 8 |
                                  (lp_src[0] & LUI4I_MASKH) << 24;
                              lp_dest += 2;
                              lp_src++;

                              lp_dest[0] = (lp_src[0] & LUI4I_MASKA) >> 56 |
                                  (lp_src[0] & LUI4I_MASKB) >> 40 |
                                  (lp_src[0] & LUI4I_MASKC) >> 24 |
                                  (lp_src[0] & LUI4I_MASKD) >> 8;
                              lp_dest[1] = (lp_src[0] & LUI4I_MASKE) >> 24 |
                                  (lp_src[0] & LUI4I_MASKF) >> 8 |
                                  (lp_src[0] & LUI4I_MASKG) << 8 |
                                  (lp_src[0] & LUI4I_MASKH) << 24;
                              lp_dest += 2;
                              lp_src++;

                              lp_dest[0] = (lp_src[0] & LUI4I_MASKA) >> 56 |
                                  (lp_src[0] & LUI4I_MASKB) >> 40 |
                                  (lp_src[0] & LUI4I_MASKC) >> 24 |
                                  (lp_src[0] & LUI4I_MASKD) >> 8;
                              lp_dest[1] = (lp_src[0] & LUI4I_MASKE) >> 24 |
                                  (lp_src[0] & LUI4I_MASKF) >> 8 |
                                  (lp_src[0] & LUI4I_MASKG) << 8 |
                                  (lp_src[0] & LUI4I_MASKH) << 24;
                              lp_dest += 2;
                              lp_src++;

                              lp_dest[0] = (lp_src[0] & LUI4I_MASKA) >> 56 |
                                  (lp_src[0] & LUI4I_MASKB) >> 40 |
                                  (lp_src[0] & LUI4I_MASKC) >> 24 |
                                  (lp_src[0] & LUI4I_MASKD) >> 8;
                              lp_dest[1] = (lp_src[0] & LUI4I_MASKE) >> 24 |
                                  (lp_src[0] & LUI4I_MASKF) >> 8 |
                                  (lp_src[0] & LUI4I_MASKG) << 8 |
                                  (lp_src[0] & LUI4I_MASKH) << 24;
                              lp_dest += 2;
                              lp_src++;

                              lp_dest[0] = (lp_src[0] & LUI4I_MASKA) >> 56 |
                                  (lp_src[0] & LUI4I_MASKB) >> 40 |
                                  (lp_src[0] & LUI4I_MASKC) >> 24 |
                                  (lp_src[0] & LUI4I_MASKD) >> 8;
                              lp_dest[1] = (lp_src[0] & LUI4I_MASKE) >> 24 |
                                  (lp_src[0] & LUI4I_MASKF) >> 8 |
                                  (lp_src[0] & LUI4I_MASKG) << 8 |
                                  (lp_src[0] & LUI4I_MASKH) << 24;
                              lp_dest += 2;
                              lp_src++;

                              lp_dest[0] = (lp_src[0] & LUI4I_MASKA) >> 56 |
                                  (lp_src[0] & LUI4I_MASKB) >> 40 |
                                  (lp_src[0] & LUI4I_MASKC) >> 24 |
                                  (lp_src[0] & LUI4I_MASKD) >> 8;
                              lp_dest[1] = (lp_src[0] & LUI4I_MASKE) >> 24 |
                                  (lp_src[0] & LUI4I_MASKF) >> 8 |
                                  (lp_src[0] & LUI4I_MASKG) << 8 |
                                  (lp_src[0] & LUI4I_MASKH) << 24;
                              lp_dest += 2;
                              lp_src++;
			num_elm -= 8;
                };

		/* The loop below is a cleanup of iterations leftover modulo 8 */
		while (num_elm > 0) {
                              lp_dest[0] = (lp_src[0] & LUI4I_MASKA) >> 56 |
                                  (lp_src[0] & LUI4I_MASKB) >> 40 |
                                  (lp_src[0] & LUI4I_MASKC) >> 24 |
                                  (lp_src[0] & LUI4I_MASKD) >> 8;
                              lp_dest[1] = (lp_src[0] & LUI4I_MASKE) >> 24 |
                                  (lp_src[0] & LUI4I_MASKF) >> 8 |
                                  (lp_src[0] & LUI4I_MASKG) << 8 |
                                  (lp_src[0] & LUI4I_MASKH) << 24;
                              lp_dest += 2;
                              lp_src++;
			num_elm -= 1;
                };
          }
          if (odd_man_out)
              *lp_dest = (lp_src[0] & LUI4I_MASKA) >> 56 |
                  (lp_src[0] & LUI4I_MASKB) >> 40 |
                  (lp_src[0] & LUI4I_MASKC) >> 24 |
                  (lp_src[0] & LUI4I_MASKD) >> 8;
#endif /* DUFF_lui4i */
#endif
      }     /* end if */
    else
      {
#ifdef NOINTCRAY2IEG
          for (i = 0; i < num_elm; i++)
            {
                dest[0] = 0;
                dest[1] = 0;
                dest[2] = 0;
                dest[3] = 0;
                dest[4] = source[3];
                dest[5] = source[2];
                dest[6] = source[1];
                dest[7] = source[0];
                dest += dest_stride;
                source += source_stride;
            }   /* end for */
#else
          tmp_dst = (uint8 *) HDmalloc(4);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          for (i = 0; i < num_elm; i++)
            {
                DFKswap(source, tmp_dst, 4, 1);
                bitoff = ((unsigned int)source) >> BITOFF;
                ierr = IEG2CRAY(&type, &n_elem, tmp_dst, &bitoff, dest);
                if (ierr != 0)
                  {
                      HDfree(tmp_dst);
                      HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
                  }     /* end if */
                dest += dest_stride;
                source += source_stride;
            }   /* end for */
          HDfree(tmp_dst);
#endif
      }     /* end else */
    return (SUCCEED);
}

#define LUI4S_MASKA 0xff00000000000000
#define LUI4S_MASKB 0x00ff000000000000
#define LUI4S_MASKC 0x0000ff0000000000
#define LUI4S_MASKD 0x000000ff00000000
#define LUI4S_MASKE 0x00000000ff000000
#define LUI4S_MASKF 0x0000000000ff0000
#define LUI4S_MASKG 0x000000000000ff00
#define LUI4S_MASKH 0x00000000000000ff
#define LUI4S_MASKI 0x0000008000000000
#define LUI4S_MASKJ 0x0000000000000080
#define LUI4S_MASKK 0xffffffff00000000

/************************************************************/
/* DFKlui4s()                                                */
/* -->Unicos routine for importing 4 signed ints            */
/************************************************************/
int
DFKlui4s(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    int         fast_processing = 0;
    uint32 i;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    long       *lptr_dest = (long *) dest;
    long       *lp_dest;
    long       *lp_src;
    char       *FUNC = "DFKui4s";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 1;       /* type of conversion to perform 1=integer */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    uint8      *tmp_dst;        /* temporary buffer to hold byte swapped values */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    if (source == dest || num_elm == 0)     /* Inplace conversions  not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* No elements to convert is an error */

    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
          tmp_dst = (uint8 *) HDmalloc(4 * num_elm);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          DFKswap(source, tmp_dst, 4, num_elm);
          bitoff = ((unsigned int)source) >> BITOFF;
          ierr = IEG2CRAY(&type, &num_elm, tmp_dst, &bitoff, dest);
          HDfree(tmp_dst);
          if (ierr != 0)
              HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
      }     /* end if */
    else
      {
#ifdef NOINTCRAY2IEG
          for (i = 0; i < num_elm; i++)
            {
                if ((source[3] & 0x80))
                  {     /* Can't forget to sign extend */
                      dest[0] = 0xff;
                      dest[1] = 0xff;
                      dest[2] = 0xff;
                      dest[3] = 0xff;
                  }     /* end if */
                else
                  {
                      dest[0] = 0;
                      dest[1] = 0;
                      dest[2] = 0;
                      dest[3] = 0;
                  }     /* end else */
                dest[4] = source[3];
                dest[5] = source[2];
                dest[6] = source[1];
                dest[7] = source[0];
                dest += dest_stride;
                source += source_stride;
            }   /* end for */
#else
          tmp_dst = (uint8 *) HDmalloc(4);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          for (i = 0; i < num_elm; i++)
            {
                DFKswap(source, tmp_dst, 4, 1);
                bitoff = ((unsigned int)source) >> BITOFF;
                ierr = IEG2CRAY(&type, &n_elem, tmp_dst, &bitoff, dest);
                if (ierr != 0)
                  {
                      HDfree(tmp_dst);
                      HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
                  }     /* end if */
                dest += dest_stride;
                source += source_stride;
            }   /* end for */
          HDfree(tmp_dst);
#endif
      }     /* end else */
    return (SUCCEED);
}

#define LUO4I_MASKA 0x00000000ff00ff00
#define LUO4I_MASKB 0x0000000000ff00ff
#define LUO4I_MASKC 0xffff0000ffff0000
#define LUO4I_MASKD 0x0000ffff0000ffff

/************************************************************/
/* DFKluo4i()                                                */
/* -->Unicos routine for exporting 4 byte data items        */
/************************************************************/
int
DFKluo4i(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    int         fast_processing = 0;
    uintn i;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    unsigned long *lp_dest;
    unsigned long *lp_src;
    char       *FUNC = "DFKuo4i";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 1;       /* type of conversion to perform 1=integer */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    uint8      *tmp_dst;        /* temporary buffer to hold byte swapped values */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    if (source == dest || num_elm == 0)     /* Inplace conversions  not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* No elements to convert is an error */

    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
#ifdef DOESNT_WORK
          tmp_dst = (uint8 *) HDmalloc(4 * num_elm);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          bitoff = ((unsigned int)dest) >> BITOFF;
          ierr = CRAY2IEG(&type, &num_elm, tmp_dst, &bitoff, source);
          if (ierr != 0)
            {
                HDfree(tmp_dst);
                HRETURN_ERROR(DFE_BADCONV, FAIL);   /* error in Cray conversion */
            }   /* end if */
          DFKswap(tmp_dst, dest, 4, num_elm);
          HDfree(tmp_dst);
#else
#ifndef DUFF_luo4i
#define TEST2_luo4i
#if defined TEST2_luo4i
          int         odd_man_out = 0;  /* By default there are even num_elm */
          intn        n;

          if (num_elm % 2)  /* If this is true, we have odd num */
              odd_man_out = 1;

          n = num_elm / 2;
          lp_dest = (unsigned long *) dest;
          lp_src = (unsigned long *) source;
          for (i = 0; i < n; i++)
            {
                *lp_dest = ((lp_src[0] & LUO4I_MASKA) << 24) | ((lp_src[0] & LUO4I_MASKB) << 40) |
                    ((lp_src[1] & LUO4I_MASKA) >> 8) | ((lp_src[1] & LUO4I_MASKB) << 8);
                *lp_dest = ((*lp_dest & LUO4I_MASKC) >> 16) | ((*lp_dest & LUO4I_MASKD) << 16);
                lp_dest++;
                lp_src += 2;
            }
          if (odd_man_out)
            {
                *lp_dest = ((lp_src[0] & LUO4I_MASKA) << 24) | ((lp_src[0] & LUO4I_MASKB) << 40);
                *lp_dest = ((*lp_dest & LUO4I_MASKC) >> 16) | ((*lp_dest & LUO4I_MASKD) << 16);
            }   /* end if */
#else
          for (i = 0; i < num_elm; i++)
            {
                dest[0] = source[7];
                dest[1] = source[6];
                dest[2] = source[5];
                dest[3] = source[4];
                dest += 4;
                source += 8;
            }
#endif
#else  /* DUFF_luo4i */
       uintn n;
       int         odd_man_out = 0;  /* By default there are even num_elm */
       uintn       orig_num_elm = num_elm;

       if (num_elm % 2)  /* If this is true, we have odd num */
              odd_man_out = 1;

        lp_dest = (unsigned long *) dest;
        lp_src = (unsigned long *) source;
	if (orig_num_elm > 1) {
		num_elm /= 2;

		/* The loop below has been unrolled in interations of 8 */
		while (num_elm > 7) {
                              *lp_dest = ((lp_src[0] & LUO4I_MASKA) << 24) |
                                  ((lp_src[0] & LUO4I_MASKB) << 40) |
                                  ((lp_src[1] & LUO4I_MASKA) >> 8) |
                                  ((lp_src[1] & LUO4I_MASKB) << 8);
                              *lp_dest = ((*lp_dest & LUO4I_MASKC) >> 16) |
                                  ((*lp_dest & LUO4I_MASKD) << 16);
                              lp_dest++;
                              lp_src += 2;

                              *lp_dest = ((lp_src[0] & LUO4I_MASKA) << 24) |
                                  ((lp_src[0] & LUO4I_MASKB) << 40) |
                                  ((lp_src[1] & LUO4I_MASKA) >> 8) |
                                  ((lp_src[1] & LUO4I_MASKB) << 8);
                              *lp_dest = ((*lp_dest & LUO4I_MASKC) >> 16) |
                                  ((*lp_dest & LUO4I_MASKD) << 16);
                              lp_dest++;
                              lp_src += 2;

                              *lp_dest = ((lp_src[0] & LUO4I_MASKA) << 24) |
                                  ((lp_src[0] & LUO4I_MASKB) << 40) |
                                  ((lp_src[1] & LUO4I_MASKA) >> 8) |
                                  ((lp_src[1] & LUO4I_MASKB) << 8);
                              *lp_dest = ((*lp_dest & LUO4I_MASKC) >> 16) |
                                  ((*lp_dest & LUO4I_MASKD) << 16);
                              lp_dest++;
                              lp_src += 2;

                              *lp_dest = ((lp_src[0] & LUO4I_MASKA) << 24) |
                                  ((lp_src[0] & LUO4I_MASKB) << 40) |
                                  ((lp_src[1] & LUO4I_MASKA) >> 8) |
                                  ((lp_src[1] & LUO4I_MASKB) << 8);
                              *lp_dest = ((*lp_dest & LUO4I_MASKC) >> 16) |
                                  ((*lp_dest & LUO4I_MASKD) << 16);
                              lp_dest++;
                              lp_src += 2;

                              *lp_dest = ((lp_src[0] & LUO4I_MASKA) << 24) |
                                  ((lp_src[0] & LUO4I_MASKB) << 40) |
                                  ((lp_src[1] & LUO4I_MASKA) >> 8) |
                                  ((lp_src[1] & LUO4I_MASKB) << 8);
                              *lp_dest = ((*lp_dest & LUO4I_MASKC) >> 16) |
                                  ((*lp_dest & LUO4I_MASKD) << 16);
                              lp_dest++;
                              lp_src += 2;

                              *lp_dest = ((lp_src[0] & LUO4I_MASKA) << 24) |
                                  ((lp_src[0] & LUO4I_MASKB) << 40) |
                                  ((lp_src[1] & LUO4I_MASKA) >> 8) |
                                  ((lp_src[1] & LUO4I_MASKB) << 8);
                              *lp_dest = ((*lp_dest & LUO4I_MASKC) >> 16) |
                                  ((*lp_dest & LUO4I_MASKD) << 16);
                              lp_dest++;
                              lp_src += 2;

                              *lp_dest = ((lp_src[0] & LUO4I_MASKA) << 24) |
                                  ((lp_src[0] & LUO4I_MASKB) << 40) |
                                  ((lp_src[1] & LUO4I_MASKA) >> 8) |
                                  ((lp_src[1] & LUO4I_MASKB) << 8);
                              *lp_dest = ((*lp_dest & LUO4I_MASKC) >> 16) |
                                  ((*lp_dest & LUO4I_MASKD) << 16);
                              lp_dest++;
                              lp_src += 2;

                              *lp_dest = ((lp_src[0] & LUO4I_MASKA) << 24) |
                                  ((lp_src[0] & LUO4I_MASKB) << 40) |
                                  ((lp_src[1] & LUO4I_MASKA) >> 8) |
                                  ((lp_src[1] & LUO4I_MASKB) << 8);
                              *lp_dest = ((*lp_dest & LUO4I_MASKC) >> 16) |
                                  ((*lp_dest & LUO4I_MASKD) << 16);
                              lp_dest++;
                              lp_src += 2;

			num_elm -= 8;
		};

		/* The loop below is a cleanup of iterations leftover modulo 8 */
		while (num_elm > 0) {
                              *lp_dest = ((lp_src[0] & LUO4I_MASKA) << 24) |
                                  ((lp_src[0] & LUO4I_MASKB) << 40) |
                                  ((lp_src[1] & LUO4I_MASKA) >> 8) |
                                  ((lp_src[1] & LUO4I_MASKB) << 8);
                              *lp_dest = ((*lp_dest & LUO4I_MASKC) >> 16) |
                                  ((*lp_dest & LUO4I_MASKD) << 16);
                              lp_dest++;
                              lp_src += 2;
			num_elm--;
		};
	}
          if (odd_man_out)
            {
                *lp_dest = ((lp_src[0] & LUO4I_MASKA) << 24) |
                    ((lp_src[0] & LUO4I_MASKB) << 40);
                *lp_dest = ((*lp_dest & LUO4I_MASKC) >> 16) |
                    ((*lp_dest & LUO4I_MASKD) << 16);
            }   /* end if */
#endif /* DUFF_luo4i */
#endif
      }     /* end if */
    else
      {
#ifdef NOINTCRAY2IEG
          for (i = 0; i < num_elm; i++)
            {
                dest[0] = source[7];
                dest[1] = source[6];
                dest[2] = source[5];
                dest[3] = source[4];
                dest += dest_stride;
                source += source_stride;
            }   /* end for */
#else
          tmp_dst = (uint8 *) HDmalloc(4);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          for (i = 0; i < num_elm; i++)
            {
                bitoff = ((unsigned int)dest) >> BITOFF;
                ierr = CRAY2IEG(&type, &n_elem, tmp_dst, &bitoff, source);
                if (ierr != 0)
                  {
                      HDfree(tmp_dst);
                      HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
                  }     /* end if */
                DFKswap(tmp_dst, dest, 4, num_elm);
                dest += dest_stride;
                source += source_stride;
            }   /* end for */
          HDfree(tmp_dst);
#endif
      }     /* end else */
    return (SUCCEED);
}

#define LUO4S_MASKA 0x00000000ff00ff00
#define LUO4S_MASKB 0x0000000000ff00ff
#define LUO4S_MASKC 0xffff0000ffff0000
#define LUO4S_MASKD 0x0000ffff0000ffff

/************************************************************/
/* DFKluo4s()                                               */
/* -->Unicos routine for exporting signed 4 byte            */
/*      little-endian data items                            */
/************************************************************/
int
DFKluo4s(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    int         fast_processing = 0;
    uintn i;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    unsigned long *lp_dest;
    unsigned long *lp_src;
    char       *FUNC = "DFKuo4s";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 1;       /* type of conversion to perform 1=integer */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    uint8      *tmp_dst;        /* temporary buffer to hold byte swapped values */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    if (source == dest || num_elm == 0)     /* Inplace conversions  not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* No elements to convert is an error */

    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
          tmp_dst = (uint8 *) HDmalloc(4 * num_elm);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          bitoff = ((unsigned int)dest) >> BITOFF;
          ierr = CRAY2IEG(&type, &num_elm, tmp_dst, &bitoff, source);
          if (ierr != 0)
            {
                HDfree(tmp_dst);
                HRETURN_ERROR(DFE_BADCONV, FAIL);   /* error in Cray conversion */
            }   /* end if */
          DFKswap(tmp_dst, dest, 4, num_elm);
          HDfree(tmp_dst);
      }     /* end if */
    else
      {
#ifdef NOINTCRAY2IEG
          for (i = 0; i < num_elm; i++)
            {
                dest[0] = source[7];
                dest[1] = source[6];
                dest[2] = source[5];
                dest[3] = source[4];
                dest += dest_stride;
                source += source_stride;
            }   /* end for */
#else
          tmp_dst = (uint8 *) HDmalloc(4);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          for (i = 0; i < num_elm; i++)
            {
                bitoff = ((unsigned int)dest) >> BITOFF; 
                ierr = CRAY2IEG(&type, &n_elem, tmp_dst, &bitoff, source);
                if (ierr != 0)
                  {
                      HDfree(tmp_dst);
                      HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
                  }     /* end if */
                DFKswap(tmp_dst, dest, 4, num_elm);
                dest += dest_stride;
                source += source_stride;
            }   /* end for */
          HDfree(tmp_dst);
#endif
      }     /* end else */
    return (SUCCEED);
}

#define LUI4F_MASKA  0x0000008000000000
#define LUI4F_MASKB1 0x0000007f00000000
#define LUI4F_MASKB2 0x0000800000000000
#define LUI4F_MASKC1 0x00007f0000000000
#define LUI4F_MASKC2 0x00ff000000000000
#define LUI4F_MASKC3 0xff00000000000000
#define LUI4F_MASKD  0x0000800000000000

#define LUI4F_MASKE  0x0000000000000080
#define LUI4F_MASKF1 0x000000000000007f
#define LUI4F_MASKF2 0x0000000000008000
#define LUI4F_MASKG1 0x0000000000007f00
#define LUI4F_MASKG2 0x0000000000ff0000
#define LUI4F_MASKG3 0x00000000ff000000

#define LUI4F_MASKH  0xffffff7f00000000
#define LUI4F_MASKI  0x00000000ffffff7f

/***************************************************************/
/* DFKlui4f()                                                  */
/* -->Unicos routine for importing little-endian 32 bit floats */
/***************************************************************/

/************************************************************

                     <<<< WARNING >>>>

    The nature of converting between 64 bit floating point
  numbers and 32 bit floating point numbers LOSES PRECISION.
  Taking a number in 64bit cray format, converting to IEEE
  (internal HDF format) and back will round the number at
  about the 7th decimal place.

 ************************************************************/

int
DFKlui4f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    int         fast_processing = 0;    /* By default not array processed */
    int         odd_man_out = 0;    /* By default there are even num_elm */
    int         i, j, n;
    long        buf1;           /* This is a temporary stride buf */
    long        buf2;           /* This is a temporary stride buf */
    uint8      *dud1 = (uint8 *) &buf1;     /* Dummy pointer to buf1 for strides */
    uint8      *dud2 = (uint8 *) &buf2;     /* Dummy pointer to buf2 for strides */
    uint8      *source = (uint8 *) s;   /* Cray does not like certain   */
    uint8      *dest = (uint8 *) d;     /* void and void* constructions */
    long       *lptr_src = (long *) source;
    long       *lptr_dest = (long *) dest;
    char       *FUNC = "DFKui4f";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 2;       /* type of conversion to perform 2=32-bit float */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    uint8      *tmp_dst;        /* temporary buffer to hold byte swapped values */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    /* Check for conversion errors */
    if (source == dest || num_elm == 0)     /* Inplace conversions not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* under UNICOS */

    /* Find out if it is OK to use faster array processing */
    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
          tmp_dst = (uint8 *) HDmalloc(4 * num_elm);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          DFKswap(source, tmp_dst, 4, num_elm);
          bitoff = ((unsigned int)source) >> BITOFF;
          ierr = IEG2CRAY(&type, &num_elm, tmp_dst, &bitoff, dest);
          HDfree(tmp_dst);
          if (ierr != 0)
              HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
      }     /* end if */
    else
      {     /* We end up here if we are doing stride based processing */
#ifdef NOSTRIDEFLOATCRAY2IEG
          buf1 = 0;
          for (i = 0; i < num_elm; i++)
            {
                dud1[0] = source[3];    /* Loop would be less efficient */
                dud1[1] = source[2];
                dud1[2] = source[1];
                dud1[3] = source[0];

                if ((float) buf1 != 0)
                  {
                      buf2 = (((buf1 & UI4F_MASKA) |
                               ((buf1 & UI4F_MASKB) >> 7) +
                               (16258 << 48)) |
                              (((buf1 & UI4F_MASKC) >> 8) | (UI4F_MASKD)));
                      if ((buf1 << 1) == 0)
                          buf2 = 0;
                  }
                else
                    buf2 = buf1;

                dest[0] = dud2[0];  /* Loop would be less efficient */
                dest[1] = dud2[1];
                dest[2] = dud2[2];
                dest[3] = dud2[3];
                dest[4] = dud2[4];
                dest[5] = dud2[5];
                dest[6] = dud2[6];
                dest[7] = dud2[7];

                source += source_stride;
                dest += dest_stride;
            }   /* end for */
#else
          float32     tmp_dst2;

          tmp_dst = (uint8 *) HDmalloc(4);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          for (i = 0; i < num_elm; i++)
            {
                DFKswap(source, tmp_dst, 4, 1);
                bitoff = ((unsigned int)source) >> BITOFF;
                ierr = IEG2CRAY(&type, &n_elem, tmp_dst, &bitoff, &tmp_dst2);
                if (ierr != 0)
                  {
                      HDfree(tmp_dst);
                      HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
                  }     /* end if */
                HDmemcpy(dest, &tmp_dst2, 4);
                source += source_stride;
                dest += dest_stride;
            }   /* end for */
          HDfree(tmp_dst);
#endif
      }     /* end else */
    return (SUCCEED);
}

#define LUO4F_MASKA 0x8000000000000000
#define LUO4F_MASKB 0x7fff000000000000
#define LUO4F_MASKC 0x00007fffff000000
#define LUO4F_MASKD 0x0000000000800000
#define LUO4F_MASKE 0xff00ff00ff00ff00
#define LUO4F_MASKF 0x00ff00ff00ff00ff
#define LUO4F_MASKG 0xffff0000ffff0000
#define LUO4F_MASKH 0x0000ffff0000ffff

/***************************************************************/
/* DFKluo4f()                                                  */
/* -->Unicos routine for exporting little-endian 32 bit floats */
/***************************************************************/

/************************************************************

                     <<<< WARNING >>>>

    The nature of converting between 64 bit floating point
  numbers and 32 bit floating point numbers LOSES PRECISION.
  Taking a number in 64bit cray format, converting to IEEE
  (internal HDF format) and back will round the number at
  about the 7th decimal place.

 ************************************************************/

int
DFKluo4f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    int         fast_processing = 0;    /* By default not array processed */
    int         odd_man_out = 0;    /* By default there are even num_elm */
    int         i, j, n;
    long        buf1;           /* This is a temporary stride buf */
    long        buf2;           /* This is a temporary stride buf */
    uint8      *dud1 = (uint8 *) &buf1;     /* Dummy pointer to buf1 for strides */
    uint8      *dud2 = (uint8 *) &buf2;     /* Dummy pointer to buf2 for strides */
    uint8      *source = (uint8 *) s;   /* Cray does not like certain   */
    uint8      *dest = (uint8 *) d;     /* void and void* constructions */
    long       *lptr_src = (long *) source;
    long       *lptr_dest = (long *) dest;
    char       *FUNC = "DFKuo4f";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 2;       /* type of conversion to perform 2=32-bit float */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    uint8      *tmp_dst;        /* temporary buffer to hold byte swapped values */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    /* Check for conversion errors */
    if (source == dest || num_elm == 0)     /* Inplace conversions not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* under UNICOS */

    /* Find out if it is OK to use faster array processing */
    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
          tmp_dst = (uint8 *) HDmalloc(4 * num_elm);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          bitoff = ((unsigned int)dest) >> BITOFF;
          ierr = CRAY2IEG(&type, &num_elm, tmp_dst, &bitoff, source);
          if (ierr != 0)
            {
                HDfree(tmp_dst);
                HRETURN_ERROR(DFE_BADCONV, FAIL);   /* error in Cray conversion */
            }   /* end if */
          DFKswap(tmp_dst, dest, 4, num_elm);
          HDfree(tmp_dst);
      }     /* end if */
    else
      {     /* We end up here if we are doing stride based processing */
#ifdef NOSTRIDEFLOATCRAY2IEG
          buf1 = 0;
          for (i = 0; i < num_elm; i++)
            {
                dud1[0] = source[0];    /* Loop would be less efficient */
                dud1[1] = source[1];
                dud1[2] = source[2];
                dud1[3] = source[3];
                dud1[4] = source[4];
                dud1[5] = source[5];
                dud1[6] = source[6];
                dud1[7] = source[7];

                if ((float) buf1 != 0)
                    buf2 = (((buf1 & LUO4F_MASKA) |
                           ((((buf1 & LUO4F_MASKB) >> 48) - 16258) << 55)) +
                            (((buf1 & LUO4F_MASKC) + ((buf1 & LUO4F_MASKD) << 1)) << 8));
                else
                    buf2 = buf1;

                dest[3] = dud2[0];  /* Loop would be less efficient */
                dest[2] = dud2[1];
                dest[1] = dud2[2];
                dest[0] = dud2[3];

                source += source_stride;
                dest += dest_stride;
            }   /* end for */
#else
          float32     tmp_src;

          tmp_dst = (uint8 *) HDmalloc(4);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          for (i = 0; i < num_elm; i++)
            {
                HDmemcpy(&tmp_src, source, sizeof(float32));
                bitoff = ((unsigned int)dest) >> BITOFF;
                ierr = CRAY2IEG(&type, &n_elem, tmp_dst, &bitoff, &tmp_src);
                if (ierr != 0)
                  {
                      HDfree(tmp_dst);
                      HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
                  }     /* end if */
                DFKswap(tmp_dst, dest, 4, 1);
                source += source_stride;
                dest += dest_stride;
            }   /* end for */
          HDfree(tmp_dst);
#endif
      }     /* end else */
    return (SUCCEED);
}

#define LUI8F_MASKA  0x0000000000000080
#define LUI8F_MASKB1 0x000000000000007f
#define LUI8F_MASKB2 0x000000000000f000
#define LUI8F_MASKC1 0x0000000000000f00
#define LUI8F_MASKC2 0x0000000000ff0000
#define LUI8F_MASKC3 0x00000000ff000000
#define LUI8F_MASKC4 0x000000ff00000000
#define LUI8F_MASKC5 0x0000ff0000000000
#define LUI8F_MASKC6 0x00ff000000000000
#define LUI8F_MASKC7 0xff00000000000000
#define LUI8F_MASKD  0x0800000000000000
#define LUI8F_MASKE  0x0000800000000000
#define LUI8F_MASKF 0xff00ff00ff00ff00
#define LUI8F_MASKG 0x00ff00ff00ff00ff
#define LUI8F_MASKH 0xffff0000ffff0000
#define LUI8F_MASKI 0x0000ffff0000ffff
#define LUI8F_MASKJ 0xffffffff00000000
#define LUI8F_MASKK 0x00000000ffffffff
#define LUI8F_MASKL 0xffffffffffffff7f

/************************************************************/
/* DFKlui8f()                                               */
/* -->Unicos routine for importing 64 bit floats            */
/************************************************************/
int
DFKlui8f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    int         fast_processing = 0;    /* By default not array processed */
    int         i, j, n;
    long        buf;            /* This is a temporary stride buf */
    uint8      *dud = (uint8 *) &buf;   /* Dummy pointer to buf1 for strides */
    uint8      *source = (uint8 *) s;   /* Cray does not like certain   */
    uint8      *dest = (uint8 *) d;     /* void and void* constructions */
    long       *lptr_src = (long *) source;
    long       *lptr_dest = (long *) dest;
    char       *FUNC = "DFKui8f";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 8;       /* type of conversion to perform 8=64-bit float */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    uint8      *tmp_dst;        /* temporary buffer to hold byte swapped values */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    /* Check for conversion errors */
    if (source == dest || num_elm == 0)     /* Inplace conversions not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* under UNICOS */

    /* Find out if it is OK to use faster array processing */
    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
          tmp_dst = (uint8 *) HDmalloc(8 * num_elm);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          DFKswap(source, tmp_dst, 8, num_elm);
          bitoff = ((unsigned int)source) >> BITOFF;
          ierr = IEG2CRAY(&type, &num_elm, tmp_dst, &bitoff, dest);
          HDfree(tmp_dst);
          if (ierr != 0)
              HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
      }     /* end if */
    else
      {
#ifdef NOSTRIDEFLOATCRAY2IEG
          for (i = 0; i < num_elm; i++)
            {
                dud[0] = source[7];
                dud[1] = source[6];
                dud[2] = source[5];
                dud[3] = source[4];
                dud[4] = source[3];
                dud[5] = source[2];
                dud[6] = source[1];
                dud[7] = source[0];

                if (buf != 0)
                  {
                      buf = (((buf & UI8F_MASKA) | ((buf & UI8F_MASKB) >> 4) +
                              (15362 << 48)) |
                             ((((buf & UI8F_MASKC) + ((buf & UI8F_MASKD) << 1)) >> 5) |
                              (UI8F_MASKE)));
                      if ((buf << 1) == 0)
                          buf = 0;
                  }
                else
                    buf = 0;

                dest[0] = dud[0];
                dest[1] = dud[1];
                dest[2] = dud[2];
                dest[3] = dud[3];
                dest[4] = dud[4];
                dest[5] = dud[5];
                dest[6] = dud[6];
                dest[7] = dud[7];

                source += source_stride;
                dest += dest_stride;
            }   /* end for */
#else
          tmp_dst = (uint8 *) HDmalloc(8);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          for (i = 0; i < num_elm; i++)
            {
                DFKswap(source, tmp_dst, 8, 1);
                bitoff = ((unsigned int)source) >> BITOFF;
                ierr = IEG2CRAY(&type, &n_elem, tmp_dst, &bitoff, dest);
                if (ierr != 0)
                  {
                      HDfree(tmp_dst);
                      HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
                  }     /* end if */
                source += source_stride;
                dest += dest_stride;
            }   /* end for */
          HDfree(tmp_dst);
#endif
      }     /* end else */
    return (SUCCEED);
}

#define LUO8F_MASKA 0x8000000000000000
#define LUO8F_MASKB 0x7fff000000000000
#define LUO8F_MASKC 0x00007fffffffffff
#define LUO8F_MASKD 0xff00ff00ff00ff00
#define LUO8F_MASKE 0x00ff00ff00ff00ff
#define LUO8F_MASKF 0xffff0000ffff0000
#define LUO8F_MASKG 0x0000ffff0000ffff
#define LUO8F_MASKH 0xffffffff00000000
#define LUO8F_MASKI 0x00000000ffffffff

/************************************************************/
/* DFKluo8f()                                               */
/* -->Unicos routine for exporting 64 bit floats            */
/************************************************************/
int
DFKluo8f(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    int         fast_processing = 0;    /* By default not array processed */
    int         odd_man_out = 0;    /* By default there are even num_elm */
    int         i, j, n;
    long        buf;            /* This is a temporary stride buf */
    uint8      *dud = (uint8 *) &buf;   /* Dummy pointer to buf1 for strides */
    uint8      *source = (uint8 *) s;   /* Cray does not like certain   */
    uint8      *dest = (uint8 *) d;     /* void and void* constructions */
    long       *lptr_src = (long *) source;
    long       *lptr_dest = (long *) dest;
    char       *FUNC = "DFKuo8f";
    int         ierr;           /* error from IEG2CRAY */
    int         type = 8;       /* type of conversion to perform 8=64-bit float */
    int         bitoff = 0;     /* bit offset in the IEEE stream */
    uint8      *tmp_dst;        /* temporary buffer to hold byte swapped values */
    intn        n_elem = 1;     /* the number of elements for stride-based processing */

    HEclear();

    /* Check for conversion errors */
    if (source == dest || num_elm == 0)     /* Inplace conversions not permitted */
        HRETURN_ERROR(DFE_BADCONV, FAIL);   /* under UNICOS */

    /* Find out if it is OK to use faster array processing */
    if (source_stride == 0 && dest_stride == 0)
        fast_processing = 1;

    if (fast_processing)
      {
          tmp_dst = (uint8 *) HDmalloc(8 * num_elm);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          bitoff = ((unsigned int)dest) >> BITOFF;
          ierr = CRAY2IEG(&type, &num_elm, tmp_dst, &bitoff, source);
          if (ierr != 0)
            {
                HDfree(tmp_dst);
                HRETURN_ERROR(DFE_BADCONV, FAIL);   /* error in Cray conversion */
            }   /* end if */
          DFKswap(tmp_dst, dest, 8, num_elm);
          HDfree(tmp_dst);
      }     /* end if */
    else
      {
#ifdef NOSTRIDEFLOATCRAY2IEG
          for (i = 0; i < num_elm; i++)
            {
                dud[0] = source[0];
                dud[1] = source[1];
                dud[2] = source[2];
                dud[3] = source[3];
                dud[4] = source[4];
                dud[5] = source[5];
                dud[6] = source[6];
                dud[7] = source[7];

                if (buf != 0)
                  {
                      buf = (((buf & LUO8F_MASKA) |     /* sign bit */
                      (((((buf & LUO8F_MASKB) >> 48) - 15362) << 53) >> 1)) |   /* exp */
                             ((buf & LUO8F_MASKC) << 5));   /* mantissa */
                  }
                else
                    buf = 0;

                dest[7] = dud[0];
                dest[6] = dud[1];
                dest[5] = dud[2];
                dest[4] = dud[3];
                dest[3] = dud[4];
                dest[2] = dud[5];
                dest[1] = dud[6];
                dest[0] = dud[7];

                source += source_stride;
                dest += dest_stride;
            }   /* end for */
#else
          tmp_dst = (uint8 *) HDmalloc(8);
	  if (!tmp_dst)
	      HRETURN_ERROR(DFE_NOSPACE, FAIL);
          for (i = 0; i < num_elm; i++)
            {
                bitoff = ((unsigned int)dest) >> BITOFF;
                ierr = CRAY2IEG(&type, &n_elem, tmp_dst, &bitoff, source);
                if (ierr != 0)
                  {
                      HDfree(tmp_dst);
                      HRETURN_ERROR(DFE_BADCONV, FAIL);     /* error in Cray conversion */
                  }     /* end if */
                DFKswap(tmp_dst, dest, 8, 1);
                source += source_stride;
                dest += dest_stride;
            }   /* end for */
          HDfree(tmp_dst);
#endif
      }     /* end else */
    return (SUCCEED);
}

#else  /* i.e. not on a cray */

int         cray_dummy;         /* prevent empty symbol table messages */

#endif /* UNICOS */

/* =============================================================*/
/* CRAY-MPP (T3D) routines 					*/
/* ============================================================ */
#if defined(CRAYMPP)

/************************************************************/
/* DFKmi2i()                                                */
/* -->CRAY-MPP routine for importing 2 byte data items      */
/* (**) This routine converts two byte IEEE to 4 bytes      */
/*      CRAY-MPP big endian integer unsigned.               */
/************************************************************/
int
DFKmi2i(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    intn	i;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    char       *FUNC = "DFKmi2i";

    HEclear();

    if (source_stride == 0 && dest_stride == 0){
	source_stride = 2;
	dest_stride   = 4;
    }

    /* parameters check */
    if (source == dest			/* Inplace conversions not permitted */
	|| num_elm == 0			/* No elements is an error */
	|| source_stride == 0		/* only positive stride allowed */
	|| dest_stride == 0		/* only positive stride allowed */
	)
        HRETURN_ERROR(DFE_BADCONV, FAIL);

  for (i = 0; i < num_elm; i++)
    {
	dest[0] = 0x00;
	dest[1] = 0x00;
	dest[2] = source[0];
	dest[3] = source[1];
	source += source_stride;
	dest += dest_stride;
    }
    return (SUCCEED);
}

/************************************************************/
/* DFKmi2s()                                                */
/* -->CRAY-MPP routine for importing 2 byte signed ints     */
/* (**) This routine converts two byte IEEE to 4 bytes      */
/*      Cray-MPP big endian integers.                       */
/************************************************************/
int
DFKmi2s(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    intn	i;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    char       *FUNC = "DFKmi2i";

    HEclear();

    if (source_stride == 0 && dest_stride == 0){
	source_stride = 2;
	dest_stride   = 4;
    }

    /* parameters check */
    if (source == dest			/* Inplace conversions not permitted */
	|| num_elm == 0			/* No elements is an error */
	|| source_stride == 0		/* only positive stride allowed */
	|| dest_stride == 0		/* only positive stride allowed */
	)
        HRETURN_ERROR(DFE_BADCONV, FAIL);

    for (i = 0; i < num_elm; i++)
    {
	if ((source[0] & 0x80))
	{   /* Need to extend sign */
	    dest[0] = 0xff;
	    dest[1] = 0xff;
	} 
	else
	{
	    dest[0] = 0x00;
	    dest[1] = 0x00;
	}
	dest[2] = source[0];
	dest[3] = source[1];
	source += source_stride;
	dest += dest_stride;
    }
    return (SUCCEED);
}

/************************************************************/
/* DFKmo2b()                                                */
/* -->CRAY-MPP routine for exporting 4 byte data as 2 byte  */
/*    data items.  This works for signed and unsigned data. */
/************************************************************/
int
DFKmo2b(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
        uint32 dest_stride)
{
    uintn	i;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    char       *FUNC = "DFKmo2b";

    HEclear();

    if (source_stride == 0 && dest_stride == 0){
	source_stride = 4;
	dest_stride   = 2;
    }

    /* parameters check */
    if (source == dest			/* Inplace conversions not permitted */
	|| num_elm == 0			/* No elements is an error */
	|| source_stride == 0		/* only positive stride allowed */
	|| dest_stride == 0		/* only positive stride allowed */
	)
        HRETURN_ERROR(DFE_BADCONV, FAIL);

    for (i = 0; i < num_elm; i++)
    {
	dest[0] = source[2];
	dest[1] = source[3];
	source += source_stride;
	dest += dest_stride;
    }
    return (SUCCEED);
}

/************************************************************/
/* DFKlmi2i()                                               */
/* -->CRAY-MPP routine for importing 2 byte data items        */
/* (**) This routine converts two byte little-endian IEEE   */
/*      to 4 byte Cray-MPP big endian integer.              */
/************************************************************/
int
DFKlmi2i(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    uintn	i;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    char       *FUNC = "DFKlmi2i";

    HEclear();

    if (source_stride == 0 && dest_stride == 0){
	source_stride = 2;
	dest_stride   = 4;
    }

    /* parameters check */
    if (source == dest			/* Inplace conversions not permitted */
	|| num_elm == 0			/* No elements is an error */
	|| source_stride == 0		/* only positive stride allowed */
	|| dest_stride == 0		/* only positive stride allowed */
	)
        HRETURN_ERROR(DFE_BADCONV, FAIL);


          for (i = 0; i < num_elm; i++)
            {
                dest[0] = 0x00;
                dest[1] = 0x00;
                dest[2] = source[1];
                dest[3] = source[0];
                source += source_stride;
                dest += dest_stride;
            }   /* end for */
    return (SUCCEED);
}

/************************************************************/
/* DFKlmi2s()                                                */
/* -->CRAY-MPP routine for importing 2 byte signed ints       */
/* (**) This routine converts two byte IEEE to 4 bytes   */
/*      Cray-MPP big endian integer.              */
/************************************************************/
int
DFKlmi2s(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{

    uintn	i;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    char       *FUNC = "DFKlmi2s";

    HEclear();

    if (source_stride == 0 && dest_stride == 0){
	source_stride = 2;
	dest_stride   = 4;
    }

    /* parameters check */
    if (source == dest			/* Inplace conversions not permitted */
	|| num_elm == 0			/* No elements is an error */
	|| source_stride == 0		/* only positive stride allowed */
	|| dest_stride == 0		/* only positive stride allowed */
	)
        HRETURN_ERROR(DFE_BADCONV, FAIL);

    for (i = 0; i < num_elm; i++)
    {
	if ((source[1] & 0x80))
	  {   /* Need to extend sign */
	      dest[0] = 0xff;
	      dest[1] = 0xff;
	  }     /* end if */
	else
	  {
	      dest[0] = 0x00;
	      dest[1] = 0x00;
	  }     /* end else */
	dest[2] = source[1];
	dest[3] = source[0];
	source += source_stride;
	dest += dest_stride;
    }
    return (SUCCEED);
}

/************************************************************/
/* DFKlmo2b()                                               */
/* -->CRAY-MPP routine for exporting 4 bytes signed int as  */
/*    2 bytes little-endian data items                      */
/************************************************************/
int
DFKlmo2b(VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride,
         uint32 dest_stride)
{
    uint32 i;
    uint8      *source = (uint8 *) s;
    uint8      *dest = (uint8 *) d;
    char       *FUNC = "DFKlmo2b";


    HEclear();

    if (source_stride == 0 && dest_stride == 0){
	source_stride = 4;
	dest_stride   = 2;
    }

    /* parameters check */
    if (source == dest			/* Inplace conversions not permitted */
	|| num_elm == 0			/* No elements is an error */
	|| source_stride == 0		/* only positive stride allowed */
	|| dest_stride == 0		/* only positive stride allowed */
	)
        HRETURN_ERROR(DFE_BADCONV, FAIL);

    for (i = 0; i < num_elm; i++)
    {
	dest[0] = source[3];
	dest[1] = source[2];
	source += source_stride;
	dest += dest_stride;
    }   /* end for */
    return (SUCCEED);
}

#else  /* i.e. not on a craympp */

int         craympp_dummy;         /* prevent empty symbol table messages */

#endif /* CRAYMPP */
