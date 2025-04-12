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

/* $Id: maldebug.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*----------------------------------------------------------------------
 *
 *  maldebug.c
 *  Memory management utilities
 *
 *  Description
 *
 *  maldebug.c contains routines to protect the programmer
 *  from errors in calling memory allocation/free routines.
 *  The programmer must use the memory calls defined
 *  in maldebug.h. When these calls are used, the
 *  allocation routines in this module add a data structure
 *  to the top of allocated memory blocks which tags them as
 *  legal memory blocks.
 *
 *  When the free routine is called, the memory block to
 *  be freed is checked for legality tag.  If the block
 *  is not legal, the memory list is dumped to stderr and
 *  the program is terminated.
 *
 *  Compilation Options
 *
 *  MEM_LIST    Link all allocated memory blocks onto
 *          an internal list. The list can be
 *          displayed using Mem_Display().
 *
 *  MEM_WHERE   Save the file/line number of allocated
 *          blocks in the header.
 *          Requires that the compilier supports
 *          __FILE__ and __LINE__ preprocessor
 *          directives.
 *          Also requires that the __FILE__ string
 *          have a static or global scope.
 *
 *  MEM_HEADER  Place a header and footer section around each
 *          allocated block to detect overwrites on the beginning
 *          and the ending of the allocated block.
 *
 *  MEM_COMP_FREE   Complement the free'd memory.
 *
 */

#define __MALDEBUG__
/*#define DEBUG_LIST */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "maldebug.h"
#include "hdf.h"

/* Constants */
/* --------- */
#define MEMTAG  0xa55a  /* Value for mh_tag */
#define HEADERTAG   0x5a    /* Value for the header and footer data */

/* Structures */
/* ---------- */
typedef struct memnod
  {                             /* Memory block header info     */
      uint16      mh_tag;       /* Special ident tag            */
      size_t      mh_size;      /* Size of allocation block     */
#if defined(MEM_LIST)
      struct memnod *mh_next;   /* Next memory block            */
      struct memnod *mh_prev;   /* Previous memory block        */
#endif
#if defined(MEM_WHERE)
      char       *mh_file;      /* File allocation was from     */
      uint16      mh_line;      /* Line allocation was from */
#endif
  }
MEMHDR;

/* Alignment macros */
/* ---------------- */
#define ALIGN_SIZE sizeof(double)

#define HDR_SIZE sizeof(MEMHDR)

#if defined(MEM_HEADER)
#define BLOCK_SIZE  5
#define HEADER_SIZE (sizeof(unsigned char)*BLOCK_SIZE)
#define RESERVE_SIZE ((((HDR_SIZE+(ALIGN_SIZE-1))/ALIGN_SIZE)*ALIGN_SIZE)+HEADER_SIZE)
#else
#define BLOCK_SIZE  0
#define HEADER_SIZE 0
#define RESERVE_SIZE (((HDR_SIZE+(ALIGN_SIZE-1))/ALIGN_SIZE)*ALIGN_SIZE)
#endif

/* Conversion macros */
/* ----------------- */
#define CLIENT_2_HDR(a) ((MEMHDR *) (((char *)(a)) - RESERVE_SIZE))
#define HDR_2_CLIENT(a) ((void *) (((char *)(a)) + RESERVE_SIZE))

/* Local variables */
/* --------------- */
static unsigned long mem_size = 0;  /* Amount of memory used */
#if defined(MEM_LIST)
static MEMHDR *memlist = NULL;  /* List of memory blocks */
#endif

/* Local functions */
/* --------------- */
static void mem_tag_err(void *, int, char *, char *, int);  /* Tag error */
#if defined(MEM_LIST)
static void mem_list_add(MEMHDR *);     /* Add block to list */
static void mem_list_delete(MEMHDR *);  /* Delete block from list */
#define Mem_Tag_Err(a,b,c) mem_tag_err(a,b,c,fil,lin)
#else
#define Mem_Tag_Err(a,b,c) mem_tag_err(a,b,c,__FILE__,__LINE__)
#endif

#define ME_BADTAG       0
#define ME_BADHEADER    1

/************************************************************************/
/**** Functions accessed only through macros ****************************/
/************************************************************************/

/*----------------------------------------------------------------------
 *
 *  mem_HDmalloc()
 *  Allocate a memory block
 *
 *  Usage:
 *
 *  void *mem_HDmalloc(size_t size)
 *
 *  Parameters:
 *
 *  size        Size of block in bytes to allocate
 *
 *  Return Value:
 *
 *  Pointer to allocated memory block
 *  NULL if not enough memory
 *
 *  Description:
 *
 *  mem_HDmalloc() makes a protected call to HDmalloc()
 *
 *  Notes
 *
 *  Access this routine using the malloc() macro in memdebug.h
 *
 *
 */

