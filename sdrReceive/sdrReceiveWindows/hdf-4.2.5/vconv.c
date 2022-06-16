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

/* $Id: vconv.c 4932 2007-09-07 17:17:23Z bmribler $ */

/* obsolete code for HDF 3.2. 26/march/92 jason ng */
/* except for the following routines:
   *    vicheckcompat()
   *    movebytes ()
   *    oldunpackvg ()
   *    oldunpackvs ()
 map_from_old_types -- Convert an old type (i.e. LOCAL_INT to DFNT_ based types)

 */

/*
   *
   * vconv.c
   * Part of the HDF Vset interface.
 */

#define VSET_INTERFACE
#include "hdf.h"

/*
   ** ==================================================================
   ** PRIVATE data areas and routines
   ** ==================================================================
   * */

/*
   * types used in defining a new field via a call to VSfdefine
 */

#define LOCAL_NOTYPE        0
#define LOCAL_CHARTYPE      1   /* 8-bit ascii text stream */
#define LOCAL_INTTYPE       2   /* 32-bit integers - don't use */
#define LOCAL_FLOATTYPE     3   /* as opposed to DOUBLE */
#define LOCAL_LONGTYPE      4   /* 32-bit integers */
#define LOCAL_BYTETYPE      5   /* 8-bit byte stream - unsupported */
#define LOCAL_SHORTTYPE     6   /* 16-bit integers - unsupported */
#define LOCAL_DOUBLETYPE    7   /* as opposed to FLOAT - unsupported */

/*
   * actual LOCAL MACHINE sizes of the above types
 */

#define LOCAL_UNTYPEDSIZE  0
#define LOCAL_CHARSIZE      sizeof(char)
#define LOCAL_INTSIZE       sizeof(int)
#define LOCAL_FLOATSIZE     sizeof(float)
#define LOCAL_LONGSIZE      sizeof(long)
#define LOCAL_BYTESIZE      sizeof(unsigned char)
#define LOCAL_SHORTSIZE     sizeof(short)
#define LOCAL_DOUBLESIZE    sizeof(double)

/*
   stores sizes of local machine's known types
 */

PRIVATE int16 local_sizetab[] =
{
    LOCAL_UNTYPEDSIZE,
    LOCAL_CHARSIZE,
    LOCAL_INTSIZE,
    LOCAL_FLOATSIZE,
    LOCAL_LONGSIZE,
    LOCAL_BYTESIZE,
    LOCAL_SHORTSIZE,
    LOCAL_DOUBLESIZE
};

#define LOCALSIZETAB_SIZE sizeof(local_sizetab)/sizeof(int)

/*
 ** returns the machine size of a field type
 ** returns FAIL if error
 */
PRIVATE int16
VSIZEOF(int16 x)
{
    if (x < 0 || x > (int16)(LOCALSIZETAB_SIZE - 1))
      {
          return (FAIL);
      }
    else
      {
          return (local_sizetab[x]);
      }
}   /* VSIZEOF */


/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */
/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */
/*                                                                    */
/* routines for converting from vsets in v1.0 to v2.x                 */
/*                                                                    */
/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */
/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */

/* ------------------------------------------------------------------ */

PRIVATE void oldunpackvg(VGROUP * vg, uint8 buf[], int32 *size);
PRIVATE void oldunpackvs(VDATA * vs, uint8 buf[], int32 *size);

/*
   *  this routine checks that the given OPENED file is compatible with
   *    version 2.0 or later of the HDF Vset library .
   *  RETURNS 1  if file already compatible with r2.
   *          0  if not compatible.
   *          -1 if error.
 */
