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

/* $Id: hdfalloc.c 4932 2007-09-07 17:17:23Z bmribler $ */

#ifdef MALDEBUG
#define __MALDEBUG__
#endif
#include "hdf.h"

/*
LOCAL ROUTINES
  None
EXPORTED ROUTINES
  HDmemfill    -- copy a chunk of memory repetitively into another chunk
  HIstrncpy    -- string copy with termination
  HDmalloc     -- dynamicly allocates memory
  HDrealloc    -- dynamicly resize (reallocate) memory
  HDfree       -- free dynamicly allocated memory
  HDcalloc     -- dynamicly allocates memory and clears it to zero
  HDstrdup     -- in-library replacement for non-ANSI strdup()
*/

/*--------------------------------------------------------------------------
 NAME
    HDmemfill -- copy a chunk of memory repetitively into another chunk

 USAGE
    void * HDmemfill(dest,src,item_size,num_items)
        void * dest;         OUT: pointer to the chunk of memory to be filled
                            with a pattern
        void * src;          IN: pointer to the pattern to copy
        uint32 item_size;   IN: size of the pattern to copy
        uint32 num_items;   IN: number of times to copy the pattern into the dest
                            buffer

 RETURNS
    Returns a pointer to the dest parameter

 DESCRIPTION
    Common code used to fill a chunk of memory with a pattern.  This
    routine can be used to copy a given "fill" value into an array
    of any number type.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    The src and dest pointers are assumed to point to valid portions of
    memory.
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
void *
HDmemfill(void * dest, const void * src, uint32 item_size, uint32 num_items)
{
    uint32      copy_size;      /* size of the buffer to copy */
    uint32      copy_items;     /* number of items currently copying */
    uint32      items_left;     /* number of items left to copy */
    uint8      *curr_dest;      /* ptr into the 'dest' memory where we are currently */

    /* minimal error check for 0 sized array or item size */
    if (num_items > 0 && item_size > 0)
      {
          HDmemcpy(dest, src, item_size);   /* copy first item */

          copy_size = item_size;
          copy_items = 1;
          items_left = num_items - 1;
          curr_dest = ((uint8 *) dest) + item_size;

          /* copy until we've copied at least half of the items */
          while (items_left >= copy_items)
            {

                HDmemcpy(curr_dest, dest, copy_size);   /* copy the current chunk */
                curr_dest += copy_size;     /* move the offset for the next chunk */
                items_left -= copy_items;   /* decrement the number of items left */

                copy_size *= 2;     /* increase the size of the chunk to copy */
                copy_items *= 2;    /* increase the count of items we are copying */
            }   /* end while */
          if (items_left > 0)   /* if there are any items left to copy */
              HDmemcpy(curr_dest, dest, items_left * item_size);
      }     /* end if */
    return (dest);
}   /* end HDmemfill() */

/*--------------------------------------------------------------------------
 NAME
    HIstrncpy -- string copy with termination
 USAGE
    char *HIstrncpy(char *dest,char *source,int32 len)
        char *dest;             OUT: location to place string
        char *source;           IN: location of string to copy
        int32 len;              IN: mas. length of dest. string
 RETURNS
    dest on success, NULL on failure.
 DESCRIPTION
    This function creates a string in dest that is at most 'len' characters
    long.  The 'len' characters *include* the NULL terminator which must be
    added.  So if you have the string "Foo\0" you must call this copy function
    with len == 4.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
char *
HIstrncpy(char *dest, const char *source, int32 len)
{
    char       *destp;

    destp = dest;
    if (len == 0)
        return (destp);
    for (; (len > 1) && (*source != '\0'); len--)
        *dest++ = *source++;
    *dest = '\0';   /* Force the last byte be '\0'   */
    return (destp);
}   /* end HIstrncpy() */
/* *INDENT-OFF* */
/* GNU indent 1.9.1 urps on this section, so turn off indenting for now -QAK */

