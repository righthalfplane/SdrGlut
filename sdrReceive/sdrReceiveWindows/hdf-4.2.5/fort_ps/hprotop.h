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

/* $Id: hprotop.h 4999 2007-12-03 20:31:07Z swegner $ */

#ifndef _H_PROTO
#define _H_PROTO

/* Usefull macros, which someday might become actual functions */
/* Wrappers for Hinquire. feb-2-92 */
#define HQueryfileid(aid, fileid)     \
  (Hinquire ((int32)   aid,  (int32*) fileid, (uint16*) NULL,\
             (uint16*) NULL, (int32*) NULL,   (int32*)  NULL,\
             (int32*)  NULL, (int16*) NULL,   (int16*)  NULL))

#define HQuerytagref(aid, tag, ref) \
  (Hinquire ((int32)   aid,  (int32*) NULL,   (uint16*) tag,\
             (uint16*) ref,  (int32*) NULL,   (int32*)  NULL,\
             (int32*)  NULL, (int16*) NULL,   (int16*)  NULL))

#define HQuerylength(aid, length)     \
  (Hinquire ((int32)    aid, (int32*) NULL, (uint16*) NULL,   \
             (uint16*) NULL, (int32*) length,   (int32*)  NULL,   \
             (int32*)  NULL, (int16*) NULL, (int16*)  NULL))

#define HQueryoffset(aid, offset)     \
  (Hinquire ((int32)    aid, (int32*) NULL, (uint16*) NULL,   \
             (uint16*) NULL, (int32*) NULL,     (int32*)  offset, \
             (int32*)  NULL, (int16*) NULL,     (int16*)  NULL))

#define HQueryposition(aid, position) \
  (Hinquire ((int32)    aid, (int32*) NULL, (uint16*) NULL,   \
             (uint16*) NULL, (int32*) NULL, (int32*)  NULL,   \
             (int32*) position, (int16*) NULL,  (int16*)  NULL))

#define HQueryaccess(aid, access)     \
  (Hinquire ((int32)    aid,    (int32*) NULL,   (uint16*) NULL,  \
             (uint16*) NULL,    (int32*) NULL,   (int32*)  NULL,  \
             (int32*)   NULL,   (int16*) access, (int16*)  NULL))

#define HQueryspecial(aid, special) \
  (Hinquire ((int32)    aid,    (int32*) NULL,  (uint16*) NULL,   \
             (uint16*) NULL,    (int32*) NULL,  (int32*)  NULL,   \
             (int32*)   NULL,   (int16*) NULL,  (int16*)  special))


#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */

/*
   ** from hfile.c
 */
    extern int32 Hopen
                (const char *path, intn acc_mode, int16 ndds);

    extern intn Hclose
                (int32 file_id);

    extern int32 Hstartread
                (int32 file_id, uint16 tag, uint16 ref);

    extern intn Hnextread
                (int32 access_id, uint16 tag, uint16 ref, intn origin);

    extern intn Hexist
                (int32 file_id, uint16 search_tag, uint16 search_ref);

    extern intn Hinquire
                (int32 access_id, int32 * pfile_id, uint16 * ptag,
          uint16 * pref, int32 * plength, int32 * poffset,
        int32 * pposn, int16 * paccess, int16 * pspecial);

    extern int32 Hstartwrite
                (int32 file_id, uint16 tag, uint16 ref, int32 length);

    extern int32 Hstartaccess
                (int32 file_id, uint16 tag, uint16 ref, uint32 flags);

    extern intn Hsetlength
                (int32 file_id, int32 length);

    extern intn Happendable
                (int32 aid);

    extern intn HPisappendable
                (int32 aid);

    extern intn HPregister_term_func
                (hdf_termfunc_t term_func);

    extern intn Hseek
                (int32 access_id, int32 offset, intn origin);

    extern int32 Htell
                (int32 access_id);

    extern int32 Hread
                (int32 access_id, int32 length, VOIDP data);

    extern int32 Hwrite
                (int32 access_id, int32 length, const VOIDP data);

    extern int32 Htrunc
                (int32 access_id, int32 trunc_len);

    extern intn Hendaccess
                (int32 access_id);

    extern intn HDgetc
                (int32 access_id);

    extern intn HDputc
                (uint8 c, int32 access_id);

    extern int32 Hgetelement
                (int32 file_id, uint16 tag, uint16 ref, uint8 * data);

    extern int32 Hputelement
                (int32 file_id, uint16 tag, uint16 ref, const uint8 * data, int32 length);

    extern int32 Hlength
                (int32 file_id, uint16 tag, uint16 ref);

    extern int32 Hoffset
                (int32 file_id, uint16 tag, uint16 ref);

    extern intn Hsync
                (int32 file_id);

    extern intn Hcache
                (int32 file_id, intn cache_on);

    extern intn Hgetlibversion
                (uint32 * majorv, uint32 * minorv,
                 uint32 * releasev, char * string);

    extern intn Hgetfileversion
                (int32 file_id, uint32 * majorv, uint32 * minorv,
                 uint32 * release, char * string);

    extern intn Hsetaccesstype(int32 access_id, uintn accesstype);

    extern uint16 HDmake_special_tag
                (uint16 tag);

    extern intn HDis_special_tag
                (uint16 tag);

    extern uint16 HDbase_tag
                (uint16 tag);

    extern int  HDerr
                (int32 file_id);

    extern intn HDvalidfid
                (int32 file_id);

    extern const char *HDgettagdesc
                (uint16 tag);

    extern char *HDgettagsname
                (uint16 tag);

    extern intn HDgettagnum
                (const char *tag_name);

    extern char *HDgetNTdesc
                (int32 nt);

    extern const char *HDfidtoname
                (int32 fid);

    extern intn Hishdf
                (const char * filename);

    extern intn Hfidinquire
                (int32 file_id, char ** fname, intn * acc_mode,
                 intn * attach);
    
    extern intn Hshutdown(void);

    extern void HPend(void);

    extern intn HDdont_atexit(void);

/*
   ** from hfiledd.c
 */
/******************************************************************************
 NAME
     Hdupdd - Duplicate a data descriptor

 DESCRIPTION
    Duplicates a data descriptor so that the new tag/ref points to the
    same data element pointed to by the old tag/ref.  Return FAIL if
    the given tag/ref are already in use.

 RETURNS
    returns SUCCEED (0) if successful, FAIL (-1) otherwise

*******************************************************************************/
intn Hdupdd(int32 file_id,      /* IN: File ID the tag/refs are in */
        uint16 tag,             /* IN: Tag of new tag/ref */
        uint16 ref,             /* IN: Ref of new tag/ref */
        uint16 old_tag,         /* IN: Tag of old tag/ref */
        uint16 old_ref          /* IN: Ref of old tag/ref */
);

/******************************************************************************
 NAME
     Hnumber - Determine the number of objects of a given type

 DESCRIPTION
    Determine how many objects of the given tag are in the file.
    tag may be set to DFTAG_WILDCARD to get back the total number
    of objects in the file.

    Note, a return value of zero is not a fail condition.

 RETURNS
    the number of objects of type 'tag' else FAIL

*******************************************************************************/
int32 Hnumber(int32 file_id,    /* IN: File ID the tag/refs are in */
        uint16 tag              /* IN: Tag to count */
);

/******************************************************************************
 NAME
     Hnewref - Returns a ref that is guaranteed to be unique in the file

 DESCRIPTION
    Returns a ref number that can be used with any tag to produce a
    unique tag/ref.  Successive calls to Hnewref will generate a
    strictly increasing sequence until the highest possible ref had been
    returned, then Hnewref will return unused ref's starting from 1.

 RETURNS
    returns the ref number, 0 otherwise

*******************************************************************************/
uint16 Hnewref(int32 file_id        /* IN: File ID the tag/refs are in */
);

/******************************************************************************
 NAME
    Htagnewref  - returns a ref that is unique in the file for a given tag

 DESCRIPTION
    Returns a ref number that can be used with any tag to produce a
    unique tag/ref.  Successive calls to Hnewref will generate a
    strictly increasing sequence until the highest possible ref had been
    returned, then Hnewref will return unused ref's starting from 1.

 RETURNS
    returns the ref number, 0 otherwise

*******************************************************************************/
uint16 Htagnewref(int32 file_id,    /* IN: File ID the tag/refs are in */
        uint16 tag                  /* IN: Tag to search for a new ref for */
);

/******************************************************************************
 NAME
    Hfind - locate the next object of a search in an HDF file

 DESCRIPTION
    Searches for the `next' DD that fits the search tag/ref.  Wildcards
    apply.  If origin is DF_FORWARD, search from current position forwards
    in the file, otherwise DF_BACKWARD searches backward from the current
    position in the file.  If *find_tag and *find_ref are both set to
    0, this indicates the beginning of a search, and the search will
    start from the beginning of the file if the direction is DF_FORWARD
    and from the and of the file if the direction is DF_BACKWARD.

 RETURNS
    returns SUCCEED (0) if successful and FAIL (-1) otherwise

*******************************************************************************/
intn Hfind(int32 file_id,       /* IN: file ID to search in */
        uint16 search_tag,      /* IN: the tag to search for (can be DFTAG_WILDCARD) */
        uint16 search_ref,      /* IN: ref to search for (can be DFREF_WILDCARD) */
        uint16 *find_tag,       /* IN: if (*find_tag==0) and (*find_ref==0) then start search */
                                /* OUT: tag matching the search tag */
        uint16 *find_ref,       /* IN: if (*find_tag==0) and (*find_ref==0) then start search */
                                /* OUT: ref matching the search ref */
        int32 *find_offset,     /* OUT: offset of the data element found */
        int32 *find_length,     /* OUT: length of the data element found */
        intn direction          /* IN: Direction to search in: */
                                /*  DF_FORWARD searches forward from the current location */
                                /*  DF_BACKWARD searches backward from the current location */
);

/******************************************************************************
 NAME
     Hdeldd - Delete a data descriptor

 DESCRIPTION
    Deletes a data descriptor of tag/ref from the dd list of the file.
    This routine is unsafe and may leave a file in a condition that is
    not usable by some routines.  Use with care.
    For example, if this element is contained in a Vgroup, that group
    will *NOT* get updated to reflect that this element has been deleted.

 RETURNS
    returns SUCCEED (0) if successful, FAIL (-1) otherwise

*******************************************************************************/
intn Hdeldd(int32 file_id,      /* IN: File ID the tag/refs are in */
        uint16 tag,             /* IN: Tag of tag/ref to delete */
        uint16 ref              /* IN: Ref of tag/ref to delete */
);

/*
   ** from hdfalloc.c
 */

    extern VOIDP HDmemfill
                (VOIDP dest, const VOIDP src, uint32 item_size, uint32 num_items);

    extern char *HIstrncpy
                (char * dest, const char * source, int32 len);

    extern int32 HDspaceleft
                (void);

#if defined MALLOC_CHECK
    extern VOIDP HDmalloc
                (uint32 qty);

    extern VOIDP HDrealloc
                (VOIDP where, uint32 qty);

    extern VOIDP HDcalloc
                (uint32 n, uint32 size);

    extern void HDfree
                (VOIDP ptr);

#endif /* defined MALLOC_CHECK */

#if defined VMS || defined macintosh || defined MAC || defined SYMANTEC_C || defined MIPSEL || defined NEXT || defined CONVEX || defined IBM6000 || defined SUN || defined IRIX
    extern char *HDstrdup
                (const char *s);

#endif

    extern intn HDc2fstr
                (char * str, intn len);

    extern char *HDf2cstring
                (_fcd fdesc, intn len);

    extern intn HDflush
                (int32 file_id);

    extern intn HDpackFstring
                (char * src, char * dest, intn len);

/*
   ** from hblocks.c
 */
    extern int32 HLcreate
                (int32 file_id, uint16 tag, uint16 ref, int32 block_length,
                 int32 number_blocks);

    extern intn HLconvert
                (int32 aid, int32 block_length, int32 number_blocks);

    extern int  HDinqblockinfo
                (int32 aid, int32 *length, int32 *first_length, int32 *block_length,
                 int32 *number_blocks);

/*
   ** from hextelt.c
 */
    extern int32 HXcreate
                (int32 file_id, uint16 tag, uint16 ref, const char * extern_file_name,
                 int32 offset, int32 start_len);

    extern intn HXsetcreatedir
                (const char *dir);

    extern intn HXsetdir
                (const char *dir);

/*
   ** from hcomp.c
 */
    extern int32 HCcreate
                (int32 file_id, uint16 tag, uint16 ref,
                 comp_model_t model_type, model_info * m_info,
                 comp_coder_t coder_type, comp_info * c_info);

    extern int32 HCPquery_encode_header(comp_model_t model_type, model_info * m_info,
             comp_coder_t coder_type, comp_info * c_info);

    extern intn HCPencode_header(uint8 *p, comp_model_t model_type, model_info * m_info,
             comp_coder_t coder_type, comp_info * c_info);

    extern intn HCPdecode_header(uint8 *p, comp_model_t *model_type, model_info * m_info,
             comp_coder_t *coder_type, comp_info * c_info);


/*
   ** from hvblocks.c
 */
    extern int32 HVcreate
                (int32 file_id, uint16 tag, uint16 ref);

/*
   ** from herr.c
 */
    extern const char *HEstring
                (hdf_err_code_t error_code);

    extern VOID HEpush
                (hdf_err_code_t error_code, const char * function_name,
                 const char * file_name, intn line);

    extern VOID HEreport
                (const char *,...);

    extern VOID HEprint
                (FILE * stream, int32 print_level);

    extern int16 HEvalue
                (int32 level);

    extern VOID HEPclear
                (void);

 	extern intn HEshutdown(void);

/*
   ** from hbitio.c
 */
    extern int32 Hstartbitread
                (int32 file_id, uint16 tag, uint16 ref);

    extern int32 Hstartbitwrite
                (int32 file_id, uint16 tag, uint16 ref, int32 length);

    extern intn Hbitappendable
                (int32 bitid);

    extern intn Hbitwrite
                (int32 bitid, intn count, uint32 data);

    extern intn Hbitread
                (int32 bitid, intn count, uint32 *data);

    extern intn Hbitseek
                (int32 bitid, int32 byte_offset, intn bit_offset);

    extern intn Hgetbit
                (int32 bitid);

    extern int32 Hendbitaccess
                (int32 bitfile_id, intn flushbit);

    extern intn HPbitshutdown(void);

/*
   ** from dfcomp.c
 */
    extern intn DFputcomp
                (int32 file_id, uint16 tag, uint16 ref, uint8 * image,
        int32 xdim, int32 ydim, uint8 * palette, uint8 * newpal,
                 int16 scheme, comp_info * cinfo);

    extern int  DFgetcomp
                (int32 file_id, uint16 tag, uint16 ref, uint8 * image,
                 int32 xdim, int32 ydim, uint16 scheme);

/*
   ** from dfrle.c
 */
    extern int32 DFCIrle
                (VOIDP buf, VOIDP bufto, int32 len);

    extern int32 DFCIunrle
                (uint8 * buf, uint8 *bufto, int32 outlen, int resetsave);

/*
   ** from dfimcomp.c
 */
    extern VOID DFCIimcomp
                (int32 xdim, int32 ydim, uint8 in[], uint8 out[],
                 uint8 in_pal[], uint8 out_pal[], int mode);

    extern VOID DFCIunimcomp
                (int32 xdim, int32 ydim, uint8 in[], uint8 out[]);

/*
   ** from dfjpeg.c
 */

    extern intn DFCIjpeg
                (int32 file_id, uint16 tag, uint16 ref, int32 xdim, int32 ydim,
                 VOIDP image, int16 scheme, comp_info * scheme_info);

/*
   ** from dfunjpeg.c
 */

    extern intn DFCIunjpeg
                (int32 file_id, uint16 tag, uint16 ref, VOIDP image, int32 xdim,
                 int32 ydim, int16 scheme);

/*
   ** from dfgroup.c
 */
    extern int32 DFdiread
                (int32 file_id, uint16 tag, uint16 ref);

    extern intn DFdiget
                (int32 list, uint16 * ptag, uint16 * pref);

    extern intn DFdinobj
                (int32 list);

    extern int32 DFdisetup
                (int maxsize);

    extern intn DFdiput
                (int32 list, uint16 tag, uint16 ref);

    extern intn DFdiwrite
                (int32 file_id, int32 list, uint16 tag, uint16 ref);

/*
   ** from dfp.c
 */
    extern intn DFPgetpal
                (const char * filename, VOIDP palette);

    extern intn DFPputpal
                (const char * filename, const VOIDP palette, intn overwrite, const char * filemode);

    extern intn DFPaddpal
                (const char * filename, const VOIDP palette);

    extern intn DFPnpals
                (const char * filename);

    extern intn DFPreadref
                (const char * filename, uint16 ref);

    extern intn DFPwriteref
                (const char * filename, uint16 ref);

    extern intn DFPrestart
                (void);

    extern uint16 DFPlastref
                (void);

