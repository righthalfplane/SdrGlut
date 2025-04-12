/*-
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*****************************************************************************
 * File: mcache.c
 *
 * This is a modfied version of the original Berkley code for
 * manipulating a memory pool. This version however is not 
 * compatible with the original Berkley version.
 *
 * NOTE: references to pages here is the same as chunks
 *
 * AUTHOR - George V.- 1996/08/22
 *****************************************************************************/ 

#ifdef RCSID
static char RcsId[] = "@(#)$Revision: 5146 $";
#endif

/* $Id: mcache.c 5146 2009-01-14 17:46:57Z fbaker $ */

/*
 *  NOTE:
 *    Here pages are the same thing as chunks.
 *    I just didn't bother to change all references from pages to chunks.
 *
 *    -georgev
 */

/* for debugging */
/*
#define MCACHE_DEBUG
*/
/*
#define STATISTICS
*/

#define	__MCACHEINTERFACE_PRIVATE
#include "hdf.h"     /* number types ..etc */
#include "hqueue.h"  /* Circluar queue functions(Macros) */
#include "mcache.h"

#if defined(hpux) || defined(__hpux) || defined(__hpux__)
#include <sys/resource.h>
#include <sys/syscall.h>
#define getrusage(a, b)  syscall(SYS_GETRUSAGE, a, b)
#endif /* hpux */


/* Private routines */
static BKT  *mcache_bkt   (MCACHE *mp);
static BKT  *mcache_look  (MCACHE *mp, int32 pgno);
static intn  mcache_write (MCACHE *mp, BKT *bkt);

/******************************************************************************
NAME
    mcache_get_npages - returns current number of pages for object

DESCRIPTION
    Finds current number of pages in object.

RETURNS
    Returns current number of pages in object.
******************************************************************************/
int32
mcache_get_npages(
    MCACHE *mp /* IN: MCACHE cookie */)
{
    if(mp != NULL)
        return mp->npages;
    else
        return 0;
} /* mcache_get_npages */

/******************************************************************************
NAME
    mcache_get_maxcache - returns current number of pages cached.

DESCRIPTION
    Finds current number of pages cached for object.

RETURNS
    Returns current number of pages cached.
******************************************************************************/
int32
mcache_get_maxcache(
    MCACHE *mp /* IN: MCACHE cookie */)
{
    if (mp != NULL)
        return mp->maxcache;
    else
        return 0;
} /* mcache_get_maxcache */

/******************************************************************************
NAME
    mcache_set_maxcache - sets current number of pages to cache.

DESCRIPTION
    Sets current number of pages to cached for object to 'maxcache'.

RETURNS
    Returns current number of pages cached.
******************************************************************************/
int32
mcache_set_maxcache(
    MCACHE *mp,     /* IN: MCACHE cookie */
    int32  maxcache /* IN: max pages to cache */)
{
    if (mp != NULL)
      { /* currently allow the current cache to grow up */
          if (mp->maxcache < maxcache)
              mp->maxcache = maxcache;
          else /* maxcache is less than current maxcache */
            {   /* if current number of cached pages is less than request 
                   then set to 'maxcache', 
                   else we don't currently handle decreasing
                   the curcache to 'maxcache' */
                if (maxcache > mp->curcache)
                    mp->maxcache = maxcache;
            }
          return mp->maxcache;
      }
    else
        return 0;
} /* mcache_set_maxcache */

/******************************************************************************
NAME
    mcache_get_pagsize - returns pagesize for object

DESCRIPTION
    Finds current pagesize used for object.

RETURNS
    returns pagesize for object.
******************************************************************************/
int32
mcache_get_pagesize( 
    MCACHE *mp /* IN: MCACHE cookie */)
{
    if (mp != NULL)
        return mp->pagesize;
    else
        return 0;
} /* mcache_get_pagesize */

