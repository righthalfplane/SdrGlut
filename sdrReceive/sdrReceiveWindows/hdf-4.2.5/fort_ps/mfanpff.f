C********************************************************************
C* NCSA HDF
C* Software Development Group
C* National Center for Supercomputing Applications
C* University of Illinois at Urbana-Champaign
C* 605 E. Springfield, Champaign IL 61820
C*
C* For conditions of distribution and use, see the accompanying
C* hdf/COPYING file.
C*
C***********************************************************************
C $Id: mfanpff.f 4995 2007-11-05 21:34:07Z swegner $
C
C-----------------------------------------------------------------------
C File:     mfanpff.f
C Purpose:  Fortran stubs for Fortran PowerStation AN routines
C Invokes:  mfanpf.c
C Contents:
C  afstart    - start annotation access on file and return annotaton id
C  affileinfo - get number of file/data annotations in file.
C                 Indices returned are used in afselect() calls.
C  afend      - end access to annotation handling on file
C  afcreate   - create a new data annotation and return a handle
C  affcreate  - create a new file annotation and return a handle
C  afselect   - returns an annotation handle(ann_id) from index for
C             a particular annotation TYPE. This handle is then used for
C              calls like afwriteann(), afreadann(), afannlen(),..etc
C  afnumann   - return number of annotations that match TYPE/tag/ref
C  afannlist  - return list of handles that match TYPE/tag/ref
C  afannlen   - get length of annotation given handle
C  afwriteann - write annotation given handle
C  afreadann  - read annotation given handle
C  afendaccess - end access to annotation using handle
C  afgettagref - get tag/ref pair to annotation ID
C  afidtagref  - get tag/ref given annotation id
C  aftagrefid  - get annotation id given tag/ref
C  afatypetag  - annotation type to corresponding annotation TAG
C  aftagatype  - annotation TAG to corresponding annotation type
C
C Remarks: none
C------------------------------------------------------------------
C----------------------------------------------------------------
C Name: afstart
C Purpose: Open file for annoation handling
C Inputs:  file_id: id of HDF file
C Returns: annotation interface handle on SUCCEED and FAIL otherwise
C Users:
C Invokes:  afistart
C----------------------------------------------------------------

      integer function afstart(file_id)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: afstart
	!MS$endif
      integer file_id
C     integer  afistart
      INTERFACE
         INTEGER FUNCTION afistart(file_id)
  	    !MS$ATTRIBUTES C,reference,decorate,alias:'AFISTART' :: afistart
            integer file_id
         END FUNCTION afistart
      END INTERFACE

      afstart = afistart(file_id)
      return
      end
C----------------------------------------------------------------
C Name:    affileinfo
C Purpose: Get number of file/data annotations in file.
C Inputs:  IN an_id:     annotation interface handle
C          OUT num_flabel: number of file labels in file
C          OUT num_fdesc:  number of file descriptions in file
C          OUT num_olabel: number of data labels in file
C          OUT num_odesc:  number of data descriptions in file
C Returns: see ANfileinfo()
C Users:   Fortran Users
C Invokes: afifinf()
C----------------------------------------------------------------

      integer function affileinfo(an_id, num_flabel, num_fdesc,
     +                 num_olabel, num_odesc)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: affileinfo
	!MS$endif
      integer an_id,num_flabel,num_fdesc,num_olabel,num_odesc
C     integer  afifinf
      INTERFACE
         INTEGER FUNCTION afifinf(an_id,n_flable,n_fdesc,
     +                      n_olabel, n_odesc) 
  	    !MS$ATTRIBUTES C,reference,decorate,alias:'AFIFINF' :: afifinf
            integer an_id, n_flable,n_fdesc,n_olabel, n_odesc
         END FUNCTION afifinf
      END INTERFACE

      affileinfo = afifinf(an_id,num_flabel,num_fdesc,
     +                 num_olabel,num_odesc)
      return
      end
C----------------------------------------------------------------
C Name: afend
C Purpose: End access to annotation handling on file
C Inputs:  file_id:
C Returns: see ANend()
C Users:   Fortran Users
C Invokes: afiend()
C----------------------------------------------------------------

      integer function afend(an_id)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: afend
	!MS$endif
      integer an_id
C     integer  afiend
      INTERFACE
         INTEGER FUNCTION afiend(an_id)
  	    !MS$ATTRIBUTES C,reference,decorate,alias:'AFIEND' :: afiend
            integer an_id
         END FUNCTION afiend
      END INTERFACE

      afend = afiend(an_id)
      return
      end