/*
   ** from dfr8.c
 */
    extern int  DFR8setcompress
                (int32 scheme, comp_info * cinfo);

    extern intn DFR8getdims
                (const char * filename, int32 * pxdim, int32 * pydim,
                 int * pispal);

    extern intn DFR8getimage
                (const char * filename, uint8 * image, int32 xdim, int32 ydim,
                 uint8 * pal);

    extern intn DFR8setpalette
                (uint8 * pal);

    extern intn DFR8putimage
                (const char * filename, VOIDP image, int32 xdim, int32 ydim, uint16 compress);

    extern intn DFR8addimage
                (const char * filename, VOIDP image, int32 xdim, int32 ydim, uint16 compress);

    extern intn DFR8nimages
                (const char * filename);

    extern intn DFR8readref
                (const char * filename, uint16 ref);

    extern intn DFR8writeref
                (const char * filename, uint16 ref);

    extern intn DFR8restart
                (void);

    extern uint16 DFR8lastref
                (void);

    extern intn DFR8getpalref(uint16 *pal_ref);

    extern intn DFR8Pshutdown(void);

/*
   ** from dfgr.c
 */
    extern intn DFGRgetlutdims
                (const char * filename, int32 * pxdim, int32 * pydim,
                 intn * pncomps, intn * pil);

    extern intn DFGRreqlutil
                (intn il);

    extern intn DFGRgetlut
                (const char * filename, VOIDP lut, int32 xdim, int32 ydim);

    extern intn DFGRgetimdims
                (const char * filename, int32 * pxdim, int32 * pydim,
                 intn * pncomps, intn * pil);

    extern intn DFGRreqimil
                (intn il);

    extern intn DFGRgetimage
                (const char * filename, VOIDP image, int32 xdim, int32 ydim);

    extern intn DFGRsetcompress
                (int32 scheme, comp_info * cinfo);

    extern intn DFGRsetlutdims
                (int32 xdim, int32 ydim, intn ncomps, intn il);

    extern intn DFGRsetlut
                (VOIDP lut, int32 xdim, int32 ydim);

    extern intn DFGRaddlut
                (const char * filename, VOIDP lut, int32 xdim, int32 ydim);

    extern intn DFGRsetimdims
                (int32 xdim, int32 ydim, intn ncomps, intn il);

    extern intn DFGRaddimage
                (const char * filename, VOIDP image, int32 xdim, int32 ydim);

    extern intn DFGRputimage
                (const char * filename, VOIDP image, int32 xdim, int32 ydim);

    extern intn DFGRreadref
                (const char * filename, uint16 ref);

    extern uint16 DFGRIlastref
                (void);

    extern intn DFGRIgetdims
                (const char * filename, int32 * pxdim, int32 * pydim,
                 intn * pncomps, intn * pil, intn type);

    extern intn DFGRIreqil
                (intn il, intn type);

    extern intn DFGRIgetimlut
                (const char * filename, VOIDP imlut, int32 xdim, int32 ydim, intn type,
                 intn isfortran, int *compressed, uint16 *compr_type, int *has_pal);

    extern intn DFGRIsetdims
                (int32 xdim, int32 ydim, intn ncomps, intn type);

    extern intn DFGRIsetil
                (intn il, intn type);

    extern intn DFGRIrestart
                (void);

    extern intn DFGRIaddimlut
                (const char * filename, VOIDP imlut, int32 xdim, int32 ydim, intn type,
                 intn isfortran, intn newfile);

    extern intn DFGRPshutdown(void);

/*
   ** from df24.c
 */
    extern intn DF24getdims
                (const char * filename, int32 * pxdim, int32 * pydim,
                 intn * pil);

    extern intn DF24reqil
                (intn il);

    extern intn DF24getimage
                (const char * filename, VOIDP image, int32 xdim, int32 ydim);

    extern intn DF24setdims
                (int32 xdim, int32 ydim);

    extern intn DF24setil
                (intn il);

    extern intn DF24setcompress
                (int32 type, comp_info * cinfo);

    extern intn DF24restart
                (void);

    extern intn DF24addimage
                (const char * filename, VOIDP image, int32 xdim, int32 ydim);

    extern intn DF24putimage
                (const char * filename, VOIDP image, int32 xdim, int32 ydim);

    extern intn DF24nimages
                (const char * filename);

    extern intn DF24readref
                (const char * filename, uint16 ref);

    extern uint16 DF24lastref
                (void);

/*
   ** from dfan.c
 */

    extern int32 DFANgetlablen
                (const char * filename, uint16 tag, uint16 ref);

    extern intn DFANgetlabel
                (const char * filename, uint16 tag, uint16 ref, char * label,
                 int32 maxlen);

    extern int32 DFANgetdesclen
                (const char * filename, uint16 tag, uint16 ref);

    extern intn DFANgetdesc
                (const char * filename, uint16 tag, uint16 ref, char * desc,
                 int32 maxlen);

    extern int32 DFANgetfidlen
                (int32 file_id, intn isfirst);

    extern int32 DFANgetfid
                (int32 file_id, char * id, int32 maxlen, intn isfirst);

    extern int32 DFANgetfdslen
                (int32 file_id, intn isfirst);

    extern int32 DFANgetfds
                (int32 file_id, char * desc, int32 maxlen, intn isfirst);

    extern intn DFANputlabel
                (const char * filename, uint16 tag, uint16 ref, char * label);

    extern intn DFANputdesc
                (const char * filename, uint16 tag, uint16 ref, char * desc,
                 int32 desclen);

    extern intn DFANaddfid
                (int32 file_id, char * id);

    extern intn DFANaddfds
                (int32 file_id, char * desc, int32 desclen);

    extern uint16 DFANlastref
                (void);

    extern intn DFANlablist
                (const char * filename, uint16 tag, uint16 reflist[],
         char * labellist, intn listsize, intn maxlen, intn startpos);

    extern intn DFANclear
                (void);

    extern intn DFANIclear
                (void);

    extern uint16 DFANIlocate
                (int32 file_id, int type, uint16 tag, uint16 ref);

    extern int  DFANIaddentry
                (int type, uint16 annref, uint16 datatag, uint16 dataref);

    extern int32 DFANIgetannlen
                (const char * filename, uint16 tag, uint16 ref, int type);

    extern intn DFANIgetann
                (const char * filename, uint16 tag, uint16 ref, uint8 * ann,
                 int32 maxlen, int type);

    extern intn DFANIputann
                (const char * filename, uint16 tag, uint16 ref, uint8 * ann,
                 int32 annlen, int type);

    extern int  DFANIlablist
                (const char * filename, uint16 tag, uint16 reflist[],
            uint8 * labellist, int listsize, int maxlen, int startpos,
                 int isfortran);

    extern int  DFANIaddfann
                (int32 file_id, char * ann, int32 annlen, int type);

    extern int32 DFANIgetfannlen
                (int32 file_id, int type, int isfirst);

    extern int32 DFANIgetfann
                (int32 file_id, char * ann, int32 maxlen, int type, int isfirst);

    extern intn DFANPshutdown(void);

/*
   ** from dfsd.c
 */

    extern int  DFSDgetdims
                (const char * filename, intn * prank, int32 sizes[], intn maxrank);

    extern int  DFSDgetdatastrs
                (char * label, char * unit, char * format, char * coordsys);

    extern int  DFSDgetdimstrs
                (int dim, char * label, char * unit, char * format);

    extern int  DFSDgetdatalen
                (int * llabel, int * lunit, int * lformat, int * lcoordsys);

    extern int  DFSDgetdimlen
                (int dim, int * llabel, int * lunit, int * lformat);

    extern int  DFSDgetdimscale
                (intn dim, int32 maxsize, VOIDP scale);

    extern int  DFSDgetrange
                (VOIDP pmax, VOIDP pmin);

    extern int  DFSDgetdata
                (const char * filename, intn rank, int32 maxsizes[], VOIDP data);

    extern int  DFSDsetlengths
                (int maxlen_label, int maxlen_unit, int maxlen_format,
                 int maxlen_coordsys);

    extern int  DFSDsetdims
                (intn rank, int32 dimsizes[]);

    extern int  DFSDsetdatastrs
                (const char * label, const char * unit, const char * format, const char * coordsys);

    extern int  DFSDsetdimstrs
                (int dim, const char * label, const char * unit, const char * format);

    extern int  DFSDsetdimscale
                (intn dim, int32 dimsize, VOIDP scale);

    extern int  DFSDsetrange
                (VOIDP maxi, VOIDP mini);

    extern int  DFSDputdata
                (const char * filename, intn rank, int32 dimsizes[], VOIDP data);

    extern int  DFSDadddata
                (const char * filename, intn rank, int32 dimsizes[], VOIDP data);

    extern int  DFSDrestart
                (void);

    extern int32 DFSDndatasets
                (char * filename);

    extern int  DFSDclear
                (void);

    extern uint16 DFSDlastref
                (void);

    extern int  DFSDreadref
                (char * filename, uint16 ref);

    extern int  DFSDgetslice
                (const char * filename, int32 winst[], int32 windims[], VOIDP data,
                 int32 dims[]);

    extern int  DFSDstartslice
                (const char * filename);

    extern int  DFSDputslice
                (int32 winend[], VOIDP data, int32 dims[]);

    extern int  DFSDendslice
                (void);

    extern int  DFSDsetNT
                (int32 numbertype);

    extern int  DFSDsetorder
                (int arrayorder);

    extern int  DFSDgetNT
                (int32 * pnumbertype);

    extern intn DFSDpre32sdg
                (char * filename, uint16 ref, intn * ispre32);

    extern int  DFSDsetcal
                (float64 cal, float64 cal_err, float64 ioff,
                 float64 ioff_err, int32 cal_nt);

    extern int  DFSDgetcal
                (float64 * pcal, float64 * pcal_err, float64 * pioff,
                 float64 * pioff_err, int32 * cal_nt);

    extern int  DFSDwriteref
                (const char * filename, uint16 ref);

    extern int  DFSDsetfillvalue
                (VOIDP fill_value);

    extern int  DFSDgetfillvalue
                (VOIDP fill_value);

    extern int  DFSDstartslab
                (const char * filename);

    extern int  DFSDwriteslab
                (int32 start[], int32 stride[], int32 count[],
                 VOIDP data);

    extern int  DFSDendslab
                (void);

    extern int  DFSDreadslab
                (const char *filename, int32 start[], int32 slab_size[],
             int32 stride[], VOIDP buffer, int32 buffer_size[]);

    extern intn DFSDPshutdown(void);

/*
   ** from dfconv.c
 */

    extern int  DFKNTsize
                (int32 number_type);

    extern int32 DFKisnativeNT
                (int32 numbertype);

    extern int32 DFKislitendNT
                (int32 numbertype);

    extern int8 DFKgetPNSC
                (int32 numbertype, int32 machinetype);

    extern intn DFKsetNT
                (int32 ntype);

    extern int32 DFKconvert
                (VOIDP source, VOIDP dest, int32 ntype, int32 num_elm,
                 int16 acc_mode, int32 source_stride, int32 dest_stride);

/*
   ** from dfknat.c
 */

    extern intn DFKnb1b
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKnb2b
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKnb4b
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKnb8b
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

/*
   ** from dfkswap.c
 */

    extern intn DFKsb2b
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKsb4b
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKsb8b
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

/*
   ** from dfkcray.c
 */

    extern intn DFKui2i
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKui2s
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKuo2i
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKuo2s
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKui4i
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKui4s
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKuo4i
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKuo4s
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKui4f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKuo4f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKui8f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKuo8f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlui2i
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlui2s
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKluo2i
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKluo2s
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlui4i
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlui4s
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKluo4i
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKluo4s
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlui4f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKluo4f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlui8f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKluo8f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

/* CRAY-MPP */
    extern intn DFKmi2i
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKmi2s
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKmo2b
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlmi2i
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlmi2s
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlmo2b
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);


/*
   ** from dfkvms.c
 */

    extern intn DFKvi4f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKvo4f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKvi8f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKvo8f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlvi4f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlvo4f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlvi8f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlvo8f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

/*
   ** from dfkconv.c
 */

    extern intn DFKci4f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKco4f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKci8f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKco8f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlci4f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlco4f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlci8f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlco8f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

/*
   ** from dfkfuji.c
 */

    extern intn DFKpi4f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKpo4f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKpi8f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKpo8f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlpi4f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlpo4f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlpi8f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

    extern intn DFKlpo8f
                (VOIDP s, VOIDP d, uint32 num_elm, uint32 source_stride, uint32 dest_stride);

/*
   ** from dfanF.c
 */
#ifndef DFAN_FNAMES
#   define  DFAN_FNAMES
#ifdef DF_CAPFNAMES
#   define ndaiganl  FNAME(DAIGANL)
#   define ndaigann  FNAME(DAIGANN)
#   define ndaipann  FNAME(DAIPANN)
#   define ndailist  FNAME(DAILIST)
#   define ndalref   FNAME(DALREF)
#   define ndaclear  FNAME(DACLEAR)
#   define ndfanlastref     FNAME(DFANLASTREF)

#   define ndfanaddfds      FNAME(DFANADDFDS)
#   define ndfangetfidlen   FNAME(DFANGETFIDLEN)
#   define ndfangetfdslen   FNAME(DFANGETFDSLEN)
#   define ndfangetfid      FNAME(DFANGETFID)
#   define ndfangetfds      FNAME(DFANGETFDS)
#   define ndaafds          FNAME(DAAFDS)
#   define ndagfidl         FNAME(DAGFIDL)
#   define ndagfdsl         FNAME(DAGFDSL)
#   define ndagfid          FNAME(DAGFID)
#   define ndagfds          FNAME(DAGFDS)
#   define ndaiafid         FNAME(DAIAFID)
#else                           /* DF_CAPFNAMES */
#   define ndaiganl  FNAME(daiganl)
#   define ndaigann  FNAME(daigann)
#   define ndaipann  FNAME(daipann)
#   define ndailist  FNAME(dailist)
#   define ndalref   FNAME(dalref)
#   define ndaclear  FNAME(daclear)
#   define ndfanlastref     FNAME(dfanlastref)

#   define ndfanaddfds      FNAME(dfanaddfds)
#   define ndfangetfidlen   FNAME(dfangetfidlen)
#   define ndfangetfdslen   FNAME(dfangetfdslen)
#   define ndfangetfid      FNAME(dfangetfid)
#   define ndfangetfds      FNAME(dfangetfds)
#   define ndaafds          FNAME(daafds)
#   define ndagfidl         FNAME(dagfidl)
#   define ndagfdsl         FNAME(dagfdsl)
#   define ndagfid          FNAME(dagfid)
#   define ndagfds          FNAME(dagfds)
#   define ndaiafid         FNAME(daiafid)
#endif                          /* DF_CAPFNAMES */
#endif                          /* DFAN_FNAMES */

    extern      FRETVAL(intf) ndaiganl
                (_fcd filename, intf * tag, intf * ref, intf * type,
                 intf * fnlen);

    extern      FRETVAL(intf) ndaigann
                (_fcd filename, intf * tag, intf * ref, _fcd annotation,
                 intf * maxlen, intf * type, intf * fnlen);

    extern      FRETVAL(intf) ndaipann
                (_fcd filename, intf * tag, intf * ref, _fcd annotation,
                 intf * annlen, intf * type, intf * fnlen);

    extern      FRETVAL(intf) ndailist
                (_fcd filename, intf * tag, intf reflist[], _fcd labellist,
          intf * listsize, intf * maxlen, intf * startpos,
                 intf * fnlen);

    extern      FRETVAL(intf) ndalref
                (void);

    extern      FRETVAL(intf) ndaclear
                (void);

    extern      FRETVAL(intf) ndfanlastref
                (void);

    extern      FRETVAL(intf) ndfanaddfds
                (intf * dfile, _fcd desc, intf * desclen);

    extern      FRETVAL(intf) ndfangetfidlen
                (intf * dfile, intf * isfirst);

    extern      FRETVAL(intf) ndfangetfdslen
                (intf * dfile, intf * isfirst);

    extern      FRETVAL(intf) ndfangetfid
                (intf * dfile, _fcd id, intf * maxlen, intf * isfirst);

    extern      FRETVAL(intf) ndfangetfds
                (intf * dfile, _fcd id, intf * maxlen, intf * isfirst);

    extern      FRETVAL(intf) ndaafds
                (intf * dfile, _fcd desc, intf * desclen);

    extern      FRETVAL(intf) ndagfidl
                (intf * dfile, intf * isfirst);

    extern      FRETVAL(intf) ndagfdsl
                (intf * dfile, intf * isfirst);

    extern      FRETVAL(intf) ndagfid
                (intf * dfile, _fcd id, intf * maxlen, intf * isfirst);

    extern      FRETVAL(intf) ndagfds
                (intf * dfile, _fcd id, intf * maxlen, intf * isfirst);

    extern      FRETVAL(intf) ndaiafid
                (intf * dfile, _fcd id, intf * idlen);

/*
   ** from dfr8F.c
 */
