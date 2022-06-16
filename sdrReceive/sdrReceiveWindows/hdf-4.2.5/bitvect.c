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
static char RcsId[] = "$Revision: 4932 $";
#endif

/* $Id: bitvect.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*
FILE
    bitvect.c

PURPOSE
    Provide an API for dynamicly-sized bit-vectors or "bit-sets"

REMARKS
    These function manipulate ordered sets of "bits".  They are designed 
    to closely resemble the functions which one can perform in C with the
    "bit-wise" algebraic functions, with some additional pizzaz thrown in.

DESIGN
        These routines a designed to be a modular component for use both
    inside the HDF library and out.
        They will use the HDF typedefs & Standard C library macros, but do
    not explicitly depend on being inside the library itself.
        Each bit-vector is stored in memory as an array of unsigned 8-bit 
    integers (uint8's in HDF types), which can grow as additional bits are
    flagged in the bit-vector.
        Each bit-vector is stored with the lowest bits in location 0 in the
    array of base type (uint8s currently) and the bits in "standard" C order
    (i.e. bit 0 is the lowest bit in the byte) in each byte.  This does make
    for a slightly strange "bit-swapped" storage, but is the most efficient.

BUGS/LIMITATIONS
   Currently the following design limitations are still in place:

EXPORTED ROUTINES

bv_ptr bv_new(int32 num_bits, uint32 flags)
    - Creates a new bit-vector with a particular starting # of bits.

intn bv_delete(bv_ptr b)
    - Deletes a bit-vector created with bv_new.

intn bv_set(bv_ptr b, int32 bit_num, bv_bool value)
    - Sets a bit in a bit-vector to a given boolean value.

intn bv_get(bv_ptr b, int32 bit_num)
    - Gets a bit from a bit-vector.

intn bv_clear(bv_ptr b, bv_bool value)
    - Clears an entire bit-vector to a given boolean value.

int32 bv_size(bv_ptr b)
    - Reports the number of bits used in a bit-vector.  

uint32 bv_flags(bv_ptr b)
    - Returns the flags used when creating the bit-vector

int32 bv_find(bv_ptr b, int32 last_find, bv_bool value)
    - Find the next bit in a bit-vector with a given value.

Functions that it would be nice to see (some day):

intn bv_bitand(bv_ptr b, int32 bit_num, bv_bool value)
    - Perform a boolean AND operation on a bit in a bit-vector.

intn bv_bitor(bv_ptr b, int32 bit_num, bv_bool value)
    - Perform a boolean OR operation on a bit in a bit-vector.

intn bv_bitxor(bv_ptr b, int32 bit_num, bv_bool value)
    - Perform a boolean XOR operation on a bit in a bit-vector.

intn bv_bitnot(bv_ptr b, int32 bit_num)
    - Perform a boolean NOT operation on a bit in a bit-vector.

bv_ptr *bv_vectand(bv_ptr b1, bv_ptr b2)
    - Perform a boolean AND operation between two bit-vectors.

bv_ptr *bv_vector(bv_ptr b1, bv_ptr b2)
    - Perform a boolean OR operation between two bit-vectors.

bv_ptr *bv_vectxor(bv_ptr b1, bv_ptr b2)
    - Perform a boolean XOR operation between two bit-vectors.

LOCAL ROUTINES

AUTHOR
   Quincey Koziol

MODIFICATION HISTORY
   12/05/95  - Starting writing specs & coding prototype
 */

#define BV_MASTER
#include "bitvect.h"       /* Multi-file raster information */

/* Local pre-processor macros */

