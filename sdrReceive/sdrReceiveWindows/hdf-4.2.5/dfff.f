C* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
C* Copyright by The HDF Group.                                               *
C* Copyright by the Board of Trustees of the University of Illinois.         *
C* All rights reserved.                                                      *
C*                                                                           *
C* This file is part of HDF.  The full HDF copyright notice, including       *
C* terms governing use, modification, and redistribution, is contained in    *
C* the files COPYING and Copyright.html.  COPYING can be found at the root   *
C* of the source code distribution tree; Copyright.html can be found at      *
C* http://hdfgroup.org/products/hdf4/doc/Copyright.html.  If you do not have *
C* access to either file, you may request a copy from help@hdfgroup.org.     *
C* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
C
C $Id: dfff.f 4932 2007-09-07 17:17:23Z bmribler $ 
C
C------------------------------------------------------------------------------
C File:     dfFf.f
C Purpose:  Fortran stubs for Fortran low level i/o routines
C Invokes:  dfF.c dfkit.c
C Contents: 
C   dfopen:     call dfiopen to open HDF file
C   dfishdf:    call dfiishdf to find is file is HDF
C -----------------------------------------------------------------------------

C------------------------------------------------------------------------------
C Name:     dfopen
C Purpose:  call dfiopen to open HDF file
C Inputs:   name: name of HDF file to open
C           access: integer for access mode: DFACC_READ etc.
C           defdds: default number of DDs per header block
C Returns:  0 on success, -1 on failure with DFerror set
C Users:    HDF Fortran programmers
C Invokes:  dfiopen
C------------------------------------------------------------------------------


      integer function dfopen(name, access, defdds)
      character*(*) name
      integer access, defdds, dfiopen

      dfopen = dfiopen(name, access, defdds, len(name))
      return
      end

C------------------------------------------------------------------------------
C Name:    dfaccess
C Purpose: call dfiaccess to set up access to a data element
C Inputs:  dfile: pointer to open HDF file
C          tag: tag of element to access
C          ref: ref of element to access
C          access: access mode requested
C Returns: 0 on success, -1 on failure with DFerror set
C Users:   HDF FORTRAN programmers
C Invokes: dfiaccess
C------------------------------------------------------------------------------

      integer function dfaccess(dfile, tag, ref, access)
      character*(*) access
      integer dfile, tag, ref, dfiaccess

      dfaccess = dfiaccess(dfile, tag, ref, access, len(access))
      return
      end

C------------------------------------------------------------------------------
C Name:     dfishdf
C Purpose:  call dfiishdf to test file
C Inputs:   name: name of file to test
C Returns:  0 on success, -1 on failure with DFerror set
C Users:    HDF Fortran programmers
C Invokes:  dfiishdf
C------------------------------------------------------------------------------


      integer function dfishdf(name)
      character*(*) name
      integer dfiishdf

      dfishdf = dfiishdf(name, len(name))
      return
      end