C----------------------------------------------------------------
C Name: afcreate
C Purpose:  Create a new data annotation and return an annotation handle
C Inputs:
C          an_id: annotation interface handle
C          etag:    tag of data to annotate
C          eref:    ref of data to annotate
C          atype:   annotation type AN_DATA_LABEL, AN_DATA_DESC
C Returns: see ANcreate()
C Users:   Fortran Users
C Invokes: aficreat()
C----------------------------------------------------------------

      integer function afcreate(an_id, etag, eref, atype)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: afcreate
	!MS$endif
      integer an_id, etag, eref, atype
C     integer  aficreat
      INTERFACE
         INTEGER FUNCTION aficreat(an_id, etag, eref, atype)
  	    !MS$ATTRIBUTES C,reference,decorate,alias:'AFICREAT' :: aficreat
            integer an_id, etag, eref, atype
         END FUNCTION aficreat
      END INTERFACE

      afcreate = aficreat(an_id, etag, eref, atype)
      return
      end
C----------------------------------------------------------------
C Name: affcreate
C Purpose: Create a new file annotation and return an annotation handle
C Inputs:  an_id: annottion inteface handle
C          atype:   annotation type AN_FILE_LABEL, AN_DATA_DESC
C Returns: see ANcreatef()
C Users:   Fortran Users
C Invokes: afifcreat()
C----------------------------------------------------------------

      integer function affcreate(an_id,  atype)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: affcreate
	!MS$endif
      integer an_id, atype
C     integer  afifcreat
      INTERFACE
         INTEGER FUNCTION afifcreat(an_id, atype)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'AFIFCREAT' :: afifcreat
            integer an_id, atype
         END FUNCTION afifcreat
      END INTERFACE

      affcreate = afifcreat(an_id, atype)
      return
      end
C---------------------------------------------------------------
C Name:    afselect
C Purpose: returns an annotation handle(an_id) from index for
C          a particular annotation TYPE. This handle is then 
C          used for calls like afwriteann(), afreadann(), 
C          afannlen(),..etc
C Inputs:  an_id: annotation interface handle
C          index: index for particular annoation type. Usually 
C                 based on number of a particular type 
C                 obtained from affileinfo()call. ZERO based.
C          atype: annotation type AN_FILE_LABEL, AN_FILE_DESC, 
C                 AN_DATA_LABEL, AN_DATA_DESC
C Returns: see ANselect()
C Users:   Fortran Users
C Invokes: afiselct()
C----------------------------------------------------------------

      integer function afselect(an_id, index, atype)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: afselect
	!MS$endif
      integer an_id, index, atype
C     integer  afiselct
      INTERFACE
         INTEGER FUNCTION afiselct(an_id, index, atype)
  	    !MS$ATTRIBUTES C,reference,decorate,alias:'AFISELCT' :: afiselct
            integer an_id, index, atype
         END FUNCTION afiselct
      END INTERFACE

      afselect = afiselct(an_id, index, atype)
      return
      end

C-------------------------------------------------------------
C Name:    afnumann
C Purpose: Return number of annotations that match TYPE/tag/ref
C Inputs:  an_id: annotation interface handle
C          atype:   annotation type AN_DATA_LABEL, AN_DATA_DESC
C          etag:    data tag to match
C          eref:    data ref to match
C Returns: see ANnumann()
C Users:   Fortran Users
C Invokes: afinann()
C-------------------------------------------------------------
      integer function afnumann(an_id, atype, etag, eref)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: afnumann
	!MS$endif
      integer an_id, atype, etag, eref
C      integer afinann
      INTERFACE
         INTEGER FUNCTION afinann(an_id, atype, etag, eref)
  	    !MS$ATTRIBUTES C,reference,decorate,alias:'AFINANN' :: afinann
            integer an_id, atype, etag, eref
         END FUNCTION afinann
      END INTERFACE
      afnumann = afinann(an_id, atype, etag, eref)
      return
      end

C-------------------------------------------------------------
C Name:    afannlist
C Purpose: Return list of handles that match TYPE/tag/ref
C Inputs:  IN an_id: annotation inteface handle
C          IN atype:   annotation type AN_DATA_LABEL, 
C                      AN_DATA_DESC
C          IN etag:    data tag to match
C          IN eref:    data ref to match
C          OUT alist[]: list of annotation handles found that 
C                       match tag/ref
C Returns: number of annoations found that match else FAIL. 
C          see ANannlist()
C Users:   Fortran Users
C Invokes: anialst
C-------------------------------------------------------------
      integer function afannlist(an_id,atype,etag,eref,alist)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: afannlist
	!MS$endif
      integer an_id, atype, etag, eref, alist(*)