/*--------------------------------------------------------------------------
 NAME
    bv_new
 PURPOSE
    Create a new bit-vector.
 USAGE
    bv_ptr bv_new(num_bits, flags)
        int32 num_bits;             IN: The initial number of bits in the vector
        uint32 flags;               IN: Flags to determine special attributes
                                        of the newly created bit-vector
 RETURNS
    Returns either a valid bv_ptr on succeed or NULL on failure.
 DESCRIPTION
    Creates a new bit-vector with a certain initial # of bits.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    If num_bits is set to (-1), then the default number of bits is used.
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
bv_ptr bv_new(int32 num_bits, uint32 flags)
{
    int32 base_elements;    /* the number of base elements needed to store the # of bits requested */
    bv_ptr b;               /* ptr to the new bit-vector */

    /* Check for invalid numbers of bits or bad flags */
    if(num_bits<(-1) || num_bits==0)
        return(NULL);

    /* Check for requesting the default # of bits */
    if(num_bits==(-1))
        num_bits=BV_DEFAULT_BITS;

    base_elements=((num_bits%(int32)BV_BASE_BITS)>0) ? (num_bits/(int32)BV_BASE_BITS)+1 : (num_bits/(int32)BV_BASE_BITS);

    if((b=(bv_ptr)HDmalloc(sizeof(bv_struct)))==NULL)
        return(NULL);
    
    b->bits_used=(uint32)num_bits;
    b->array_size=(uint32)(((base_elements/BV_CHUNK_SIZE)+1)*BV_CHUNK_SIZE);
    b->flags=flags;
    if((b->buffer=(bv_base *)HDmalloc(sizeof(bv_base)*b->array_size))==NULL)
      {
          HDfree(b);
          return(NULL);
      } /* end if */
    
    /* Check the initial bit settings */
    if(flags&BV_INIT_TO_ONE)
      {
        HDmemset(b->buffer,255,b->array_size);
        b->last_zero=(-1);  /* Set the last zero to unknown */
      } /* end if */
    else
      {
        HDmemset(b->buffer,0,b->array_size);
        b->last_zero=0;
      } /* end else */

    return(b);
}   /* bv_new() */

/*--------------------------------------------------------------------------
 NAME
    bv_delete
 PURPOSE
    Dispose of a new bit-vector.
 USAGE
    intn bv_delete(b)
        bv_ptr b;                   IN: Bit-vector to dispose of
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Disposes of a bit-vector created by bv_new.  This routine is responsible
    for completely cleaning up the bit-vector and disposing of all dynamicly
    allocated space.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn bv_delete(bv_ptr b)
{
    /* Error checking */
    if(b==NULL || b->buffer==NULL)
        return(FAIL);
    
    /* Free the space used */
    HDfree(b->buffer);
    HDfree(b);

    return(SUCCEED);
}   /* bv_delete() */

/*--------------------------------------------------------------------------
 NAME
    bv_set
 PURPOSE
    Set a bit in a bit-vector
 USAGE
    intn bv_set(b,bit_num,value)
        bv_ptr b;                   IN: Bit-vector to use
        int32 bit_num;              IN: bit to set
        bv_bool value;              IN: bit value to set the bit to
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Sets a bit in a bit-vector to a bit value.  Also extends the bit-vector
    if the bit to set is beyond the end of the current bit-vector and the
    bit-vector is marked as extendable.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn bv_set(bv_ptr b, int32 bit_num, bv_bool value)
{
    int32 base_elem;    /* the base array index of the bit */
    int32 bit_elem;      /* the bit index of the bit to set */

    /* Error checking */
    if(b==NULL || bit_num<0)
        return(FAIL);
    
    base_elem=bit_num/(int32)BV_BASE_BITS;
    bit_elem=bit_num%(int32)BV_BASE_BITS;

    /* Check if the bit is beyond the end of the current bit-vector */
    if((uint32)bit_num>=b->bits_used)
      {
          /* OK to extend? */
          if(b->flags&BV_EXTENDABLE)
            {
              if((uint32)base_elem<b->array_size)
                {   /* just use more bits in the currently allocated block */
                    b->bits_used=(uint32)(bit_num+1);
                } /* end if */
              else
                {   /* allocate more space for bits */
                    bv_base *old_buf=b->buffer;   /* ptr to the old buffer */
                    int32 num_chunks;               /* number of chunks to grab */
                    
                    num_chunks=(int32)(((((uint32)bit_num/BV_BASE_BITS)+1)-b->array_size)/BV_CHUNK_SIZE)+1;
                    if((b->buffer=(bv_base *)HDrealloc(b->buffer,b->array_size+(uint32)num_chunks*BV_CHUNK_SIZE))==NULL)
                      {
                          b->buffer=old_buf;
                          return(FAIL); /* fail to allocate a larger bit buffer */
                      } /* end if */

                    /* Check the initial bit settings, for the new bits */
                    if(b->flags&BV_INIT_TO_ONE)
                        HDmemset(&b->buffer[b->array_size],255,num_chunks*BV_CHUNK_SIZE);
                    else
                        HDmemset(&b->buffer[b->array_size],0,num_chunks*BV_CHUNK_SIZE);

                    b->array_size+=(uint32)(num_chunks*BV_CHUNK_SIZE);
                    b->bits_used=(uint32)bit_num+1;
                } /* end else */
            } /* end if */
          else
              return(FAIL); /* can't extend */
      } /* end if */

    if(value==BV_FALSE)
      {
        b->buffer[base_elem]&=~bv_bit_value[bit_elem];
        if(base_elem<b->last_zero)
            b->last_zero=base_elem;
      } /* end if */
    else
        b->buffer[base_elem]|=bv_bit_value[bit_elem];

    return(SUCCEED);
}   /* bv_set() */