/******************************************************************************
NAME
   mcache_open -- Open a memory pool on the given object

DESCRIPTION
   Initialize a memory pool for object using the given pagesize
   and size of object. 

   Note for 'flags' input only '0' should be used for now.

RETURNS
   A memory pool cookie if successful else NULL

NOTE: 
      The key string byte for sharing buffers is not implemented.
******************************************************************************/
MCACHE *
mcache_open(
    VOID    *key,        /* IN: byte string used as handle to share buffers */
    int32   object_id,   /* IN: object handle */
    int32   pagesize,    /* IN: chunk size in bytes  */
    int32   maxcache,    /* IN: maximum number of pages to cache at any time */
    int32   npages,      /* IN: number of chunks currently in object */
    int32   flags        /* IN: 0= object exists, 1= does not exist  */)
{
    CONSTR(FUNC, "mcache_open");   /* for HERROR */
    struct _lhqh *lhead = NULL; /* head of an entry in list hash chain */
    MCACHE       *mp    = NULL; /* MCACHE cookie */
    L_ELEM       *lp    = NULL;
    intn          ret_value   = RET_SUCCESS;
    intn          entry;         /* index into hash table */
    int32         pageno;

    /* shut compiler up */
    key=key;

    /* Set the pagesize and max # of pages to cache */
    if(pagesize == 0)
        pagesize = (int32)DEF_PAGESIZE;
    if (maxcache == 0)
        maxcache = (int32)DEF_MAXCACHE;

    /* Allocate and initialize the MCACHE cookie. */
    if ((mp = (MCACHE *)HDcalloc(1, sizeof(MCACHE))) == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    CIRCLEQ_INIT(&mp->lqh);
    for (entry = 0; entry < HASHSIZE; ++entry)
      {
          CIRCLEQ_INIT(&mp->hqh[entry]);
          CIRCLEQ_INIT(&mp->lhqh[entry]);
      }

    /* Initialize max # of pages to cache and number of pages in object */
    mp->maxcache = (int32)maxcache;
    mp->npages   = npages;

    /* Set pagesize and object handle and current object size */
    mp->pagesize = pagesize;
    mp->object_id = object_id;
    mp->object_size = pagesize * npages;

    /* Initialize list hash chain */
    for (pageno = 1; pageno <= mp->npages; ++pageno)
      {
          lhead = &mp->lhqh[HASHKEY(pageno)];
          if ((lp = (L_ELEM *)HDmalloc(sizeof(L_ELEM))) == NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
          lp->pgno   = (int32)pageno;     /* set page number */

          /* check if object exists already 
             The usefullness of this flag is yet to be
             determined. Currently '0' should be used */
          if (flags == 0)
              lp->eflags = (uint8)ELEM_SYNC; /* valid page exists on disk */
          else
              lp->eflags = (uint8)0; /* page does not exist on disk */
#ifdef STATISTICS
          lp->elemhit = 0;
          ++(mp->listalloc);
#endif
          CIRCLEQ_INSERT_HEAD(lhead, lp, hl); /* add to list */
      } /* end for pageno */

    /* initialize input/output filters and cookie to NULL */
    mp->pgin     = NULL;
    mp->pgout    = NULL;        
    mp->pgcookie = NULL;        
#ifdef STATISTICS
    mp->listhit    = 0;
    mp->cachehit   = 0;
    mp->cachemiss  = 0;
    mp->pagealloc  = 0;
    mp->pageflush  = 0;
    mp->pageget    = 0;
    mp->pagenew    = 0;
    mp->pageput    = 0;
    mp->pageread   = 0;
    mp->pagewrite  = 0;
#endif

  done:
    if(ret_value == RET_ERROR)
      { /* error cleanup */
          if (mp != NULL)
              HDfree(mp);
          /* free up list elements */
          for (entry = 0; entry < HASHSIZE; ++entry)
            {
                while ((lp = mp->lhqh[entry].cqh_first) != (VOID *)&mp->lhqh[entry]) 
                  {
                      CIRCLEQ_REMOVE(&mp->lhqh[entry], mp->lhqh[entry].cqh_first, hl);
                      HDfree(lp);
                  }
            } /* end for entry */
#ifdef MCACHE_DEBUG
          (VOID)fprintf(stderr,"mcache_open: ERROR \n");
#endif      
          mp = NULL; /* return value */
      } /* end error cleanup */
    /* Normal cleanup */
#ifdef MCACHE_DEBUG
    (VOID)fprintf(stderr,"mcache_open: mp->pagesize=%lu\n",mp->pagesize);
    (VOID)fprintf(stderr,"mcache_open: mp->maxcache=%u\n",mp->maxcache);
    (VOID)fprintf(stderr,"mcache_open: mp->npages=%u\n",mp->npages);
    (VOID)fprintf(stderr,"mcache_open: flags=%u\n",flags);
#ifdef STATISTICS
    (VOID)fprintf(stderr,"mcache_open: mp->listalloc=%lu\n",mp->listalloc);
#endif
#endif

    return (mp);
} /* mcache_open () */

/******************************************************************************
NAME
   mcache_filter -- Initialize input/output filters.

DESCRIPTION
   Initialize input/output filters for user page processing.
   These are used to read/write the chunks.

RETURNS
   Nothing

******************************************************************************/
VOID
mcache_filter( 
    MCACHE *mp,                                            /* IN: MCACHE cookie */
    int32 (*pgin) (VOID * /* cookie */, int32 /* pgno */, VOID * /* page */),  /* IN: page in filter */
    int32 (*pgout) (VOID * /* cookie */, int32 /* pgno */, const VOID * /*page */), /* IN: page out filter */
    VOID *pgcookie                                         /* IN: filter cookie */)
{
    mp->pgin     = pgin;
    mp->pgout    = pgout;
    mp->pgcookie = pgcookie;
} /* mcache_filter() */

#if 0 /* NOT USED YET */
/******************************************************************************
NAME
   mcache_new -- get a new page of memory and extend memory pool.

DESCRIPTION
    Get a new page of memory. This is where we get new pages for the object.
    This will only return a full page of memory. 
    If the last page is an odd size the user must keep track
    of this as only lastpagesize bytes will be written out.
    As a result if the user fills the last page and
    lastpagesize does not equal pagesize the user will lose data.

    If 'flags' = 0, increase number of pages by 1 and return
                   *pgnoaddr = npages

    If 'flags' = MCACHE_EXTEND, set page to *pgnoaddr and
                               npages = *pgnoaddr.

    All returned pages are pinned.

RETURNS
    Returns the new page if successfull and NULL otherwise

NOTE:
    This is not used currently.
******************************************************************************/
VOID *
mcache_new(
    MCACHE *mp,       /* IN: MCACHE cookie */
    int32  *pgnoaddr, /* IN/OUT: address of newly create page */
    int32  flags      /* IN:MCACHE_EXTEND or 0 */)
{
    CONSTR(FUNC, "mcache_new");   /* for HERROR */
    struct _hqh  *head  = NULL; /* head of an entry in hash chain */
    struct _lhqh *lhead = NULL; /* head of an entry in list hash chain */
    BKT          *bp   = NULL;  /* bucket element */
    L_ELEM       *lp   = NULL;
    intn          ret_value = RET_SUCCESS;

    /* check inputs */
    if (mp == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* page overflow? */
    if (mp->npages == MAX_PAGE_NUMBER) 
        HE_REPORT_GOTO("page allocation overflow", FAIL);

#ifdef STATISTICS
    ++mp->pagenew;
#endif
    /*
     * Get a BKT from the cache.  
     * Assign a new page number based upon 'flags'. If flags 
     * is MCACHE_EXTEND then we want to extend object up to '*pgnoaddr' pages.
     * attach it to the head of the hash chain, the tail of the lru chain,
     * and return.
     */
    if ((bp = mcache_bkt(mp)) == NULL)
        HE_REPORT_GOTO("unable to get a new page from bucket", FAIL);

    if (!(flags & MCACHE_EXTEND))
      { /* we increase by one page */
          mp->npages++;                      /* number of pages */
          *pgnoaddr = bp->pgno = mp->npages; /* page number */
      } 
    else 
      { /* we extend to *pgnoaddr pages */
          if (*pgnoaddr > MAX_PAGE_NUMBER) 
            {
                (VOID)fprintf(stderr, "mcache_new: page allocation overflow.\n");
                abort();
            }

          bp->pgno = *pgnoaddr;      /* page number to create */
          mp->npages= *pgnoaddr; /* number of pages */
      }
#ifdef MCACHE_DEBUG
    (VOID)fprintf(stderr,"mcache_new: increasing #of pages to=%d\n",mp->npages);
#endif  

    /* Pin the page and insert into head of hash chain 
     * and tail of lru chain */
    bp->flags = MCACHE_PINNED;
    head = &mp->hqh[HASHKEY(bp->pgno)];
    CIRCLEQ_INSERT_HEAD(head, bp, hq);
    CIRCLEQ_INSERT_TAIL(&mp->lqh, bp, q);
  
    /* Check to see if this page has ever been referenced */
    lhead = &mp->lhqh[HASHKEY(bp->pgno)];
    for (lp = lhead->cqh_first; lp != (VOID *)lhead; lp = lp->hl.cqe_next)
        if (lp->pgno == bp->pgno)
          { /* hit */
#ifdef STATISTICS
              ++mp->listhit;
              ++lp->elemhit;
#endif
              /* we are done */
              ret_value = RET_SUCCESS;
              goto done;
          } /* end if lp->pgno */

    /* NO hit, new list element */
    if ((lp = (L_ELEM *)HDmalloc(sizeof(L_ELEM))) == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    lp->pgno   = bp->pgno;
    lp->eflags = 0;
#ifdef STATISTICS
    lp->elemhit = 0;
    ++mp->listalloc;
#endif
    CIRCLEQ_INSERT_HEAD(lhead, lp, hl); /* add to list */

#ifdef MCACHE_DEBUG
#ifdef STATISTICS
    (VOID)fprintf(stderr,"mcache_new: mp->listalloc=%d \n", mp->listalloc);
#endif
#endif
  done:
    if(ret_value == RET_ERROR)
      { /* error cleanup */
          if(lp != NULL)
              HDfree(lp);

          return NULL;
      }
    /* Normal cleanup */

    return (bp->page);
} /* mcache_new() */

#endif /* commented out for later */

/******************************************************************************
NAME
   mcache_get - get a specified page by page number.

DESCRIPTION
    Get a page specified by 'pgno'. If the page is not cached then
    we need to create a new page. All returned pages are pinned.

RETURNS
   The specifed page if successful and NULL otherwise
******************************************************************************/
VOID *
mcache_get( 
    MCACHE *mp,  /* IN: MCACHE cookie */
    int32  pgno, /* IN: page number */
    int32  flags /* IN: XXX not used? */)
{
    CONSTR(FUNC, "mcache_get");   /* for HERROR */
    struct _hqh  *head  = NULL; /* head of lru queue */
    struct _lhqh *lhead = NULL; /* head of an entry in list hash chain */
    BKT          *bp   = NULL;  /* bucket element */
    L_ELEM       *lp   = NULL;
    intn         ret_value   = RET_SUCCESS;
#ifdef UNUSED
    int32        rpagesize;   /* pagesize to read */
#endif /* UNUSED */
    intn         list_hit;    /* hit flag */

    /* shut compiler up */
    flags=flags;

#ifdef MCACHE_DEBUG
    (VOID)fprintf(stderr,"mcache_get: entering \n");
#endif
    /* check inputs */
    if (mp == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Check for attempting to retrieve a non-existent page. 
     *  remember pages go from 1 ->npages  */
    if (pgno > mp->npages) 
        HE_REPORT_GOTO("attempting to get a non existant page from cache", FAIL);

#ifdef STATISTICS
    ++mp->pageget;
#endif

    /* Check for a page that is cached. */
    if ((bp = mcache_look(mp, pgno)) != NULL) 
      {
#ifdef MCACHE_DEBUG
          if (bp->flags & MCACHE_PINNED) 
            {
                (VOID)fprintf(stderr,
                              "mcache_get: page %d already pinned\n", bp->pgno);
                abort();
            }
#endif
          /*
           * Move the page to the head of the hash chain and the tail
           * of the lru chain.
           */
          head = &mp->hqh[HASHKEY(bp->pgno)];
          CIRCLEQ_REMOVE(head, bp, hq);
          CIRCLEQ_INSERT_HEAD(head, bp, hq);
          CIRCLEQ_REMOVE(&mp->lqh, bp, q);
          CIRCLEQ_INSERT_TAIL(&mp->lqh, bp, q);
          /* Return a pinned page. */
          bp->flags |= MCACHE_PINNED;

#ifdef MCACHE_DEBUG
          (VOID)fprintf(stderr,"mcache_get: getting cached bp->pgno=%d,npages=%d\n",
                        bp->pgno,mp->npages);
#endif   
          /* update this page reference */
          lhead = &mp->lhqh[HASHKEY(bp->pgno)];
          for (lp = lhead->cqh_first; lp != (VOID *)lhead; lp = lp->hl.cqe_next)
              if (lp->pgno == bp->pgno)
                { /* hit */
#ifdef STATISTICS
                    ++mp->listhit;
                    ++lp->elemhit;
#endif
                    break;
                } /* end if lp->pgno */

          /* we are done */
          ret_value = RET_SUCCESS;
          goto done;
      } /* end if bp */

#ifdef MCACHE_DEBUG
    (VOID)fprintf(stderr,"mcache_get: NOT cached page\n");
#endif

    /* Page not cached so
     * Get a page from the cache to use or create one. */
    if ((bp = mcache_bkt(mp)) == NULL)
        HE_REPORT_GOTO("unable to get a new page from bucket", FAIL);

    /* Check to see if this page has ever been referenced */
    list_hit = 0;
    lhead = &mp->lhqh[HASHKEY(pgno)];
    for (lp = lhead->cqh_first; lp != (VOID *)lhead; lp = lp->hl.cqe_next)
        if (lp->pgno == pgno && lp->eflags != 0)
          { /* hit */
#ifdef STATISTICS
              ++mp->listhit;
              ++lp->elemhit;
#endif
              list_hit = 1;
              break;
          } /* end if lp->pgno */

    /* If there is no hit then we allocate a new element 
     *  and insert into hash table */
    if (!list_hit)
      { /* NO hit, new list element 
         * no need to read this page from disk */
          if ((lp = (L_ELEM *)HDmalloc(sizeof(L_ELEM))) == NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);

          lp->pgno = pgno;
          lp->eflags = 0;
#ifdef STATISTICS
          ++mp->listalloc;
          lp->elemhit =1;
#endif
          CIRCLEQ_INSERT_HEAD(lhead, lp, hl); /* add to list */
#ifdef MCACHE_DEBUG
          (VOID)fprintf(stderr,"mcache_get: skiping reading in page=%u\n",pgno);
#endif
      } /*end if !list_hit */
    else
      { /* list hit, need to read page */
          lp->eflags = ELEM_READ; /* Indiate we are reading this page */

#ifdef STATISTICS
          ++mp->pageread;
#endif

#ifdef UNUSED
          rpagesize = mp->pagesize;
#endif /* UNUSED */

          /* Run through the user's filter. 
             we use this fcn to read in the data chunk/page.
             Not the original intention. */
          if (mp->pgin != NULL)
            {  /* Note page numbers in HMCPxxx are 0 based not 1 based */
                if (((mp->pgin)(mp->pgcookie, pgno-1, bp->page)) == FAIL)
                  {
                      HEreport("mcache_get: error reading chunk=%d\n",(intn)pgno-1);
                      lp = NULL; /* don't clobber the cache! */
                      ret_value = RET_ERROR;
                      goto done;
                  }
            }
          else
            {
                HEreport("mcache_get: reading fcn not set,chunk=%d\n",(intn)pgno-1);
                lp = NULL;
                ret_value = RET_ERROR;
                goto done;
            }
      } /* end else list hit */

    /* Set the page number, pin the page. */
    bp->pgno = pgno;
    bp->flags = MCACHE_PINNED;

    /*
     * Add the page to the head of the hash chain and the tail
     * of the lru chain.
     */
    head = &mp->hqh[HASHKEY(bp->pgno)];
    CIRCLEQ_INSERT_HEAD(head, bp, hq);
    CIRCLEQ_INSERT_TAIL(&mp->lqh, bp, q);

  done:
    if(ret_value == RET_ERROR)
      { /* error cleanup */
#ifdef MCACHE_DEBUG
          (VOID)fprintf(stderr,"mcache_get: Error exiting \n");
#endif
          if (lp!=NULL)
              HDfree(lp);
          return NULL;
      }
    /* Normal cleanup */
#ifdef MCACHE_DEBUG
    (VOID)fprintf(stderr,"mcache_get: Exiting \n");
#endif
    return (bp->page);
} /* mcache_get() */

/******************************************************************************
NAME
   mcache_put -- put a page back into the memory buffer pool

DESCRIPTION
    Return a page to the buffer pool. Unpin it and mark it 
    appropriately i.e. MCACHE_DIRTY

RETURNS
    RET_SUCCESS if succesful and RET_ERROR otherwise
******************************************************************************/
intn
mcache_put(
    MCACHE  *mp,   /* IN: MCACHE cookie */
    VOID    *page, /* IN: page to put */
    int32   flags  /* IN: flags = 0, MCACHE_DIRTY */)
{
    CONSTR(FUNC, "mcache_put");   /* for HERROR */
    struct _lhqh *lhead = NULL; /* head of an entry in list hash chain */
    L_ELEM       *lp    = NULL;
    BKT          *bp = NULL;    /* bucket element ptr */
    intn          ret_value = RET_SUCCESS;

    /* check inputs */
    if (mp == NULL || page == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

#ifdef STATISTICS
    ++mp->pageput;
#endif
    /* get pointer to bucket element */
    bp = (BKT *)((char *)page - sizeof(BKT));
#ifdef MCACHE_DEBUG
    (VOID)fprintf(stderr,"mcache_put: putting page=%d\n",bp->pgno);
    if (!(bp->flags & MCACHE_PINNED)) 
      {
          (VOID)fprintf(stderr,
                        "mcache_put: page %d not pinned\n", bp->pgno);
          abort();
      }
#endif
    /* Unpin the page and mark it appropriately */
    bp->flags &= ~MCACHE_PINNED;
    bp->flags |= flags & MCACHE_DIRTY;

    if (bp->flags & MCACHE_DIRTY)
      { /* update this page reference */
          lhead = &mp->lhqh[HASHKEY(bp->pgno)];
          for (lp = lhead->cqh_first; lp != (VOID *)lhead; lp = lp->hl.cqe_next)
              if (lp->pgno == bp->pgno)
                { /* hit */
#ifdef STATISTICS
                    ++mp->listhit;
                    ++lp->elemhit;
#endif
                    lp->eflags = ELEM_WRITTEN;
                    break;
                } /* end if lp->pgno */
      }

  done:
    if(ret_value == RET_ERROR)
      { /* error cleanup */
          return ret_value;
      }
    /* Normal cleanup */

    return ret_value;
} /* mcache_put () */

/******************************************************************************
NAME
   mcache_close - close the memory buffer pool

DESCRIPTION
   Close the buffer pool.  Frees the buffer pool.
   Does not sync the buffer pool.

RETURNS
   RET_SUCCESS if succesful and RET_ERROR otherwise   
******************************************************************************/
intn
mcache_close(
    MCACHE *mp /* IN: MCACHE cookie */)
{
    CONSTR(FUNC, "mcache_close");   /* for HERROR */
    L_ELEM  *lp = NULL;
    BKT     *bp = NULL;   /* bucket element */
    intn     nelem = 0;
    intn     ret_value   = RET_SUCCESS;
    intn     entry;      /* index into hash table */

#ifdef MCACHE_DEBUG
    (VOID)fprintf(stderr,"mcache_close: entered \n");
#endif
    /* check inputs */
    if (mp == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Free up any space allocated to the lru pages. */
    while ((bp = mp->lqh.cqh_first) != (VOID *)&mp->lqh) 
      {
          CIRCLEQ_REMOVE(&mp->lqh, mp->lqh.cqh_first, q);
          HDfree(bp);
      }

    /* free up list elements */
    for (entry = 0; entry < HASHSIZE; ++entry)
      {
          while ((lp = mp->lhqh[entry].cqh_first) != (VOID *)&mp->lhqh[entry]) 
            {
                CIRCLEQ_REMOVE(&mp->lhqh[entry], mp->lhqh[entry].cqh_first, hl);
                HDfree(lp);
                nelem++;
            }
      } /* end for entry */

  done:
    if(ret_value == RET_ERROR)
      { /* error cleanup */
          return ret_value;
      }
    /* Normal cleanup */

    /* Free the MCACHE cookie. */
    HDfree(mp);

#ifdef MCACHE_DEBUG
    (VOID)fprintf(stderr,"mcache_close: freed %d list elements\n\n",nelem);
#endif
    return ret_value;
} /* mcache_close() */

/******************************************************************************
NAME
   mcache_sync -- sync the memory buffer pool

DESCRIPTION
   Sync the pool to disk. Does NOT Free the buffer pool.

RETURNS
   RET_SUCCESS if succesful and RET_ERROR otherwise   
******************************************************************************/
intn
mcache_sync(
    MCACHE *mp /* IN: MCACHE cookie */)
{
    CONSTR(FUNC, "mcache_sync");   /* for HERROR */
    BKT *bp = NULL; /* bucket element */
    intn ret_value = RET_SUCCESS;

#ifdef MCACHE_DEBUG
    (VOID)fprintf(stderr,"MCACHE_sync: entering \n");
#endif
    /* check inputs */
    if (mp == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Walk the lru chain, flushing any dirty pages to disk. */
    for (bp = mp->lqh.cqh_first; bp != (VOID *)&mp->lqh; bp = bp->q.cqe_next)
      {
          if (bp->flags & MCACHE_DIRTY && mcache_write(mp, bp) == RET_ERROR)
              HE_REPORT_GOTO("unable to flush a dirty page", FAIL);
      } /* end for bp */

  done:
    if(ret_value == RET_ERROR)
      { /* error cleanup */
          return ret_value;
      }
    /* Normal cleanup */

#ifdef MCACHE_DEBUG
    (VOID)fprintf(stderr,"mcache_sync: exiting \n");
#endif

    return ret_value;
} /* mcache_sync() */

#if 0  /* NOT USED */
/******************************************************************************
NAME
   mcache_page_sync -- write the specified page to disk given its page number

DESCRIPTION
   Write a cached page to disk given it's page number
   If the page is not cached return an error.

RETURNS
   RET_SUCCESS if succesful and RET_ERROR otherwise     

NOTE: No longer used.
      This was mainly used in the case where we extend the object.
      We need to mark the current object size by writing out
      the last page(or part of it) otherwise MCACHE_get() on
      an intermediate page between the current end of the object
      and the new end of object will fail.
******************************************************************************/
intn
mcache_page_sync(
    MCACHE  *mp,   /* IN: MCACHE cookie */
    int32   pgno,  /* IN: page number */
    int32   flags  /* IN: XXX not used? */)
{
    CONSTR(FUNC, "mcache_page_sync");   /* for HERROR */
    struct _lhqh *lhead = NULL; /* head of an entry in list hash chain */
    L_ELEM       *lp    = NULL;
    BKT          *bp    = NULL; /* bucket element */
    intn          ret_value = RET_SUCCESS;
    int32        wpagesize;         /* page size to write */

#ifdef MCACHE_DEBUG
    (VOID)fprintf(stderr,"mcache_page_sync: entering\n");
#endif  
    /* check inputs */
    if (mp == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Check for attempting to sync a non-existent page. 
     *  remember pages go from 1 ->npages  */
    if (pgno > mp->npages) 
        HE_REPORT_GOTO("attempting to get a non existant page from cache", FAIL);

    /* Check for a page that is cached. */
    if ((bp = mcache_look(mp, pgno)) != NULL) 
      {
#ifdef MCACHE_DEBUG
          if (bp->flags & MCACHE_PINNED) 
            {
                (VOID)fprintf(stderr,
                              "mcache_page_sync: page %u already pinned\n", bp->pgno);
                abort();
            }
#endif

          /* only flush the page if dirty */
          if (!(bp->flags & MCACHE_DIRTY))
            { /* done */
                ret_value = RET_SUCCESS;
                goto done;
            }

#ifdef STATISTICS
          ++mp->pagewrite;
#endif

          /* update this page reference */
          lhead = &mp->lhqh[HASHKEY(bp->pgno)];
          for (lp = lhead->cqh_first; lp != (VOID *)lhead; lp = lp->hl.cqe_next)
              if (lp->pgno == bp->pgno)
                { /* hit */
#ifdef STATISTICS
                    ++mp->listhit;
                    ++lp->elemhit;
#endif
                    lp->eflags = ELEM_SYNC;
                    break;
                }

          /* Run page through the user's filter.
             we use this to write the data chunk/page out.
             This deviates from the orignal purpose of the filter. */
          if (mp->pgout)
            { /* Note page numbers in HMCPxxx are 0 based not 1 based */
                if (((mp->pgout)(mp->pgcookie, bp->pgno - 1, bp->page)) == FAIL)
                  {
                      HEreport("error writing chunk=%d\n",(intn)bp->pgno);
                      ret_value = RET_ERROR;
                      goto done;
                  }
            }
          else
            {
                HEreport("writing fcn not set,chunk=%d\n",(intn)bp->pgno);
                ret_value = RET_ERROR;
                goto done;
            }

#ifdef MCACHE_DEBUG
          (VOID)fprintf(stderr,"mcache_page_sync: npages=%u\n",mp->npages);
#endif

          wpagesize = mp->pagesize;

          /* mark page as clean */
          bp->flags &= ~MCACHE_DIRTY;

      } /* end if cached page */
    else /* not a cached page!...we shouldn't encounter this */
        ret_value = RET_ERROR;

  done:
    if(ret_value == RET_ERROR)
      { /* error cleanup */
          return ret_value;
      }
    /* Normal cleanup */

#ifdef MCACHE_DEBUG
    (VOID)fprintf(stderr,"mcache_page_sync: exiting\n");
#endif  
    return ret_value;
} /* mcache_page_sync() */
#endif

/******************************************************************************
NAME
   mcache_bkt - Get a page from the cache (or create one).

DESCRIPTION
   Private routine. Get a page from the cache (or create one).

RETURNS
   A page if successful and NULL otherwise.
       
NOTE: Note that the size of the page allocated is equal to
      sizeof(bucket element) + pagesize. We only return the
      pagesize fragment to the user. The only caveat here is
      that a user could inadvertently clobber the bucket element
      information by writing out of the page size bounds.
******************************************************************************/
static BKT *
mcache_bkt(
    MCACHE *mp  /* IN: MCACHE cookie */)
{
    CONSTR(FUNC, "mcache_bkt");   /* for HERROR */
    struct _hqh *head = NULL;  /* head of hash chain */
    BKT         *bp   = NULL;  /* bucket element */
    intn         ret_value  = RET_SUCCESS;

    /* check inputs */
    if (mp == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* If under the max cached, always create a new page. */
    if ((int32)mp->curcache < (int32)mp->maxcache)
        goto new;

    /*
     * If the cache is max'd out, walk the lru list for a buffer we
     * can flush.  If we find one, write it (if necessary) and take it
     * off any lists.  If we don't find anything we grow the cache anyway.
     * The cache never shrinks.
     */
    for (bp = mp->lqh.cqh_first; bp != (VOID *)&mp->lqh; bp = bp->q.cqe_next)
        if (!(bp->flags & MCACHE_PINNED)) 
          { /* Flush if dirty. */
              if (bp->flags & MCACHE_DIRTY  && mcache_write(mp, bp) == RET_ERROR)
                  HE_REPORT_GOTO("unable to flush a dirty page", FAIL);
#ifdef STATISTICS
              ++mp->pageflush;
#endif
              /* Remove from the hash and lru queues. */
              head = &mp->hqh[HASHKEY(bp->pgno)];
              CIRCLEQ_REMOVE(head, bp, hq);
              CIRCLEQ_REMOVE(&mp->lqh, bp, q);
#ifdef MCACHE_DEBUG
              { VOID *spage;
              spage = bp->page;
              memset(bp, 0xff, sizeof(BKT) + mp->pagesize);
              bp->page = spage;
              }
#endif
              /* done */
              ret_value = RET_SUCCESS;
              goto done;
          } /* end if bp->flags */

    /* create a new page */
  new: if ((bp = (BKT *)HDmalloc(sizeof(BKT) + (uintn)mp->pagesize)) == NULL)
      HGOTO_ERROR(DFE_NOSPACE, FAIL);

#ifdef STATISTICS
  ++mp->pagealloc;
#endif

#if defined(MCACHE_DEBUG) || defined(PURIFY)
  memset(bp, 0xff, sizeof(BKT) + mp->pagesize);
#endif

  /* set page ptr past bucket element section */
  bp->page = (char *)bp + sizeof(BKT);
  ++mp->curcache; /* increase number of cached pages */

  done:
  if(ret_value == RET_ERROR)
    { /* error cleanup */
        if (bp != NULL)
            HDfree(bp);

        return NULL;
    }
  /* Normal cleanup */

  return (bp); /* return only the pagesize fragement */
} /* mcache_bkt() */

/******************************************************************************
NAME
   mcache_write - write a page to disk given it's bucket handle.

DESCRIPTION
   Private routine. Write a page to disk given it's bucket handle.

RETURNS
   RET_SUCCESS if succesful and RET_ERROR otherwise    
******************************************************************************/
static int
mcache_write(
    MCACHE *mp, /* IN: MCACHE cookie */
    BKT *bp     /* IN: bucket element */)
{
    CONSTR(FUNC, "mcache_write");   /* for HERROR */
    struct _lhqh *lhead = NULL; /* head of an entry in list hash chain */
    L_ELEM       *lp   = NULL;
    intn          ret_value = RET_SUCCESS;
#ifdef UNUSED
    int32         wpagesize;  /* page size to write */
#endif /* UNUSED */


#ifdef MCACHE_DEBUG
    (VOID)fprintf(stderr,"mcache_write: entering \n");
#endif
    /* check inputs */
    if (mp == NULL || bp == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

#ifdef STATISTICS
    ++mp->pagewrite;
#endif

    /* update this page reference */
    lhead = &mp->lhqh[HASHKEY(bp->pgno)];
    for (lp = lhead->cqh_first; lp != (VOID *)lhead; lp = lp->hl.cqe_next)
        if (lp->pgno == bp->pgno)
          { /* hit */
#ifdef STATISTICS
              ++mp->listhit;
              ++lp->elemhit;
#endif
              lp->eflags = ELEM_SYNC;
              break;
          }

    /* Run page through the user's filter.
       we use this to write the data chunk/page out.
       This deviates from the orignal purpose of the filter. */
    if (mp->pgout)
      { /* Note page numbers in HMCPxxx are 0 based not 1 based */
          if (((mp->pgout)(mp->pgcookie, bp->pgno - 1, bp->page)) == FAIL)
            {
                HEreport("mcache_write: error writing chunk=%d\n",(intn)bp->pgno);
                ret_value = RET_ERROR;
                goto done;
            }
      }
    else
      {
          HEreport("mcache_write: writing fcn not set,chunk=%d\n",(intn)bp->pgno);
          ret_value = RET_ERROR;
          goto done;
      }

#ifdef MCACHE_DEBUG
    (VOID)fprintf(stderr,"mcache_write: npages=%u\n",mp->npages);
#endif

#ifdef UNUSED
    wpagesize = mp->pagesize;
#endif /* UNUSED */

    /* mark page as clean */
    bp->flags &= ~MCACHE_DIRTY;

  done:
    if(ret_value == RET_ERROR)
      { /* error cleanup */
#ifdef MCACHE_DEBUG
          (VOID)fprintf(stderr,"mcache_write: error exiting\n");
#endif
          return ret_value;
      }
    /* Normal cleanup */

#ifdef MCACHE_DEBUG
    (VOID)fprintf(stderr,"mcache_write: exiting\n");
#endif
    return ret_value;
} /* mcache_write() */

/******************************************************************************
NAME
   mcache_look - lookup a page in the cache.

DESCRIPTION
   Private routine. Lookup a page in the cache and return pointer to it.

RETURNS
   Page if successfull and NULL othewise.
******************************************************************************/
static BKT *
mcache_look(
    MCACHE *mp, /* IN: MCACHE cookie */
    int32 pgno  /* IN: page to look up in cache */)
{
    CONSTR(FUNC, "mcache_look");   /* for HERROR */
    struct _hqh *head = NULL; /* head of hash chain */
    BKT         *bp   = NULL; /* bucket element */
    intn         ret_value  = RET_SUCCESS;

    /* check inputs */
    if (mp == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Check for attempt to look up a non-existent page. */
    if (pgno > mp->npages) 
        HE_REPORT_GOTO("attempting to get a non existant page from cache", FAIL);

    /* search through hash chain */
    head = &mp->hqh[HASHKEY(pgno)];
    for (bp = head->cqh_first; bp != (VOID *)head; bp = bp->hq.cqe_next)
        if (bp->pgno == pgno) 
          { /* hit....found page in cache */
#ifdef STATISTICS
              ++mp->cachehit;
#endif
              /* done */
              ret_value = RET_SUCCESS;
              goto done;
          }

    /* Well didn't find page in cache so mark return
     * value as NULL */
    bp = NULL; 

#ifdef STATISTICS
    ++mp->cachemiss;
#endif
  done:
    if(ret_value == RET_ERROR)
      { /* error cleanup */
          return NULL;
      }
    /* Normal cleanup */

    return (bp);
} /* mcache_look() */

#ifdef STATISTICS
#ifdef HAVE_GETRUSAGE
/******************************************************************************
NAME
   myrusage - print some process usage statistics

DESCRIPTION
   prints some process usage statistics to STDERR

RETURNS
   Nothing
******************************************************************************/
VOID
myrusage()
{
    struct rusage r;
    double sys, user, idle;
    double per;
    double timespent();

    getrusage(RUSAGE_SELF,&r);
    fprintf(stderr,"USAGE: shmem=%d,unshdata=%d,unshstack=%d\n",
            r.ru_ixrss,r.ru_idrss,r.ru_isrss);
    fprintf(stderr,"       pager=%d,pagef=%d,nswap=%d\n",
            r.ru_minflt,r.ru_majflt,r.ru_nswap);
    fprintf(stderr,"       block_in=%d,block_out=%d,nioch=%d\n",
            r.ru_inblock,r.ru_oublock,r.ru_ioch);
    fprintf(stderr,"       mesgs=%d,mesgr=%d,nsignals=%d\n",
            r.ru_msgsnd,r.ru_msgrcv,r.ru_nsignals);
}
#endif /* HAVE_GETRUSAGE */

/******************************************************************************
NAME
   mcache_stat - print out cache statistics

DESCRIPTION
   Print out cache statistics to STDERR.

RETURNS
   Nothing
******************************************************************************/
VOID
mcache_stat(
    MCACHE *mp /* IN: MCACHE cookie */)
{
    struct _lhqh *lhead = NULL; /* head of an entry in list hash chain */
    BKT          *bp    = NULL; /* bucket element */
    L_ELEM       *lp    = NULL;
    char         *sep   = NULL;
    intn          entry;         /* index into hash table */
    intn          cnt;
    intn          hitcnt; 

#ifdef HAVE_GETRUSAGE
    myrusage();
#endif

    /* check inputs */
    if (mp != NULL)
      {
          (VOID)fprintf(stderr, "%u pages in the object\n", mp->npages);
          (VOID)fprintf(stderr,
                        "page size %u, cacheing %u pages of %u page max cache\n",
                        mp->pagesize, mp->curcache, mp->maxcache);
          (VOID)fprintf(stderr, "%u page puts, %u page gets, %u page new\n",
                        mp->pageput, mp->pageget, mp->pagenew);
          (VOID)fprintf(stderr, "%u page allocs, %u page flushes\n",
                        mp->pagealloc, mp->pageflush);
          if (mp->cachehit + mp->cachemiss)
              (VOID)fprintf(stderr,
                            "%.0f%% cache hit rate (%u hits, %u misses)\n", 
                            ((double)mp->cachehit / (mp->cachehit + mp->cachemiss))
                            * 100, mp->cachehit, mp->cachemiss);
          (VOID)fprintf(stderr, "%u page reads, %u page writes\n",
                        mp->pageread, mp->pagewrite);
          (VOID)fprintf(stderr, "%u listhits, %u listallocs\n",
                        mp->listhit, mp->listalloc);
          (VOID)fprintf(stderr, "sizeof(MCACHE)=%d, sizeof(BKT)=%d, sizeof(L_ELEM)=%d\n",
                        sizeof(MCACHE), sizeof(BKT), sizeof(L_ELEM));
          (VOID)fprintf(stderr, "memory pool used %u bytes\n",
                        (int32)(sizeof(MCACHE)+ (sizeof(BKT)+mp->pagesize)*mp->curcache +
                                (sizeof(L_ELEM)*mp->npages)));
          sep = "";
          cnt = 0;
          for (bp = mp->lqh.cqh_first; bp != (VOID *)&mp->lqh; bp = bp->q.cqe_next) 
            {
                (VOID)fprintf(stderr, "%s%u", sep, bp->pgno);
                if (bp->flags & MCACHE_DIRTY)
                    (VOID)fprintf(stderr, "d");
                if (bp->flags & MCACHE_PINNED)
                    (VOID)fprintf(stderr, "P");
                if (++cnt == 10) 
                  {
                      sep = "\n";
                      cnt = 0;
                  } 
                else
                    sep = ", ";
            }
          (VOID)fprintf(stderr, "\n");
          (VOID)fprintf(stderr, "Element hits\n");
          sep = "";
          cnt = 0;
          hitcnt = 0;
          for (entry = 0; entry < HASHSIZE; ++entry)
            {
                lhead = &mp->lhqh[entry];
                for (lp = lhead->cqh_first; lp != (VOID *)lhead; lp = lp->hl.cqe_next)
                  {
                      cnt++;
                      (VOID)fprintf(stderr, "%s%u(%u)", sep, lp->pgno, lp->elemhit);
                      hitcnt += lp->elemhit;
                      if (cnt >= 8) 
                        {
                            sep = "\n";
                            cnt = 0;
                        } 
                      else
                          sep = ", ";
                  }
                if (cnt >= 8) 
                  {
                      (VOID)fprintf(stderr, "\n");
                      cnt = 0;
                  } 
            }
          (VOID)fprintf(stderr, "\n");
          (VOID)fprintf(stderr, "Total num of elemhits=%d\n",hitcnt);
      } /* end if mp */
}
#endif /* STATISTICS */