#ifndef DFR8_FNAMES
#   define DFR8_FNAMES
#ifdef DF_CAPFNAMES
#   define nd8spal   FNAME(D8SPAL)
#   define nd8first  FNAME(D8FIRST)
#   define nd8igdim  FNAME(D8IGDIM)
#   define nd8igimg  FNAME(D8IGIMG)
#   define nd8ipimg  FNAME(D8IPIMG)
#   define nd8iaimg  FNAME(D8IAIMG)
#   define nd8irref  FNAME(D8IRREF)
#   define nd8iwref  FNAME(D8IWREF)
#   define nd8inims  FNAME(D8INIMS)
#   define nd8lref   FNAME(D8LREF)
#   define ndfr8lastref      FNAME(DFR8LASTREF)
#   define ndfr8setpalette   FNAME(DFR8SETPALETTE)
#   define ndfr8restart  FNAME(DFR8RESTART)
#   define nd8scomp  FNAME(D8SCOMP)
#   define ndfr8scompress FNAME(DFR8SCOMPRESS)
#   define nd8sjpeg  FNAME(D8SJPEG)
#   define ndfr8sjpeg FNAME(DFR8SJPEG)
#else                           /* !DF_CAPFNAMES */
#   define nd8spal   FNAME(d8spal)
#   define nd8first  FNAME(d8first)
#   define nd8igdim  FNAME(d8igdim)
#   define nd8igimg  FNAME(d8igimg)
#   define nd8ipimg  FNAME(d8ipimg)
#   define nd8iaimg  FNAME(d8iaimg)
#   define nd8irref  FNAME(d8irref)
#   define nd8iwref  FNAME(d8iwref)
#   define nd8inims  FNAME(d8inims)
#   define nd8lref   FNAME(d8lref)
#   define ndfr8lastref      FNAME(dfr8lastref)
#   define ndfr8setpalette   FNAME(dfr8setpalette)
#   define ndfr8restart  FNAME(dfr8restart)
#   define nd8scomp  FNAME(d8scomp)
#   define ndfr8scompress FNAME(dfr8scompress)
#   define nd8sjpeg  FNAME(d8sjpeg)
#   define ndfr8sjpeg FNAME(dfr8sjpeg)
#endif                          /* DF_CAPFNAMES */
#endif                          /* DFR8_FNAMES */

    extern      FRETVAL(intf) nd8spal
                (_fcd pal);

    extern      FRETVAL(intf) nd8first
                (void);

    extern      FRETVAL(intf) nd8igdim
                (_fcd filename, intf * xdim, intf * ydim, intf * ispal,
                 intf * lenfn);

    extern      FRETVAL(intf) nd8igimg
                (_fcd filename, _fcd image, intf * xdim, intf * ydim,
                 _fcd pal, intf * lenfn);

    extern      FRETVAL(intf) nd8ipimg
                (_fcd filename, _fcd image, intf * xdim, intf * ydim,
                 intf * compress, intf * lenfn);

    extern      FRETVAL(intf) nd8iaimg
                (_fcd filename, _fcd image, intf * xdim, intf * ydim,
                 intf * compress, intf * lenfn);

    extern      FRETVAL(intf) nd8irref
                (_fcd filename, intf * ref, intf * fnlen);

    extern      FRETVAL(intf) nd8iwref
                (_fcd filename, intf * ref, intf * fnlen);

    extern      FRETVAL(intf) nd8inims
                (_fcd filename, intf * fnlen);

    extern      FRETVAL(intf) nd8lref
                (void);

    extern      FRETVAL(intf) ndfr8lastref
                (void);

    extern      FRETVAL(intf) ndfr8setpalette
                (_fcd pal);

    extern      FRETVAL(intf) ndfr8restart
                (void);

    extern      FRETVAL(intf) nd8scomp
                (intf * scheme);

    extern      FRETVAL(intf) ndfr8scompress
                (intf * scheme);

    extern      FRETVAL(intf) nd8sjpeg
                (intf * quality, intf * force_baseline);

    extern      FRETVAL(intf) ndfr8sjpeg
                (intf * quality, intf * force_baseline);

/*
   ** from dfsdF.c
 */
#ifndef DFSD_FNAMES
#   define DFSD_FNAMES
#ifdef DF_CAPFNAMES
#   define ndsgdast  FNAME(DSGDAST)
#   define ndsgdisc  FNAME(DSGDISC)
#   define ndsgrang  FNAME(DSGRANG)
#   define ndssdims  FNAME(DSSDIMS)
#   define ndssdisc  FNAME(DSSDISC)
#   define ndssrang  FNAME(DSSRANG)
#   define ndsclear  FNAME(DSCLEAR)
#   define ndsslens  FNAME(DSSLENS)
#   define ndsgdiln  FNAME(DSGDILN)
#   define ndsgdaln  FNAME(DSGDALN)
#   define ndsfirst  FNAME(DSFIRST)
#   define ndspslc   FNAME(DSPSLC)
#   define ndseslc   FNAME(DSESLC)
#   define ndsgnt    FNAME(DSGNT)
#   define ndssnt    FNAME(DSSNT)
#   define ndsigdim  FNAME(DSIGDIM)
#   define ndsigdat  FNAME(DSIGDAT)
#   define ndsipdat  FNAME(DSIPDAT)
#   define ndsiadat  FNAME(DSIADAT)
#   define ndsigdas  FNAME(DSIGDAS)
#   define ndsigslc  FNAME(DSIGSLC)
#   define ndsigdis  FNAME(DSIGDIS)
#   define ndsisslc  FNAME(DSISSLC)
#   define ndsisdas  FNAME(DSISDAS)
#   define ndsisdis  FNAME(DSISDIS)
#   define ndsirref  FNAME(DSIRREF)
#   define ndslref   FNAME(DSLREF)
#   define ndsinum   FNAME(DSINUM)
#   define ndsip32s  FNAME(DSIP32S)
#   define ndsscal   FNAME(DSSCAL)
#   define ndsgcal   FNAME(DSGCAL)
#   define ndfsdgetdatastrs  FNAME(DFSDGETDATASTRS)
#   define ndfsdgetdimscale  FNAME(DFSDGETDIMSCALE)
#   define ndfsdgetrange     FNAME(DFSDGETRANGE)
#   define ndfsdsetdims      FNAME(DFSDSETDIMS)
#   define ndfsdsetdimscale  FNAME(DFSDSETDIMSCALE)
#   define ndfsdsetrange     FNAME(DFSDSETRANGE)
#   define ndfsdclear        FNAME(DFSDCLEAR)
#   define ndfsdsetlengths   FNAME(DFSDSETLENGTHS)
#   define ndfsdgetdimlen    FNAME(DFSDGETDIMLEN)
#   define ndfsdgetdatalen   FNAME(DFSDGETDATALEN)
#   define ndfsdrestart      FNAME(DFSDRESTART)
#   define ndfsdputslice     FNAME(DFSDPUTSLICE)
#   define ndfsdendslice     FNAME(DFSDENDSLICE)
#   define ndfsdsetnt        FNAME(DFSDSETNT)
#   define ndfsdgetnt        FNAME(DFSDGETNT)
#   define ndfsdlastref      FNAME(DFSDLASTREF)
#   define ndsiwref          FNAME(DSIWREF)
#   define ndssfill          FNAME(DSSFILL)
#   define ndsgfill          FNAME(DSGFILL)
#   define ndsisslab         FNAME(DSISSLAB)
#   define ndswslab          FNAME(DSWSLAB)
#   define ndseslab          FNAME(DSESLAB)
#   define ndsirslab         FNAME(DSIRSLAB)
#else
#   define ndsgdast  FNAME(dsgdast)
#   define ndsgdisc  FNAME(dsgdisc)
#   define ndsgrang  FNAME(dsgrang)
#   define ndssdims  FNAME(dssdims)
#   define ndssdisc  FNAME(dssdisc)
#   define ndssrang  FNAME(dssrang)
#   define ndsclear  FNAME(dsclear)
#   define ndsslens  FNAME(dsslens)
#   define ndsgdiln  FNAME(dsgdiln)
#   define ndsgdaln  FNAME(dsgdaln)
#   define ndsfirst  FNAME(dsfirst)
#   define ndspslc   FNAME(dspslc)
#   define ndseslc   FNAME(dseslc)
#   define ndsgnt    FNAME(dsgnt)
#   define ndssnt    FNAME(dssnt)
#   define ndsigdim  FNAME(dsigdim)
#   define ndsigdat  FNAME(dsigdat)
#   define ndsipdat  FNAME(dsipdat)
#   define ndsiadat  FNAME(dsiadat)
#   define ndsigdas  FNAME(dsigdas)
#   define ndsigslc  FNAME(dsigslc)
#   define ndsigdis  FNAME(dsigdis)
#   define ndsisslc  FNAME(dsisslc)
#   define ndsisdas  FNAME(dsisdas)
#   define ndsisdis  FNAME(dsisdis)
#   define ndsirref  FNAME(dsirref)
#   define ndslref   FNAME(dslref)
#   define ndsinum   FNAME(dsinum)
#   define ndsip32s  FNAME(dsip32s)
#   define ndsscal   FNAME(dsscal)
#   define ndsgcal   FNAME(dsgcal)
#   define ndfsdgetdatastrs  FNAME(dfsdgetdatastrs)
#   define ndfsdgetdimscale  FNAME(dfsdgetdimscale)
#   define ndfsdgetrange     FNAME(dfsdgetrange)
#   define ndfsdsetdims      FNAME(dfsdsetdims)
#   define ndfsdsetdimscale  FNAME(dfsdsetdimscale)
#   define ndfsdsetrange     FNAME(dfsdsetrange)
#   define ndfsdclear        FNAME(dfsdclear)
#   define ndfsdsetlengths   FNAME(dfsdsetlengths)
#   define ndfsdgetdimlen    FNAME(dfsdgetdimlen)
#   define ndfsdgetdatalen   FNAME(dfsdgetdatalen)
#   define ndfsdrestart      FNAME(dfsdrestart)
#   define ndfsdputslice     FNAME(dfsdputslice)
#   define ndfsdendslice     FNAME(dfsdendslice)
#   define ndfsdsetnt        FNAME(dfsdsetnt)
#   define ndfsdgetnt        FNAME(dfsdgetnt)
#   define ndfsdlastref      FNAME(dfsdlastref)
#   define ndsiwref          FNAME(dsiwref)
#   define ndssfill          FNAME(dssfill)
#   define ndsgfill          FNAME(dsgfill)
#   define ndsisslab         FNAME(dsisslab)
#   define ndswslab          FNAME(dswslab)
#   define ndseslab          FNAME(dseslab)
#   define ndsirslab         FNAME(dsirslab)
#endif                          /* DF_CAPFNAMES */
#endif                          /* DFSD_FNAMES */

    extern      FRETVAL(intf) ndsgdisc
                (intf * dim, intf * maxsize, VOIDP scale);

    extern      FRETVAL(intf) ndsgrang
                (VOIDP pmax, VOIDP pmin);

    extern      FRETVAL(intf) ndssdims
                (intf * rank, intf dimsizes[]);

    extern      FRETVAL(intf) ndssdisc
                (intf * dim, intf * dimsize, VOIDP scale);

    extern      FRETVAL(intf) ndssrang
                (VOIDP max, VOIDP min);

    extern      FRETVAL(intf) ndsclear
                (void);

    extern      FRETVAL(intf) ndsslens
                (intf * maxlen_label, intf * maxlen_unit,
                 intf * maxlen_format, intf * maxlen_coordsys);

    extern      FRETVAL(intf) ndsgdiln
                (intf * dim, intf * llabel, intf * lunit,
                 intf * lformat);

    extern      FRETVAL(intf) ndsgdaln
                (intf * llabel, intf * lunit, intf * lformat,
                 intf * lcoordsys);

    extern      FRETVAL(intf) ndsfirst
                (void);

    extern      FRETVAL(intf) ndspslc
                (intf windims[], VOIDP data, intf dims[]);

    extern      FRETVAL(intf) ndseslc
                (void);

    extern      FRETVAL(intf) ndssnt
                (intf * numbertype);

    extern      FRETVAL(intf) ndsgnt
                (intf * pnumbertype);

    extern      FRETVAL(intf) ndsigdim
                (_fcd filename, intf * prank, intf sizes[],
                 intf * maxrank, intf * lenfn);

    extern      FRETVAL(intf) ndsigdat
                (_fcd filename, intf * rank, intf maxsizes[],
                 VOIDP data, intf * fnlen);

    extern      FRETVAL(intf) ndsipdat
                (_fcd filename, intf * rank, intf dimsizes[],
                 VOIDP data, intf * fnlen);

    extern      FRETVAL(intf) ndsiadat
                (_fcd filename, intf * rank, intf dimsizes[],
                 VOIDP data, intf * fnlen);

    extern      FRETVAL(intf) ndsigslc
                (_fcd filename, intf winst[], intf windims[],
                 VOIDP data, intf dims[], intf * fnlen);

    extern      FRETVAL(intf) ndsisslc
                (_fcd filename, intf * fnlen);

    extern      FRETVAL(intf) ndsirref
                (_fcd filename, intf * ref, intf * fnlen);

    extern      FRETVAL(intf) ndslref
                (void);

    extern      FRETVAL(intf) ndsinum
                (_fcd filename, intf * len);

    extern      FRETVAL(intf) ndsip32s
                (_fcd filename, intf * ref, intf * ispre32, intf * len);

    extern      FRETVAL(intf) ndfsdgetdatastrs
                (_fcd label, _fcd unit, _fcd format, _fcd coordsys);

    extern      FRETVAL(intf) ndfsdgetdimstrs
                (intf * dim, _fcd label, _fcd unit, _fcd format);

    extern      FRETVAL(intf) ndfsdgetdimscale
                (intf * dim, intf * maxsize, VOIDP scale);

    extern      FRETVAL(intf) ndfsdgetrange
                (VOIDP pmax, VOIDP pmin);

    extern      FRETVAL(intf) ndfsdsetdims
                (intf * rank, intf dimsizes[]);

    extern      FRETVAL(intf) ndfsdsetdimscale
                (intf * dim, intf * dimsize, VOIDP scale);

    extern      FRETVAL(intf) ndfsdsetrange
                (VOIDP max, VOIDP min);

    extern      FRETVAL(intf) ndfsdclear
                (void);

    extern      FRETVAL(intf) ndfsdsetlengths
                (intf * maxlen_label, intf * maxlen_unit,
                 intf * maxlen_format, intf * maxlen_coordsys);

    extern      FRETVAL(intf) ndfsdgetdimlen
                (intf * dim, intf * llabel, intf * lunit,
                 intf * lformat);

    extern      FRETVAL(intf) ndfsdgetdatalen
                (intf * llabel, intf * lunit, intf * lformat,
                 intf * lcoordsys);

    extern      FRETVAL(intf) ndfsdrestart
                (void);

    extern      FRETVAL(intf) ndfsdputslice
                (intf windims[], VOIDP data, intf dims[]);

    extern      FRETVAL(intf) ndfsdendslice
                (void);

    extern      FRETVAL(intf) ndfsdsetnt
                (intf * numbertype);

    extern      FRETVAL(intf) ndfsdgetnt
                (intf * pnumbertype);

    extern      FRETVAL(intf) ndfsdlastref
                (void);

    extern      FRETVAL(intf) ndsisdis
                (intf * dim, _fcd flabel, _fcd funit, _fcd fformat,
             intf * llabel, intf * lunit, intf * lformat);

    extern      FRETVAL(intf) ndsigdis
                (intf * dim, _fcd label, _fcd unit, _fcd format,
             intf * llabel, intf * lunit, intf * lformat);

    extern      FRETVAL(intf) ndsisdas
                (_fcd flabel, _fcd funit, _fcd fformat, _fcd fcoordsys,
            intf * isfortran, intf * llabel, intf * lunit,
                 intf * lformat, intf * lcoordsys);

    extern      FRETVAL(intf) ndsigdas
                (_fcd label, _fcd unit, _fcd format, _fcd coordsys, intf * llabel,
             intf * lunit, intf * lformat, intf * lcoord);

    extern      FRETVAL(intf) ndsscal
                (float64 * cal, float64 * cal_err, float64 * ioff,
                 float64 * ioff_err, intf * cal_type);

    extern      FRETVAL(intf) ndsgcal
                (float64 * cal, float64 * cal_err, float64 * ioff,
                 float64 * ioff_err, intf * cal_type);

    extern      FRETVAL(intf) ndswref
                (_fcd filename, intf * fnlen, intf * ref);

    extern      FRETVAL(intf) ndssfill
                (VOIDP fill_value);

    extern      FRETVAL(intf) ndsgfill
                (VOIDP fill_value);

    extern      FRETVAL(intf) ndssslab
                (_fcd filename, intf * fnlen);

    extern      FRETVAL(intf) ndswslab
                (intf start[], intf  stride[],
                 intf  cont[], VOIDP data);

    extern      FRETVAL(intf) ndseslab
                (void);

    extern	FRETVAL(intf) ndsiwref
		(_fcd filename, intf * fnlen, intf * ref);

    extern	FRETVAL(intf) ndsisslab
		(_fcd filename, intf * fnlen);

    extern	FRETVAL(intf) ndsirslab
		(_fcd filename, intf * fnlen, intf start[], intf slab_size[],
	         intf stride[], VOIDP buffer, intf buffer_size[]);