void       *
mem_HDmalloc(
#if defined(MEM_WHERE)
                  size_t size,
                  char *fil,
                  int lin
#else
                  size_t size
#endif
)

{
    MEMHDR     *p;

/* Allocate memory block */
/* --------------------- */
    p = HDmalloc(RESERVE_SIZE + size + HEADER_SIZE);
    if (p == NULL)
      {
          fprintf(stdaux, "NULL pointer malloc'ed in %s, line %d\n", fil, lin);
          return (NULL);
      }     /* end if */

/* Init header */
/* ----------- */
    p->mh_tag = MEMTAG;
    p->mh_size = size;
    mem_size += size;
#if defined(MEM_WHERE)
    p->mh_file = fil;
    p->mh_line = (uint16) lin;
#endif

#if defined(MEM_HEADER)
    HDmemset((char *) HDR_2_CLIENT(p) - HEADER_SIZE, HEADERTAG, HEADER_SIZE);
    HDmemset((char *) HDR_2_CLIENT(p) + size, HEADERTAG, HEADER_SIZE);
#endif

#if defined(MEM_LIST)
    mem_list_add(p);
#endif

/* Return pointer to client data */
/* ----------------------------- */
    return (HDR_2_CLIENT(p));
}   /* end mem_alloc() */

/*----------------------------------------------------------------------
 *
 *  mem_HDrealloc()
 *  Reallocate a memory block
 *
 *  Usage:
 *
 *  void *mem_HDrealloc(void *ptr,size_t size)
 *
 *  Parameters:
 *
 *  ptr     Pointer to current block
 *  size    Size to adjust block to
 *
 *  Return Value:
 *
 *  Pointer to new memory block
 *  NULL if memory cannot be reallocated
 *
 *  Description:
 *
 *  mem_HDrealloc() makes a protected call to HDrealloc().
 *
 *  Notes:
 *
 *  Access this routine using the realloc() macro in maldebug.h
 *
 *
 */

void       *
mem_HDrealloc(
#if defined(MEM_WHERE)
                    void *ptr,
                    size_t size,
                    char *fil,
                    int lin
#else
                    void *ptr,
                    size_t size
#endif
)

{
    MEMHDR     *p;
#if defined(MEM_HEADER) || defined(MEM_COMP_FREE)
    unsigned char *q;
    int         i;
#endif
    char       *FUNC = "HDrealloc";

/* Check for equivalent to malloc() call, i.e. where ptr==NULL */
    if (ptr == NULL)
        return (mem_HDmalloc(size
#if defined(MEM_WHERE)
                               ,fil, lin
#endif
                ));

/* Convert client pointer to header pointer */
/* ---------------------------------------- */
    p = CLIENT_2_HDR(ptr);

/* Check for valid block */
/* --------------------- */
    if (p->mh_tag != MEMTAG)
      {
          Mem_Tag_Err(p, ME_BADTAG, FUNC);
          return (NULL);
      }     /* end if */

/* Check for overwrites into the header & footer */
/* --------------------------------------------- */
#if defined(MEM_HEADER)
    q = (unsigned char *) ptr - HEADER_SIZE;    /* Check the Header to consistancy */
    for (i = 0; i < BLOCK_SIZE; i++)
      {
          if (q[i] != HEADERTAG)
            {
                Mem_Tag_Err(p, ME_BADHEADER, FUNC);
                return (NULL);
            }   /* end if */
      }     /* end for */
    q = (unsigned char *) ptr + p->mh_size;     /* Check the Footer for consistancy */
    for (i = 0; i < BLOCK_SIZE; i++)
      {
          if (q[i] != HEADERTAG)
            {
                Mem_Tag_Err(p, ME_BADHEADER, FUNC);
                return (NULL);
            }   /* end if */
      }     /* end for */
#endif

/* Invalidate header */
/* ----------------- */
    p->mh_tag = ~MEMTAG;
    mem_size -= p->mh_size;

#if defined(MEM_WHERE)
    mem_list_delete(p);     /* Remove block from list */
#endif

/* Reallocate memory block */
/* ----------------------- */
    p = (MEMHDR *) HDrealloc(p, RESERVE_SIZE + size + HEADER_SIZE);
    if (p == NULL)
        return (NULL);

/* Update header */
/* ------------- */
    p->mh_tag = MEMTAG;
    p->mh_size = size;
    mem_size += size;
#if defined(MEM_LIST)
    p->mh_file = fil;
    p->mh_line = (uint16) lin;
#endif

#if defined(MEM_WHERE)
    mem_list_add(p);    /* Add block to list */
#endif

#if defined(MEM_HEADER)
    HDmemset((char *) HDR_2_CLIENT(p) - HEADER_SIZE, HEADERTAG, HEADER_SIZE);
    HDmemset((char *) HDR_2_CLIENT(p) + size, HEADERTAG, HEADER_SIZE);
#endif

/* Return pointer to client data */
/* ----------------------------- */
    return (HDR_2_CLIENT(p));
}   /* end mem_realloc() */

