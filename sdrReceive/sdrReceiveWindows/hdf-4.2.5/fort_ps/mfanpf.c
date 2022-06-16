/****************************************************************************
 * NCSA HDF                                                                 *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 *                                                                          *
 * For conditions of distribution and use, see the accompanying             *
 * hdf/COPYING file.                                                        *
 *                                                                          *
 ****************************************************************************/

#ifdef RCSID
static char RcsId[] = "@(#)$Revision: 4888 $";
#endif

/* $Id: mfanpf.c 4888 2007-07-17 21:16:15Z swegner $ */

/*-----------------------------------------------------------------------------
 * File:     mfanpf.c
 * Author:   GeorgeV.
 * Purpose:  C-stubs for multi-file Fortran annotation routines
 * Invokes:  C-Routines in "mfan.c"
 * Contents: SEE annotation source/header files "mfan.c" and "mfan.h"
 *           for more info on the annotation interface.
 *
 *  NOTES: TYPE here refers to file/data label/description types 
 *         They are AN_FILE_LABEL, AN_FILE_DESC, AN_DATA_LABEL, AN_DATA_DESC
 *         THE tag/ref refers to data tag/ref. The fortran equivalents
 *         are defined in 'hdf.inc'.
 *
 *  C-stubs directly callable by Fortran Users
 *  ------------------------------------------
 *    afistart    - start annotation access on file and return annotaton id
 *    afifinf - get number of file/data annotations in file. 
 *                 Indices returned are used in afselect() calls.
 *    afiend      - end access to annotation handling on file
 *    aficreat   - create a new data annotation and return a handle
 *    afifcreat  - create a new file annotation and return a handle
 *    afiselct   - returns an annotation handle(ann_id) from index for 
 *                 a particular annotation TYPE. This handle is then used for
 *                 calls like afwriteann(), afreadann(), afannlen(),..etc
 *    afinann   - return number of annotations that match TYPE/tag/ref
 *    afialst  - return list of handles that match TYPE/tag/ref
 *    afialen   - get length of annotation given handle
 *    afiwann - write annotation given handle
 *    afirann  - read annotation given handle
 *    afiendac - end access to annotation using handle
 *    afigtr  - get tag/ref pair to annotation ID
 *    afiid2tr  - get tag/ref given annotation id 
 *    afitr2id  - get annotation id given tag/ref
 *    afitp2tg  - annotation type to corresponding annotation TAG
 *    afitg2tp  - annotation TAG to corresponding annotation type
 *---------------------------------------------------------------------------*/

#include "hdf.h"
#include "hproto_fortran.h"
#include "mfan.h"

/*-----------------------------------------------------------------------------
  FUNCTION NAMEING CONVENTION:
  ---------------------------
  This file contains the HDF-style C stubs for the multi-file Annotation
  interface. They call the corresponding C-functions in "mfan.c"

  The basic routines called by fortran will be of the form afxxxx

  If only a C stub is needed it will be named nafxxxx have the FNAME()
  function applied to it.

  If a Fortran stub is also required the fortran stub will be called
  afxxxx(mfanff.f) and the one in here will be nacxxx and again be FNAME()ed

-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Name:    afistart
 * Purpose: Open file for annoation handling
 * Inputs:  file_id: id of HDF file
 * Returns: annotation interface handle on SUCCEED and FAIL otherwise
 * Users:   
 * Invokes: ANstart()
 * Author: GeorgeV
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
nafistart(intf *file_id)
{
  intf ret;

  ret = ANstart((int32)*file_id);

  return (ret);
} /* nafistart() */

