C****************************************************************************
C* NCSA HDF                                                                 *
C* Software Development Group                                               *
C* National Center for Supercomputing Applications                          *
C* University of Illinois at Urbana-Champaign                               *
C* 605 E. Springfield, Champaign IL 61820                                   *
C*                                                                          *
C* For conditions of distribution and use, see the accompanying             *
C* hdf/COPYING file.                                                        *
C*                                                                          *
C****************************************************************************
C
C $Id: hfilepff.f 4995 2007-11-05 21:34:07Z swegner $
C
C------------------------------------------------------------------------------
C File:     hfilepFf.f, based on hfileff.f,v 1.7
C Purpose:  Fortran stubs for H Fortran PowerStation routines
C Invokes:  hfilepF.c 
C Contents: 
C   hopen:          Call hiopen to open file
C hnumber:          Call hnumber
C Remarks: none
C--------------------------------------------------------------


C------------------------------------------------------------------
C Name: hopen
C Purpose:  call hiopen, open file
C Inputs:   path: Name of file to be opened
C           access: DFACC_READ, DFACC_WRITE, DFACC_CREATE,
C                      or any bitwise-or of the above.
C           ndds: Number of dds in header block if file needs to be created.
C Returns: 0 on success, FAIL on failure with error set
C Users:    Fortran stub routine
C Invokes: hiopen
C-------------------------------------------------------------------

      integer function hopen(filename, access, defdds)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: hopen
	!MS$endif

      character*(*) filename
      integer       access, defdds
C      integer       hiopen
      INTERFACE 
        INTEGER FUNCTION hiopen(filename,access, defdds, nmlen)
	    !MS$ATTRIBUTES C, reference, decorate,alias: 'HIOPEN' :: hiopen
	    !DEC$ ATTRIBUTES reference :: filename
          integer access, defdds, nmlen
          character*(*) filename
        END FUNCTION hiopen
      END INTERFACE

      hopen = hiopen(filename, access, defdds, len(filename))
      return
      end