/*--------------------------------------------------------------------------
 NAME
    bv_get
 PURPOSE
    Get a bit from a bit-vector
 USAGE
    intn bv_get(b,bit_num)
        bv_ptr b;                   IN: Bit-vector to use
        int32 bit_num;              IN: bit to set
 RETURNS
    Returns either BV_TRUE/BV_FALSE on success, or FAIL on error
 DESCRIPTION
    Gets a bit from a bit-vector.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn bv_get(bv_ptr b, int32 bit_num)
{
    int32 base_elem;    /* the base array index of the bit */
    int32 bit_elem;     /* the bit index of the bit to set */
    intn ret_value;     /* the variable to store the return value */

    /* Error checking */
    if(b==NULL || b->buffer==NULL || bit_num<0)
        return(FAIL);
    
    /* Check for asking for a bit off of the end of the vector */
    if((uint32)bit_num>=b->bits_used)
        return((b->flags&BV_INIT_TO_ONE) ? BV_TRUE : BV_FALSE);

    base_elem=bit_num/(int32)BV_BASE_BITS;
    bit_elem=bit_num%(int32)BV_BASE_BITS;

    ret_value=b->buffer[base_elem]&bv_bit_value[bit_elem];
    ret_value>>=bit_elem;

    return(ret_value);
}   /* bv_get() */

/*--------------------------------------------------------------------------
 NAME
    bv_clear
 PURPOSE
    Clears a bit-vector to a given boolean value
 USAGE
    intn bv_clear(b,value)
        bv_ptr b;                   IN: Bit-vector to use
        bv_bool value;              IN: bit value to set the bit to
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Clears an entire bit vector to a given boolean value, but does not
    change the status of the BV_INIT_TO_ONE flag for future bits which
    might be allocated.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn bv_clear(bv_ptr b, bv_bool value)
{
    /* Error checking */
    if(b==NULL || b->buffer==NULL)
        return(FAIL);

    if(value==BV_TRUE)
      {
        HDmemset(b->buffer,255,b->array_size);
        b->last_zero=(-1);
      } /* end if */
    else
      {
        HDmemset(b->buffer,0,b->array_size);
        b->last_zero=0;
      } /* end else */

    return(SUCCEED);
}   /* bv_clear() */

/*--------------------------------------------------------------------------
 NAME
    bv_size
 PURPOSE
    Report the number of bits in the bit-vector
 USAGE
    int32 bv_size(b)
        bv_ptr b;                   IN: Bit-vector to use
 RETURNS
    Returns number of bits in use on success, FAIL on error
 DESCRIPTION
    Report the number of bits currently in-use for a bit-vector.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32 bv_size(bv_ptr b)
{
    /* Error checking */
    if(b==NULL)
        return(FAIL);

    return((int32)b->bits_used);
}   /* bv_size() */

/*--------------------------------------------------------------------------
 NAME
    bv_flags
 PURPOSE
    Returns the flags for a bit-vector
 USAGE
    uint32 bv_size(b)
        bv_ptr b;                   IN: Bit-vector to use
 RETURNS
    Returns bit-vector flags on success, FAIL on error
 DESCRIPTION
    Returns the current flags for the bit-vector.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
uint32 bv_flags(bv_ptr b)
{
    /* Error checking */
    if(b==NULL)
        return(FAIL);

    return(b->flags);
}   /* bv_flags() */