/*-----------------------------------------------------------------------------
 * Name:    afifinf
 * Purpose: Get number of file/data annotations in file. 
 * Inputs:  IN an_id:     annotation interface handle
 *          OUT num_flabel: number of file labels in file
 *          OUT num_fdesc:  number of file descriptions in file
 *          OUT num_olabel: number of data labels in file
 *          OUT num_odesc:  number of data descriptions in file
 * Returns: see ANfileinfo()
 * Users:   Fortran Users
 * Invokes: ANfileinfo()
 * Author: GeorgeV
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
nafifinf(intf *an_id, intf *num_flabel, intf *num_fdesc, intf *num_olabel,
            intf *num_odesc)
{
#ifdef LATER
  CONSTR(FUNC, "afifinf");
#endif /* LATER */
  intf  ret;
  int32 nflabel, nfdesc, nolabel, nodesc; 

  ret = ANfileinfo((int32)*an_id, &nflabel, &nfdesc, &nolabel, &nodesc);

  /* fill in values to return */
  *num_flabel = nflabel;
  *num_fdesc  = nfdesc;
  *num_olabel = nolabel;
  *num_odesc  = nodesc;

  return ret;
} /* nafifinf() */

/*-----------------------------------------------------------------------------
 * Name:    afiend    
 * Purpose: End access to annotation handling on file
 * Inputs:  file_id:
 * Returns: see ANend()
 * Users:   Fortran Users
 * Invokes: ANend()
 * Author: GeorgeV
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
nafiend(intf *an_id)
{
#ifdef LATER
  CONSTR(FUNC, "afiend");
#endif /* LATER */

  return (intf)ANend((int32) *an_id);
} /* nafiend() */

/*-----------------------------------------------------------------------------
 * Name:    aficreat
 * Purpose: Create a new data annotation and return an annotation handle
 * Inputs:  an_id: annotation interface handle
 *          etag:    tag of data to annotate
 *          eref:    ref of data to annotate
 *          atype:   annotation type AN_DATA_LABEL, AN_DATA_DESC
 * Returns: see ANcreate()
 * Users:   Fortran Users
 * Invokes: ANcreate()
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
naficreat(intf *an_id, intf *etag, intf *eref, intf *atype)
{
#ifdef LATER
  CONSTR(FUNC, "aficreat");
#endif /* LATER */

  return (intf)ANcreate((int32)*an_id,(uint16)*etag,(uint16)*eref,(ann_type)*atype);
} /* naficreat() */

/*-----------------------------------------------------------------------------
 * Name:    afifcreat
 * Purpose: Create a new file annotation and return an annotation handle
 * Inputs:  an_id: annottion inteface handle
 *          atype:   annotation type AN_FILE_LABEL, AN_DATA_DESC
 * Returns: see ANcreatef()
 * Users:   Fortran Users
 * Invokes: ANcreatf()
 * Author: GeorgeV
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
nafifcreat(intf *an_id, intf *atype)
{
#ifdef LATER
  CONSTR(FUNC, "afifcreat");
#endif /* LATER */

  return (intf)ANcreatef((int32)*an_id,(ann_type)*atype);
} /* nafifcreat() */

/*-----------------------------------------------------------------------------
 * Name:    afiselct
 * Purpose: returns an annotation handle(ann_id) from index for 
 *          a particular annotation TYPE. This handle is then used for
 *          calls like afwriteann(), afreadann(), afannlen(),..etc
 * Inputs:  an_id: annotation interface handle
 *          index:   index for particular annoation type. Usually based on
 *                   number of a particular type obtained from affileinfo()call.
 *                   ZERO based.
 *          atype:   annotation type AN_FILE_LABEL, AN_FILE_DESC, AN_DATA_LABEL
 *                   AN_DATA_DESC
 * Returns: see ANselect()
 * Users:   Fortran Users
 * Invokes: ANselect()
 * Author: GeorgeV
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
nafiselct(intf *an_id, intf *index, intf *atype)
{
#ifdef LATER
  CONSTR(FUNC, "afiselct");
#endif /* LATER */

  return (intf)ANselect((int32)*an_id,(int32)*index, (ann_type)*atype);
} /* nafiselct() */

/*-----------------------------------------------------------------------------
 * Name:    afinann
 * Purpose: Return number of annotations that match TYPE/tag/ref
 * Inputs:  an_id: annotation interface handle
 *          atype:   annotation type AN_DATA_LABEL, AN_DATA_DESC
 *          etag:    data tag to match
 *          eref:    data ref to match
 * Returns: see ANnumann()
 * Users:   Fortran Users
 * Invokes: ANnumann()
 * Author: GeorgeV
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
nafinann(intf *an_id, intf *atype, intf *etag, intf *eref)
{
#ifdef LATER
  CONSTR(FUNC, "afinann");
#endif /* LATER */

  return (intf)ANnumann((int32)*an_id,(ann_type)*atype,(uint16)*etag,(uint16)*eref);
} /* nafinann() */

