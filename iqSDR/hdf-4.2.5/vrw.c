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

/* $Id: vrw.c 4932 2007-09-07 17:17:23Z bmribler $ */

/***********************************************************************
*
* vrw.c
* Part of the HDF VSet interface.
* This module handles reading and writing of Vdatas.
*

LOCAL ROUTINES
 VSPshutdown  --  Free the Vtbuf buffer.

EXPORTED ROUTINES
 VSseek  -- Seeks to an element boundary within a vdata i.e. 2nd element.
 VSread  -- Reads a specified number of elements' worth of data from a vdata.
             Data will be returned to you interlaced in the way you specified.
 VSwrite -- Writes a specified number of elements' worth of data to a vdata.
	     You must specify how your data in your buffer is interlaced.
             Creates an aid, and writes it out if this is the first time.

 NOTE: Another pass needs to made through this file to update some of
       the comments about certain sections of the code. -GV 9/8/97

************************************************************************/

#define VSET_INTERFACE
#include "hdf.h"

#ifndef MIN
#define MIN(a,b)     ((a) < (b) ? (a) : (b))
#endif /* MIN */

PRIVATE uint32 Vtbufsize = 0;
PRIVATE uint8 *Vtbuf = NULL;

/*******************************************************************************
 NAME
    VSPshutdown  --  Free the Vtbuf buffer.

 DESCRIPTION
    For completeness, when the VSet interface is shut-down, free the Vtbuf.

    Should only ever be called by the "atexit" function HDFend

 RETURNS
    Returns SUCCEED/FAIL

*******************************************************************************/
intn 
VSPshutdown(void)
{
  intn  ret_value = SUCCEED;

  /* free global buffers */
  if(Vtbuf != NULL)
    {
      HDfree(Vtbuf);
      Vtbuf = NULL;
      Vtbufsize = 0;
    } /* end if */

  /* Clear the local buffers in vio.c */
  ret_value = VSPhshutdown();

  return ret_value;
} /* end VSPshutdown() */