/*--------------------------------------------------------------------------
 NAME
    bv_find
 PURPOSE
    Find the next bit of a given value
 USAGE
    int32 bv_find(b,last_find,value)
        bv_ptr b;                   IN: Bit-vector to use
        int32 last_find;            IN: bit offset of last bit found
        bv_bool value;              IN: boolean value to look for
 RETURNS
    Returns offset of next bit on success, FAIL on error
 DESCRIPTION
    Find the next highest bit of a given bit value.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    "last_find" capability not currently implemented for '0' searches - QAK
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32 bv_find(bv_ptr b,int32 last_find,bv_bool value)
{
    uint32 old_bits_used;   /* the last number of bits used */
    uint32 bytes_used;  /* number of full bytes used */
    uint32 first_byte=0;    /* The first byte to begin searching at */
    bv_base slush_bits; /* extra bits which don't fit into a byte */
    uint32 u;   /* local counting variable */

    /* Error checking */
    if(b==NULL || b->buffer==NULL)
        return(FAIL);

    bytes_used=b->bits_used/BV_BASE_BITS;
    if(value==BV_TRUE)
      { /* looking for first '1' in the bit-vector */
          if(last_find>=0)
            {   /* if the last bit found option is used, look for more bits in that same byte */
              intn bit_off;

              first_byte=(uint32)last_find/BV_BASE_BITS;
              bit_off=(intn)(((uint32)last_find-(first_byte*BV_BASE_BITS))+1);
              slush_bits=(bv_base)(b->buffer[first_byte]&(~bv_bit_mask[bit_off]));
              if(slush_bits!=0)
                  return((int32)(first_byte*BV_BASE_BITS)+bv_first_zero[~slush_bits]);
              first_byte++;
            } /* end if */

          for(u=first_byte; u<bytes_used; u++)
            {
                if(b->buffer[u]!=0)
                    return((int32)(u*BV_BASE_BITS)+bv_first_zero[~b->buffer[u]]);
            } /* end for */

          /* Any extra bits left over? */
          if((bytes_used*BV_BASE_BITS)<b->bits_used)
            {
                slush_bits=(bv_base)(b->buffer[u]&bv_bit_mask[b->bits_used-(bytes_used*BV_BASE_BITS)]);
                if(slush_bits!=0)
                    return((int32)(u*BV_BASE_BITS)+bv_first_zero[~slush_bits]);
            } /* end if */
      } /* end if */
    else
      { /* looking for first '0' in the bit-vector */
          bv_base *tmp_buf;

          if(b->last_zero>=0)
              u=(uint32)b->last_zero;
          else
              u=0;
#ifdef OLD_WAY
          for(; u<bytes_used; u++)
            {
                if(b->buffer[u]!=255)
                  {
                    b->last_zero=u;
                    return((u*BV_BASE_BITS)+bv_first_zero[b->buffer[u]]);
                  } /* end if */
            } /* end for */
#else /* OLD_WAY */
          tmp_buf=&b->buffer[u];
          while(u<bytes_used && *tmp_buf==255)
            {
              u++;
              tmp_buf++;
            } /* end while */
          if(u<bytes_used)
            {
              b->last_zero=(int32)u;
              return((int32)(u*BV_BASE_BITS)+bv_first_zero[*tmp_buf]);
            } /* end if */
#endif /* OLD_WAY */

          /* Any extra bits left over? */
          if((bytes_used*BV_BASE_BITS)<b->bits_used)
            {
                slush_bits=(bv_base)(b->buffer[u]&bv_bit_mask[b->bits_used-(bytes_used*BV_BASE_BITS)]);
                if(slush_bits!=255)
                  {
                    b->last_zero=(int32)u;
                    return((int32)(u*BV_BASE_BITS)+bv_first_zero[slush_bits]);
                  } /* end if */
            } /* end if */
      } /* end else */

    /* Beyond the current end of the bit-vector, extend the bit-vector */
    old_bits_used=b->bits_used;
    if(bv_set(b, (int32)b->bits_used, (bv_bool)((b->flags&BV_INIT_TO_ONE) ? BV_TRUE : BV_FALSE))==FAIL)
        return(FAIL);
    
    return((int32)old_bits_used);
}   /* bv_find() */