/*-----------------------------------------------------------------------------
 * Name:    afialst
 * Purpose: Return list of handles that match TYPE/tag/ref
 * Inputs:  IN an_id: annotation inteface handle
 *          IN atype:   annotation type AN_DATA_LABEL, AN_DATA_DESC
 *          IN etag:    data tag to match
 *          IN eref:    data ref to match
 *          OUT alist[]: list of annotation handles found that match tag/ref
 * Returns: number of annoations found that match else FAIL. see ANannlist()
 * Users:   Fortran Users
 * Invokes: ANnumann(), ANannlist()
 * Author: GeorgeV
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
nafialst(intf *an_id, intf *atype, intf *etag, intf *eref, intf alist[])
{
  CONSTR(FUNC, "afialst");
  intf  ret;
  int32 *tempanlist;
  intf  nanns;
  intn  i;

  /* Get number of annotations that match tag/ref pair */
  nanns = ANnumann((int32)*an_id,(ann_type)*atype,(uint16)*etag,(uint16)*eref);
  if (nanns < 0)
    HE_REPORT_RETURN("ANnumann: failed to any annotations", FAIL);

  /* create annlist with true int32s to maintain compatibility
  ** with machines that allocate less than 32 bits per int. */
  if ((tempanlist = (int32 *) HDmalloc(nanns * sizeof(int32))) == NULL)
    HRETURN_ERROR(DFE_NOSPACE, FAIL);

  /* Get list of annoation handles to return */
  ret = ANannlist((int32)*an_id,(ann_type)*atype,(uint16)*etag,(uint16)*eref,
                  tempanlist);
  if (ret < 0)
    HE_REPORT_RETURN("ANannlist:failed to any annotations", FAIL);

  /* move annotation handles into caller's alist */
  for (i = 0; i < nanns; i++)
    alist[i] = tempanlist[i];

  HDfree((VOIDP) tempanlist); /* free allocated space */

  return ret;
} /* nafialst() */

/*-----------------------------------------------------------------------------
 * Name:    afialen
 * Purpose: Get length of annotation given handle
 * Inputs:  an_id:annotation handle
 * Returns: see ANannlen()
 * Users:   Fortran Users
 * Invokes: ANannlen()
 * Author: GeorgeV
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
nafialen(intf *an_id)
{
#ifdef LATER
  CONSTR(FUNC, "afialen");
#endif /* LATER */

    return (intf)ANannlen((int32)*an_id);
} /* nafialen() */

/*-----------------------------------------------------------------------------
 * Name:    afiwann
 * Purpose: Write annotation given handle
 * Inputs:  ann_id: annotation handle
 *          ann:   annotation to write out
 *          annlen:length of annotation
 * Returns: see ANwriteann()
 * Users:   Fortran Users
 * Invokes: ANwriteann()
 * Author: GeorgeV
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
nafiwann(intf *ann_id,_fcd ann, intf *annlen)
{
#ifdef LATER
  CONSTR(FUNC, "afiwann");
#endif /* LATER */

    char       *iann = NULL;
    intf        status;

    /* Convert fortran string to C-String */
    iann = HDf2cstring(ann, (intn) *annlen);
    if (!iann)
        return(FAIL);

    status = ANwriteann((int32)*ann_id, (char *) _fcdtocp(ann), (int32)*annlen);

    HDfree(iann); /* free allocaed space by HDf2cstring */

    return status;
} /* nafiwann() */