/*******************************************************************************
NAME
   VSseek

DESCRIPTION
   Seeks to an element boundary within a vdata
   Vdata must be attached with "r" or "w" access.
   Specify eltpos = 0 for 1st element, 1 for 2nd element etc.

   (eg  returns 5 if seek to the 6th element, etc)

RETURNS
   RETURNS FAIL on error
   RETURNS position of element seeked to (0 or a +ve integer)

*******************************************************************************/
int32
VSseek(int32 vkey,   /* IN: vdata key */
       int32 eltpos  /* IN: element position in vdata */)
{
    int32        ret;
    int32        offset;
    vsinstance_t *w = NULL;
    VDATA        *vs = NULL;
    int32        ret_value = SUCCEED;
    CONSTR(FUNC, "VSseek");

    /* clear error stack */
    HEclear();

    /* check if vdata is part of vdata group */
    if (HAatom_group(vkey)!=VSIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get vdata instance */
    if (NULL == (w = (vsinstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vdata itself and check it. Check element postion also. */
    vs = w->vs;
    if ((vs == NULL) || (eltpos < 0))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Don't allow seeks in 0-field vdatas */
    if (vs->wlist.n<=0)
        HGOTO_ERROR(DFE_BADFIELDS, FAIL);

    /* calculate offset of element in vdata */
    offset = eltpos * vs->wlist.ivsize;

    /* seek to element */
    if (( ret = Hseek(vs->aid, offset, DF_START)) == FAIL)
        HGOTO_ERROR(DFE_BADSEEK, FAIL);

    ret_value = (eltpos);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}	/* VSseek */

/*******************************************************************************
NAME
   VSread

DESCRIPTION
   Reads a specified number of elements' worth of data from a vdata.
   Data will be returned to you interlaced in the way you specified.

RETURNS
   RETURNS FAIL if error
   RETURNS the number of elements read (0 or a +ve integer).

*******************************************************************************/
int32 
VSread(int32 vkey,       /* IN: vdata key */
       uint8 buf[],      /* IN/OUT: space to put elements in */
       int32 nelt,       /* IN: number of elements to read */
       int32 interlace   /* IN: interlace to return elements in 'buf' */)
{
    intn isize = 0;
    intn order = 0;
    intn index = 0;
    intn esize = 0;
    intn hsize = 0;
    uint8 *Src;
    uint8 *b1 = NULL;
    uint8 *b2 = NULL;
    int32  i, j;
    int32  nv;
    int32  offset;
    int32  type;
    int32  uvsize;         /* size of "element" as NEEDED by user */
    int32  total_bytes;     /* total number of bytes that need to be read in */
    int32  bytes;           /* number of elements / bytes to read next time */
    int32  chunk;           /* number of records in a buffer */
    int32  done;            /* number of records to do / done */
    DYN_VWRITELIST *w = NULL;
    DYN_VREADLIST  *r = NULL;
    vsinstance_t   *wi = NULL;
    VDATA          *vs = NULL;
    int32      ret_value = SUCCEED;
    CONSTR(FUNC, "VSread");

    /* clear error stack */
    HEclear();

    /* check if vdata is part of vdata group */
    if (HAatom_group(vkey) != VSIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get vdata instance */
    if (NULL == (wi = (vsinstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vdata itself and check it */
    vs = wi->vs;
    if (vs == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* check access id and number of vertices in vdata */
    if ((vs->aid == 0) || (vs->nvertices == 0))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Don't allow reads in 0-field vdatas */
    if (vs->wlist.n<=0)
        HGOTO_ERROR(DFE_BADFIELDS, FAIL);

    /* check if vdata exists in file */
    if (vexistvs(vs->f, vs->oref) == FAIL)
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* check interlace parameter */
    if (interlace != FULL_INTERLACE && interlace != NO_INTERLACE)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* read/write lists */
    w = &(vs->wlist);
    r = &(vs->rlist);
    hsize = (intn)vs->wlist.ivsize;   /* size as stored in HDF */
    total_bytes = hsize * nelt;

    /*
       Now, convert and repack field(s) from Vtbuf into buf.

       This section of the code deals with interlacing. In all cases
       the items for each of the fields are converted and shuffled
       around from the internal buffer "Vtbuf" to the user's buffer
       "buf".

       There are 5 cases :
       (A) user=NO_INTERLACE   & vdata=FULL_INTERLACE)
       (B) user=NO_INTERLACE   & vdata=NO_INTERLACE)
       (C) user=FULL_INTERLACE & vdata=FULL_INTERLACE)
       (D) user=FULL_INTERLACE & vadat=NO_INTERLACE)
       (E) SPECIAL CASE when only one field.

       Cases (A)-(D) handles multiple fields.
       Case (E) handles reading from a Vdata with a single field.

       Cases (E) and (C) are the most frequently used.  Limit buffer
       allocations to VDATA_BUFFER_MAX size so that we conserve
       memory.  Doing this involves a certain degree of added code
       complexity so don't bother doing it for the less frequent
       cases.  Cases E and C have been rolled together since they are
       very similar and both need the incremental writing.

     */

    /* ----------------------------------------------------------------- */
    /* CASE  (E + C): Easy to unroll case */
    if ((w->n == 1) || (interlace == FULL_INTERLACE && vs->interlace == FULL_INTERLACE))
      {
          /*
           * figure out how many elements we can move at a time and
           * make sure our buffer is big enough
           */

          if ((uint32) total_bytes < Vtbufsize)
            {
                chunk = nelt;
            }
          else
            {
                int32       buf_size;

                /* we are bounded above by VDATA_BUFFER_MAX */
                buf_size = MIN(total_bytes, VDATA_BUFFER_MAX);

                /* make sure there is at least room for one record in our buffer */
                chunk = buf_size / hsize + 1;

                /* get a buffer big enough to hold the values */
                Vtbufsize = (size_t)chunk * (size_t)hsize;
                if (Vtbuf)
                    HDfree(Vtbuf);
                if ((Vtbuf = (uint8 *) HDmalloc(Vtbufsize)) == NULL)
                    HGOTO_ERROR(DFE_NOSPACE, FAIL);
            }

          done = 0;

          /* set loop invariant parameters */
          Src = buf;
          bytes = hsize * chunk;

          for (uvsize = 0, j = 0; j < r->n; j++)
              uvsize += w->esize[r->item[j]];

          while (done < nelt)
            {

            /* chunk has changed so update the byte counts */
            if (nelt - done < chunk)
              {
                  chunk = nelt - done;
                  bytes = hsize * chunk;
              }

            /* ================ start reading ============================== */
            if ((nv = Hread(vs->aid, bytes, (uint8 *) Vtbuf)) != bytes)
              {
                  HERROR(DFE_READERROR);
                  HEreport("Tried to read %d, only read %d", bytes, nv);
                  HGOTO_DONE(FAIL);
              }

            /* CASE  (E): Only a single field in the Vdata */
            if (w->n == 1)
              {
                  DFKconvert(Vtbuf,Src,w->type[0], (uint32) w->order[0] * (uint32)chunk, DFACC_READ, 0, 0);
              }     /* case (e) */
            /* ----------------------------------------------------------------- */
            /* CASE  (C):  iu=full, iv=full */
            else
              {
                  offset = 0;
                  for (j = 0; j < r->n; j++)
                    {
                        i = r->item[j];
                        b1 = Src + offset;
                        b2 = Vtbuf + (size_t)w->off[i];
                        type = (int32)w->type[r->item[j]];
                        esize = (intn)w->esize[i];
                        isize = (intn)w->isize[i];
                        order = (intn)w->order[i];

                        for (index = 0; index < order; index++)
                          {
                              DFKconvert(b2, b1, type, (uint32) chunk, DFACC_READ, (uint32) hsize, (uint32) uvsize);
                              b1 += (int) esize / order;
                              b2 += (int) isize / order;
                          }
                        offset += esize;
                    }
                } /* case (E) */

                /* record what we've done and move to next group */
                done += chunk;
                Src += chunk * uvsize;
            } /* end while */
          }     /* case (C + E) */
    else {
	  /* 
	   * Handle the other cases now.
	   * These cases are less frequent so don't bother unrolling
	   *   the loops for now.  As a result, we may get into memory
	   *   problems since we may end up allocating a huge buffer
	   */

        /* alloc space (Vtbuf) for reading in the raw data from vdata */
        if (Vtbufsize < (size_t)nelt * (size_t) hsize)
          {
              Vtbufsize = (size_t)nelt * (size_t) hsize;
              if (Vtbuf)
                  HDfree(Vtbuf);
              if ((Vtbuf = (uint8 *) HDmalloc(Vtbufsize)) == NULL)
                  HGOTO_ERROR(DFE_NOSPACE, FAIL);
          }

        /* ================ start reading ============================== */

        nv = Hread(vs->aid, nelt * hsize, (uint8 *) Vtbuf);

        if (nv != nelt * hsize)
          {
              HERROR(DFE_READERROR);
              HEreport("Tried to read %d, only read %d", nelt * hsize, nv);
              HGOTO_DONE(FAIL);
          }

        /* ----------------------------------------------------------------- */
        /* CASE  (A):  user=none, vdata=full */
        if (interlace == NO_INTERLACE && vs->interlace == FULL_INTERLACE)
          {
              b1 = buf;
              for (j = 0; j < r->n; j++)
                {
                    i = r->item[j];
                    b2 = Vtbuf + (size_t)w->off[i];
                    type = (int32)w->type[i];
                    isize = (intn)w->isize[i];
                    esize = (intn)w->esize[i];
                    order = (intn)w->order[i];

                    for (index = 0; index < order; index++)
                      {
                          DFKconvert(b2, b1, type, (uint32) nelt, DFACC_READ, (uint32) hsize, (uint32) esize);
                          b2 += isize / order;
                          b1 += esize / order;
                      }
                    b1 += ((nelt - 1) * esize);
                }
          }     /* case (a) */

        /* ----------------------------------------------------------------- */
        /* CASE  (B):  user=none, vdata=none */
        else if (interlace == NO_INTERLACE && vs->interlace == NO_INTERLACE)
          {
              b1 = buf;
              for (j = 0; j < r->n; j++)
                {
                    i = r->item[j];
                    b2 = Vtbuf + (size_t)w->off[i] * (size_t)nelt;
                    type = (int32)w->type[i];
                    esize = (intn)w->esize[i];
                    isize = (intn)w->isize[i];
                    order = (intn)w->order[i];

                    for (index = 0; index < order; index++)
                      {
                          DFKconvert(b2, b1, type, (uint32) nelt, DFACC_READ, (uint32) isize, (uint32) esize);
                          b1 += esize / order;
                          b2 += isize / order;
                      }
                    b1 += ((nelt - 1) * esize);
                }
          }     /* case (b) */

        /* ----------------------------------------------------------------- */
        /* CASE  (D):  user=full, vdata=none */
        else if (interlace == FULL_INTERLACE && vs->interlace == NO_INTERLACE)
          {

              for (uvsize = 0, j = 0; j < r->n; j++)
                  uvsize += w->esize[r->item[j]];

              offset = 0;
              for (j = 0; j < r->n; j++)
                {
                    i = r->item[j];
                    b1 = buf + offset;
                    b2 = Vtbuf + (size_t)w->off[i] * (size_t)nelt;
                    type = (int32)w->type[i];
                    isize = (intn)w->isize[i];
                    esize = (intn)w->esize[i];
                    order = (intn)w->order[i];

                    for (index = 0; index < order; index++)
                      {
                          DFKconvert(b2, b1, type, (uint32) nelt, DFACC_READ, (uint32) isize, (uint32) uvsize);
                          b1 += esize / order;
                          b2 += isize / order;
                      }
                    offset += isize;
                }
          }     /* case (d) */
      } /* end else, cases a, b, and d */

    ret_value = (nelt);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* VSread */

/*******************************************************************************
NAME
   VSwrite

DESCRIPTION
   Writes a specified number of elements' worth of data to a vdata.
   You must specify how your data in your buffer is interlaced.

   NEW
   create an aid, and write out if this is the first time.
   (otherwise) subsequent writes result in link-blocks.

RETURNS
   RETURNS FAIL if error
   RETURNS the number of elements written (0 or a +ve integer).

*******************************************************************************/
int32 
VSwrite(int32 vkey,         /* IN: vdata key */
        const uint8 buf[],  /* IN: elements to write to vdata */
        int32 nelt,         /* IN: number of elements */
        int32 interlace     /* IN: interlace of elements 'buf' */)
{
    intn isize = 0;
    intn order = 0;
    intn index = 0;
    intn esize = 0;
    uint8 *dest = NULL;
    const uint8 *src, *Src;
    int32       j;
    int32       type;
    int32       offset;
    int32       position = 0;
    int32       new_size;
    int32       status;
    int32       total_bytes;    /* total number of bytes that need to be written out */
    DYN_VWRITELIST *w = NULL;
    int32       int_size;       /* size of "element" as needed by user in memory */
    intn        hdf_size = 0;   /* size of record in HDF file */
    vsinstance_t *wi = NULL;
    VDATA        *vs = NULL;
    int32       bytes;          /* number of elements / bytes to write next time */
    int32       chunk;
    int32       done;    /* number of records to do / done */
    int32       ret_value = SUCCEED;
    CONSTR(FUNC, "VSwrite");

    /* clear error stack */
    HEclear();

    /* check if vdata is part of vdata group */
    if (HAatom_group(vkey) != VSIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get vdata instance */
    if (NULL == (wi = (vsinstance_t *) HAatom_object(vkey)))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get vdata itself and check it. Also check number of elements */
    vs = wi->vs;
    if ((nelt <= 0) || (vs == NULL))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* check if write access to vdata */
    if (vs->access != 'w')
        HGOTO_ERROR(DFE_BADACC, FAIL);

    /* check if vdata exists in the file */
    if (FAIL == vexistvs(vs->f, vs->oref))
        HGOTO_ERROR(DFE_NOVS, FAIL);

    /* get write list */
    w = & vs->wlist;
    if (w->n == 0)
      {
          HERROR(DFE_NOVS);
          HEreport("No fields set for writing");
          HGOTO_DONE(FAIL);
      }

    /* check interlace of input buffer */
    if (interlace != NO_INTERLACE && interlace != FULL_INTERLACE)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    hdf_size = (intn)w->ivsize;   /* as stored in HDF file */
    total_bytes = hdf_size * nelt;

    /* make sure we have a valid AID */
    if (vs->aid == 0)
      {
#ifdef OLD_WAY
          vs->aid = Hstartwrite(vs->f, DFTAG_VS, vs->oref, total_bytes);
          if (vs->aid == FAIL)
              HGOTO_ERROR(DFE_BADAID, FAIL);
#else /* OLD_WAY */
          HGOTO_ERROR(DFE_BADAID, FAIL);
#endif /* OLD_WAY */
      }

    /*
     * promote to link-block if vdata exists and is not already one
     *  AND we are incresing its size
     */
    HQueryposition(vs->aid, &position);
    new_size = (position / (intn)vs->wlist.ivsize) + nelt;

    /* this should really be cached in the Vdata structure */
    for (int_size = 0, j = 0; j < w->n; j++)
        int_size += w->esize[j];

    /*
       First, convert and repack field(s) from Vtbuf into buf.

       This section of the code deals with interlacing. In all cases
       the items for each of the fields are converted and shuffled
       around from the user's buffer "buf" to the internal's buffer
       "Vtbuf".  The data in "Vtbuf" is then written out to the vdata.

       There are 5 cases :
       (A) user=NO_INTERLACE   & vdata=FULL_INTERLACE)
       (B) user=NO_INTERLACE   & vdata=NO_INTERLACE)
       (C) user=FULL_INTERLACE & vdata=FULL_INTERLACE)
       (D) user=FULL_INTERLACE & vadat=NO_INTERLACE)
       (E) SPECIAL CASE when only one field.

       Cases (A)-(D) handles multiple fields
       Case (E) handles single field Vdatas

       Cases (E) and (C) are the most frequently used.  Limit buffer
       allocations to VDATA_BUFFER_MAX size so that we conserve
       memory.  Doing this involves a certain degree of added code
       complexity so don't bother doing it for the less frequent
       cases.  Cases E and C have been rolled together since they are
       very similar and both need the incremental writing.

       --------------------------------------------------------------------- */
    /* CASE  (E + C): Easy to unroll case */
    if ((w->n == 1) || (interlace == FULL_INTERLACE && vs->interlace == FULL_INTERLACE))
      {

          /*
           * figure out how many elements we can move at a time and
           * make sure our buffer is big enough
           */

          if ((uint32) total_bytes < Vtbufsize)
            {
                chunk = nelt;
            }
          else
            {
                int32       buf_size;

                /* we are bounded above by VDATA_BUFFER_MAX */
                buf_size = MIN(total_bytes, VDATA_BUFFER_MAX);

                /* make sure there is at least room for one record in our buffer */
                chunk = buf_size / hdf_size + 1;

                /* get a buffer big enough to hold the values */
                Vtbufsize = (size_t)chunk * (size_t)hdf_size;
                if (Vtbuf)
                    HDfree(Vtbuf);
                if ((Vtbuf = (uint8 *) HDmalloc(Vtbufsize)) == NULL)
                    HGOTO_ERROR(DFE_NOSPACE, FAIL);
            }

          done = 0;

          /* set loop invariant parameters */
          Src = buf;
          dest = Vtbuf;
          bytes = hdf_size * chunk;

          while (done < nelt)
            {

                /* chunk has changed so update the byte counts */
                if (nelt - done < chunk)
                  {
                      chunk = nelt - done;
                      bytes = hdf_size * chunk;
                  }
/*
   printf("Case E/C: [%d,%d] writing %d (elems) %d bytes\n", done, nelt, chunk, bytes);
 */

                offset = 0;
                for (j = 0; j < w->n; j++)
                  {
                      src = Src + offset;
                      dest = Vtbuf + (size_t)w->off[j];
                      type = (int32)w->type[j];
                      esize = (intn)w->esize[j];
                      isize = (intn)w->isize[j];
                      order = (intn)w->order[j];

                      for (index = 0; index < order; index++)
                        {
                            DFKconvert((VOIDP)src, dest, type, (uint32) chunk, DFACC_WRITE, (uint32) int_size, (uint32) hdf_size);
                            dest += isize / order;
                            src += esize / order;
                        }
                      offset += esize;
                  }

                /* write the converted data to the file */
                status = Hwrite(vs->aid, bytes, (uint8 *) Vtbuf);
                if (status != bytes)
                    HGOTO_ERROR(DFE_WRITEERROR, FAIL);

                /* record what we've done and move to next group */
                done += chunk;
                Src += chunk * int_size;
            }

      }     /* case (C + E) */

    else
      {

	  /* 
	   * Handle the other cases now.
	   * These cases are less frequent so don't bother unrolling
	   *   the loops for now.  As a result, we may get into memory
	   *   problems since we may end up allocating a huge buffer
	   */

	  /* alloc space (Vtbuf) for writing out the data */
	  if (Vtbufsize < (uint32) total_bytes)
	    {
          Vtbufsize = (uint32)total_bytes;
          if (Vtbuf)
              HDfree(Vtbuf);
          if ((Vtbuf = (uint8 *) HDmalloc(Vtbufsize)) == NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
	    }

	  /* ----------------------------------------------------------------- */
	  /* CASE  (A):  user=none, vdata=full */
	  if (interlace == NO_INTERLACE && vs->interlace == FULL_INTERLACE)
	    {

		src = buf;
		for (j = 0; j < w->n; j++)
		  {
		      dest = Vtbuf + (size_t)w->off[j];
		      type = (int32)w->type[j];
		      esize = (intn)w->esize[j];
		      isize = (intn)w->isize[j];
		      order = (intn)w->order[j];

		      for (index = 0; index < order; index++) {
			    DFKconvert((VOIDP)src, dest, type, (uint32) nelt, DFACC_WRITE, (uint32) esize, (uint32) hdf_size);
			    src += esize / order;
			    dest += isize / order;
			  }
		      src += ((nelt - 1) * esize);
		  }

	    }	/* case (a) */

	  /* --------------------------------------------------------------------- */
	  /* CASE  (B):  user=none, vdata=none */
	  else if (interlace == NO_INTERLACE && vs->interlace == NO_INTERLACE)
	    {

		src = buf;
		for (j = 0; j < w->n; j++)
		  {
		      dest = Vtbuf + w->off[j] * nelt;
		      type = (int32)w->type[j];
		      esize = (intn)w->esize[j];
		      isize = (intn)w->isize[j];
		      order = (intn)w->order[j];

		      for (index = 0; index < order; index++) {
			    DFKconvert((VOIDP)src, dest, type, (uint32) nelt, DFACC_WRITE, (uint32) esize, (uint32) isize);
			    dest += isize / order;
			    src += esize / order;
			  }
		      src += ((nelt - 1) * esize);
		  }

	    }	/* case (b) */

	  /* ----------------------------------------------------------------- */
	  /* CASE  (D):  user=full, vdata=none */
	  else if (interlace == FULL_INTERLACE && vs->interlace == NO_INTERLACE)
	    {
		offset = 0;
		for (j = 0; j < w->n; j++)
		  {
		      src = buf + offset;
		      dest = Vtbuf + w->off[j] * nelt;
		      type = (int32)w->type[j];
		      isize = (intn)w->isize[j];
		      esize = (intn)w->esize[j];
		      order = (intn)w->order[j];

		      for (index = 0; index < order; index++) {
			    DFKconvert((VOIDP)src, dest, type, (uint32) nelt, DFACC_WRITE, (uint32) int_size, (uint32) isize);
			    dest += isize / order;
			    src += esize / order;
  			  }
		      offset += esize;
		  }
	    }	/* case (d) */

	  status = Hwrite(vs->aid, total_bytes, (uint8 *) Vtbuf);
	  if (status != total_bytes)
	      HGOTO_ERROR(DFE_WRITEERROR, FAIL);

      }     /* cases a, b, and d */

    /* update the internal structure to reflect write */
    if (new_size > vs->nvertices)
        vs->nvertices = new_size;
    vs->marked = 1;

    ret_value = (nelt);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}	/* VSwrite */