/*----------------------------------------------------------------------
 *
 *  mem_HDfree()
 *  Free a memory block
 *
 *  Usage:
 *
 *  void mem_HDfree(void   *ptr)
 *
 *  Parameters:
 *
 *  ptr     Pointer to memory to free
 *
 *  Return Value:
 *
 *  None
 *
 *  Description:
 *
 *  mem_HDfree() frees the specified memory block. The
 *  block must be allocated using mem_HDmalloc() or mem_HDrealloc().
 *
 *  Notes
 *
 *  Access this routine using the HDfree() macro in maldebug.h
 *
 *
 */

void       *
mem_HDfree(
#if defined(MEM_WHERE)
                   void *ptr,
                   char *fil,
                   int lin
#else
                   void *ptr
#endif
)

{
    MEMHDR     *p;
#if defined(MEM_HEADER) || defined(MEM_COMP_FREE)
    unsigned char *q;
    uintn       i;
#endif
    char       *FUNC = "HDfree";

/* Convert client pointer to header pointer */
/* ---------------------------------------- */
    p = CLIENT_2_HDR(ptr);

/* Check for valid block */
/* --------------------- */
    if (p->mh_tag != MEMTAG)
      {
          Mem_Tag_Err(p, ME_BADTAG, FUNC);
          return (NULL);
      }     /* end if */

/* Check for overwrites into the header & footer */
/* --------------------------------------------- */
#if defined(MEM_HEADER)
    q = (unsigned char *) ptr - HEADER_SIZE;    /* Check the Header to consistancy */
    for (i = 0; i < BLOCK_SIZE; i++)
      {
          if (q[i] != HEADERTAG)
            {
                Mem_Tag_Err(p, ME_BADHEADER, FUNC);
                return (NULL);
            }   /* end if */
      }     /* end for */
    q = (unsigned char *) ptr + p->mh_size;     /* Check the Footer for consistancy */
    for (i = 0; i < BLOCK_SIZE; i++)
      {
          if (q[i] != HEADERTAG)
            {
                Mem_Tag_Err(p, ME_BADHEADER, FUNC);
                return (NULL);
            }   /* end if */
      }     /* end for */
#endif

/* Invalidate header */
/* ----------------- */
    p->mh_tag = ~MEMTAG;
    mem_size -= p->mh_size;

/* Invalidate the block of memory to be free'd */
/* ------------------------------------------- */
#if defined(MEM_COMP_FREE)
    q = (unsigned char *) ptr;
    for (i = 0; i < p->mh_size; i++)
        q[i] = (unsigned char) ~q[i];
#endif

#if defined(MEM_LIST)
    mem_list_delete(p);     /* Remove block from list */
#endif

/* Free memory block */
/* ----------------- */
    return (HDfree(p));
}   /* end mem_free() */

/************************************************************************/
/**** Functions accessed directly ***************************************/
/************************************************************************/

/*----------------------------------------------------------------------
 *
 *  Mem_Used()
 *  Return amount of memory currently allocated
 *
 *  Usage:
 *
 *  unsigned long Mem_Used()
 *
 *  Parameters:
 *
 *  None.
 *
 *  Description:
 *
 *  Mem_Used() returns the number of bytes currently allocated
 *  using the memory management system. The value returned is
 *  simply the sum of the size requests to allocation routines.
 *  It does not reflect any overhead required by the memory
 *  management system.
 *
 *  Notes:
 *
 *  None
 *
 *
 */

unsigned long
Mem_Used(void)
{
    return (mem_size);
}   /* end Mem_Used() */