C------------------------------------------------------------------
C Name: hclose
C Purpose:  call hiclose, close file
C           fid:  handle to HDF file to close
C Returns: 0 on success, FAIL on failure with error set
C Users:    Fortran stub routine
C Invokes: hiclose
C-------------------------------------------------------------------

      integer function hclose(fid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: hclose
	!MS$endif

      integer       fid
C      integer       hiclose
      INTERFACE 
        INTEGER FUNCTION hiclose(fid)
	    !MS$ATTRIBUTES C, reference, decorate,alias: 'HICLOSE' :: hiclose
          integer fid
        END FUNCTION hiclose
      END INTERFACE

      hclose = hiclose(fid)
      return
      end

C------------------------------------------------------------------
C Name: hnumber
C Purpose:  call hinumbr, get number of elements with tag 
C           fid:  handle to HDF file to close
C           tag: the tag which the elements have
C Returns: number of element on success, FAIL on failure with error set
C Users:    Fortran stub routine
C Invokes: hinumbr
C-------------------------------------------------------------------

      integer function hnumber(fid, tag)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: hnumber
	!MS$endif

      integer       fid, tag
C      integer       hinumbr
      INTERFACE 
        INTEGER FUNCTION hinumbr(fid, tag)
	    !MS$ATTRIBUTES C, reference, decorate,alias: 'HINUMBR' :: hinumbr
          integer fid, tag
        END FUNCTION hinumbr
      END INTERFACE

      hnumber = hinumbr(fid, tag)
      return
      end

C--------------------------------------------------------------------
C Name: hxsdir
C Purpose:  call hxisdir to set directory variable for locating an external file
C Inputs:   dir: names of directory separated by colons.
C Returns:  SUCCEED if no error, else FAIL
C Users:    Fortran stub routine
C Invokes: hxisdir
C------------------------------------------------------------------

      integer function hxsdir(dir)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: hxsdir
	!MS$endif

      character*(*) dir
C      integer       hxisdir
      INTERFACE
        INTEGER FUNCTION hxisdir(dir, dirlen)
	    !MS$ATTRIBUTES C, reference, decorate,alias: 'HXISDIR' :: hxisdir
	    !DEC$ ATTRIBUTES reference :: dir
          character*(*) dir
          integer dirlen
        END FUNCTION hxisdir
      END INTERFACE

      hxsdir = hxisdir(dir, len(dir))
      return
      end

C---------------------------------------------------------------------
C Name: hxscdir
C Purpose:  call hxiscdir to set directory variable for creating an external file
C Inputs:   dir: name of the directory
C Returns:  SUCCEED if no error, else FAIL
C Users:    Fortran stub routine
C Invokes: hxiscdir
C--------------------------------------------------------------*/

      integer function hxscdir(dir)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: hxscdir
	!MS$endif

      character*(*) dir
C      integer       hxiscdir
      INTERFACE
        INTEGER FUNCTION hxiscdir(dir, dirlen)
	    !MS$ATTRIBUTES C, reference, decorate,alias: 'HXISCDIR' :: hxiscdir
	    !DEC$ ATTRIBUTES reference :: dir
          character*(*) dir
          integer dirlen
        END FUNCTION hxiscdir
      END INTERFACE

      hxscdir = hxiscdir(dir, len(dir))
      return
      end

C-----------------------------------------------------------------------------
C Name: hglibver
C Purpose: retrieves the version information for the current HDF library
C Outputs: major_v - major version number
C          minor_v - minor version number
C          release - release number
C          string  - version number test string
C Retruns: SUCCEED (0) if successful and FAIL(-1) otherwise
C-----------------------------------------------------------------------------*/

      integer function hglibver(major_v, minor_v, release, string)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: hglibver
	!MS$endif

      integer major_v, minor_v, release
      character*(*) string
C      integer hglibverc 
      INTERFACE
        INTEGER FUNCTION hglibverc(major_v, minor_v, release, string,
     +	   stringlen)
	  !MS$ATTRIBUTES C, reference, decorate,alias: 'HGLIBVERC' :: hglibverc
	    !DEC$ ATTRIBUTES reference :: string
          integer major_v, minor_v, release, stringlen
          character*(*) string
        END FUNCTION hglibverc
      END INTERFACE

      hglibver = hglibverc(major_v, minor_v, release, string,
     .                     len(string))
      return
      end
C-----------------------------------------------------------------------------
C Name: hgfilver
C Purpose: retrieves the version information for the current HDF library
C Inputs:  file_id - file identifier
C Outputs: major_v - major version number
C          minor_v - minor version number
C          release - release number
C          string  - version number test string
C Retruns: SUCCEED (0) if successful and FAIL(-1) otherwise
C-----------------------------------------------------------------------------*/

      integer function hgfilver(file_id, major_v, minor_v, release,
     .                          string)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: hgfilver
	!MS$endif

      integer file_id, major_v, minor_v, release
      character*(*) string
C      integer hgfilverc 
      INTERFACE
        INTEGER FUNCTION hgfilverc(file_id, major_v, minor_v, release,
     +	                             string, stringlen)
	  !MS$ATTRIBUTES C, reference, decorate,alias: 'HGFILVERC' :: hgfilverc
	    !DEC$ ATTRIBUTES reference :: string
          integer file_id, major_v, minor_v, release, stringlen
          character*(*) string
        END FUNCTION hgfilverc
      END INTERFACE

      hgfilver = hgfilverc(file_id, major_v, minor_v, release, string,
     .                     len(string))
      return
      end

C------------------------------------------------------------------------------
C Name: hishdf
C Purpose:  Identifies if the file "file_name" is an HDF file. 
C Inputs:   file_name:  File name
C Returns: TRUE (1) if successful, FALSE (0) otherwise.
C Invokes: hiopen
C----------------------------------------------------------------------------*/

      integer function hishdf(filename)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: hishdf
	!MS$endif
      character*(*) filename
C      integer       hiishdf
	INTERFACE
        INTEGER FUNCTION hiishdf(filename, length)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'HIISHDF' :: hiishdf
	    !DEC$ ATTRIBUTES reference :: filename
          integer length
	    character*(*) filename
        END FUNCTION hiishdf
      END INTERFACE
      hishdf = hiishdf(filename, len(filename))
      return
      end
C-----------------------------------------------------------------------------
C Name: hconfinf
C Purpose: return info about configuration of a compression method
C Inputs:  coder_type -  the compression type queried  
C          info       -  flag to indicate compression status
C                         0 - no compression found
C                         1 - decoder only found
C                         3 - both decoder and encoder found 
C Retruns: SUCCEED (0) if successful and FAIL(-1) otherwise
C
C Currently this routine is used with SZIP compression only 
C-----------------------------------------------------------------------------*/

      integer function hconfinf(coder_type, info)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: hconfinf
	!MS$endif
      integer coder_type, info, status

	INTERFACE
        INTEGER FUNCTION hconfinfc(coder_type, info)
	  !MS$ATTRIBUTES C, reference, decorate,alias: 'HCONFINFC' :: hconfinfc
          integer coder_type, info
        END FUNCTION hconfinfc
      END INTERFACE
      status = hconfinfc(coder_type, info)
      return
      end