int32
vicheckcompat(HFILEID f)
{
    int16       foundold, foundnew;
    int32       aid;

    foundold = 0;
    foundnew = 0;
    /* locate any OLD vgs */
    aid = Hstartread(f, (uint16) OLD_VGDESCTAG, DFREF_WILDCARD);
    if (aid != FAIL)
      {
        foundold++;
        Hendaccess(aid);
      }

    /* locate any OLD vdatas */
    aid = Hstartread(f, (uint16) OLD_VSDESCTAG, DFREF_WILDCARD);
    if (aid != FAIL)
      {
        foundold++;
        Hendaccess(aid);
      }

    /* locate any NEW vgs */
    aid = Hstartread(f, NEW_VGDESCTAG, DFREF_WILDCARD);
    if (aid != FAIL)
      {
        foundnew++;
        Hendaccess(aid);
      }

    /* locate any NEW vdatas */
    aid = Hstartread(f, NEW_VSDESCTAG, DFREF_WILDCARD);
    if (aid != FAIL)
      {
        foundnew++;
        Hendaccess(aid);
      }

    HEclear();	/* clear the stack to remove faux failures - bug #655 */
    if (foundold == 0)  /* has no old vset elements */
        return (1);     /* just assume compatible */

    if (foundnew > 0)
        return (1);     /* file is already compatible */
    else
        return (0);     /* file is not compatible */
}   /* vicheckcompat */

/* ------------------------------------------------------------------ */
/*
   * This routine will modify a given OPENED file so that is becomes compatible
   * with version 2.0 or later of the HDF Vset library.
   * Note that the file is assumed to be not compatible to begin with.
   * This routine will not check to see if the file is already compatible,
   * but it is harmless to run an already-compatible file through again.
   *
   * However, be aware that each time, the file gets larger.
   * Also, file must be opened with DFACC_ALL access.
   *
   * returns  1 if successful. if error, returns 0
 */