/*----------------------------------------------------------------------
 *
 *  Mem_Display()
 *  Display memory allocation list
 *
 *  Usage:
 *
 *  void Mem_Display(FILE *fp)
 *
 *  Parameters:
 *
 *  fp      File to output data to
 *
 *  Description:
 *
 *  Mem_Display() displays the contents of the memory
 *  allocation list.
 *
 *  This function is a no-op if MEM_LIST is not defined.
 *
 *  Notes:
 *
 *  None
 *
 *
 */

void
Mem_Display(FILE * fp)
{
#if defined(MEM_LIST)
    MEMHDR     *p;
    int         idx;
#if defined(MEM_HEADER)
    unsigned char *q;
    int         i;
#endif

#if defined(MEM_WHERE)
    fprintf(fp, "Index   Size  File(Line) - total size %lu\n", mem_size);
#else
    fprintf(fp, "Index   Size - total size %lu\n", mem_size);
#endif

    idx = 0;
    p = memlist;
    while (p != NULL)
      {
          fprintf(fp, "%-5d %6u", idx++, p->mh_size);
#if defined(MEM_WHERE)
          fprintf(fp, "  %s(%d)", p->mh_file, p->mh_line);
#endif
          if (p->mh_tag != MEMTAG)
              fprintf(fp, " INVALID TAG");

/* Check for overwrites into the header & footer */
/* --------------------------------------------- */
#if defined(MEM_HEADER)
          q = (unsigned char *) HDR_2_CLIENT(p) - HEADER_SIZE;  /* Check the Header to consistancy */
          for (i = 0; i < BLOCK_SIZE; i++)
            {
                if (q[i] != HEADERTAG)
                  {
                      fprintf(fp, " HEADER OVERWRITTEN");
                      break;
                  }     /* end if */
            }   /* end for */
          q = (unsigned char *) HDR_2_CLIENT(p) + p->mh_size;   /* Check the Footer for consistancy */
          for (i = 0; i < BLOCK_SIZE; i++)
            {
                if (q[i] != HEADERTAG)
                  {
                      fprintf(fp, " FOOTER OVERWRITTEN");
                      break;
                  }     /* end if */
            }   /* end for */
#endif
          fprintf(fp, "\n");
          p = p->mh_next;
      }     /* end while */
#else
    fprintf(fp, "Memory list not compiled (MEM_LIST not defined)\n");
#endif
}   /* end Mem_Display() */

/************************************************************************/
/**** Memory list manipulation functions ********************************/
/************************************************************************/

/*
 * mem_list_add()
 * Add block to list
 */

#if defined(MEM_LIST)
static void
mem_list_add(MEMHDR * p)
{
    p->mh_next = memlist;
    p->mh_prev = NULL;
    if (memlist != NULL)
        memlist->mh_prev = p;
    memlist = p;

#if defined(DEBUG_LIST)
    printf("mem_list_add()\n");
    Mem_Display(stdout);
#endif
}   /* end mem_list_add() */
#endif

/*----------------------------------------------------------------------*/

/*
 * mem_list_delete()
 * Delete block from list
 */

#if defined(MEM_LIST)
static void
mem_list_delete(MEMHDR * p)
{
    if (p->mh_next != NULL)
        p->mh_next->mh_prev = p->mh_prev;
    if (p->mh_prev != NULL)
        p->mh_prev->mh_next = p->mh_next;
    else
        memlist = p->mh_next;

#if defined(DEBUG_LIST)
    printf("mem_list_delete()\n");
    Mem_Display(stdout);
#endif
}   /* end mem_list_delete() */
#endif

/************************************************************************/
/**** Error display *****************************************************/
/************************************************************************/

/*
 *  mem_tag_err()
 *  Display memory tag error
 */
static void
mem_tag_err(void *p, int type, char *func, char *fil, int lin)
{
    FILE       *fp;

    /* shut compiler up */
    fp = fp;

#ifdef OLD_WAY
    fprintf(stdaux, "Malloc tag error #%d, in %s : %p - %s(%d)\n", type, func, p, fil, lin);
    if ((fp = fopen("impro.err", "wt+")) != NULL)
      {     /* open impro.err to output the error file */
          fprintf(fp, "Malloc tag error - %p - %s(%d)\n", p, fil, lin);
#if defined(MEM_LIST)
          Mem_Display(fp);
#endif
          fclose(fp);
      }     /* end if */
#else
    fprintf(stdaux, "Malloc tag error #%d, in %s : %p - %s(%d)\n", type, func, p, fil, lin);
    getch();
#if defined(MEM_LIST)
    Mem_Display(stdaux);
#endif
#endif
}   /* end mem_tag_err() */