/*
   ** from dfpF.c
 */

#ifndef DFP_FNAMES
#   define DFP_FNAMES
#ifdef DF_CAPFNAMES
#   define ndpigpal  FNAME(DPIGPAL)
#   define ndpippal  FNAME(DPIPPAL)
#   define ndpinpal  FNAME(DPINPAL)
#   define ndpiwref  FNAME(DPIWREF)
#   define ndpirref  FNAME(DPIRREF)
#   define ndprest   FNAME(DPREST)
#   define ndplref   FNAME(DPLREF)
#   define ndfprestart   FNAME(DFPRESTART)
#   define ndfplastref   FNAME(DFPLASTREF)
#else                           /* !DF_CAPNAMES */
#   define ndpigpal  FNAME(dpigpal)
#   define ndpippal  FNAME(dpippal)
#   define ndpinpal  FNAME(dpinpal)
#   define ndpiwref  FNAME(dpiwref)
#   define ndpirref  FNAME(dpirref)
#   define ndprest   FNAME(dprest)
#   define ndplref   FNAME(dplref)
#   define ndfprestart   FNAME(dfprestart)
#   define ndfplastref   FNAME(dfplastref)
#endif                          /* DF_CAPFNAMES */
#endif                          /* DFP_FNAMES */

    extern      FRETVAL(intf) ndpigpal
                (_fcd filename, _fcd pal, intf  * fnlen);

    extern      FRETVAL(intf) ndpippal
                (_fcd filename, _fcd pal, intf  * overwrite, _fcd filemode,
                 intf  * fnlen);

    extern      FRETVAL(intf) ndpinpal
                (_fcd filename, intf  * fnlen);

    extern      FRETVAL(intf) ndpirref
                (_fcd filename, uint16  * ref, intf  * fnlen);

    extern      FRETVAL(intf) ndpiwref
                (_fcd filename, uint16  * ref, intf  * fnlen);

    extern      FRETVAL(intf) ndprest
                (void);

    extern      FRETVAL(intf) ndplref
                (void);

    extern      FRETVAL(intf) ndfprestart
                (void);

    extern      FRETVAL(intf) ndfplastref
                (void);

/*
   ** from df24F.c
 */
#ifndef DF24_FNAMES
#   define DF24_FNAMES
#ifdef DF_CAPFNAMES
#   define nd2reqil  FNAME(D2REQIL)
#   define ndf24reqil    FNAME(DF24REQIL)
#   define nd2sdims  FNAME(D2SDIMS)
#   define ndf24setdims  FNAME(DF24SETDIMS)
#   define nd2setil  FNAME(D2SETIL)
#   define ndf24setil    FNAME(DF24SETIL)
#   define nd2first  FNAME(D2FIRST)
#   define ndf24restart  FNAME(DF24RESTART)
#   define nd2igdim  FNAME(D2IGDIM)
#   define nd2igimg  FNAME(D2IGIMG)
#   define nd2iaimg  FNAME(D2IAIMG)
#   define nd2irref  FNAME(D2IRREF)
#   define nd2inimg  FNAME(D2INIMG)
#   define nd2lref   FNAME(D2LREF)
#   define nd2scomp  FNAME(D2SCOMP)
#   define ndf24scompress FNAME(DF24SCOMPRESS)
#   define nd2sjpeg  FNAME(D2SJPEG)
#   define ndf24sjpeg FNAME(DF24SJPEG)
#else
#   define nd2reqil  FNAME(d2reqil)
#   define ndf24reqil    FNAME(df24reqil)
#   define nd2sdims  FNAME(d2sdims)
#   define ndf24setdims  FNAME(df24setdims)
#   define nd2setil  FNAME(d2setil)
#   define ndf24setil    FNAME(df24setil)
#   define nd2first  FNAME(d2first)
#   define ndf24restart  FNAME(df24restart)
#   define nd2igdim  FNAME(d2igdim)
#   define nd2igimg  FNAME(d2igimg)
#   define nd2iaimg  FNAME(d2iaimg)
#   define nd2irref  FNAME(d2irref)
#   define nd2inimg  FNAME(d2inimg)
#   define nd2lref   FNAME(d2lref)
#   define nd2scomp  FNAME(d2scomp)
#   define ndf24scompress FNAME(df24scompress)
#   define nd2sjpeg  FNAME(d2sjpeg)
#   define ndf24sjpeg FNAME(df24sjpeg)
#endif                          /* DF_CAPFNAMES */
#endif                          /* DF24_FNAMES */

    extern      FRETVAL(intf) nd2reqil
                (intf  * il);

    extern      FRETVAL(intf) nd2sdims
                (intf  * xdim, intf  * ydim);

    extern      FRETVAL(intf) nd2igdim
                (_fcd filename, intf  * pxdim, intf  * pydim, intf  * pil,
                 intf  * fnlen);

    extern      FRETVAL(intf) nd2igimg
                (_fcd filename, _fcd image, intf  * xdim, intf  * ydim,
                 intf  * fnlen);

    extern      FRETVAL(intf) nd2iaimg
                (_fcd filename, _fcd image, intf  * xdim, intf  * ydim,
                 intf  * fnlen, intf  * newfile);

    extern      FRETVAL(intf) nd2setil
                (intf  * il);

    extern      FRETVAL(intf) nd2first
                (void);

    extern      FRETVAL(intf) ndf24reqil
                (intf  * il);

    extern      FRETVAL(intf) ndf24setdims
                (intf  * xdim, intf  * ydim);

    extern      FRETVAL(intf) ndf24setil
                (intf  * il);

    extern      FRETVAL(intf) ndf24restart
                (void);

    extern      FRETVAL(intf) nd2irref
                (_fcd filename, intf  * ref, intf  * fnlen);

    extern      FRETVAL(intf) nd2inimg
                (_fcd filename, intf  * fnlen);

    extern      FRETVAL(intf) nd2lref
                (void);

    extern      FRETVAL(intf) nd2scomp
                (intf * scheme);

    extern      FRETVAL(intf) ndf24scompress
                (intf * scheme);

    extern      FRETVAL(intf) nd2sjpeg
                (intf * quality, intf * force_baseline);

    extern      FRETVAL(intf) ndf24sjpeg
                (intf * quality, intf * force_baseline);

/*
   ** from dfF.c
 */
#ifndef DF_FNAMES
#   define DF_FNAMES
#ifdef DF_CAPFNAMES
#   define ndfiaccess FNAME(DFIACCESS)
#   define ndfiopen  FNAME(DFIOPEN)
#   define ndfclose  FNAME(DFCLOSE)
#   define ndfdesc   FNAME(DFDESC)
#   define ndfdup    FNAME(DFDUP)
#   define ndfdel    FNAME(DFDEL)
#   define ndfstart  FNAME(DFSTART)
#   define ndfread   FNAME(DFREAD)
#   define ndfseek   FNAME(DFSEEK)
#   define ndfwrite  FNAME(DFWRITE)
#   define ndfupdate FNAME(DFUPDATE)
#   define ndfget    FNAME(DFGET)
#   define ndfput    FNAME(DFPUT)
#   define ndfsfind  FNAME(DFSFIND)
#   define ndffind   FNAME(DFFIND)
#   define ndferrno  FNAME(DFERRNO)
#   define ndfnewref FNAME(DFNEWREF)
#   define ndfnumber FNAME(DFNUMBER)
#   define ndfstat   FNAME(DFSTAT)
#   define ndfiishdf FNAME(DFIISHDF)
#else                           /* !DF_CAPFNAMES */
#   define ndfiaccess FNAME(dfiaccess)
#   define ndfiopen  FNAME(dfiopen)
#   define ndfclose  FNAME(dfclose)
#   define ndfdesc   FNAME(dfdesc)
#   define ndfdup    FNAME(dfdup)
#   define ndfdel    FNAME(dfdel)
#   define ndfstart  FNAME(dfstart)
#   define ndfread   FNAME(dfread)
#   define ndfseek   FNAME(dfseek)
#   define ndfwrite  FNAME(dfwrite)
#   define ndfupdate FNAME(dfupdate)
#   define ndfget    FNAME(dfget)
#   define ndfput    FNAME(dfput)
#   define ndfsfind  FNAME(dfsfind)
#   define ndffind   FNAME(dffind)
#   define ndferrno  FNAME(dferrno)
#   define ndfnewref FNAME(dfnewref)
#   define ndfnumber FNAME(dfnumber)
#   define ndfstat   FNAME(dfstat)
#   define ndfiishdf FNAME(dfiishdf)
#endif                          /* DF_CAPFNAMES */
#endif                          /* DF_FNAMES */

    extern      FRETVAL(intf) ndfiopen
                (_fcd name, intf  * acc_mode, intf  * defdds, intf  * namelen);

    extern      FRETVAL(intf) ndfclose
                (intf  * dfile);

    extern      FRETVAL(intf) ndfdesc
                (intf  * dfile, intf  ptr[][4], intf  * begin,
                 intf  * num);

    extern      FRETVAL(intf) ndfdup
                (intf  * dfile, intf  * tag, intf  * ref, intf  * otag,
                 intf  * oref);

    extern      FRETVAL(intf) ndfdel
                (intf  * dfile, intf  * tag, intf  * ref);

    extern      FRETVAL(intf) ndfiaccess
                (intf  * dfile, intf  * tag, intf  * ref, _fcd acc_mode, intf  * acclen);

    extern      FRETVAL(intf) ndfstart
                (intf  * dfile, intf  * tag, intf  * ref, char  * acc_mode);

    extern      FRETVAL(intf) ndfread
                (intf  * dfile, _fcd ptr, intf  * len);

    extern      FRETVAL(intf) ndfseek
                (intf  * dfile, intf  * offset);

    extern      FRETVAL(intf) ndfwrite
                (intf  * dfile, _fcd ptr, intf  * len);

    extern      FRETVAL(intf) ndfupdate
                (intf  * dfile);

    extern      FRETVAL(intf) ndfget
                (intf  * dfile, intf  * tag, intf  * ref, _fcd ptr);

    extern      FRETVAL(intf) ndfput
                (intf  * dfile, intf  * tag, intf  * ref, _fcd ptr, intf  * len);

    extern      FRETVAL(intf) ndfsfind
                (intf  * dfile, intf  * tag, intf  * ref);

    extern      FRETVAL(intf) ndffind
                (intf  * dfile, intf  * itag, intf  * iref, intf  * len);

    extern      FRETVAL(intf) ndferrno
                (void);

    extern      FRETVAL(intf) ndfnewref
                (intf  * dfile);

    extern      FRETVAL(intf) ndfnumber
                (intf  * dfile, intf  * tag);

    extern      FRETVAL(intf) ndfiishdf
                (_fcd name, intf  * namelen);

/*
   ** from dfutil.c
 */
    extern uint16 DFfindnextref
                (int32 file_id, uint16 tag, uint16 lref);

/*
   ** from dfutilF.c
 */
#ifndef DFUTIL_FNAMES
#   define DFUTIL_FNAMES
#ifdef DF_CAPFNAMES
#   define ndfindnr          FNAME(DFINDNR)
#   define ndffindnextref    FNAME(DFFINDNEXTREF)
#else
#   define ndfindnr          FNAME(dfindnr)
#   define ndffindnextref    FNAME(dffindnextref)
#endif                          /* DF_CAPFNAMES */
#endif                          /* DFUTIL_FNAMES */

    extern      FRETVAL(intf) ndfindnr
                (intf  * dfile, intf  * tag, intf  * lref);

    extern      FRETVAL(intf) ndffindnextref
                (intf  * dfile, intf  * tag, intf  * lref);

/*
   ** from herrF.c
 */
#ifndef HERR_FNAMES
#   define HERR_FNAMES
#ifdef DF_CAPFNAMES
#   define nheprnt   FNAME(HEPRNT)
#else
#   define nheprnt   FNAME(heprnt)
#endif                          /* DF_CAPFNAMES */
#endif                          /* HERR_FNAMES */

    extern      FRETVAL(VOID) nheprnt
                (intf  * print_levels);

/*
   ** from hfileF.c
 */
#ifndef HFILE_FNAMES
#   define HFILE_FNAMES
#ifdef DF_CAPFNAMES
#  if !(defined INTEL86) && !(defined _WIN32)
#   define nhiopen   FNAME(HIOPEN)
#   define nhclose   FNAME(HCLOSE)
#   define nhnumber  FNAME(HNUMBER)
#   define nhxisdir  FNAME(HXISDIR)
#   define nhxiscdir FNAME(HXISCDIR)
#  else
#   define nhiopen   FNAME(HIOPEN)
#   define nhiclose   FNAME(HICLOSE)
#   define nhinumbr  FNAME(HINUMBR)
#   define nhxisdir  FNAME(HXISDIR)
#   define nhxiscdir FNAME(HXISCDIR)
#  endif
#else
#  if !(defined INTEL86) && !(defined _WIN32)
#   define nhiopen   FNAME(hiopen)
#   define nhclose   FNAME(hclose)
#   define nhnumber  FNAME(hnumber)
#   define nhxisdir  FNAME(hxisdir)
#   define nhxiscdir FNAME(hxiscdir)
#  else
#   define nhiopen   FNAME(hiopen)
#   define nhiclose   FNAME(hiclose)
#   define nhinumbr  FNAME(hinumbr)
#   define nhxisdir  FNAME(hxisdir)
#   define nhxiscdir FNAME(hxiscdir)
#  endif
#endif                          /* DF_CAPFNAMES */
#endif                          /* HFILE_FNAMES */

    extern      FRETVAL(intf) nhiopen
                (_fcd name, intf  * acc_mode, intf  * defdds, intf  * namelen);

    extern      FRETVAL(intf) nhclose
                (intf  * file_id);

    extern      FRETVAL(intf) nhiclose
                (intf  * file_id);

    extern	FRETVAL(intf) nhnumber
		(int32 file_id, uint16 tag);

    extern	FRETVAL(intf) nhinumbr
		(int32 file_id, uint16 tag);

    extern	FRETVAL(intf) nhxisdir
		(_fcd dir, intf * dirlen);

    extern	FRETVAL(intf) nhxiscdir
		(_fcd dir, intf * dirlen);
    
/*
   ** from dfufp2im.c
 */
#ifndef DFUFP2I_FNAMES
#   define DFUFP2I_FNAMES
#ifdef DF_CAPFNAMES
#   define nduif2i       FNAME(DUIF2I)
#else
#   define nduif2i       FNAME(duif2i)
#endif                          /* DF_CAPFNAMES */
#endif                          /* DFUFP2I_FNAMES */

    extern      FRETVAL(int) nduif2i
                (int32  * hdim, int32  * vdim, float32  * max,
        float32  * min, float32  hscale[], float32  vscale[],
                 float32  data[], _fcd palette, _fcd outfile,
              int  * ct_method, int32  * hres, int32  * vres,
                 int  * compress, int  * lenfn);

    extern int  DFUfptoimage
                (int32 hdim, int32 vdim, float32 max, float32 min,
       float32  * hscale, float32  * vscale, float32  * data,
                 uint8  * palette, char  * outfile, int ct_method,
                 int32 hres, int32 vres, int compress);