int32
vimakecompat(HFILEID f)
{
    VGROUP     *vg;
    VDATA      *vs;
    uint8      *buf = NULL;     /* to store an old vdata or vgroup descriptor  */
    int32       old_bsize = 0, bsize=0;
    int32       aid;
    int32       ret;
    uintn       u;
    uint16      tag=DFTAG_NULL, ref=DFTAG_NULL;
    CONSTR(FUNC, "vimakecompat");

    /* =============================================  */
    /* --- read all vgs and convert each --- */

    /* allocate space for vg */
    if (NULL == (vg =VIget_vgroup_node()))
        HRETURN_ERROR(DFE_NOSPACE, 0);
    ret = aid = Hstartread(f, (uint16) OLD_VGDESCTAG, DFREF_WILDCARD);
    while (ret != FAIL)
      {
          HQuerytagref(aid, &tag, &ref);
          HQuerylength(aid, &bsize);
          if (buf == NULL || bsize > old_bsize)
            {
                if (buf != NULL)
                    HDfree((VOIDP) buf);
                if ((buf = (uint8 *) HDmalloc(bsize)) == NULL)
                    HRETURN_ERROR(DFE_NOSPACE, 0);
                old_bsize = bsize;
            }   /* end if */
          ret = Hgetelement(f, (uint16) OLD_VGDESCTAG, ref, (uint8 *) buf);
          if (ret == FAIL)
            {
                HDfree((VOIDP) buf);
                HRETURN_ERROR(DFE_READERROR, 0)
            }   /* end if */

          oldunpackvg(vg, buf, &bsize);
          /* add new items */
          vg->vgclass[0] = '\0';
          vg->extag = 0;
          vg->exref = 0;
          vg->version = 2;  /* version 2 */
          vg->more = 0;
          /* inside each vgroup, change the old tags to new */
          for (u = 0; u < (uintn)vg->nvelt; u++)
              if (vg->tag[u] == OLD_VGDESCTAG)
                  vg->tag[u] = NEW_VGDESCTAG;
              else if (vg->tag[u] == OLD_VSDESCTAG)
                  vg->tag[u] = NEW_VSDESCTAG;
              else  /* BAD */
                  HERROR(DFE_NOTINSET);
          vpackvg(vg, buf, &bsize);

          ret = Hputelement(f, VGDESCTAG, ref, (uint8 *) buf, bsize);
          HDfree((VOIDP) buf);
          if (ret == FAIL)
              HRETURN_ERROR(DFE_WRITEERROR, 0);

          ret = Hnextread(aid, (uint16) OLD_VGDESCTAG, DFREF_WILDCARD, DF_CURRENT);
      }     /* while */
    Hendaccess(aid);
    VIrelease_vgroup_node(vg);

    /* =============================================  */
    /* --- read all vdata descs  and convert each --- */
    /* --- then dup a tag for each vdata data elt --- */

    old_bsize = 0;  /* reset state variables */
    buf = NULL;
    if ((vs = VSIget_vdata_node()) == NULL)
        HRETURN_ERROR(DFE_NOSPACE, 0);
    ret = aid = Hstartread(f, (uint16) OLD_VSDESCTAG, DFREF_WILDCARD);
    while (ret != FAIL)
      {

          HQuerytagref(aid, &tag, &ref);
          HQuerylength(aid, &bsize);
          if (buf == NULL || bsize > old_bsize)
            {
                if (buf != NULL)
                    HDfree((VOIDP) buf);
                if ((buf = (uint8 *) HDmalloc(bsize)) == NULL)
                    HRETURN_ERROR(DFE_NOSPACE, 0);
                old_bsize = bsize;
            }   /* end if */
          ret = Hgetelement(f, tag, ref, (uint8 *) buf);
          if (ret == FAIL)
            {
                HDfree((VOIDP) buf);
                HRETURN_ERROR(DFE_READERROR, 0)
            }   /* end if */

          oldunpackvs(vs, buf, &bsize);

          /* add new items */
          vs->vsclass[0] = '\0';
          vs->extag = 0;
          vs->exref = 0;
          vs->version = 2;  /* version 2 */
          vs->more = 0;
          vpackvs(vs, buf, &bsize);

          ret = Hputelement(f, VSDESCTAG, ref, (uint8 *) buf, bsize);
          if (ret == FAIL)
            {
                HDfree((VOIDP) buf);
                HRETURN_ERROR(DFE_WRITEERROR, 0)
            }   /* end if */

          /* duplicate a tag to point to vdata data */
          ret = Hdupdd(f, NEW_VSDATATAG, ref, (uint16) OLD_VSDATATAG, ref);
          HDfree((VOIDP) buf);
          if (ret == FAIL)
              HRETURN_ERROR(DFE_DUPDD, 0);
          ret = Hnextread(aid, (uint16) OLD_VSDESCTAG, DFREF_WILDCARD, DF_CURRENT);
      }     /* while */

    Hendaccess(aid);
    VSIrelease_vdata_node(vs);

    return (1);

}   /* vimakecompat */

/* ================================================================== */
/*
   *  this routine checks that the given file is compatible with
   *    version 2.0 or later of the HDF Vset library .
   *
   *  All it does is to open the file, call vicheckcompat to do all the
   *  checking, and then closes it.
   *    See comments for vicheckcompat().

   *  returns 1 if file already compatible with r2.
   *          0 if not compatible.
   *          -1 if error.
 */

int32
vcheckcompat(char *fs)
{

    HFILEID     f;
    int32       ret;
    CONSTR(FUNC, "vcheckcompat");

    f = Hopen(fs, DFACC_ALL, 0);
    if (f == FAIL)
        HRETURN_ERROR(DFE_BADOPEN, FAIL);
    ret = vicheckcompat(f);
    Hclose(f);

    return (ret);
}   /* vcheckcompat */

/* ================================================================== */
/*
   * This routine will modify a given file so that is becomes compatible
   * with version 2.0 or later of the HDF Vset library.
   *
   * All this routine does is to open the file, call vimakecompat to
   * do all the conversion, and then to close the file.
   * See comments for vimakecompat().
   *
   * returns  1 if successful. if error, returns 0
 */