/* define MALLOC_CHECK to get some more information when malloc/realloc fail */
#ifdef MALLOC_CHECK
/*--------------------------------------------------------------------------
 NAME
    HDmalloc -- dynamicly allocates memory
 USAGE
    void * HDmalloc(qty)
        uint32 qty;         IN: the (minimum) number of bytes to allocate in
                                the memory block.
 RETURNS
    Pointer to the memory allocated on success, NULL on failure.
 DESCRIPTION
    Dynamicly allocates a block of memory and returns a pointer to it.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Acts like malloc().
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
void * HDmalloc(uint32 qty)
{
    char FUNC[]="HDmalloc";
    char *p;

    p = (char *) malloc(qty);
    if (p== (char *) NULL) {
        HEreport("Attempted to allocate %d bytes", qty);
        HRETURN_ERROR(DFE_NOSPACE,NULL);
      } /* end if */
    return(p);
}   /* end HDmalloc() */

/*--------------------------------------------------------------------------
 NAME
    HDrealloc -- dynamicly resize (reallocate) memory
 USAGE
    void * HDrealloc(vfp,qty)
        void * vfp;          IN: pointer to the memory block to resize.
        uint32 qty;         IN: the (minimum) number of bytes to allocate in
                                the new memory block.
 RETURNS
    Pointer to the memory allocated on success, NULL on failure.
 DESCRIPTION
    Dynamicly re-allocates a block of memory and returns a pointer to it.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Acts like realloc().
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
void * HDrealloc(void * where, uint32 qty)
{
    char FUNC[]="HDrealloc";
    char *p;

    p = (char *) realloc(where, qty);
    if (p== (char *) NULL) {
        HEreport("Attempted to re-allocate %d bytes", qty);
        HRETURN_ERROR(DFE_NOSPACE,NULL);
      } /* end if */
    return(p);
}   /* end HDrealloc() */

/*--------------------------------------------------------------------------
 NAME
    HDfree -- free dynamicly allocated memory
 USAGE
    void HDfree(vfp)
        void * vfp;          IN: pointer to the memory block to free.
 RETURNS
    NULL?
 DESCRIPTION
    Free dynamicly allocated blocks of memory.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Acts like free().
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
void HDfree(void * ptr)
{
    if (ptr!=NULL)
        free(ptr);
}   /* end HDfree() */

/*--------------------------------------------------------------------------
 NAME
    HDcalloc -- dynamicly allocates memory and clears it to zero
 USAGE
    void * HDcalloc(n,size)
        uint32 n;         IN: the number of blocks to allocate
        uint32 size;      IN: the size of the block
 RETURNS
    Pointer to the memory allocated on success, NULL on failure.
 DESCRIPTION
    Dynamicly allocates a block of memory and returns a pointer to it
    after setting it to zero.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Acts like calloc().  Instead of doing all the work ourselves, this calls
    HDmalloc and HDmemset().
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
void *
HDcalloc(uint32 n, uint32 size)
{
    char        FUNC[] = "HDcalloc";
    void *       p;

    p = HDmalloc(n * size);
    if (p == NULL)
      {
          HEreport("Attempted to allocate %d blocks of %d bytes", (int) n, (int) size);
          HRETURN_ERROR(DFE_NOSPACE, NULL);
      }     /* end if */
    else
        HDmemset(p, 0, n * size);
    return (p);
}   /* end HDcalloc() */
#endif /* MALLOC_CHECK */

#if defined VMS || defined macintosh || defined MAC || defined SYMANTEC_C || defined MIPSEL || defined NEXT || defined CONVEX || defined IBM6000 || defined ANSISUN || defined IRIX
/*--------------------------------------------------------------------------
 NAME
    HDstrdup -- in-library replacement for non-ANSI strdup()
 USAGE
    char *HDstrdup(s)
        const char *s;          IN: pointer to the string to duplicate
 RETURNS
    Pointer to the duplicated string, or NULL on failure.
 DESCRIPTION
    Duplicates a string (i.e. allocates space and copies it over).
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Acts like strdup().
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
char       *
HDstrdup(const char *s)
{
    char       *ret;

    ret = (char *) HDmalloc((uint32) HDstrlen(s) + 1);
    if (ret == NULL)
        return (NULL);
    HDstrcpy(ret, s);
    return (ret);
}   /* end HDstrdup() */

#endif /* VMS | macinosh */