/* for Multi-file fortran Annotation inteface */
#ifndef MFAN_FNAMES
#   define  MFAN_FNAMES
#ifdef DF_CAPFNAMES
# if !(defined INTEL86) && !(defined _WIN32)
#  define nafstart      FNAME(AFSTART)
#  define naffileinfo   FNAME(AFFILEINFO)
#  define nafend        FNAME(AFEND)
#  define nafcreate     FNAME(AFCREATE)
#  define naffcreate    FNAME(AFFCREATE)
#  define nafselect     FNAME(AFSELECT)
#  define nafnumann     FNAME(AFNUMANN)
#  define nafannlist    FNAME(AFANNLIST)
#  define nafannlen     FNAME(AFANNLEN)
#  define nafwriteann   FNAME(AFWRITEANN)
#  define nafreadann    FNAME(AFREADANN)
#  define nafendaccess  FNAME(AFENDACCESS)
#  define nafgettagref  FNAME(AFGETTAGREF)
#  define nafidtagref   FNAME(AFIDTAGREF)
#  define naftagrefid   FNAME(AFTAGREFID)
#  define nafatypetag   FNAME(AFATYPETAG)
#  define naftagatype   FNAME(AFTAGATYPE)
# else
#  define nafistart     FNAME(AFISTART)
#  define nafifinf      FNAME(AFIFINF)
#  define nafiend       FNAME(AFIEND)
#  define naficreat     FNAME(AFICREAT)
#  define nafifcreat    FNAME(AFIFCREAT)
#  define nafiselct     FNAME(AFISELCT)
#  define nafinann      FNAME(AFINANN)
#  define nafialst      FNAME(AFIALST)
#  define nafialen      FNAME(AFIALEN)
#  define nafiwann      FNAME(AFIWANN)
#  define nafirann      FNAME(AFIRANN)
#  define nafiendac     FNAME(AFIENDAC)
#  define nafigtr       FNAME(AFIGTR)
#  define nafiid2tr     FNAME(AFIID2TR)
#  define nafitr2id     FNAME(AFITR2ID)
#  define nafitp2tg     FNAME(AFITP2TG)
#  define nafitg2tp     FNAME(AFITG2TP)
# endif
#else  /* !DF_CAPFNAMES */
# if !(defined INTEL86) && !(defined _WIN32)
#  define nafstart      FNAME(afstart)
#  define naffileinfo   FNAME(affileinfo)
#  define nafend        FNAME(afend)
#  define nafcreate     FNAME(afcreate)
#  define naffcreate    FNAME(affcreate)
#  define nafselect     FNAME(afselect)
#  define nafnumann     FNAME(afnumann)
#  define nafannlist    FNAME(afannlist)
#  define nafannlen     FNAME(afannlen)
#  define nafwriteann   FNAME(afwriteann)
#  define nafreadann    FNAME(afreadann)
#  define nafendaccess  FNAME(afendaccess)
#  define nafgettagref  FNAME(afgettagref)
#  define nafidtagref   FNAME(afidtagref)
#  define naftagrefid   FNAME(aftagrefid)
#  define nafatypetag   FNAME(afatypetag)
#  define naftagatype   FNAME(aftagatype)
# else
#  define nafistart     FNAME(afistart)
#  define nafifinf      FNAME(afifinf)
#  define nafiend       FNAME(afiend)
#  define naficreat     FNAME(aficreat)
#  define nafifcreat    FNAME(afifcreat)
#  define nafiselct     FNAME(afiselct)
#  define nafinann      FNAME(afinamm)
#  define nafialst      FNAME(afialst)
#  define nafialen      FNAME(afialen)
#  define nafiwann      FNAME(afiwann)
#  define nafirann      FNAME(afirann)
#  define nafiendac     FNAME(afiendac)
#  define nafigtr       FNAME(afigtr)
#  define nafiid2tr     FNAME(afiid2tr)
#  define nafitr2id     FNAME(afitr2id)
#  define nafitp2tg     FNAME(afitp2tg)
#  define nafitg2tp     FNAME(afitg2tp)
# endif
#endif /* DF_CAPFNAMES */
#endif /* MFAN_FNAMES */

/* Multi-file Annotation C-stubs for fortan interface found in mfanf.c */

extern FRETVAL(intf)
nafistart(intf *file_id);

extern FRETVAL(intf)
nafifinf(intf *an_id, intf *num_flabel, intf *num_fdesc, intf *num_olabel,
            intf *num_odesc);

extern FRETVAL(intf)
nafiend(intf *an_id);

extern FRETVAL(intf)
naficreat(intf *an_id, intf *etag, intf *eref, intf *atype);

extern FRETVAL(intf)
nafifcreat(intf *an_id, intf *atype);

extern FRETVAL(intf)
nafiselct(intf *an_id, intf *index, intf *atype);

extern FRETVAL(intf)
nafinann(intf *an_id, intf *atype, intf *etag, intf *eref);

extern FRETVAL(intf)
nafialst(intf *an_id, intf *atype, intf *etag, intf *eref, intf alist[]);

extern FRETVAL(intf)
nafialen(intf *ann_id);

extern FRETVAL(intf)
nafiwann(intf *ann_id,_fcd ann, intf *annlen);

extern FRETVAL(intf)
nafirann(intf *ann_id,_fcd ann, intf *maxlen);

extern FRETVAL(intf)
nafiendac(intf *ann_id);

extern FRETVAL(intf)
nafigtr(intf *an_id, intf *index, intf *type, intf *tag, intf *ref);

extern FRETVAL(intf)
nafiid2tr(intf *ann_id, intf *tag, intf *ref);

extern FRETVAL(intf)
nafitr2id(intf *an_id, intf *tag, intf *ref);

extern FRETVAL(intf)
nafitp2tg(intf *atype);

extern FRETVAL(intf)
nafitg2tp(intf *tag);

/* Multi-file Annotation C-routines found in mfan.c */
extern int32 ANstart(int32 file_id);

extern intn  ANfileinfo(int32 an_id, int32 *n_file_label, int32 *n_file_desc,
                        int32 *n_obj_label, int32 *n_obj_desc);

extern int32 ANend(int32 an_id);

extern int32 ANcreate(int32 an_id, uint16 elem_tag, uint16 elem_ref, 
                      ann_type type);

extern int32 ANcreatef(int32 an_id, ann_type type);

extern int32 ANselect(int32 an_id, int32 index, ann_type type);

extern intn  ANnumann(int32 an_id, ann_type type, uint16 elem_tag, 
                      uint16 elem_ref);

extern intn  ANannlist(int32 an_id, ann_type type, uint16 elem_tag, 
                       uint16 elem_ref, int32 ann_list[]);

extern int32 ANannlen(int32 ann_id);

extern int32 ANwriteann(int32 ann_id, const char *ann, int32 annlen);

extern int32 ANreadann(int32 ann_id, char *ann, int32 maxlen);

extern intn  ANendaccess(int32 ann_id);

extern int32 ANget_tagref(int32 an_id, int32 index, ann_type type,
                          uint16 *ann_tag, uint16 *ann_ref);

extern int32 ANid2tagref(int32 an_id, uint16 *ann_tag, uint16 *ann_ref);

extern int32 ANtagref2id(int32 an_id, uint16 ann_tag, uint16 ann_ref);

extern uint16 ANatype2tag(ann_type atype);

extern ann_type ANtag2atype(uint16 atag);

extern intn ANdestroy(void);

/* for Multi-file fortran GR interface */
#ifndef MFGR_FNAMES
#   define  MFGR_FNAMES
#ifdef DF_CAPFNAMES
# if !(defined INTEL86) && !(defined _WIN32)
#  define nmgstart      FNAME(MGSTART)
#  define nmgfinfo      FNAME(MGFINFO)
#  define nmgend        FNAME(MGEND)
#  define nmgicreat     FNAME(MGICREAT)
#  define nmgselct      FNAME(MGSELCT)
#  define nmgin2ndx     FNAME(MGIN2NDX)
#  define nmggiinf      FNAME(MGGIINF)
#  define nmgwcimg      FNAME(MGWCIMG)
#  define nmgrcimg      FNAME(MGRCIMG)
#  define nmgwrimg      FNAME(MGWRIMG)
#  define nmgrdimg      FNAME(MGRDIMG)
#  define nmgendac      FNAME(MGENDAC)
#  define nmgid2rf      FNAME(MGID2RF)
#  define nmgr2idx      FNAME(MGR2IDX)
#  define nmgrltil      FNAME(MGRLTIL)
#  define nmgrimil      FNAME(MGRIMIL)
#  define nmggltid      FNAME(MGGLTID)
#  define nmgglinf      FNAME(MGGLINF)
#  define nmgwrlut      FNAME(MGWRLUT)
#  define nmgwclut      FNAME(MGWCLUT)
#  define nmgrdlut      FNAME(MGRDLUT)
#  define nmgrclut      FNAME(MGRCLUT)
#  define nmgisxfil     FNAME(MGISXFIL)
#  define nmgssctp      FNAME(MGSACTP)
#  define nmgiscatt     FNAME(MGISCATT)
#  define nmgisattr     FNAME(MGISATTR)
#  define nmgatinf      FNAME(MGATINF)
#  define nmggcatt      FNAME(MGGCATT)
#  define nmggnatt      FNAME(MGGNATT)
#  define nmggattr      FNAME(MGGATTR)
#  define nmgifndat     FNAME(MGIFNDAT)
# else
#  define nmgistrt      FNAME(MGISTRT)
#  define nmgifinf      FNAME(MGIFINF)
#  define nmgiend       FNAME(MGIEND)
#  define nmgicreat     FNAME(MGICREAT)
#  define nmgislct      FNAME(MGISLCT)
#  define nmgin2ndx     FNAME(MGIN2NDX)
#  define nmgigiinf     FNAME(MGIGIINF)
#  define nmgiwcim      FNAME(MGIWCIM)
#  define nmgircim      FNAME(MGIRCIM)
#  define nmgiwimg      FNAME(MGIWIMG)
#  define nmgirimg      FNAME(MGIRIMG)
#  define nmgiendac     FNAME(MGIENDAC)
#  define nmgiid2r      FNAME(MGIID2R)
#  define nmgir2dx      FNAME(MGIR2DX)
#  define nmgiltil      FNAME(MGILTIL)
#  define nmgiimil      FNAME(MGIIMIL)
#  define nmgiglid      FNAME(MGIGLID)
#  define nmgiglinf     FNAME(MGIGLINF)
#  define nmgiwrlt      FNAME(MGIWRLT)
#  define nmgiwclt      FNAME(MGIWCLT)
#  define nmgirdlt      FNAME(MGIRDLT)
#  define nmgirclt      FNAME(MGIRCLT)
#  define nmgisxfil     FNAME(MGISXFIL)
#  define nmgiactp      FNAME(MGIACTP)
#  define nmgiscatt     FNAME(MGISCATT)
#  define nmgisattr     FNAME(MGISATTR)
#  define nmgiainf      FNAME(MGIAINF)
#  define nmgigcat      FNAME(MGIGCAT)
#  define nmgignat      FNAME(MGIGNAT)
#  define nmgigatt      FNAME(MGIGATT)
#  define nmgifndat     FNAME(MGIFNDAT)
# endif
#else  /* !DF_CAPFNAMES */
# if !(defined INTEL86) && !(defined _WIN32)
#  define nmgstart      FNAME(mgstart)
#  define nmgfinfo      FNAME(mgfinfo)
#  define nmgend        FNAME(mgend)
#  define nmgicreat     FNAME(mgicreat)
#  define nmgselct      FNAME(mgselct)
#  define nmgin2ndx     FNAME(mgin2ndx)
#  define nmggiinf      FNAME(mggiinf)
#  define nmgwcimg      FNAME(mgwcimg)
#  define nmgrcimg      FNAME(mgrcimg)
#  define nmgwrimg      FNAME(mgwrimg)
#  define nmgrdimg      FNAME(mgrdimg)
#  define nmgendac      FNAME(mgendac)
#  define nmgid2rf      FNAME(mgid2rf)
#  define nmgr2idx      FNAME(mgr2idx)
#  define nmgrltil      FNAME(mgrltil)
#  define nmgrimil      FNAME(mgrimil)
#  define nmggltid      FNAME(mggltid)
#  define nmgglinf      FNAME(mgglinf)
#  define nmgwrlut      FNAME(mgwrlut)
#  define nmgwclut      FNAME(mgwclut)
#  define nmgrdlut      FNAME(mgrdlut)
#  define nmgrclut      FNAME(mgrclut)
#  define nmgisxfil     FNAME(mgisxfil)
#  define nmgssctp      FNAME(mgsactp)
#  define nmgiscatt     FNAME(mgiscatt)
#  define nmgisattr     FNAME(mgisattr)
#  define nmgatinf      FNAME(mgatinf)
#  define nmggcatt      FNAME(mggcatt)
#  define nmggnatt      FNAME(mggnatt)
#  define nmggattr      FNAME(mggattr)
#  define nmgifndat     FNAME(mgifndat)
# else
#  define nmgistrt      FNAME(mgistrt)
#  define nmgifinf      FNAME(mgifinf)
#  define nmgiend        FNAME(mgiend)
#  define nmgicreat     FNAME(mgicreat)
#  define nmgislct      FNAME(mgislct)
#  define nmgin2ndx     FNAME(mgin2ndx)
#  define nmgigiinf      FNAME(mgigiinf)
#  define nmgiwcim      FNAME(mgiwcim)
#  define nmgircim      FNAME(mgircim)
#  define nmgwimg      FNAME(mgiwimg)
#  define nmgrimg      FNAME(mgirimg)
#  define nmgiendac      FNAME(mgiendac)
#  define nmgiid2r      FNAME(mgiid2r)
#  define nmgir2dx      FNAME(mgir2dx)
#  define nmgiltil      FNAME(mgiltil)
#  define nmgiimil      FNAME(mgiimil)
#  define nmgiglid      FNAME(mgiglid)
#  define nmgiglinf     FNAME(mgiglinf)
#  define nmgiwrlt      FNAME(mgiwrlt)
#  define nmgiwclt      FNAME(mgiwclt)
#  define nmgirdlt      FNAME(mgirdlt)
#  define nmgirclt      FNAME(mgirclt)
#  define nmgisxfil     FNAME(mgisxfil)
#  define nmgiactp      FNAME(mgiactp)
#  define nmgiscatt     FNAME(mgiscatt)
#  define nmgisattr     FNAME(mgisattr)
#  define nmgiainf      FNAME(mgiainf)
#  define nmgigcat      FNAME(mgigcat)
#  define nmgignat      FNAME(mgignat)
#  define nmgigatt      FNAME(mgigatt)
#  define nmgifndat     FNAME(mgifndat)
# endif
#endif /* DF_CAPFNAMES */
#endif /* MFGR_FNAMES */

/* Multi-file GR C-stubs for FORTRAN interface found in mfgrf.c */

extern FRETVAL(intf)
nmgistrt(intf * fid);

extern FRETVAL(intf)
nmgifinf(intf * grid,intf *n_datasets,intf *n_attrs);

extern FRETVAL(intf)
nmgiend(intf * grid);

extern FRETVAL(intf)
nmgicreat(intf * grid, _fcd name, intf *ncomp, intf *nt, intf *il, intf dimsizes[2], intf *nlen);

extern FRETVAL(intf)
nmgislct(intf * grid, intf *index);

extern FRETVAL(intf)
nmgin2ndx(intf * grid, _fcd name, intf *nlen);

extern FRETVAL(intf)
nmgigiinf(intf * riid, _fcd name, intf *ncomp, intf *nt, intf *il, intf *dimsizes, intf *nattr);


extern FRETVAL(intf)
nmgiwcim(intf * riid, intf *start, intf *stride, intf *count, _fcd data);

extern FRETVAL(intf)
nmgircim(intf * riid, intf *start, intf *stride, intf *count, _fcd data);

extern FRETVAL(intf)
nmgiwimg(intf * riid, intf *start, intf *stride, intf *count, VOIDP data);

extern FRETVAL(intf)
nmgirimg(intf * riid, intf *start, intf *stride, intf *count, VOIDP data);

extern FRETVAL(intf)
nmgiendac(intf * riid);

extern FRETVAL(intf)
nmgiid2r(intf * riid);

extern FRETVAL(intf)
nmgir2dx(intf * grid, intf *ref);

extern FRETVAL(intf)
nmgiltil(intf * riid, intf *il);

extern FRETVAL(intf)
nmgiimil(intf * riid, intf *il);

extern FRETVAL(intf)
nmgiglid(intf * riid, intf *lut_index);

extern FRETVAL(intf)
nmgiglinf(intf * lutid, intf *ncomp, intf *nt, intf *il, intf *nentries);

extern FRETVAL(intf)
nmgiwrlt(intf * lutid, intf *ncomp, intf *nt, intf *il, intf *nentries, VOIDP data);

extern FRETVAL(intf)
nmgiwclt(intf * lutid, intf *ncomp, intf *nt, intf *il, intf *nentries, _fcd data);

extern FRETVAL(intf)
nmgirdlt(intf * lutid, VOIDP data);

extern FRETVAL(intf)
nmgirclt(intf * lutid, _fcd data);

extern FRETVAL(intf)
nmgisxfil(intf * riid, _fcd filename, intf *offset, intf *nlen);

extern FRETVAL(intf)
nmgiactp(intf * riid, intf *accesstype);

extern FRETVAL(intf)
nmgiscatt(intf * riid, _fcd name, intf *nt, intf *count, _fcd data, intf *nlen);

extern FRETVAL(intf)
nmgisattr(intf * riid, _fcd name, intf *nt, intf *count, VOIDP data, intf *nlen);

extern FRETVAL(intf)
nmgiainf(intf * riid, intf *index, _fcd name, intf *nt, intf *count);

extern FRETVAL(intf)
nmgigcat(intf * riid, intf *index, _fcd data);

extern FRETVAL(intf)
nmgignat(intf * riid, intf *index, VOIDP data);

extern FRETVAL(intf)
nmgigatt(intf * riid, intf *index, VOIDP data);

extern FRETVAL(intf)
nmgifndat(intf * riid, _fcd name, intf *nlen);

/* Multi-file Raster C-routines found in mfgr.c */
extern intn rigcompare(VOIDP k1, VOIDP k2, intn cmparg);

extern int32 GRstart(int32 hdf_file_id);

extern intn GRfileinfo(int32 grid,int32 *n_datasets,int32 *n_attrs);

extern intn GRend(int32 grid);

extern int32 GRcreate(int32 grid,const char *name,int32 ncomp,int32 nt,int32 il,
    int32 dimsizes[2]);

extern int32 GRselect(int32 grid,int32 index);