int32
vmakecompat(char *fs)
{
    HFILEID     f;
    int32       ret;
    CONSTR(FUNC, "vmakecompat");

    f = Hopen(fs, DFACC_ALL, 0);
    if (f == FAIL)
        HRETURN_ERROR(DFE_BADOPEN, FAIL);
    ret = vimakecompat(f);
    Hclose(f);
    return (ret);
}   /* vmakecompat */

/* ==================================================================== */

static void
oldunpackvg(VGROUP * vg, uint8 buf[], int32 *size)
{
    uint8      *bb;
    int16       int16var;
    uintn       i;
#ifdef LATER
    CONSTR(FUNC, "oldunpackvg");
#endif

    *size = *size;  /* dummy, so that compiler thinks it is used  */

    bb = &buf[0];

    /* retrieve nvelt */
    INT16DECODE(bb, int16var);
    vg->nvelt=(uint16)int16var;

    /* retrieve the tags */
    for (i = 0; i < (uintn)vg->nvelt; i++)
        UINT16DECODE(bb, vg->tag[i]);

    /* retrieve the refs */
    for (i = 0; i < (uintn)vg->nvelt; i++)
        UINT16DECODE(bb, vg->ref[i]);

    /* retrieve vgname */
    HDstrcpy(vg->vgname, (char *) bb);
}   /* oldunpackvg */

/* ================================================================= */

static void
oldunpackvs(VDATA * vs, uint8 buf[], int32 *size)
{
    uint8      *bb;
    int16       int16var;
    intn        i;
#ifdef LATER
    CONSTR(FUNC, "oldunpackvs");
#endif

    *size = *size;  /* dummy */

    bb = &buf[0];

    INT16DECODE(bb, vs->interlace);

    INT32DECODE(bb, vs->nvertices);

    UINT16DECODE(bb, vs->wlist.ivsize);

    INT16DECODE(bb, int16var);
    vs->wlist.n=(intn)int16var;

    for (i = 0; i < vs->wlist.n; i++)   /* retrieve the type */
        INT16DECODE(bb, vs->wlist.type[i]);

    for (i = 0; i < vs->wlist.n; i++)   /* retrieve the isize */
        UINT16DECODE(bb, vs->wlist.isize[i]);

    for (i = 0; i < vs->wlist.n; i++)   /* retrieve the off */
        UINT16DECODE(bb, vs->wlist.off[i]);

    for (i = 0; i < vs->wlist.n; i++)   /* retrieve the order */
        UINT16DECODE(bb, vs->wlist.order[i]);

    for (i = 0; i < vs->wlist.n; i++)
      {
          HDstrcpy(vs->wlist.name[i], (char *) bb);
          bb += (HDstrlen(vs->wlist.name[i]) + 1);
      }

    HDstrcpy(vs->vsname, (char *) bb);
    bb += (HDstrlen(vs->vsname) + 1);

    /* **EXTRA**  fill in the machine-dependent size fields */
    for (i = 0; i < vs->wlist.n; i++) /* FAIL check on VSIZEOF()? */
        vs->wlist.esize[i] = (uint16) (vs->wlist.order[i] * VSIZEOF((int16) vs->wlist.type[i]));

}   /* oldunpackvs */

/* ----------------------- map_from_old_types ------------------------------- */
/*
   Convert an old type (i.e. LOCAL_INT) to DFNT_ based types
 */
int16
map_from_old_types(intn type)
{
    switch (type)
      {
          case LOCAL_CHARTYPE:
              return DFNT_CHAR;

          case LOCAL_BYTETYPE:
              return DFNT_INT8;

          case LOCAL_SHORTTYPE:
          case LOCAL_INTTYPE:
              return DFNT_INT16;

          case LOCAL_LONGTYPE:
              return DFNT_INT32;

          case LOCAL_FLOATTYPE:
              return DFNT_FLOAT32;

          case LOCAL_DOUBLETYPE:
              return DFNT_FLOAT64;

          default:
              return (int16) type;
      }
}   /* map_from_old_types */

/* ------------------------------------------------------------------ */
