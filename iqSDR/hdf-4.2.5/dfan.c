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
static char RcsId[] = "@(#)$Revision: 5322 $";
#endif

/* $Id: dfan.c 5322 2010-01-19 06:26:11Z brtnfld $ */

/*-----------------------------------------------------------------------------
 * File:     dfan.c
 * Purpose:  read and write annotations: labels and descriptions of data items
 * Invokes:  df.c
 * Contents:
 *
 *  DFANclear:      - reset DFAN interface
 *  DFANgetlablen:  - get length of label of tag/ref
 *  DFANgetlabel:   - get label of tag/ref
 *  DFANgetdesclen: - get length of description of tag/ref
 *  DFANgetdesc:    - get description of tag/ref
 *
 *  DFANgetfidlen:  - get length of file ID
 *  DFANgetfid:     - get file ID
 *  DFANgetfdslen:  - get length of file description
 *  DFANgetfds:     - get file description
 *
 *  DFANputlabel:   - put label of tag/ref
 *  DFANputdesc:    - put description of tag/ref
 *
 *  DFANaddfid:     - add file ID
 *  DFANaddfds:     - add file description
 *
 *  DFANlastref:    - return ref of last annotation read or written
 *  DFANlablist:    - get list of labels for a particular tag
 *
 *  DFANIclear:     - clear Lastref, label/desc entries and directories
 *  DFANIopen:      - open/reopen file
 *  DFANIlocate:    - return ref of label/desc of tag/ref
 *  DFANIaddentry:  - add entry in annotation directory
 *  DFANIgetannlen: - get length of annotation of tag/ref
 *  DFANIgetann:    - get annotation of tag/ref
 *  DFANIputann:    - put annotation of tag/ref
 *  DFANIlablist:   - get list of labels for a particular tag
 *
 *  DFANIaddfann:   - add file annotation (ID or description)
 *  DFANIgetfannlen: - get length of file annotation
 *  DFANIgetfann:    - get file annotation
 *---------------------------------------------------------------------------*/

#include "hdf.h"
#include "dfan.h"
PRIVATE uint16 Lastref = 0;     /* Last ref read/written */
PRIVATE uint16 Next_label_ref = 0;  /* Next file label ref to read/write */
PRIVATE uint16 Next_desc_ref = 0;   /* Next file desc ref to read/write */

#if 0
static char Lastfile[DF_MAXFNLEN] = "";     /* last file opened */
#endif
PRIVATE char *Lastfile = NULL;

/* pointers to directories of object annotations */
PRIVATE DFANdirhead *DFANdir[2] =
{NULL,                          /* object labels       */
 NULL                           /* object descriptions */
};

/* Whether we've installed the library termination function yet for this interface */
PRIVATE intn library_terminate = FALSE;

/*
   ** Prototypes for local functions
 */

PRIVATE int32 DFANIopen(const char *filename, intn acc_mode);
PRIVATE intn DFANIstart(void);

/*-----------------------------------------------------------------------------
 * HDF object (i.e. tag/ref) label and description input routines
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 NAME
       DFANgetlablen -- get length of label of tag/ref
 USAGE
       int32 DFANgetlablen(filename, tag, ref)
       char *filename;     IN: filename: name of HDF file
       uint16 tag;         IN: tag of data object assigned the label
       uint16 ref;         IN: reference number of the data object assigned
                               the label
 RETURNS
       length of label on success, FAIL (-1) otherwise
 DESCRIPTION
       Calls DFANIgetannlen to get label length.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
       This routine should be used to insure that there is enough space
       allocated for a label before actually reading it.
 EXAMPLES
 REVISION LOG
 *---------------------------------------------------------------------------*/