extern int32 GRnametoindex(int32 grid,const char *name);

extern intn GRgetiminfo(int32 riid,char *name,int32 *ncomp,int32 *nt,int32 *il,
    int32 dimsizes[2],int32 *n_attr);

extern intn GRwriteimage(int32 riid,int32 start[2],int32 stride[2],
    int32 count[2],VOIDP data);

extern intn GRreadimage(int32 riid,int32 start[2],int32 stride[2],
    int32 count[2],VOIDP data);

extern intn GRendaccess(int32 riid);

extern uint16 GRidtoref(int32 riid);

extern int32 GRreftoindex(int32 grid,uint16 ref);

extern intn GRreqlutil(int32 riid,intn il);

extern intn GRreqimageil(int32 riid,intn il);

extern int32 GRgetlutid(int32 riid,int32 index);

extern uint16 GRluttoref(int32 lutid);

extern intn GRgetlutinfo(int32 riid,int32 *ncomp,int32 *nt,
    int32 *il,int32 *nentries);

extern intn GRwritelut(int32 riid,int32 ncomps,int32 nt,
    int32 il,int32 nentries,VOIDP data);

extern intn GRreadlut(int32 lutid,VOIDP data);

extern intn GRsetexternalfile(int32 riid,const char *filename,int32 offset);

extern intn GRsetaccesstype(int32 riid,uintn accesstype);

extern intn GRsetcompress(int32 riid,int32 comp_type,comp_info *cinfo);

extern intn GRsetattr(int32 id,const char *name,int32 attr_nt,int32 count,const VOIDP data);

extern intn GRattrinfo(int32 id,int32 index,char *name,int32 *attr_nt,int32 *count);

extern intn GRgetattr(int32 id,int32 index,VOIDP data);

extern int32 GRfindattr(int32 id,const char *name);

extern intn GRPshutdown(void);

#ifdef HAVE_FMPOOL
/******************************************************************************
NAME
     Hmpset - set pagesize and maximum number of pages to cache on next open/create

DESCRIPTION
     Set the pagesize and maximum number of pages to cache on the next 
     open/create of a file. A pagesize that is a power of 2 is recommended.

     The values set here only affect the next open/creation of a file and
     do not change a particular file's paging behaviour after it has been
     opened or created. This maybe changed in a later release.

     Use flags arguement of 'MP_PAGEALL' if the whole file is to be cached 
     in memory otherwise passs in zero.

RETURNS
     Returns SUCCEED if successful and FAIL otherwise

NOTE
     This calls the real routine MPset().
     Currently 'maxcache' has to be greater than 1. Maybe use special 
     case of 0 to specify you want to turn page buffering off or use
     the flags arguement. 

******************************************************************************/
extern int  Hmpset(int pagesize, /* IN: pagesize to use for next open/create */
                   int maxcache, /* IN: max number of pages to cache */
                   int flags     /* IN: flags = 0, MP_PAGEALL */);

/******************************************************************************
NAME
     Hmpget - get last pagesize and max number of pages cached for open/create

DESCRIPTION
     This gets the last pagesize and maximum number of pages cached for 
     the last open/create of a file.

RETURNS
     Returns SUCCEED.

NOTES
     This routine calls the real routine MPget().
******************************************************************************/
extern int  Hmpget(int *pagesize, /*OUT: pagesize to used in last open/create */
                   int *maxcache, /*OUT: max number of pages cached in last open/create */
                   int flags      /* IN: */);

#endif /* HAVE_FMPOOL */

/* Vset interface functions (used to be in vproto.h) */

/* Useful macros, which someday might become actuall functions */
/*
 * macros for VSinquire
 * all these macros should be public for users
 */
#define VSQuerycount(vs, count) \
        (VSinquire (vs, (int32 *) count, (int32*) NULL, (char*) NULL, (int32*) NULL, (char*) NULL))

#define VSQueryinterlace(vs, intr) \
        (VSinquire (vs, (int32 *) NULL, (int32*) intr, (char*) NULL, (int32*) NULL, (char*) NULL))

#define VSQueryfields(vs, flds) \
        (VSinquire (vs, (int32 *) NULL, (int32*) NULL, (char*) flds, (int32*) NULL, (char*) NULL))

#define VSQueryvsize(vs, size) \
        (VSinquire (vs, (int32 *) NULL, (int32*) NULL, (char*) NULL, (int32*) size, (char*) NULL))

#define VSQueryname(vs, name) \
        (VSinquire (vs, (int32 *) NULL, (int32*) NULL, (char*) NULL, (int32*) NULL, (char*) name))

/* 
   ** from vattr.c
 */
   extern intn Vsetattr
                (int32 vgid,  char *attrname, int32 datatype,
                 int32 count, VOIDP values);
   extern intn Vnattrs
                (int32 vgid);
   extern intn Vfindattr
                (int32 vgid, char *attrname);
   extern intn Vattrinfo
                (int32 vgid, intn attrindex, char *name, 
                 int32 *datatype, int32 *count, int32 *size);
   extern intn Vgetattr
                (int32 vgid, intn attrindex, VOIDP values);
   extern int32 Vgetversion
                (int32 vgid);
   extern intn VSfindex
                 (int32 vsid, char *fieldname, int32 *fldindex);
   extern intn VSsetattr
                (int32 vsid, int32 findex, char *attrname,
                 int32 datatype, int32 count, VOIDP values);
   extern intn VSnattrs
                (int32 vsid);
   extern intn VSfnattrs
                (int32 vsid, int32 findex);
   extern intn VSfindattr
                (int32 vsid, int32 findex, char *attrname);
   extern intn VSattrinfo
                (int32 vsid, int32 findex, intn attrindex,
                 char *name, int32 *datatype, int32 *count, 
                 int32 *size);
   extern intn VSgetattr
                (int32 vsid, int32 findex, intn attrindex,
                  VOIDP values);
   extern intn VSisattr
                (int32 vsid);
/*
   ** from vconv.c
 */
    extern int32 vicheckcompat
                (HFILEID f);

    extern int32 vimakecompat
                (HFILEID f);

    extern int32 vcheckcompat
                (char  * fs);

    extern int32 vmakecompat
                (char  * fs);

/*
   ** from vg.c
 */
    extern int32 VSelts
                (int32 vkey);

    extern int32 VSgetinterlace
                (int32 vkey);

    extern intn VSsetinterlace
                (int32 vkey, int32 interlace);

    extern int32 VSgetfields
                (int32 vkey, char  * fields);

    extern intn VSfexist
                (int32 vkey, char  * fields);

    extern int32 VSsizeof
                (int32 vkey, char  * fields);

    extern VOID VSdump
                (int32 vkey);

    extern int32 VSsetname
                (int32 vkey, const char  * vsname);

    extern int32 VSsetclass
                (int32 vkey, const char  * vsclass);

    extern int32 VSgetname
                (int32 vkey, char  * vsname);

    extern int32 VSgetclass
                (int32 vkey, char  * vsclass);

    extern intn VSinquire
                (int32 vkey, int32  * nelt, int32  * interlace,
           char  * fields, int32  * eltsize, char  * vsname);

    extern int32 VSlone
                (HFILEID f, int32  * idarray, int32 asize);

    extern int32 Vlone
                (HFILEID f, int32  * idarray, int32 asize);

    extern int32 Vfind
                (HFILEID f, const char  * vgname);

    extern int32 VSfind
                (HFILEID f, const char  * vsname);

    extern int32 Vfindclass
                (HFILEID f, const char  * vgclass);

    extern int32 VSfindclass
                (HFILEID f, const char  * vsclass);

    extern VOID Vsetzap
                (void);

/*
   ** from vgp.c
 */
    extern intn vcompare
                (VOIDP k1, VOIDP k2, intn cmparg);

    extern intn vcompareref
                (VOIDP k1, VOIDP k2, intn cmparg);

    extern VOID vdestroynode
                (VOIDP n);

    extern VOID vtfreekey
                (VOIDP k);

    extern intn Vinitialize
                (HFILEID f);

    extern intn Vfinish
                (HFILEID f);

    extern HFILEID Vopen
                (char *path, intn acc_mode, int16 ndds);

    extern intn Vclose
                (HFILEID f);

    extern int32 vexistvg
                (HFILEID f, uint16 vgid);

    extern int32 Vattach
                (HFILEID f, int32 vgid, const char  * accesstype);

    extern int32 Vdetach
                (int32 vkey);

    extern int32 Vinsert
                (int32 vkey, int32 vskey);
    /* note: 2nd arg of Vinsert can also be (VGROUP *) */

    extern int32 Vflocate
                (int32 vkey, char  * field);

    extern intn Vinqtagref
                (int32 vkey, int32 tag, int32 ref);

    extern int32 Vntagrefs
                (int32 vkey);

    extern int32 Vnrefs
                (int32 vkey,int32 tag);

    extern int32 Vgettagrefs
                (int32 vkey, int32  tagarray[], int32  refarray[], int32 n);

    extern intn Vgettagref
                (int32 vkey, int32 which, int32  * tag, int32  * ref);

    extern int32 VQueryref
                (int32 vkey);

    extern int32 VQuerytag
                (int32 vkey);

    extern int32 Vaddtagref
                (int32 vkey, int32 tag, int32 ref);

    extern int32 Ventries
                (HFILEID f, int32 vgid);

    extern int32 Vsetname
                (int32 vkey, const char  * vgname);

    extern int32 Vsetclass
                (int32 vkey, const char  * vgclass);

    extern intn Visvg
                (int32 vkey, int32 id);

    extern intn Visvs
                (int32 vkey, int32 id);

    extern int32 Vgetid
                (HFILEID f, int32 vgid);

    extern int32 Vgetnext
                (int32 vkey, int32 id);

    extern int32 Vgetname
                (int32 vkey, char  * vgname);

    extern int32 Vgetclass
                (int32 vkey, char  * vgclass);

    extern intn Vinquire
                (int32 vkey, int32  * nentries, char  * vgname);

    extern int32 Vdelete
                (int32 f, int32 ref);

    extern intn VPshutdown(void);

/*
   ** from vparse.c
 */
    extern int32 scanattrs
                (const char  * attrs, int32  * attrc, char  *** attrv);

/*
   ** from vhi.c
 */
    extern int32 VHstoredata
                (HFILEID f, char  * field, uint8  buf[], int32 n, int32 datatype,
                 const char  * vsname, const char  * vsclass);

    extern int32 VHstoredatam
                (HFILEID f, const char * field, const uint8  buf[], int32 n, int32 datatype,
                 const char  * vsname, const char  * vsclass, int32 order);

    extern int32 VHmakegroup
                (HFILEID f, int32  tagarray[], int32  refarray[], int32 n, char  * vgname,
                 char  * vgclass);

/*
   ** from vio.c
 */

    extern intn VSPhshutdown(void);

    extern int32 vexistvs
                (HFILEID f, uint16 vsref);

    extern VOID vsdestroynode
                (VOIDP n);

    extern VOID vfdestroynode
                (VOIDP n);

    extern int32 VSattach
                (HFILEID f, int32 vsref, const char  * accesstype);

    extern int32 VSdetach
                (int32 vkey);

    extern int32 VSQuerytag
                (int32 vkey);

    extern int32 VSQueryref
                (int32 vkey);
 
    extern int32 VSgetid
                (HFILEID f, int32 vsref);

    extern int32 VSgetversion
                (int32 vkey);

    extern int32 VSdelete
                (int32 f, int32 ref);

    extern int32 VSappendable
                (int32 vkey, int32 blk);

/*
   ** from vsfld.c
 */

    extern intn VSsetfields
                (int32 vkey, const char  * fields);

    extern intn VSfdefine
                (int32 vkey, const char  * field, int32 localtype, int32 order);

    extern int32 VFnfields
                (int32 vkey);

    extern char *VFfieldname
                (int32 vkey, int32 index);

    extern int32 VFfieldtype
                (int32 vkey, int32 index);

    extern int32 VFfieldisize
                (int32 vkey, int32 index);

    extern int32 VFfieldesize
                (int32 vkey, int32 index);

    extern int32 VFfieldorder
                (int32 vkey, int32 index);

    extern intn VSsetexternalfile
		(int32 vkey, const char *filename, int32 offset);

    extern intn VSfpack
                (int32 vsid, intn packtype, char *fields_in_buf,
                VOIDP buf, intn bufsz, intn n_records, 
                char *fields, VOIDP fldbufpt[]);

/*
   ** from vrw.c
 */
    extern intn VSPshutdown(void);

    extern int32 VSseek
                (int32 vkey, int32 eltpos);

    extern int32 VSread
                (int32 vkey, uint8  buf[], int32 nelt, int32 interlace);

    extern int32 VSwrite
                (int32 vkey, const uint8  buf[], int32 nelt, int32 interlace);

/*
   ** from vgF.c
 */