C      integer afialst
      INTERFACE
         INTEGER FUNCTION afialst(an_id,atype,etag,eref,alist)
  	    !MS$ATTRIBUTES C,reference,decorate,alias:'AFIALST' :: afialst
            integer an_id, atype, etag, eref, alist(*)
         END FUNCTION afialst
      END INTERFACE
      afannlist = afialst(an_id, atype, etag, eref,alist)
      return
      end
C-------------------------------------------------------------
C Name:    afannlen
C Purpose: Get length of annotation given handle
C Inputs:  ann_id:annotation handle
C Returns: see ANannlen()
C Users:   Fortran Users
C Invokes: afialen()
C---------------------------------------------------------------------
      integer function afannlen(ann_id)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: afannlen
	!MS$endif
      integer ann_id
C      integer afialen
      INTERFACE
         INTEGER FUNCTION afialen(ann_id)
  	    !MS$ATTRIBUTES C,reference,decorate,alias:'AFIALEN' :: afialen
            integer ann_id
         END FUNCTION afialen
      END INTERFACE
      afannlen = afialen(ann_id)
      return
      end
C--------------------------------------------------------------
C Name:    afwriteann
C Purpose: Write annotation given handle
C Inputs:  ann_id: annotation handle
C          ann:   annotation to write out
C          annlen:length of annotation
C Returns: see ANwriteann()
C Users:   Fortran Users
C Invokes: aniwann()
C--------------------------------------------------------------
      integer function afwriteann(ann_id,ann,annlen)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: afwriteann
	!MS$endif
      integer ann_id, annlen
      character*(*) ann
C      integer afiwann
      INTERFACE
         INTEGER FUNCTION afiwann(ann_id, ann, annlen)
  	    !MS$ATTRIBUTES C,reference,decorate,alias:'AFIWANN' :: afiwann
   	      !DEC$ ATTRIBUTES reference :: ann
            integer ann_id, annlen
            character*(*) ann
         END FUNCTION afiwann
      END INTERFACE
      afwriteann = afiwann(ann_id, ann, annlen)
      return
      end

C--------------------------------------------------------------
C Name:    afreadann
C Purpose: Read annotation given handle
C Inputs:  ann_id: annotation handle
C          ann:   annotation read
C          maxlen:maximum space allocated for "ann" 
C Returns: see ANreadann(), SUCCEED (0) if successful, else FAIL
C Users:   Fortran Users
C Invokes: anirann()
C--------------------------------------------------------------
      integer function afreadann(ann_id,ann,maxlen)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: afreadann
	!MS$endif
      integer ann_id, maxlen
      character*(*) ann
C      integer afirann
      INTERFACE
         INTEGER FUNCTION afirann(ann_id, ann, maxlen)
  	    !MS$ATTRIBUTES C,reference,decorate,alias:'AFIRANN' :: afirann
	      !DEC$ ATTRIBUTES reference :: ann
            integer ann_id, maxlen
            character*(*) ann
         END FUNCTION afirann
      END INTERFACE
      afreadann = afirann(ann_id, ann, maxlen)
      return
      end
C-------------------------------------------------------------
C Name:    afendaccess
C Purpose: End access to annotation using handle
C Inputs:  ann_id:annotation handle
C Returns: see ANendaccess()
C Users:   Fortran Users
C Invokes: aniendac()
C------------------------------------------------------------
      integer function afendaccess(ann_id)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: afendaccess
	!MS$endif
      integer ann_id
C      integer afiendac
      INTERFACE
         INTEGER FUNCTION afiendac(ann_id)
  	    !MS$ATTRIBUTES C,reference,decorate,alias:'AFIENDAC' :: afiendac
            integer ann_id
         END FUNCTION afiendac
      END INTERFACE
      afendaccess = afiendac(ann_id)
      return
      end