int32
DFANgetlablen(const char *filename, uint16 tag, uint16 ref)
{
  int32 ret_value;

  ret_value = (DFANIgetannlen(filename, tag, ref, DFAN_LABEL));

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANgetlabel -- get label of object identified by tag/ref
 USAGE
       int DFANgetlabel(filename, tag, ref, label, maxlen)
       char *filename;   IN: name of HDF file
       uint16 tag;       IN: tag of object of assigned the label
       uint16 ref;       IN: ref number of object of assigned the label
       char *label;      OUT: buffer allocated to hold the label
       int32 maxlen;     IN: size of buffer allocated to hold the label
 RETURNS
       SUCCEED (0) if successful and FAIL (-1) otherwise
 DESCRIPTION
       Invokes DFANIgetann to get label
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
       The value of maxlen must be at least one greater than the
       anticipated length of the label, because a NULL byte is appended
       to the annotation
 EXAMPLES
 REVISION LOG
 *---------------------------------------------------------------------------*/
intn
DFANgetlabel(const char *filename, uint16 tag, uint16 ref, char *label,
             int32 maxlen)
{
  intn ret_value;

  ret_value = (DFANIgetann(filename, tag, ref, (uint8 *) label, maxlen, DFAN_LABEL, 0));

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANgetdesclen -- get length of description of tag/ref
 USAGE
       int32 DFANgetdesclen(filename, tag, ref)
       char *filename;  IN: name of HDF file
       uint16 tag;      IN: tag of item of which we want description
       uint16 ref;      IN: ref number of item of which we want description
 RETURNS
       Length of description if successful, and FAIL (-1) otherwise
 DESCRIPTION
       Calls DFANIgetannlen to get description length
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
       This routine should be used to insure that there is enough space
       allocated for a description before actually reading it.
 EXAMPLES
 REVISION LOG
 *------------------------------------------------------------------------*/
int32
DFANgetdesclen(const char *filename, uint16 tag, uint16 ref)
{
  int32 ret_value;

  ret_value = (DFANIgetannlen(filename, tag, ref, DFAN_DESC));

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANgetdesc -- Reads the description assigned to the data object
                      with the given tag and reference number.
 USAGE
       int DFANgetdesc(filename, tag, ref, desc, maxlen)
       char * filename; IN: name of HDF file
       uint16 tag;      IN: tag of object of assigned the label
       uint16 ref;      IN: ref number of object of assigned the label
       char *desc;      OUT: buffer allocated to hold the description
       int32 maxlen;    IN: size of buffer allocated to hold the description
 RETURNS
       SUCCEED (0) if successful and FAIL (-1) otherwise
 DESCRIPTION
       Invokes DFANIgetann to get description
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
       The value of maxlen must be at least one greater than the
       anticipated length of the description, because a NULL byte is
       appended to the annotation
 EXAMPLES
 REVISION LOG
 *------------------------------------------------------------------------*/

intn
DFANgetdesc(const char *filename, uint16 tag, uint16 ref, char *desc,
            int32 maxlen)
{
  intn ret_value;

  ret_value = (DFANIgetann(filename, tag, ref, (uint8 *) desc, maxlen, DFAN_DESC, 0));

  return ret_value;
}

/*-----------------------------------------------------------------------------
 * File ID and description input routines
 *---------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 NAME
       DFANgetfidlen -- get length of file label
 USAGE
       int32 DFANgetfidlen(file_id, isfirst)
       int32 file_id;    IN: HDF file label returned by Hopen
       int isfirst;      IN: 1 to read the first label in the file
                             0 to read the next label in the file
 RETURNS
       Length of file label if successful, and FAIL (-1) otherwise
 DESCRIPTION
       Calls DFANIgetfannlen to get label length
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
       When first called for a given file, DFANgetfidlen returns the
       length of the first file label.  In order to get the lengths
       of successive labels, you must call DFANgetfid between calls
       to DFANgetfidlen.  Otherwise, successive calls to DFANgetfidlen
       will return the length of the same file label.
 EXAMPLES
 REVISION LOG
 *------------------------------------------------------------------------*/
int32
DFANgetfidlen(int32 file_id, int isfirst)
{
  int32 ret_value;

  ret_value = (DFANIgetfannlen(file_id, DFAN_LABEL, isfirst));

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANgetfid -- get next file label
 USAGE
       int32 DFANgetfid(file_id, label, maxlen, isfirst)
       int32 file_id;   IN: HDF file label returned by Hopen
       char *label;     OUT: buffer allocated to hold the label
       int32 maxlen;    IN: size of buffer allocated to hold the label
       int isfirst;     IN: 0 to read the next label in the file;
                            1 to read the first label in the file
 RETURNS
       On success, length of label; FAIL (-1) otherwise
 DESCRIPTION
       Invokes DFANIgetfann to get label
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
       If maxlen is not great enough, the label is truncated to
       maxlen-1 characters.
 EXAMPLES
 REVISION LOG
 *---------------------------------------------------------------------------*/
int32
DFANgetfid(int32 file_id, char *label, int32 maxlen, intn isfirst)
{
  int32 ret_value;

  ret_value = (DFANIgetfann(file_id, label, maxlen, DFAN_LABEL, isfirst));

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANgetfdslen -- get length of file description
 USAGE
       int32 DFANgetfdslen(file_id, isfirst)
       int32 file_id;    IN: HDF file description returned by Hopen
       int isfirst;      IN: 1 to read the first description in the file
                             0 to read the next description in the file
 RETURNS
       Length of file description if successful, and FAIL (-1) otherwise
 DESCRIPTION
       Calls DFANIgetfannlen to get description length
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
       When first called for a given file, DFANgetfdslen returns the
       length of the first file description.  In order to get the lengths
       of successive descriptions, you must call DFANgetfds between calls
       to DFANgetfdslen.  Otherwise, successive calls to DFANgetfdslen
       will return the length of the same file description.
 EXAMPLES
 REVISION LOG
 *------------------------------------------------------------------------*/
int32
DFANgetfdslen(int32 file_id, intn isfirst)
{
  int32 ret_value;

  ret_value = (DFANIgetfannlen(file_id, DFAN_DESC, isfirst));

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANgetfds -- get next file description
 USAGE
       int32 DFANgetfds(file_id, desc, maxlen, isfirst)
       int32 file_id;  IN: HDF file description returned by Hopen
       char *desc;     OUT: buffer allocated to hold the description
       int32 maxlen;   IN: size of buffer allocated to hold the description
       int isfirst;    IN: 0 to read the next description in the file;
                           1 to read the first description in the file
 RETURNS
       On success, length of description; FAIL (-1) otherwise
 DESCRIPTION
       Invokes DFANIgetfann to get description
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
       If maxlen is not great enough, the description is truncated to
       maxlen-1 characters.
 EXAMPLES
 REVISION LOG
 *---------------------------------------------------------------------------*/
int32
DFANgetfds(int32 file_id, char *desc, int32 maxlen, intn isfirst)
{
  int32 ret_value;

  ret_value = (DFANIgetfann(file_id, desc, maxlen, DFAN_DESC, isfirst));

  return ret_value;
}

/*-----------------------------------------------------------------------------
 * HDF object (i.e. tag/ref) label and description output routines
 *---------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 NAME
       DFANputlabel -- Assign a label to the data object with the given
                       tag and reference number
 USAGE
       int DFANputlabel(filename, tag, ref, label)
       char *filename;  IN: name of HDF file
       uint16 tag;      IN: tag of item to be assigned the label
       uint16 ref;      IN: reference number of item to be assigned the label
       char *label;     IN: label to write to file; a single string of
                            NULL-terminated text
 RETURNS
       SUCCEED (0) if successful and FAIL (-1) otherwise
 DESCRIPTION
       Invokes DFANIgetfann to write out label
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
 *------------------------------------------------------------------------*/
intn
DFANputlabel(const char *filename, uint16 tag, uint16 ref, char *label)
{
  intn ret_value;

  ret_value = (DFANIputann(filename, tag, ref, (uint8 *) label,
                           (int32) HDstrlen(label), DFAN_LABEL));

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANputdesc -- Assign a description to the data object with the given
                      tag and reference number
 USAGE
       int DFANputdesc(filename, tag, ref, desc, desclen)
       char *filename;   IN: name of HDF file
       uint16 tag;       IN: tag of item to be assigned the description
       uint16 ref;       IN: ref number of item to be assigned description
       char *desc;       IN: description to write to file; a single string
                             of NULL-terminated text
       int32 desclen;  IN: length of description
 RETURNS
       SUCCEED (0) if successful and FAIL (-1) otherwise
 DESCRIPTION
       Invokes DFANIgetfann to write out description
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
       The parameter desc can contain any sequence of ASCII characters.
       It does not have to be a string.
 EXAMPLES
 REVISION LOG
 *------------------------------------------------------------------------*/
intn
DFANputdesc(const char *filename, uint16 tag, uint16 ref, char *desc,
            int32 desclen)
{
  intn ret_value;

  ret_value = (DFANIputann(filename, tag, ref, (uint8 *) desc, desclen, DFAN_DESC));

  return ret_value;
}

/*-----------------------------------------------------------------------------
 * File ID and description output routines
 *---------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 NAME
       DFANaddfid -- Write a file label to a file
 USAGE
       int DFANaddfid(file_id, id)
       int32 file_id;   IN: file identifier
       char *id;        IN: label to write to file
 RETURNS
       SUCCEED (0) if successful and FAIL (-1) otherwise
 DESCRIPTION
       Invokes DFANIaddfann to write out label
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
 *------------------------------------------------------------------------*/
intn
DFANaddfid(int32 file_id, char *id)
{
  intn ret_value;

  ret_value = (DFANIaddfann(file_id, id, (int32) HDstrlen(id), DFAN_LABEL));

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANaddfds -- Write a file description to a file
 USAGE
       int DFANaddfds(file_id, desc, desclen)
       int32 file_id;   IN: file identifier
       char *desc;      IN: description to write to file
       int32 desclen;   IN: length of description
 RETURNS
       SUCCEED (0) if successful and FAIL (-1) otherwise
 DESCRIPTION
       Invokes DFANIaddfann to write out description
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
       The parameter desc can contain any sequence of ASCII characters.
       It does not have to be a string.
 EXAMPLES
 REVISION LOG
 *------------------------------------------------------------------------*/
intn
DFANaddfds(int32 file_id, char *desc, int32 desclen)
{
  intn ret_value;

  ret_value = (DFANIaddfann(file_id, desc, desclen, DFAN_DESC));

  return ret_value;
}

/*-----------------------------------------------------------------------------
 * Miscellaneous other routines
 *---------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 NAME
       DFANlastref -- Return the reference number of the annotation last
                      written or read
 USAGE
       uint16 DFANlastref()
 RETURNS
       Reference number if successful and 0 otherwise
 DESCRIPTION
 GLOBAL VARIABLES
       Lastref
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
 *------------------------------------------------------------------------*/
uint16
DFANlastref(void)
{
  uint16 ret_value;

  ret_value = (Lastref);

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANlablist -- Return list of refs and labels for a given tag
 USAGE
       int DFANlablist(filename, tag, reflist, labellist,
                                                listsize, maxlen, startpos)
       char *filename;   IN: name of HDF file
       uint16 tag;       IN: tag to use when searching for ref numbers
                             and labels
       uint16 reflist[]; OUT: array allocated to place ref numbers in
       char *labellist;  OUT: array of strings allocated to place labels in
       int listsize;     IN: size of ref number list and label list
       int maxlen;       IN: maximum length allocated for label
       int startpos;     IN: Starting position.  Beginning from the
                             startpos'th entry, up to listsize entries
                             will be returned.
 RETURNS
       Number of ref numbers found if successful and FAIL (-1) otherwise
 DESCRIPTION
       Invokes DFANIlablist to get label list.
       Where there is no corresponding label, the position in
       labellist is zero filled (C) or blank filled (Fortran).
       Revised 04/17/90 so that it returns all ref numbers for
       the given tag, rather than just those that have labels.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
       Revised 04/17/90.  (See DESCRIPTION.)
 *------------------------------------------------------------------------*/
intn
DFANlablist(const char *filename, uint16 tag, uint16 reflist[], char *labellist,
            intn listsize, intn maxlen, intn startpos)
{
  intn ret_value;

  ret_value = (DFANIlablist(filename, tag, reflist, (uint8 *) labellist,
                            listsize, maxlen, startpos, 0));
  return ret_value;
}

/*-------------------------------------------------------------------
 Name
        DFANclear --  Clear DFAN interface
 Usage
        intn DFANclear()
 Returns
        SUCCEED if ok; FAIL otherwise.
 DESCRIPTION
        Invokes DFANIclear.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
        When a file is re-created in a single run by other
          interface, such as DFSDputdata(), user should
          call DFANclear to reset DFAN interface structures.
 EXAMPLES
 REVISION LOG
 *------------------------------------------------------------------*/
intn DFANclear(void)
{
  intn ret_value;

  ret_value = DFANIclear();

  return ret_value;
}

/******************************************************************************/
/*----------------------- Internal routines ---------------------------------*/
/******************************************************************************/

/*---------------------------------------------------------------------------
 Name
       DFANIclear -- Clear label/desc entries and directories
          Reset DFANdir[i] and Lastref
 USAGE
       intn DFANIclear()
 Returns
       SUCCEED if ok; FAIL otherwise.
 DESCRIPTION
       Reset DFANdir[i] and Lastref
 GLOBAL VARIABLES
        Lastref, DFANdir
 COMMENTS, BUGS, ASSUMPTIONS

 EXAMPLES
 REVISION LOG

 *-------------------------------------------------------------------------*/
intn
DFANIclear(void)
{
    CONSTR(FUNC, "DFANIclear");
  DFANdirhead *p, *q;
  intn ret_value = SUCCEED;

    HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFANIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  for (p=DFANdir[0]; p!=NULL; p=q) {  /* free linked list space */
    q = p->next;
    if (p->entries != NULL)
	    HDfree((VOIDP) p->entries);
    p->nentries=0;
    p->entries = NULL;
    p->next=NULL;
    HDfree((VOIDP) p);
  }
  for (p=DFANdir[1]; p!=NULL; p=q) {
    q = p->next;
    if (p->entries != NULL)
	    HDfree((VOIDP) p->entries);
    p->nentries=0;
    p->entries = NULL;
    p->next=NULL;
    HDfree((VOIDP) p);
  }
  DFANdir[0] = DFANdir[1] = NULL;

  Lastref = 0; /* 0 is invalid ref */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANIopen -- open or reopen a file
 USAGE
       PRIVATE int32 DFANIopen(filename, acc_mode)
       char *filename;  IN: name of file to open
       intn acc_mode;     IN: access mode
 RETURNS
       File identifier if successful and NULL on failure.
 DESCRIPTION
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
        This is a hook for someday providing more efficient ways to
        reopen a file, to avoid re-reading all the headers

 EXAMPLES
 REVISION LOG

 *------------------------------------------------------------------------*/
PRIVATE int32
DFANIopen(const char *filename, intn acc_mode)
{
  CONSTR(FUNC, "DFANIopen");
  int32       file_id;
  DFANdirhead *p, *q;
  int32      ret_value = SUCCEED;

    HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFANIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  /* Check if filename buffer has been allocated */
  if (Lastfile == NULL)
    {
      Lastfile = (char *) HDmalloc((DF_MAXFNLEN + 1) * sizeof(char));
      if (Lastfile == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);
      *Lastfile = '\0';     /* initialize with 0-length string */
    }

  /* use reopen if same file as last time - more efficient */
  if (HDstrncmp(Lastfile, filename, DF_MAXFNLEN) || (acc_mode == DFACC_CREATE))
    {
      /* treat create as different file */
      if (( file_id = Hopen(filename, acc_mode, 0))== FAIL)
          HGOTO_ERROR(DFE_BADOPEN,FAIL);

      for (p = DFANdir[0]; p != NULL; p = q)
        {   /* free linked list space */
          q = p->next;
	    if (p->entries != NULL)
		    HDfree((VOIDP) p->entries);
	    p->nentries=0;
	    p->entries = NULL;
	    p->next=NULL;
          HDfree((VOIDP) p);
        }
      for (p = DFANdir[1]; p != NULL; p = q)
        {
          q = p->next;
	    if (p->entries != NULL)
		    HDfree((VOIDP) p->entries);
	    p->nentries=0;
	    p->entries = NULL;
	    p->next=NULL;
          HDfree((VOIDP) p);
        }
      DFANdir[0] = DFANdir[1] = NULL;
    }
  else
    {
      if (( file_id = Hopen(filename, acc_mode, 0))== FAIL)
          HGOTO_ERROR(DFE_BADOPEN,FAIL);
    }

  HIstrncpy(Lastfile, filename, DF_MAXFNLEN);
  /* remember filename, so reopen may be used next time if same file */

  ret_value = file_id;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANIlocate -- get ref of annotation of given data tag/ref
 USAGE
       uint16 DFANIlocate(file_id, type, tag, ref)
       int32 file_id:   IN: pointer to HDF file
       int type:        IN: DFAN_LABEL for labels, DFAN_DESC for descriptions
       uint16 tag, ref: IN: tag/ref of item for which we want ref of
                            annotation

 RETURNS
        ref of annotation on if successful; 0 otherwise
 DESCRIPTION
       Searches through directory for annotations with given tag/ref.  (If
       there is no directory, it creates one.)

 GLOBAL VARIABLES

 COMMENTS, BUGS, ASSUMPTIONS
       BUG: When FORTRAN calls this routine with type "label", the string
       returned is incorrect in length by one character
 EXAMPLES

 REVISION LOG

 *------------------------------------------------------------------------*/
uint16
DFANIlocate(int32 file_id, int type, uint16 tag, uint16 ref)
{
  CONSTR(FUNC, "DFANIlocate");
  uint8       datadi[4];
  int32       more_anns;
  int32       aid;
  int32       nanns, i;
  uint16      anntag, annref = 0;
  DFANdirhead *p;
  uint8      *ptr;
  uint16     ret_value = 0; /* FAIL ? */

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFANIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, (uint16)FAIL);

  anntag = (uint16) ((type == DFAN_LABEL) ? DFTAG_DIL : DFTAG_DIA);

  /* if no directory for this type of annotation, make one */
  if (DFANdir[type] == NULL)
    {
      nanns = Hnumber(file_id, anntag);
      if (nanns == 0)
        HGOTO_ERROR(DFE_INTERNAL, 0);

      /* allocate directory space, and space for entries. */
      DFANdir[type] = (DFANdirhead *) HDmalloc((uint32) sizeof(DFANdirhead));
      if (DFANdir[type] == NULL)
        HGOTO_ERROR(DFE_NOSPACE, 0);
      DFANdir[type]->entries = (DFANdirentry *)
        HDmalloc( (size_t)nanns * sizeof(DFANdirentry));
      if (DFANdir[type]->entries == NULL) 
        HGOTO_ERROR(DFE_NOSPACE, 0);

      DFANdir[type]->next = NULL;
      DFANdir[type]->nentries = nanns;

      /* fill directory table */
      if (( aid = Hstartread(file_id, anntag, DFREF_WILDCARD))== FAIL)
        {
          HGOTO_ERROR(DFE_BADAID, 0);
        } /* end if */
      else
        more_anns = SUCCEED;

      for (i = 0; (i < nanns) && (more_anns != FAIL); i++)
        {
          if (FAIL == Hinquire(aid, (int32 *) NULL, (uint16 *) NULL, &annref,
                               (int32 *) NULL, (int32 *) NULL, (int32 *) NULL,
                               (int16 *) NULL, (int16 *) NULL))
                HGOTO_ERROR(DFE_INTERNAL, 0);

          if ((int32) FAIL == Hread(aid, (int32) 4, datadi))
                HGOTO_ERROR(DFE_READERROR, 0);

          /* get data tag/ref */
          DFANdir[type]->entries[i].annref = annref;
          ptr = (uint8 *) &(datadi[0]);
          UINT16DECODE(ptr, DFANdir[type]->entries[i].datatag);
          UINT16DECODE(ptr, DFANdir[type]->entries[i].dataref);

          more_anns = Hnextread(aid, anntag, DFREF_WILDCARD, DF_CURRENT);
        }
      Hendaccess(aid);
    }

  if (!tag)
    HGOTO_DONE(1);

  /* find annotation that goes with this tag/ref */
  for (p = (DFANdirhead *) DFANdir[type]; p != NULL; p = p->next)
    for (i = 0; i < p->nentries; i++)
      if (p->entries[i].annref != 0)
        if ((p->entries[i].dataref == ref) && (p->entries[i].datatag == tag))
            HGOTO_DONE(p->entries[i].annref);

  HERROR(DFE_NOMATCH);

done:
  if(ret_value == 0)   
    { /* Error condition cleanup */

    } /* end if */
  /* Normal function cleanup */

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANIaddentry -- add entry to annotation directory
 USAGE
       int type:        IN: DFAN_LABEL for labels, DFAN_DESC for descriptions
       uint16 annref:   IN: ref of annotation
       uint16 datatag,: IN: tag of item of which this is annotation
       uint16 dataref;  IN: ref of item of which this is annotation

 RETURNS
       SUCCEED (0) if successful and FAIL (-1) otherwise
 DESCRIPTION
       Moves to end of directory and appends entry.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
 *------------------------------------------------------------------------*/
int
DFANIaddentry(int type, uint16 annref, uint16 datatag, uint16 dataref)
{
  CONSTR(FUNC, "DFANIaddentry");
  int32       i;
  DFANdirhead *p, *q;
  int         ret_value = SUCCEED;

    HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFANIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  /* move to last entry in list */
  for (p = DFANdir[type]; (p != NULL) && (p->next != NULL); p = p->next)
    ;

  if (p)
    {     /* not new list */
      for (i = 0; i < p->nentries; i++)     /* check all entries */
        if (p->entries[i].annref == 0)
          {   /* empty slot */
            p->entries[i].annref = annref;  /* store entry */
            p->entries[i].datatag = datatag;
            p->entries[i].dataref = dataref;
            HGOTO_DONE(SUCCEED);
          }
    }

  /* need new list or new node in list */
  /* allocate directory space and space for entries. */
  if ((q = (DFANdirhead *) HDmalloc((uint32) sizeof(DFANdirhead))) == NULL)
      HGOTO_ERROR(DFE_NOSPACE,FAIL);
  q->entries = (DFANdirentry *) HDmalloc( DFAN_DEFENTRIES * sizeof(DFANdirentry));
  if (q->entries == NULL)
      HGOTO_ERROR(DFE_NOSPACE,FAIL);

  q->next = NULL;
  q->nentries = DFAN_DEFENTRIES;
  if (!p)
    DFANdir[type] = q;  /* set pointer to this new node */
  else
    p->next = q;

  /* store entry */
  q->entries[0].annref = annref;
  q->entries[0].datatag = datatag;
  q->entries[0].dataref = dataref;

  for (i = 1; i < DFAN_DEFENTRIES; i++)   /* mark rest unused */
    q->entries[i].annref = 0;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */
  /* Normal function cleanup */

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANIgetannlen -- get length of annotation of tag/ref
 USAGE
       int32 DFANIgetannlen(filename, tag, ref, type)
       char *filename;   IN: name of HDF file
       int32 tag, ref;   IN: tag/ref of item of which we want annotation
       int type;         IN: DFAN_LABEL for labels, DFAN_DESC for descriptions
 RETURNS
       length of annotation if successful and FAIL (-1) otherwise
 DESCRIPTION
       Calls DFANIlocate to get ref of annotation.
       Calls Hlength to get its length.
 GLOBAL VARIABLES
       Lastref
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
 *------------------------------------------------------------------------*/
int32
DFANIgetannlen(const char *filename, uint16 tag, uint16 ref, int type)
{
  CONSTR(FUNC, "DFANIgetannlen");
  int32       file_id, annlength;
  uint16      anntag, annref;
  int32       ret_value = SUCCEED;

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFANIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (!tag)
    HGOTO_ERROR(DFE_BADTAG,FAIL);

  if (!ref)
    HGOTO_ERROR(DFE_BADREF,FAIL);

  if (( file_id = DFANIopen(filename, DFACC_READ))== FAIL)
    HGOTO_ERROR(DFE_BADOPEN,FAIL);

    /* get ref of annotation of tag/ref */
  annref = DFANIlocate(file_id, type, tag, ref);
  if (annref == 0)
    HCLOSE_GOTO_ERROR(file_id,DFE_INTERNAL,FAIL);
  anntag = (uint16) ((type == DFAN_LABEL) ? DFTAG_DIL : DFTAG_DIA);   /* set type tag */

  annlength = Hlength(file_id, anntag, annref) - 4;   /* 4=len of data tag/ref */
  if (annlength == FAIL)
    HCLOSE_GOTO_ERROR(file_id,DFE_BADLEN,FAIL);
  Lastref = annref;   /* remember ref last accessed */
  if (Hclose(file_id) == FAIL)    /* close file */
    ret_value = FAIL;

  ret_value = annlength;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANIgetann -- get annotation of tag/ref
 USAGE
       intn DFANIgetann(filename, tag, ref, ann, maxlen, type)
       char *filename;   IN: name of HDF file
       uint16 tag, ref;  IN: tag/ref of item of which we want annotation
       uint8 *ann;       OUT: space to return annotation in
       int32 maxlen;     IN: size of space to return annotation in
       int type;         IN: DFAN_LABEL for labels, DFAN_DESC for descriptions
       int isfortran;    IN: 0 if C, 1 if Fortran
 RETURNS
       SUCCEED (0) if successful and FAIL (-1) otherwise
 DESCRIPTION
       Gets tag and ref of annotation.  Finds DD for that annotation.
       Reads the annotation, taking care of NULL terminator, if necessary.
 GLOBAL VARIABLES
       Lastref.
 COMMENTS, BUGS, ASSUMPTIONS
       BUG 1640: Added isfortran flag to avoid termination of the string with 
       the null character when the routine is call from FORTRAN.
 EXAMPLES
 REVISION LOG
 *------------------------------------------------------------------------*/
intn
DFANIgetann(const char *filename, uint16 tag, uint16 ref, uint8 *ann,
            int32 maxlen, int type, int isfortran)
{
  CONSTR(FUNC, "DFANIgetann");
  int32       file_id, aid;
  int32       annlen;
  uint16      anntag, annref;
  uint8       datadi[4];      /* to read in and discard data/ref! */
  intn        ret_value = SUCCEED;

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFANIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (!ann)
    HGOTO_ERROR(DFE_BADPTR,FAIL);

  if (!tag)
    HGOTO_ERROR(DFE_BADTAG,FAIL);

  if (!ref)
    HGOTO_ERROR(DFE_BADREF,FAIL);

  if (( file_id = DFANIopen(filename, DFACC_READ))== FAIL)
    HGOTO_ERROR(DFE_BADOPEN,FAIL);

  /* get annref and anntag of annotation of tag/ref */
  annref = DFANIlocate(file_id, type, tag, ref);
  if (annref == 0)
    HCLOSE_GOTO_ERROR(file_id,DFE_INTERNAL,FAIL);
  anntag = (uint16) ((type == DFAN_LABEL) ? DFTAG_DIL : DFTAG_DIA);

    /* find DD for that annotation */
  aid = Hstartread(file_id, anntag, annref);
  if (aid == FAIL)
      HCLOSE_GOTO_ERROR(file_id,DFE_BADAID,FAIL);

  if (FAIL == Hinquire(aid, (int32 *) NULL, (uint16 *) NULL, (uint16 *) NULL, 
                       &annlen, (int32 *) NULL, (int32 *) NULL, (int16 *) NULL, 
                       (int16 *) NULL))
    {
      Hendaccess(aid);
      HCLOSE_GOTO_ERROR(file_id,DFE_INTERNAL,FAIL);
    }
  annlen -= 4;    /* first four bytes were tag/ref, so they don't count */

  /* check length, if not enough space, truncate annotation */
  /* In C labels need space for null terminator, descriptions don't */
  if (isfortran)
    {
      if (annlen > maxlen )
	annlen = maxlen;
    }
  else
    if (type == DFAN_LABEL)
      {
	if (annlen > maxlen - 1)
	  annlen = maxlen - 1;
      }
    else
      {
	if (annlen > maxlen)
	  annlen = maxlen;
      }
  
  /* read annotation */
  if ((int32) FAIL == Hread(aid, (int32) 4, datadi))
    {     /* go past tag/ref */
      Hendaccess(aid);
      HCLOSE_GOTO_ERROR(file_id,DFE_READERROR,FAIL);
    }
  if ((int32) FAIL == Hread(aid, annlen, ann))
    {     /* read the annotation */
      Hendaccess(aid);
      HCLOSE_GOTO_ERROR(file_id,DFE_READERROR,FAIL);
    }
  /* add null for C */
  if (type == DFAN_LABEL)
    if (!isfortran)
      ann[annlen] = '\0'; /* terminate string properly for C */

  Lastref = annref;   /* remember ref last accessed */
  Hendaccess(aid);
  ret_value = (Hclose(file_id));

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */
  /* Normal function cleanup */

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANIputann -- put annotation of tag/ref into file
 USAGE
       intn DFANIputann(filename, tag, ref, ann, annlen, type)
       char *filename;   IN: name of HDF file
       uint16 tag, ref;  IN: tag/ref of item of which this is the annotation
       uint8 *ann;       IN: space to return annotation in
       int32 annlen;     IN: length of annotation
       int type;         IN: DFAN_LABEL for labels, DFAN_DESC for descriptions

 RETURNS
       SUCCEED (0) if successful and FAIL (-1) otherwise
 DESCRIPTION
       Checks for pre-existence of annotation, replacing old one if it
       exists. Writes out annotation, and updates directory..
 GLOBAL VARIABLES
       Lastref.
 COMMENTS, BUGS, ASSUMPTIONS
       If the given object already has this type of annotation, it replaces
       the old annotation with this one.
 EXAMPLES
 REVISION LOG
 *------------------------------------------------------------------------*/
intn
DFANIputann(const char *filename, uint16 tag, uint16 ref, uint8 *ann,
            int32 annlen, int type)
{
  CONSTR(FUNC, "DFANIputann");
  int32       file_id, aid;
  int         newflag = 0;
  uint16      anntag, annref;
  uint8       datadi[4];      /* to hold data tag/ref for writing */
  uint8      *ptr;
  intn        ret_value = SUCCEED;

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFANIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (!ann)
    HGOTO_ERROR(DFE_BADPTR,FAIL);

  if (!tag)
    HGOTO_ERROR(DFE_BADTAG,FAIL);

  if (!ref)
    HGOTO_ERROR(DFE_BADREF,FAIL);

  if (( file_id = DFANIopen(filename, DFACC_RDWR))== 0)
    HGOTO_ERROR(DFE_BADOPEN,FAIL);

  anntag = (uint16) ((type == DFAN_LABEL) ? DFTAG_DIL : DFTAG_DIA);

  /* check if this tag/ref already has this type of annotation */
  annref = DFANIlocate(file_id, type, tag, ref);
  if (annref == 0)
    {
      annref = Htagnewref(file_id,anntag);
      if (annref == 0)
        HCLOSE_GOTO_ERROR(file_id,DFE_NOREF,FAIL);
      newflag = 1;  /* remember to add ann tag/ref to directory */
    }

  /*
   * if annotation exists, reuse tag/ref and rewrite new annotation
   */
  if (newflag == 0)
    {     /* does prev annotation exist? */
      if (HDreuse_tagref(file_id, anntag, annref) == FAIL)
        {
          Hclose(file_id);
          HEreport("Unable to replace old annotation");
          HGOTO_DONE(FAIL);
        }
    }

  /* put annotation */
  /* Note: cannot use DFputelement because need to write data tag/ref */
  aid = Hstartwrite(file_id, anntag, annref, annlen + 4);
  if (aid == FAIL)
    {
      Hendaccess(aid);
      HCLOSE_GOTO_ERROR(file_id,DFE_BADAID,FAIL);
    }

  /* write annotation */
  ptr = (uint8 *) &(datadi[0]);   /* first, write the object's tag/ref */
  UINT16ENCODE(ptr, tag);
  UINT16ENCODE(ptr, ref);
  if ((int32) FAIL == Hwrite(aid, (int32) 4, datadi))
      HCLOSE_GOTO_ERROR(file_id,DFE_WRITEERROR,FAIL);
  if ((int32) FAIL == Hwrite(aid, annlen, ann))
    {     /* then write the annotation */
      Hendaccess(aid);
      HCLOSE_GOTO_ERROR(file_id,DFE_WRITEERROR,FAIL);
    }

  /* put annotation tag/ref into directory if new */
  if (newflag)
    {
      if (FAIL == DFANIaddentry(type, annref, tag, ref))
        {
          Hendaccess(aid);
          HCLOSE_GOTO_ERROR(file_id,DFE_INTERNAL,FAIL);
        }
    }

  Lastref = annref;   /* remember ref last accessed */
  Hendaccess(aid);
  ret_value = (Hclose(file_id));

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */
  /* Normal function cleanup */

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANIlablist -- Return list of refs and labels for a given tag
 USAGE
       int DFANlablist(filename, tag, reflist, labellist,
                                                listsize, maxlen, startpos)
       char *filename;   IN: name of HDF file
       uint16 tag;       IN: tag to use when searching for ref numbers
                             and labels
       uint16 reflist[]; OUT: array allocated to place ref numbers in
       char *labellist;  OUT: array of strings allocated to place labels in
       int listsize;     IN: size of ref number list and label list
       int maxlen;       IN: maximum length allocated for label
       int startpos;     IN: Starting position.  Beginning from the
                             startpos'th entry, up to listsize entries
                             will be returned.
       int isfortran: 0 if C, 1 if Fortran
 RETURNS
       Number of ref numbers found if successful and FAIL (-1) otherwise
 DESCRIPTION
       Where there is no corresponding label, the position in
       labellist is zero filled (C) or blank filled (Fortran).
       Revised 04/17/90 so that it returns all ref numbers for
       the given tag, rather than just those that have labels.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
       Revised 04/17/90.  (See DESCRIPTION.)
 *------------------------------------------------------------------------*/
intn
DFANIlablist(const char *filename, uint16 tag, uint16 reflist[],
             uint8 *labellist, int listsize, int maxlen, int startpos,
             int isfortran)
{
  CONSTR(FUNC, "DFANIlablist");
  int32       i;
  int         j, k;
  int32       file_id, aid, len;
  uint16      ref=0;
  DFANdirhead *p;
  uint8      *lp;             /* pointer to label */
  intn        nrefs, nlabs;
  uint8       labeldi[4];     /* to read in and discard data/ref */
  intn        ret_value = SUCCEED;

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFANIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (!reflist || !labellist)
    HGOTO_ERROR(DFE_BADPTR,FAIL);

  if (!tag)
    HGOTO_ERROR(DFE_BADTAG,FAIL);

  if (( file_id = DFANIopen(filename, DFACC_READ))== 0)
    HGOTO_ERROR(DFE_BADOPEN,FAIL);

    /* clear labellist.  pad with blanks for Fortran; add null for C  */
  if (isfortran)
    HDmemset(labellist, ' ', (uint32) maxlen * (uint32) listsize);
  else
    HDmemset(labellist, '\0', (uint32) maxlen * (uint32) listsize);

    /* find all refs for this tag; store them in reflist */
  nrefs = (intn) Hnumber(file_id, tag);   /* how many times is tag in file? */
  if (nrefs == FAIL)
    HCLOSE_GOTO_ERROR(file_id,DFE_NOMATCH,FAIL);

  aid = Hstartread(file_id, tag, DFREF_WILDCARD);     /* set search for refs */
  if (aid == FAIL)
    HCLOSE_GOTO_ERROR(file_id,DFE_BADAID,FAIL);

  for (i = 0, j = 0; i < nrefs && j < listsize; i++)
    {
      if (HQuerytagref(aid, (uint16 *) NULL, &ref) == FAIL)
        {
          Hendaccess(aid);
          HCLOSE_GOTO_ERROR(file_id,DFE_NOMATCH,FAIL);
        }
      if (i >= startpos - 1)
        reflist[j++] = ref;   /* store next ref in reflist */
      Hnextread(aid, tag, DFREF_WILDCARD, DF_CURRENT);
    }
  nrefs = j;
  Hendaccess(aid);

  /* get directory of all labels */

  nlabs = (intn) Hnumber(file_id, DFTAG_DIL);
  if (nlabs != 0)
    {
      if (DFANdir[DFAN_LABEL] == NULL)
        {   /* if no dir info create dir */
          if (0 == DFANIlocate(file_id, DFAN_LABEL, 0, 0))
            {
              Hendaccess(aid);
              HCLOSE_GOTO_ERROR(file_id,DFE_INTERNAL,FAIL);
            }
        }

      lp = labellist;

      /* Look through all labels. Get those that correspond to the tag,
         and match them with corresponding tag/refs in the reflist.      */
      for (p = DFANdir[DFAN_LABEL]; p != NULL; p = p->next)
        {   /* for each ann dir */
          for (i = 0; i < p->nentries; i++)
            {     /* for each tag in dir */
              if (p->entries[i].datatag == tag)
                {   /* if this tag==our tag */

                  aid = Hstartread(file_id, DFTAG_DIL, p->entries[i].annref);
                  if (aid == FAIL)
                      HCLOSE_GOTO_ERROR(file_id,DFE_BADAID,FAIL);
                  if ((int32) FAIL == Hread(aid, (int32) 4, labeldi))
                    {     /* data tag/ref */
                      Hendaccess(aid);
                      HCLOSE_GOTO_ERROR(file_id,DFE_READERROR,FAIL);
                    }
                  /* look for corresponding ref in reflist */
                  for (k = 0; k < nrefs && p->entries[i].dataref != reflist[k]; k++)
                    ;
                  if (k < nrefs)
                    {     /* if ref found */

                      lp = labellist + k * maxlen;  /* get pos to copy to */

                      /* note len on read may be too big, but OK for DFread */
                      len = Hread(aid, (int32) (maxlen - 1), lp);
                      if (len == FAIL)
                        {
                          Hendaccess(aid);
                          HCLOSE_GOTO_ERROR(file_id,DFE_READERROR,FAIL);
                        }
                      /* ret now contains actual length read */
                      /* pad with blanks for Fortran; add null for C */
                      if (isfortran)
                        while (len++ < maxlen)
                          lp[len] = ' ';
                      else
                        lp[len] = '\0';
                    }
                  Hendaccess(aid);
                }   /* tag == our tag  */
            }     /* for each tag in dir  */
        }   /* for each ann dir  */
    }     /* nlabs != 0  */
  if (FAIL == Hclose(file_id))    /* close file */
    ret_value = FAIL;
  else
    ret_value = nrefs;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANaddfann -- Write a file label or description to a file
 USAGE
       int DFANaddfid(file_id, id)
       int32 file_id;  IN: pointer to HDF file
       char *ann;      IN: annotation to write to file
       int32 annlen:   IN: length of annotation
       int type:       IN: DFAN_LABEL for labels, DFAN_DESC for descriptions
 RETURNS
       SUCCEED (0) if successful and FAIL (-1) otherwise
 DESCRIPTION
       Calls Hputelement to putput the annotation.
 GLOBAL VARIABLES
       Lastref
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
 *------------------------------------------------------------------------*/
int
DFANIaddfann(int32 file_id, char *ann, int32 annlen, int type)
{
  CONSTR(FUNC, "DFANIaddfann");
  uint16      anntag, annref;
  int         ret_value = SUCCEED;

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFANIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (!ann)
    HGOTO_ERROR(DFE_BADPTR,FAIL);

  anntag = (uint16) ((type == DFAN_LABEL) ? DFTAG_FID : DFTAG_FD);

  annref = Htagnewref(file_id,anntag);
  if (annref == 0)
    HGOTO_ERROR(DFE_NOREF,FAIL);

    /* write out annotation */
  if (FAIL == Hputelement(file_id, anntag, annref, (uint8 *) ann, annlen))
    HGOTO_ERROR(DFE_PUTELEM,FAIL);

  Lastref = annref;   /* remember ref last accessed */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */
  /* Normal function cleanup */

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANIgetfannlen -- get length of next file annotation
 USAGE
       int32 DFANIgetannlen(filename, tag, ref, type)
       int32 file_id;   IN: id of HDF file
       int type;        IN: DFAN_LABEL for labels, DFAN_DESC for descriptions
       int isfirst;     IN: 1: start with first one; 0: get next one
 RETURNS
       length of annotation if successful and FAIL (-1) otherwise
 DESCRIPTION
       Uses isfirst and Next_label_ref (or Next_desc_ref) to determine
       which annotation to pick up next.
 GLOBAL VARIABLES
       Lastref
       Next_label_ref
       Next_desc_ref
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
 *------------------------------------------------------------------------*/
int32
DFANIgetfannlen(int32 file_id, int type, int isfirst)
{
  CONSTR(FUNC, "DFANIgetfannlen");
  uint16      anntag, annref;
  int32       aid;
  int32       length;
  int32       ret_value = SUCCEED;

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFANIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

    /* Identify tag for this "type" of access; determine which ref to key on. */
  if (type == DFAN_LABEL)
    {
      anntag = DFTAG_FID;
      annref = (uint16) ((isfirst == 1) ? DFREF_WILDCARD : Next_label_ref);
    }
  else
    {
      anntag = DFTAG_FD;
      annref = (uint16) ((isfirst == 1) ? DFREF_WILDCARD : Next_desc_ref);
    }
  aid = Hstartread(file_id, anntag, annref);
  if (aid == FAIL)
    HGOTO_ERROR(DFE_BADAID, FAIL);
  if (FAIL == Hinquire(aid, (int32 *) NULL, (uint16 *) NULL, &annref, &length,
                       (int32 *) NULL, (int32 *) NULL, (int16 *) NULL, (int16 *) NULL))
    {
      Hendaccess(aid);
      HGOTO_ERROR(DFE_NOMATCH, FAIL);
    }
  if (type == DFAN_LABEL)     /* prepare for next call */
    Next_label_ref = annref;
  else
    Next_desc_ref = annref;

  Hendaccess(aid);
  Lastref = annref;   /* remember ref last accessed */

  if (length >= 0)    /* (length == 0) => no length found */
    ret_value = length;
  else
    HGOTO_ERROR(DFE_NOMATCH,FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */
  /* Normal function cleanup */

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
       DFANIgetfann -- get next file annotation (file ID or file description)
 USAGE
       intn DFANIgetfann(filename, tag, ref, ann, maxlen, type)
       int32 file_id;    IN: id of HDF file
       uint8 *ann;       OUT: space to return annotation in
       int32 annlen;     IN: size of space to return annotation in
       int type;         IN: DFAN_LABEL for labels, DFAN_DESC for descriptions
       int isfirst;      IN: 1: start with first one; 0: get next one

 RETURNS
       Length of annotation if successful and FAIL (-1) otherwise
 DESCRIPTION
       Gets tag and ref of annotation.  Finds DD for that annotation.
       Reads the annotation, taking care of NULL terminator, if necessary.
 GLOBAL VARIABLES
       Lastref, Next_desc_ref, Next_label_ref
 COMMENTS, BUGS, ASSUMPTIONS
       If maxlen not great enough, ann is truncated to maxlen-1 chars
       BUG: If ref is high possible ref value, setting of Next_label_ref
            or Next_desc_ref behave unpredictably.
 EXAMPLES
 REVISION LOG
 *---------------------------------------------------------------------------*/
int32
DFANIgetfann(int32 file_id, char *ann, int32 maxlen, int type,
             int isfirst)
{
  CONSTR(FUNC, "DFANIgetfann");
  uint16      anntag, annref;
  int32       length, aid;
  int32       ret_value = SUCCEED;

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFANIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (!ann)
    HGOTO_ERROR(DFE_BADPTR,FAIL);

    /* Identify tag for this "type" of access; determine which ref to key on. */
  if (type == DFAN_LABEL)
    {
      anntag = DFTAG_FID;
      annref = (uint16) ((isfirst == 1) ? DFREF_WILDCARD : Next_label_ref);
    }
  else
    {
      anntag = DFTAG_FD;
      annref = (uint16) ((isfirst == 1) ? DFREF_WILDCARD : Next_desc_ref);
    }

  if (( aid = Hstartread(file_id, anntag, annref))== FAIL)
    HGOTO_ERROR(DFE_BADAID, FAIL);
  if (FAIL == Hinquire(aid, (int32 *) NULL, (uint16 *) NULL, &annref, &length,
                   (int32 *) NULL, (int32 *) NULL, (int16 *) NULL, (int16 *) NULL))
    {
      Hendaccess(aid);
      HGOTO_ERROR(DFE_NOMATCH, FAIL);
    }
  length = (length > maxlen) ? maxlen : length;   /* truncate if too long */

  if ((int32) FAIL == Hread(aid, length, (uint8 *) ann))  /* get the annotation */
    {
      Hendaccess(aid);
      HGOTO_ERROR(DFE_READERROR, FAIL);
    }

  if (length > maxlen - 1)
    length = maxlen - 1;

  ann[length] = '\0';

  Lastref = annref;   /* remember ref last accessed */

  /* prepare for next call */
  if (FAIL == Hnextread(aid, anntag, DFREF_WILDCARD, DF_CURRENT))
    {     /* If no more of them, set Next_ ???_ref */
      if (type == DFAN_LABEL)   /*    to one higher than current value   */
        Next_label_ref++;     /*    so that next call will fail.       */
      else
        Next_desc_ref++;
    }
  else
    {     /* Otherwise save the next ref */
      if (FAIL == Hinquire(aid, (int32 *) NULL, (uint16 *) NULL, &annref,
                           (int32 *) NULL, (int32 *) NULL, (int32 *) NULL,
                           (int16 *) NULL, (int16 *) NULL))
        {
          Hendaccess(aid);
          HGOTO_ERROR(DFE_NOMATCH, FAIL);
        }
      if (type == DFAN_LABEL)
        Next_label_ref = annref;
      else
        Next_desc_ref = annref;
    }

  Hendaccess(aid);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */
  else
    ret_value = length;

  /* Normal function cleanup */

  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
    DFANIstart
 PURPOSE
    DFAN-level initialization routine
 USAGE
    intn DFANIstart()
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Register the shut-down routine (DFANPshutdown) for call with atexit
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE intn DFANIstart(void)
{
    CONSTR(FUNC, "DFANIstart");    /* for HERROR */
    intn        ret_value = SUCCEED;

    /* Don't call this routine again... */
    library_terminate = TRUE;

    /* Install atexit() library cleanup routine */
    if (HPregister_term_func(&DFANPshutdown) != 0)
      HGOTO_ERROR(DFE_CANTINIT, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

    return(ret_value);
} /* end DFANIstart() */

/*--------------------------------------------------------------------------
 NAME
    DFANPshutdown
 PURPOSE
    Terminate various static buffers.
 USAGE
    intn DFANPshutdown()
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Free various buffers allocated in the DFAN routines.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Should only ever be called by the "atexit" function HDFend
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn DFANPshutdown(void)
{
    DFANIclear();   /* frees the directory lists */

    if(Lastfile!=NULL)
      {
          HDfree(Lastfile);
          Lastfile=NULL;
      } /* end if */
    return(SUCCEED);
} /* end DFANPshutdown() */