#ifndef VG_FNAMES
#   define VG_FNAMES
#ifdef DF_CAPFNAMES
#  if !(defined INTEL86) && !(defined _WIN32)
#   define  ndfivopn FNAME(DFIVOPN)
#   define  ndfvclos FNAME(DFVCLOS)
#   define  nvatchc  FNAME(VATCHC)
#   define  nvdtchc  FNAME(VDTCHC)
#   define  nvgnamc  FNAME(VGNAMC)
#   define  nvgclsc  FNAME(VGCLSC)
#   define  nvinqc   FNAME(VINQC)
#   define  nvdelete FNAME(VDELETE)
#   define  nvgidc   FNAME(VGIDC)
#   define  nvgnxtc  FNAME(VGNXTC)
#   define  nvsnamc  FNAME(VSNAMC)
#   define  nvsclsc  FNAME(VSCLSC)
#   define  nvinsrtc FNAME(VINSRTC)
#   define  nvisvgc  FNAME(VISVGC)
#   define  nvfstart FNAME(VFSTART)
#   define  nvfend   FNAME(VFEND)
#   define  nvisvsc  FNAME(VISVSC)
#   define  nvsatchc FNAME(VSATCHC)
#   define  nvsdtchc FNAME(VSDTCHC)
#   define  nvsqref  FNAME(VSQREF)
#   define  nvsqtag  FNAME(VSQTAG)
#   define  nvsgver  FNAME(VSGVER)
#   define  nvsseekc FNAME(VSSEEKC)
#   define  nvsgnamc FNAME(VSGNAMC)
#   define  nvsgclsc FNAME(VSGCLSC)
#   define  nvsinqc  FNAME(VSINQC)
#   define  nvsfexc  FNAME(VSFEXC)
#   define  nvsfndc  FNAME(VSFNDC)
#   define  nvsgidc  FNAME(VSGIDC)
#   define  nvsdltc  FNAME(VSDLTC)
#   define  nvsapp   FNAME(VSAPP)
#   define  nvssnamc FNAME(VSSNAMC)
#   define  nvssclsc FNAME(VSSCLSC)
#   define  nvssfldc FNAME(VSSFLDC)
#   define  nvssintc FNAME(VSSINTC)
#   define  nvsfdefc FNAME(VSFDEFC)
#   define  nvssextfc FNAME(VSSEXTFC)
#   define  nvfnflds FNAME(VFNFLDS)
#   define  nvffname FNAME(VFFNAME)
#   define  nvfftype FNAME(VFFTYPE)
#   define  nvffisiz FNAME(VFFISIZ)
#   define  nvffesiz FNAME(VFFESIZ)
#   define  nvffordr FNAME(VFFORDR)
#   define  nvsfrdc  FNAME(VSFRDC)
#   define  nvsfrd   FNAME(VSFRD)
#   define  nvsreadc FNAME(VSREADC)
#   define  nvsfwrtc FNAME(VSFWRTC)
#   define  nvsfwrt  FNAME(VSFWRT)
#   define  nvswritc FNAME(VSWRITC)
#   define  nvsgintc FNAME(VSGINTC)
#   define  nvseltsc FNAME(VSELTSC)
#   define  nvsgfldc FNAME(VSGFLDC)
#   define  nvssizc  FNAME(VSSIZC)
#   define  nventsc  FNAME(VENTSC)
#   define  nvlonec  FNAME(VLONEC)
#   define  nvslonec FNAME(VSLONEC)
#   define  nvfindc  FNAME(VFINDC)
#   define  nvfndclsc FNAME(VFNDCLSC)
#   define  nvhsdc   FNAME(VHSDC)
#   define  nvhscdc  FNAME(VHSCDC)
#   define  nvhscdmc FNAME(VHSCDMC)
#   define  nvhsdmc  FNAME(VHSDMC)
#   define  nvhmkgpc FNAME(VHMKGPC)
#   define  nvffloc  FNAME(VFFLOC)
#   define  nvnrefs  FNAME(VNREFS)
#   define  nvqref   FNAME(VQREF)
#   define  nvqtag   FNAME(VQTAG)
#   define  nvinqtrc FNAME(VINQTRC)
#   define  nvntrc   FNAME(VNTRC)
#   define  nvgttrsc FNAME(VGTTRSC)
#   define  nvgttrc  FNAME(VGTTRC)
#   define  nvadtrc  FNAME(VADTRC)
#   define  nvsqintr FNAME(VSIQINTR)
#   define  nvsqfnelt   FNAME(VSQFNELT)
#   define  nvsqfintr   FNAME(VSQFINTR)
#   define  nvsqfldsc   FNAME(VSQFLDSC)
#   define  nvsqfvsiz   FNAME(VSQVSIZ)
#   define  nvsqnamec   FNAME(VSQNAMEC)
#   define  nvsfccpk    FNAME(VSFCCPK)
#   define  nvsfncpk    FNAME(VSFNCPK)
#  else /* PowerStation */
#   define  ndfivopn FNAME(DFIVOPN)
#   define  ndfvclos FNAME(DFVCLOS)
#   define  nvatchc  FNAME(VATCHC)
#   define  nvdtchc  FNAME(VDTCHC)
#   define  nvgnamc  FNAME(VGNAMC)
#   define  nvgclsc  FNAME(VGCLSC)
#   define  nvinqc   FNAME(VINQC)
#   define  nvdelete FNAME(VDELETE)
#   define  nvgidc   FNAME(VGIDC)
#   define  nvgnxtc  FNAME(VGNXTC)
#   define  nvsnamc  FNAME(VSNAMC)
#   define  nvsclsc  FNAME(VSCLSC)
#   define  nvinsrtc FNAME(VINSRTC)
#   define  nvisvgc  FNAME(VISVGC)
#   define  nvfistart FNAME(VFISTART)
#   define  nvfiend  FNAME(VFIEND)
#   define  nvisvsc  FNAME(VISVSC)
#   define  nvsatchc FNAME(VSATCHC)
#   define  nvsdtchc FNAME(VSDTCHC)
#   define  nvsiqref FNAME(VSIQREF)
#   define  nvsiqtag FNAME(VSIQTAG)
#   define  nvsigver FNAME(VSIGVER)
#   define  nvsseekc FNAME(VSSEEKC)
#   define  nvsgnamc FNAME(VSGNAMC)
#   define  nvsgclsc FNAME(VSGCLSC)
#   define  nvsinqc  FNAME(VSINQC)
#   define  nvsfexc  FNAME(VSFEXC)
#   define  nvsfndc  FNAME(VSFNDC)
#   define  nvsgidc  FNAME(VSGIDC)
#   define  nvsdltc  FNAME(VSDLTC)
#   define  nvsapp   FNAME(VSAPP)
#   define  nvssnamc FNAME(VSSNAMC)
#   define  nvssclsc FNAME(VSSCLSC)
#   define  nvssfldc FNAME(VSSFLDC)
#   define  nvssintc FNAME(VSSINTC)
#   define  nvsfdefc FNAME(VSFDEFC)
#   define  nvssextfc FNAME(VSSEXTFC)
#   define  nvfinflds FNAME(VFINFLDS)
#   define  nvfifnm  FNAME(VFIFNM)
#   define  nvfiftp  FNAME(VFIFTP)
#   define  nvfifisz FNAME(VFIFISZ)
#   define  nvfifesz FNAME(VFIFESZ)
#   define  nvfifodr FNAME(VFIFODR)
#   define  nvsfirdc FNAME(VSFIRDC)
#   define  nvsfird  FNAME(VSFIRD)
#   define  nvsreadc FNAME(VSREADC)
#   define  nvsfiwrc FNAME(VSFIWRC)
#   define  nvsfiwr  FNAME(VSFIWR)
#   define  nvswritc FNAME(VSWRITC)
#   define  nvsgintc FNAME(VSGINTC)
#   define  nvseltsc FNAME(VSELTSC)
#   define  nvsgfldc FNAME(VSGFLDC)
#   define  nvssizc  FNAME(VSSIZC)
#   define  nventsc  FNAME(VENTSC)
#   define  nvlonec  FNAME(VLONEC)
#   define  nvslonec FNAME(VSLONEC)
#   define  nvfindc  FNAME(VFINDC)
#   define  nvfndclsc FNAME(VFNDCLSC)
#   define  nvhsdc   FNAME(VHSDC)
#   define  nvhscdc  FNAME(VHSCDC)
#   define  nvhscdmc FNAME(VHSCDMC)
#   define  nvhsdmc  FNAME(VHSDMC)
#   define  nvhmkgpc FNAME(VHMKGPC)
#   define  nvflocc  FNAME(VFLOCC)
#   define  nvfirefs FNAME(VFIREFS)
#   define  nvfiqref  FNAME(VFIQREF)
#   define  nvfiqtag  FNAME(VFIQTAG)
#   define  nvinqtrc FNAME(VINQTRC)
#   define  nvntrc   FNAME(VNTRC)
#   define  nvgttrsc FNAME(VGTTRSC)
#   define  nvgttrc  FNAME(VGTTRC)
#   define  nvadtrc  FNAME(VADTRC)
#   define  nvsiqintr FNAME(VSIQINTR)
#   define  nvsiqnelt FNAME(VSIQNELT)
#   define  nvsqfldsc   FNAME(VSQFLDSC)
#   define  nvsiqvsz    FNAME(VSIQVSZ)
#   define  nvsqnamec   FNAME(VSQNAMEC)
#   define  nvsfccpk    FNAME(VSFCCPK)
#   define  nvsfncpk    FNAME(VSFNCPK)
#  endif
#else                    /* !DF_CAPFNAMES */
#  if !(defined INTEL86) && !(defined _WIN32)
#   define  ndfivopn FNAME(dfivopn)
#   define  ndfvclos FNAME(dfvclos)
#   define  nvatchc  FNAME(vatchc)
#   define  nvdtchc  FNAME(vdtchc)
#   define  nvgnamc  FNAME(vgnamc)
#   define  nvgclsc  FNAME(vgclsc)
#   define  nvinqc   FNAME(vinqc)
#   define  nvdelete FNAME(vdelete)
#   define  nvgidc   FNAME(vgidc)
#   define  nvgnxtc  FNAME(vgnxtc)
#   define  nvsnamc  FNAME(vsnamc)
#   define  nvsclsc  FNAME(vsclsc)
#   define  nvinsrtc FNAME(vinsrtc)
#   define  nvisvgc  FNAME(visvgc)
#   define  nvfstart FNAME(vfstart)
#   define  nvfend   FNAME(vfend)
#   define  nvisvsc  FNAME(visvsc)
#   define  nvsatchc FNAME(vsatchc)
#   define  nvsdtchc FNAME(vsdtchc)
#   define  nvsqref  FNAME(vsqref)
#   define  nvsqtag  FNAME(vsqtag)
#   define  nvsgver  FNAME(vsgver)
#   define  nvsseekc FNAME(vsseekc)
#   define  nvsgnamc FNAME(vsgnamc)
#   define  nvsgclsc FNAME(vsgclsc)
#   define  nvsinqc  FNAME(vsinqc)
#   define  nvsfexc  FNAME(vsfexc)
#   define  nvsfndc  FNAME(vsfndc)
#   define  nvsgidc  FNAME(vsgidc)
#   define  nvsdltc  FNAME(vsdltc)
#   define  nvsapp   FNAME(vsapp)
#   define  nvssnamc FNAME(vssnamc)
#   define  nvssclsc FNAME(vssclsc)
#   define  nvssfldc FNAME(vssfldc)
#   define  nvssintc FNAME(vssintc)
#   define  nvsfdefc FNAME(vsfdefc)
#   define  nvssextfc FNAME(vssextfc)
#   define  nvfnflds FNAME(vfnflds)
#   define  nvffname FNAME(vffname)
#   define  nvfftype FNAME(vfftype)
#   define  nvffisiz FNAME(vffisiz)
#   define  nvffesiz FNAME(vffesiz)
#   define  nvffordr FNAME(vffordr)
#   define  nvsfrdc  FNAME(vsfrdc)
#   define  nvsfrd   FNAME(vsfrd)
#   define  nvsreadc FNAME(vsreadc)
#   define  nvsfwrtc FNAME(vsfwrtc)
#   define  nvsfwrt  FNAME(vsfwrt)
#   define  nvswritc FNAME(vswritc)
#   define  nvsgintc FNAME(vsgintc)
#   define  nvseltsc FNAME(vseltsc)
#   define  nvsgfldc FNAME(vsgfldc)
#   define  nvssizc  FNAME(vssizc)
#   define  nventsc  FNAME(ventsc)
#   define  nvlonec  FNAME(vlonec)
#   define  nvslonec FNAME(vslonec)
#   define  nvfindc  FNAME(vfindc)
#   define  nvfndclsc FNAME(vfndclsc)
#   define  nvhscdc  FNAME(vhscdc)
#   define  nvhsdc   FNAME(vhsdc)
#   define  nvhscdmc  FNAME(vhscdmc)
#   define  nvhsdmc  FNAME(vhsdmc)
#   define  nvhmkgpc FNAME(vhmkgpc)
#   define  nvflocc  FNAME(vflocc)
#   define  nvinqtrc FNAME(vinqtrc)
#   define  nvntrc   FNAME(vntrc)
#   define  nvnrefs  FNAME(vnrefs)
#   define  nvgttrsc FNAME(vgttrsc)
#   define  nvqref   FNAME(vqref)
#   define  nvqtag   FNAME(vqtag)
#   define  nvgttrc  FNAME(vgttrc)
#   define  nvadtrc  FNAME(vadtrc)
#   define  nvfstart FNAME(vfstart)
#   define  nvfend   FNAME(vfend)
#   define  nvsqfnelt   FNAME(vsqfnelt)
#   define  nvsqfintr   FNAME(vsqfintr)
#   define  nvsqfldsc   FNAME(vsqfldsc)
#   define  nvsqfvsiz   FNAME(vsqfvsiz)
#   define  nvsqnamec   FNAME(vsqnamec)
#   define  nvsfccpk    FNAME(vsfccpk)
#   define  nvsfncpk    FNAME(vsfncpk)
#  else  /* PowerStation */
#   define  ndfivopn FNAME(dfivopn)
#   define  ndfvclos FNAME(dfvclos)
#   define  nvatchc  FNAME(vatchc)
#   define  nvdtchc  FNAME(vdtchc)
#   define  nvgnamc  FNAME(vgnamc)
#   define  nvgclsc  FNAME(vgclsc)
#   define  nvinqc   FNAME(vinqc)
#   define  nvdelete FNAME(vdelete)
#   define  nvgidc   FNAME(vgidc)
#   define  nvgnxtc  FNAME(vgnxtc)
#   define  nvsnamc  FNAME(vsnamc)
#   define  nvsclsc  FNAME(vsclsc)
#   define  nvinsrtc FNAME(vinsrtc)
#   define  nvisvgc  FNAME(visvgc)
#   define  nvfistart FNAME(vfistart)
#   define  nvfiend  FNAME(vfiend)
#   define  nvisvsc  FNAME(visvsc)
#   define  nvsatchc FNAME(vsatchc)
#   define  nvsdtchc FNAME(vsdtchc)
#   define  nvsiqref FNAME(vsiqref)
#   define  nvsiqtag FNAME(vsiqtag)
#   define  nvsigver FNAME(vsigver)
#   define  nvsseekc FNAME(vsseekc)
#   define  nvsgnamc FNAME(vsgnamc)
#   define  nvsgclsc FNAME(vsgclsc)
#   define  nvsinqc  FNAME(vsinqc)
#   define  nvsfexc  FNAME(vsfexc)
#   define  nvsfndc  FNAME(vsfndc)
#   define  nvsgidc  FNAME(vsgidc)
#   define  nvsdltc  FNAME(vsdltc)
#   define  nvsapp   FNAME(vsapp)
#   define  nvssnamc FNAME(vssnamc)
#   define  nvssclsc FNAME(vssclsc)
#   define  nvssfldc FNAME(vssfldc)
#   define  nvssintc FNAME(vssintc)
#   define  nvsfdefc FNAME(vsfdefc)
#   define  nvssextfc FNAME(vssextfc)
#   define  nvfinflds FNAME(vfinflds)
#   define  nvfifnm  FNAME(vfifnm)
#   define  nvfiftp  FNAME(vfiftp)
#   define  nvfifisz FNAME(vfifisz)
#   define  nvfifesz FNAME(vfifesz)
#   define  nvfifodr FNAME(vfifodr)
#   define  nvsfirdc FNAME(vsfirdc)
#   define  nvsfird  FNAME(vsfird)
#   define  nvsreadc FNAME(vsreadc)
#   define  nvsfiwrc FNAME(vsfiwrc)
#   define  nvsfiwr  FNAME(vsfiwr)
#   define  nvswritc FNAME(vswritc)
#   define  nvsgintc FNAME(vsgintc)
#   define  nvseltsc FNAME(vseltsc)
#   define  nvsgfldc FNAME(vsgfldc)
#   define  nvssizc  FNAME(vssizc)
#   define  nventsc  FNAME(ventsc)
#   define  nvlonec  FNAME(vlonec)
#   define  nvslonec FNAME(vslonec)
#   define  nvfindc  FNAME(vfindc)
#   define  nvfndclsc FNAME(vfndclsc)
#   define  nvhsdc   FNAME(vhsdc)
#   define  nvhscdc  FNAME(vhscdc)
#   define  nvhscdmc FNAME(vhscdmc)
#   define  nvhsdmc  FNAME(vhsdmc)
#   define  nvhmkgpc FNAME(vhmkgpc)
#   define  nvflocc  FNAME(vflocc)
#   define  nvfirefs FNAME(vfirefs)
#   define  nvfiqref  FNAME(vfiqref)
#   define  nvfiqtag  FNAME(vfiqtag)
#   define  nvinqtrc FNAME(vinqtrc)
#   define  nvntrc   FNAME(vntrc)
#   define  nvgttrsc FNAME(vgttrsc)
#   define  nvgttrc  FNAME(vgttrc)
#   define  nvadtrc  FNAME(vadtrc)
#   define  nvsiqintr FNAME(vsiqintr)
#   define  nvsiqnelt FNAME(vsiqnelt)
#   define  nvsqfldsc   FNAME(vsqfldsc)
#   define  nvsiqvsz   FNAME(vsiqvsz)
#   define  nvsqnamec   FNAME(vsqnamec)
#   define  nvsfccpk    FNAME(vsfccpk)
#   define  nvsfncpk    FNAME(vsfncpk)
#  endif
#endif                          /* DF_CAPFNAMES */
#endif                          /* VG_FNAMES */

    extern      FRETVAL(intf) ndfivopn
                (_fcd filename, intf  * acc_mode, intf  * defdds, intf  * namelen);

    extern      FRETVAL(intf) ndfvclos
                (intf  * file_id);

    extern      FRETVAL(intf) nvatchc
                (HFILEID  * f, intf  * vgid, _fcd accesstype);

    extern      FRETVAL(intf) nvdtchc
                (intf  * vkey);

    extern      FRETVAL(intf) nvgnamc
                (intf  * vkey, _fcd vgname);

    extern      FRETVAL(intf) nvgclsc
                (intf  * vkey, _fcd vgclass);

    extern      FRETVAL(intf) nvinqc
                (intf  * vkey, intf  * nentries, _fcd vgname);

    extern      FRETVAL(intf) nvdelete
                (intf  *f, intf  * vkey);

    extern      FRETVAL(intf) nvgidc
                (HFILEID  * f, intf  * vgid);

    extern      FRETVAL(intf) nvgnxtc
                (intf  * vkey, intf  * id);

    extern      FRETVAL(intf) nvsnamc
                (intf  * vkey, _fcd vgname, intf  * vgnamelen);

    extern      FRETVAL(intf) nvsclsc
                (intf  * vkey, _fcd vgclass, intf  * vgclasslen);

    extern      FRETVAL(intf) nvinsrtc
                (intf  * vkey, intf  * vobjptr);

    extern      FRETVAL(intf) nvisvgc
                (intf  * vkey, intf  * id);

    extern      FRETVAL(intf) nvfistart
                (intf  * f);

    extern      FRETVAL(intf) nvfiend
                (intf  * f);

    extern      FRETVAL(intf) nvisvsc
                (intf  * vkey, intf  * id);

    extern      FRETVAL(intf) nvsatchc
                (HFILEID  * f, intf  * vsref, _fcd accesstype);

    extern      FRETVAL(intf) nvsdtchc
                (intf  * vkey);

    extern      FRETVAL(intf) nvsqref
                (intf  * vkey);

    extern      FRETVAL(intf) nvsiqref
                (intf  * vkey);

    extern      FRETVAL(intf) nvsqtag
                (intf  * vkey);
  
    extern      FRETVAL(intf) nvsiqtag
                (intf  * vkey);
  
    extern      FRETVAL(intf) nvsigver
                (intf  * vkey);

    extern      FRETVAL(intf) nvsgver
                (intf  * vkey);

    extern      FRETVAL(intf) nvsseekc
                (intf  * vkey, intf  * eltpos);

    extern      FRETVAL(intf) nvsgnamc
                (intf  * vkey, _fcd vsname, intf *vsnamelen);

    extern      FRETVAL(intf) nvsgclsc
                (intf  * vkey, _fcd vsclass, intf *vsclasslen);

    extern      FRETVAL(intf) nvsinqc
                (intf  * vkey, intf  * nelt, intf  * interlace, _fcd fields,
		intf  * eltsize, _fcd vsname, intf *fieldslen, intf *vsnamelen);

    extern      FRETVAL(intf) nvsfexc
                (intf  * vkey, _fcd fields, intf  * fieldslen);

    extern      FRETVAL(intf) nvsfndc
                (HFILEID  * f, _fcd name, intf  * namelen);

    extern      FRETVAL(intf) nvsgidc
                (HFILEID  * f, intf  * vsref);

    extern      FRETVAL(intf) nvsdltc
                (HFILEID  * f, intf  * vsref);

    extern      FRETVAL(intf) nvsapp
                (intf  * vkey, intf  *blk);

    extern      FRETVAL(intf) nvssnamc
                (intf  * vkey, _fcd vsname, intf  * vsnamelen);

    extern      FRETVAL(intf) nvssclsc
                (intf  * vkey, _fcd vsclass, intf  * vsclasslen);

    extern      FRETVAL(intf) nvssfldc
                (intf  * vkey, _fcd fields, intf  * fieldslen);

    extern      FRETVAL(intf) nvssintc
                (intf  * vkey, intf  * interlace);

    extern      FRETVAL(intf) nvsfdefc
                (intf  * vkey, _fcd field, intf  * localtype,
                 intf  * order, intf  * fieldlen);

    extern      FRETVAL(intf) nvssextfc
                (intf  * vkey, _fcd fname, intf  * offset,
                 intf  * fnamelen);

    extern      FRETVAL(intf) nvfinflds
                (intf  * vkey);

    extern      FRETVAL(intf) nvfnflds
                (intf  * vkey);

    extern      FRETVAL(intf) nvffname
                (intf  * vkey, intf  *index, _fcd fname);

    extern      FRETVAL(intf) nvfifnm
                (intf  * vkey, intf  *index, _fcd fname);

    extern      FRETVAL(intf) nvfftype
                (intf  * vkey, intf  *index);

    extern      FRETVAL(intf) nvfiftp
                (intf  * vkey, intf  *index);

    extern      FRETVAL(intf) nvffisiz
                (intf  * vkey, intf  *index);

    extern      FRETVAL(intf) nvfifisz
                (intf  * vkey, intf  *index);

    extern      FRETVAL(intf) nvffesiz
                (intf  * vkey, intf  *index);

    extern      FRETVAL(intf) nvfifesz
                (intf  * vkey, intf  *index);

    extern      FRETVAL(intf) nvffordr
                (intf  * vkey, intf  *index);

    extern      FRETVAL(intf) nvfifodr
                (intf  * vkey, intf  *index);

    extern      FRETVAL(intf) nvsfrdc
                (intf  * vkey, _fcd  cbuf, intf  * nelt,
                 intf  * interlace);

    extern      FRETVAL(intf) nvsfirdc
                (intf  * vkey, _fcd  cbuf, intf  * nelt,
                 intf  * interlace);

    extern      FRETVAL(intf) nvsfrd
                (intf  * vkey, intf  * buf, intf  * nelt,
                 intf  * interlace);

    extern      FRETVAL(intf) nvsfird
                (intf  * vkey, intf  * buf, intf  * nelt,
                 intf  * interlace);

    extern      FRETVAL(intf) nvsreadc
                (intf  * vkey, uint8  * buf, intf  * nelt,
                 intf  * interlace);

    extern      FRETVAL(intf) nvsfwrtc
                (intf  * vkey, _fcd  cbuf, intf  * nelt,
                 intf  * interlace);

    extern      FRETVAL(intf) nvsfiwrc
                (intf  * vkey, _fcd  cbuf, intf  * nelt,
                 intf  * interlace);

    extern      FRETVAL(intf) nvsfwrt
                (intf  * vkey, intf  * buf, intf  * nelt,
                 intf  * interlace);

    extern      FRETVAL(intf) nvsfiwr
                (intf  * vkey, intf  * buf, intf  * nelt,
                 intf  * interlace);

    extern      FRETVAL(intf) nvswritc
                (intf  * vkey, uint8  * buf, intf  * nelt,
                 intf  * interlace);

    extern      FRETVAL(intf) nvsgintc
                (intf  * vkey);

    extern      FRETVAL(intf) nvseltsc
                (intf  * vkey);

    extern      FRETVAL(intf) nvsgfldc
                (intf  * vkey, _fcd fields);

    extern      FRETVAL(intf) nvssizc
                (intf  * vkey, _fcd fields, intf  * fieldslen);

    extern      FRETVAL(intf) nventsc
                (HFILEID  * f, intf  * vgid);

    extern      FRETVAL(intf) nvlonec
                (HFILEID  * f, intf  * idarray, intf  * asize);

    extern      FRETVAL(intf) nvslonec
                (HFILEID  * f, intf  * idarray, intf  * asize);

    extern      FRETVAL(intf) nvfindc
                (HFILEID  * f, _fcd name, intf  * namelen);

    extern      FRETVAL(intf) nvfndclsc
                (HFILEID  * f, _fcd vgclass, intf  * classlen);

    extern      FRETVAL(intf) nvhscdc
                (HFILEID  * f, _fcd field, _fcd  cbuf, intf  * n, 
                 intf  * datatype, _fcd vsname, _fcd vsclass, 
                 intf  * fieldlen, intf  * vsnamelen,
                 intf  * vsclasslen);

    extern      FRETVAL(intf) nvhsdc
                (HFILEID  * f, _fcd field, uint8  * buf, intf  * n, 
                 intf  * datatype, _fcd vsname, _fcd vsclass, 
                 intf  * fieldlen, intf  * vsnamelen,
                 intf  * vsclasslen);

    extern      FRETVAL(intf) nvhscdmc
                (HFILEID  * f, _fcd field, _fcd  cbuf, intf  * n,
                 intf  * datatype, _fcd vsname, _fcd vsclass, 
                 intf  * order, intf  * fieldlen, intf * vsnamelen,
                 intf  * vsclasslen);

    extern      FRETVAL(intf) nvhsdmc
                (HFILEID  * f, _fcd field, uint8  * buf, intf  * n,
                 intf  * datatype, _fcd vsname, _fcd vsclass, 
                 intf  * order, intf  * fieldlen, intf * vsnamelen,
                 intf  * vsclasslen);

    extern      FRETVAL(intf) nvhmkgpc
                (HFILEID  * f, intf  * tagarray, intf  * refarray, intf  * n,
                 _fcd vgname, _fcd vgclass, intf  * vgnamelen, intf  * vgclasslen);

    extern      FRETVAL(intf) nvflocc
                (intf  * vkey, _fcd field, intf  * fieldlen);

    extern      FRETVAL(intf) nvinqtrc
                (intf  * vkey, intf  * tag, intf  * ref);

    extern      FRETVAL(intf) nvntrc
                (intf  * vkey);

    extern      FRETVAL(intf) nvnrefs
                (intf  * vkey, intf  *tag);

    extern      FRETVAL(intf) nvfirefs
                (intf  * vkey, intf  *tag);

    extern      FRETVAL(intf) nvqref
                (intf  * vkey);

    extern      FRETVAL(intf) nvfiqref
                (intf  * vkey);

    extern      FRETVAL(intf) nvqtag
                (intf  * vkey);

    extern      FRETVAL(intf) nvfiqtag
                (intf  * vkey);

    extern      FRETVAL(intf) nvgttrsc
                (intf  * vkey, intf  * tagarray, intf  * refarray, intf  * n);

    extern      FRETVAL(intf) nvgttrc
                (intf  * vkey, intf  * which, intf  * tag, intf  * ref);

    extern      FRETVAL(intf) nvadtrc
                (intf  * vkey, intf  * tag, intf  * ref);

    extern      FRETVAL(intf) nvsiqnelt
                (intf * vkey, intf *nelt);

    extern      FRETVAL(intf) nvsiqintr
                (intf * vkey, intf *interlace);

    extern      FRETVAL(intf) nvsqfldsc
                (intf * vkey, _fcd fields, intf *fieldslen);

    extern      FRETVAL(intf) nvsiqvsz
                (intf * vkey, intf *ret_size);

    extern      FRETVAL(intf) nvsqnamec
                (intf * vkey, _fcd name, intf *namelen);

    extern      FRETVAL(intf) nvsfccpk
                (intf * vkey, intf *packtype, _fcd buflds, intf *buf,
                intf *bufsz,intf *nrecs, _fcd pckfld, _fcd fldbuf, 
                intf *buflds_len, intf *fld_len);

    extern      FRETVAL(intf) nvsfncpk
                (intf * vkey, intf *packtype, _fcd buflds, intf *buf,
                 intf *bufsz, intf *nrecs, _fcd pckfld, intf *fldbuf, 
                 intf *buflds_len, intf *fld_len);