C------------------------------------------------------------
C Name:    afgettagref
C Purpose: get tag/ref pair to annotation ID
C Inputs: int32 an_id  IN: annotation interface ID 
C         int32 index  IN: index of annottion to get tag/ref for
C         ann_type type IN: AN_DATA_LABEL for data labels,
C                           AN_DATA_DESC for data descriptions,
C                           AN_FILE_LABEL for file labels,
C                           AN_FILE_DESC for file descritpions.
C         uint16 *tag  OUT: Tag for annotation 
C         uint16 *ref  OUT: ref for annotation 
C Returns: see ANget_tagref(), tag/ref pair
C Users:   Fortran Users
C Invokes: afigtr
C-----------------------------------------------------------
      integer function afgettagref(an_id,index,type,tag,ref)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: afgettagref
	!MS$endif
      integer an_id,index,type,tag,ref
C      integer afigtr
      INTERFACE
         INTEGER FUNCTION afigtr(an_id,index,type,tag,ref)
  	    !MS$ATTRIBUTES C,reference,decorate,alias:'AFIGTR' :: afigtr
            integer an_id,index,type,tag,ref
         END FUNCTION afigtr
      END INTERFACE
      afgettagref = afigtr(an_id,index,type,tag,ref)
      return
      end
C-----------------------------------------------------------
C Name:    afidtagref
C Purpose: get tag/ref given annotation id
C Inputs: int32 ann_id IN: annotation id 
C            uint16 *tag OUT: Tag for annotation 
C            uint16 *ref OUT: ref for annotation 
C Returns: see ANid2tagref(),SUCCEED(0) if successful 
C              and FAIL (-1) otherwise
C Users:   Fortran Users
C Invokes: afiid2tr()
C------------------------------------------------------------
      integer function afidtagref(ann_id,tag,ref)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: afidtagref
	!MS$endif
      integer ann_id,tag,ref
C      integer afiid2tr
      INTERFACE
         INTEGER FUNCTION afiid2tr(ann_id,tag,ref)
  	    !MS$ATTRIBUTES C,reference,decorate,alias:'AFIID2TR' :: afiid2tr
            integer ann_id,tag,ref
         END FUNCTION afiid2tr
      END INTERFACE
      afidtagref = afiid2tr(ann_id,tag,ref)
      return
      end
C-----------------------------------------------------------
C Name:    aftagrefid
C Purpose: get annotation id given tag/ref
C Inputs: int32 an_id   IN  Annotation interface id 
C            uint16 ann_tag IN: Tag for annotation
C            uint16 ann_ref IN: ref for annotation 
C Returns: see ANtagref2id(),annotation id if succeed, 
C             FAIL otherwise.
C Users:   Fortran Users
C Invokes: afitr2id()
C------------------------------------------------------------
      integer function aftagrefid(an_id,tag,ref)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: aftagrefid
	!MS$endif
      integer an_id,tag,ref
C      integer afitr2id
      INTERFACE
         INTEGER FUNCTION afitr2id(an_id,tag,ref)
  	    !MS$ATTRIBUTES C,reference,decorate,alias:'AFITR2ID' :: afitr2id
            integer an_id,tag,ref
         END FUNCTION afitr2id
      END INTERFACE
      aftagrefid = afitr2id(an_id,tag,ref)
      return
      end

C-----------------------------------------------------------
C Name:    afatypetag
C Purpose: Translate annotation type to corresponding TAG.
C Inputs: integer atype   IN  Annotation type 
C Returns: see ANtype2tag(), TAG corresponding to anno type.
C Users:   Fortran Users
C Invokes: afitp2tg()
C------------------------------------------------------------
      integer function afatypetag(atype)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: afatypetag
	!MS$endif
      integer atype
C      integer afitp2tg
      INTERFACE
         INTEGER FUNCTION afitp2tg(atype)
  	    !MS$ATTRIBUTES C,reference,decorate,alias:'AFITP2TG' :: afitp2tg
            integer atype
         END FUNCTION afitp2tg
      END INTERFACE
      afatypetag = afitp2tg(atype)
      return
      end
C-----------------------------------------------------------
C Name:    aftagatype
C Purpose: anno TAG to corresponding anno type
C Inputs: int32 tag   IN  Annotation TAG
C Returns: see ANtag2atype(),TAG id if succeed, 
C             FAIL otherwise.
C Users:   Fortran Users
C Invokes: afitg2tp()
C------------------------------------------------------------
      integer function aftagatype(tag)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: aftagatype
	!MS$endif
      integer tag
C      integer afitg2tp
      INTERFACE
         INTEGER FUNCTION afitg2tp(tag)
  	    !MS$ATTRIBUTES C,reference,decorate,alias:'AFITG2TP' :: afitg2tp
            integer tag
         END FUNCTION afitg2tp
      END INTERFACE
      aftagatype = afitg2tp(tag)
      return
      end