/*-----------------------------------------------------------------------------
 * Name:    afirann
 * Purpose: Read annotation given handle
 * Inputs:  ann_id:  annotation handle
 *          ann:    annotation read
 *          maxlen: maximum space allocted for "ann"
 * Returns: see ANreadann() (SUCCEED (0) if successful, else FAIL (-1))
 * Users:   Fortran Users
 * Invokes: ANreadann()
 * Author: GeorgeV
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
nafirann(intf *ann_id,_fcd ann, intf *maxlen)
{
    CONSTR(FUNC, "afirann");
    char	*iann = NULL;
    intn        status;

    /* Allocate space for fortran string */
    if (*maxlen)
        iann = (char *) HDmalloc((uint32) *maxlen + 1);

    if (!iann)
        HRETURN_ERROR(DFE_NOSPACE, FAIL);

    status = ANreadann((int32)*ann_id, iann, (int32)*maxlen);

    /* C-String to Fortran String */
    HDpackFstring(iann, _fcdtocp(ann), (intn) *maxlen);

    if (iann)
        HDfree(iann); /* free allocated space */

    return status;
} /* nafirann() */

/*-----------------------------------------------------------------------------
 * Name:    afiendac
 * Purpose: End access to annotation using handle
 * Inputs:  ann_id:annotation handle
 * Returns: see ANendaccess()
 * Users:   Fortran Users
 * Invokes: ANendaccess()
 * Author: GeorgeV
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
nafiendac(intf *ann_id)
{
#ifdef LATER
  CONSTR(FUNC, "afiendac");
#endif /* LATER */

  return (intf)ANendaccess((int32)*ann_id);
} /* nafiendac() */

/*-----------------------------------------------------------------------------
 * Name:    afigtgr 
 * Purpose: 
 * Inputs:  
 * Returns: see ANget_tagref()
 * Users:   Fortran Users
 * Invokes: ANget_tagref()
 * Author: GeorgeV
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
nafigtr(intf *an_id, intf *index, intf *type, intf *tag, intf *ref)
{
#ifdef LATER
  CONSTR(FUNC, "afigtr");
#endif /* LATER */
  intf   ret;
  uint16 otag, oref;

  ret = (intf)ANget_tagref((int32)*an_id,(int32)*index,(ann_type)*type,
                           &otag, &oref);

  *tag = otag;
  *ref = oref;

  return ret;
} /* nafigtr() */

/*-----------------------------------------------------------------------------
 * Name:    afiid2tr
 * Purpose: 
 * Inputs:  
 * Returns: see ANid2tagref()
 * Users:   Fortran Users
 * Invokes: ANid2tagerf()
 * Author: GeorgeV
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
nafiid2tr(intf *ann_id, intf *tag, intf *ref)
{
#ifdef LATER
  CONSTR(FUNC, "afiid2tr");
#endif /* LATER */
  intf   ret;
  uint16 otag, oref;

  ret = (intf)ANid2tagref((int32)*ann_id, &otag, &oref);

  *tag = otag;
  *ref = oref;

  return ret;
} /* nafiid2tr() */

/*-----------------------------------------------------------------------------
 * Name:    afitr2id
 * Purpose: 
 * Inputs:  
 * Returns: see ANtagref2id()
 * Users:   Fortran Users
 * Invokes: ANtagref2id()
 * Author: GeorgeV
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
nafitr2id(intf *an_id, intf *tag, intf *ref)
{
#ifdef LATER
  CONSTR(FUNC, "afitr2id");
#endif /* LATER */
  
  return (intf)ANtagref2id((int32)*an_id, (uint16)*tag, (uint16)*ref);

} /* nafitr2id() */

/*-----------------------------------------------------------------------------
 * Name:    afitp2tg
 * Purpose: 
 * Inputs:  
 * Returns: see ANatype2tag()
 * Users:   Fortran Users
 * Invokes: ANatype2tag()
 * Author: GeorgeV
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
nafitp2tg(intf *atype)
{
#ifdef LATER
  CONSTR(FUNC, "afitp2tg");
#endif /* LATER */

  return (intf)ANatype2tag((ann_type)*atype);

} /* nafitp2tg() */

/*-----------------------------------------------------------------------------
 * Name:    afitg2tp
 * Purpose: 
 * Inputs:  
 * Returns: see ANtag2atype()
 * Users:   Fortran Users
 * Invokes: ANtag2atype()
 * Author: GeorgeV
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
nafitg2tp(intf *tag)
{
#ifdef LATER
  CONSTR(FUNC, "afitg2tp");
#endif /* LATER */

  return (intf)ANtag2atype((uint16)*tag);

} /* nafitg2tp() */