/* 
  ** from vattrf.c
 */
#ifndef VATTR_FNAMES
#  define VATTR_FNAMES
#ifdef DF_CAPFNAMES
# if !(defined INTEL86) && !(defined _WIN32)
#  define nvsfcfdx   FNAME(VSFCFDX)
#  define nvsfcsat   FNAME(VSFCSAT)
#  define nvsfcsca   FNAME(VSFCSCA)
#  define nvsfnats   FNAME(VSFNATS)
#  define nvsffnas   FNAME(VSFFNAS)
#  define nvsfcfda   FNAME(VSFCFDA)
#  define nvsfainf   FNAME(VSFAINF)
#  define nvsfgnat   FNAME(VSFGNAT)
#  define nvsfgcat   FNAME(VSFGCAT)
#  define nvsfisat   FNAME(VSFISAT)
#  define nvfcsatt   FNAME(VFCSATT)
#  define nvfcscat   FNAME(VFCSCAT)
#  define nvfnatts   FNAME(VFNATTS)
#  define nvfcfdat   FNAME(VFCFDAT)
#  define nvfainfo   FNAME(VFAINFO)
#  define nvfgnatt   FNAME(VFGNATT)
#  define nvfgcatt   FNAME(VFGCATT)
#  define nvfgver    FNAME(VFGVER)
# else  /* PowerStation */
#  define nvsfcfdx   FNAME(VSFCFDX)
#  define nvsfcsat   FNAME(VSFCSAT)
#  define nvsfcsca   FNAME(VSFCSCA)
#  define nvsfcnats  FNAME(VSFCNATS)
#  define nvsfcfnas  FNAME(VSFCFNAS)
#  define nvsfcfda   FNAME(VSFCFDA)
#  define nvsfcainf  FNAME(VSFCAINF)
#  define nvsfcgna   FNAME(VSFCGNA)
#  define nvsfcgca   FNAME(VSFCGCA)
#  define nvsfcisa   FNAME(VSFCISA)
#  define nvfcsatt   FNAME(VFCSATT)
#  define nvfcscat   FNAME(VFCSCAT)
#  define nvfcnats   FNAME(VFCNATS)
#  define nvfcfdat   FNAME(VFCFDAT)
#  define nvfcainf   FNAME(VFCAINF)
#  define nvfcgnat   FNAME(VFCGNAT)
#  define nvfcgcat   FNAME(VFCGCAT)
#  define nvfcgver   FNAME(VFCGVER)
# endif
#else       /* !DF_CAPFNAMES  */
# if !(defined INTEL86) && !(defined _WIN32)
#  define nvsfcfdx   FNAME(vsfcfdx)
#  define nvsfcsat   FNAME(vsfcsat)
#  define nvsfcsca   FNAME(vsfcsca)
#  define nvsfnats   FNAME(vsfnats)
#  define nvsffnas   FNAME(vsffnas)
#  define nvsfcfda   FNAME(vsfcfda)
#  define nvsfainf   FNAME(vsfainf)
#  define nvsfgnat   FNAME(vsfgnat)
#  define nvsfgcat   FNAME(vsfgcat)
#  define nvsfisat   FNAME(vsfisat)
#  define nvfcsatt   FNAME(vfcsatt)
#  define nvfcscat   FNAME(vfcscat)
#  define nvfnatts   FNAME(vfnatts)
#  define nvfcfdat   FNAME(vfcfdat)
#  define nvfainfo   FNAME(vfainfo)
#  define nvfgnatt   FNAME(vfgnatt)
#  define nvfgcatt   FNAME(vfgcatt)
#  define nvfgver    FNAME(vfgver)
# else  /* PowerStation */
#  define nvsfcfdx   FNAME(vsfcfdx)
#  define nvsfcsat   FNAME(vsfcsat)
#  define nvsfcsca   FNAME(vsfcsca)
#  define nvsfcnats  FNAME(vsfcnats)
#  define nvsfcfnas  FNAME(vsfcfnas)
#  define nvsfcfda   FNAME(vsfcfda)
#  define nvsfcainf  FNAME(vsfcainf)
#  define nvsfcgna   FNAME(vsfcgna)
#  define nvsfcgca   FNAME(vsfcgca)
#  define nvsfcisa   FNAME(vsfcisa)
#  define nvfcsatt   FNAME(vfcsatt)
#  define nvfcscat   FNAME(vfcscat)
#  define nvfcnats   FNAME(vfcnats)
#  define nvfcfdat   FNAME(vfcfdat)
#  define nvfcainf   FNAME(vfcainf)
#  define nvfcgnat   FNAME(vfcgnat)
#  define nvfcgcat   FNAME(vfcgcat)
#  define nvfcgver   FNAME(vfcgver)
# endif
#endif   /* DF_CAPFNAMES */
#endif   /* VATTR_FNAMES  */
   extern   FRETVAL(intf) nvsfcfdx
            (intf *vsid, _fcd fldnm, intf *findex,
             intf *fldnmlen);
   extern   FRETVAL(intf) nvsfcsat
            (intf *vsid, intf *findex, _fcd attrnm, intf *dtype,
             intf *count, intf *values, intf *attrnmlen);
   extern   FRETVAL(intf) nvsfcsca
            (intf *vsid, intf *findex, _fcd attrnm, intf *dtype,
             intf *count, _fcd values, intf *attrnmlen);
   extern   FRETVAL(intf) nvsfnats
            (intf *vsid);
   extern   FRETVAL(intf) nvsfcnats
            (intf *vsid);
   extern   FRETVAL(intf) nvsffnas
            (intf *vsid, intf *findex);
   extern   FRETVAL(intf) nvsfcfnas
            (intf *vsid, intf *findex);
   extern   FRETVAL(intf) nvsfcfda
            (intf *vsid, intf *findex, _fcd attrnm, intf *attrnmlen);
   extern   FRETVAL(intf) nvsfainf
            (intf *vsid, intf *findex, intf *aindex, _fcd attrname,
             intf *dtype, intf *count, intf *size);
   extern   FRETVAL(intf) nvsfcainf
            (intf *vsid, intf *findex, intf *aindex, _fcd attrname,
             intf *dtype, intf *count, intf *size);
   extern   FRETVAL(intf) nvsfgnat
            (intf *vsid, intf *findex, intf *aindex, intf *values);
   extern   FRETVAL(intf) nvsfcgna
            (intf *vsid, intf *findex, intf *aindex, intf *values);
   extern   FRETVAL(intf) nvsfgcat
            (intf *vsid, intf *findex, intf *aindex, _fcd values);
   extern   FRETVAL(intf) nvsfcgca
            (intf *vsid, intf *findex, intf *aindex, _fcd values);
   extern   FRETVAL(intf) nvsfisat
            (intf *vsid);
   extern   FRETVAL(intf) nvsfcisa
            (intf *vsid);
   extern   FRETVAL(intf) nvfcsatt
            (intf *vgid, _fcd attrnm, intf *dtype,
             intf *count, intf *values, intf *attrnmlen);
   extern   FRETVAL(intf) nvfcscat
            (intf *vgid, _fcd attrnm, intf *dtype, intf *count,
             _fcd values, intf *attrnmlen);
   extern   FRETVAL(intf) nvfnatts
            (intf *vgid);
   extern   FRETVAL(intf) nvfcnats
            (intf *vgid);
   extern   FRETVAL(intf) nvfcfdat
            (intf *vgid, _fcd attrnm, intf *attrnmlen);
   extern   FRETVAL(intf) nvfainfo
            (intf *vgid, intf *aindex, _fcd attrname,
             intf *dtype, intf *count, intf *size);
   extern   FRETVAL(intf) nvfcainf
            (intf *vgid, intf *aindex, _fcd attrname,
             intf *dtype, intf *count, intf *size);
   extern   FRETVAL(intf) nvfgnatt
            (intf *vgid, intf *aindex, intf *values);
   extern   FRETVAL(intf) nvfcgnat
            (intf *vgid, intf *aindex, intf *values);
   extern   FRETVAL(intf) nvfgcatt
            (intf *vgid, intf *aindex, _fcd values);
   extern   FRETVAL(intf) nvfcgcat
            (intf *vgid, intf *aindex, _fcd values);
   extern   FRETVAL(intf) nvfgver
            (intf *vgid);
   extern   FRETVAL(intf) nvfcgver
            (intf *vgid);

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif                          /* _H_PROTO */

