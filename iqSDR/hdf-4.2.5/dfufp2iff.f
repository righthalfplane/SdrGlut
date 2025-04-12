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
C $Id: dfufp2if.f 1014 1994-01-13 21:16:42Z georgev $
C
C------------------------------------------------------------------------------
C File:     DFUfptoimFf.f
C Purpose:  Fortran stub for DFUfptoimage()
C Invokes:  DFUfptoimage.c
C Contents: 
C   duf2im:       call duif2i_ to invoke DFUfptoimage()
C   dfufptoimage: call duif2i_ to invoke DFUfptoimage()
C -----------------------------------------------------------------------------

C------------------------------------------------------------------------------
C Name:     duf2im
C Purpose:   call duif2i_ to invoke DFUfptoimage()
C Inputs:   
C      hdim, vdim: horizontal and vertical dimensions of input data
C      max, min:   maximum and minimum values in the data
C      hscale,vscale: optional horizontal and vertical scales
C      data:       input data
C      palette:    optional palette to be stored with the image
C      outfile:n   name of hdf file to store image in
C      ctmethod:  color transform method: 1=EXPAND; 2=INTERP
C      hres, vres: resolutions desired for output image
C      compress:   compression flag: 0=don't; 1=do
C  Returns: 0 on success, -1 on failure with DFerror set
C  Users:       HDF HLL (high-level library) users, utilities, other routines
C  Invokes: process
C  Remarks: none
C----------------------------------------------------------------------------


      integer function duf2im(hdim,vdim,max,min,hscale,vscale,data,
     *                palette,outfile,ctmethod,hres,vres,compress)

      integer       hdim, vdim
      real          max, min, hscale, vscale, data
      character*(*) palette
      character*(*) outfile
      integer       ctmethod, hres, vres, compress

      duf2im = duif2i(hdim,vdim,max,min,hscale,vscale,data,palette,
     *              outfile,ctmethod,hres,vres,compress, len(outfile))        
      return
      end

CEND7MAX

C------------------------------------------------------------------------------
C Name:     dfufptoimage
C Purpose:   call duif2i_ to invoke DFUfptoimage()
C Inputs:   
C      hdim, vdim: horizontal and vertical dimensions of input data
C      max, min:   maximum and minimum values in the data
C      hscale,vscale: optional horizontal and vertical scales
C      data:       input data
C      palette:    optional palette to be stored with the image
C      outfile:n   name of hdf file to store image in
C      ctmethod:  color transform method: 1=EXPAND; 2=INTERP
C      hres, vres: resolutions desired for output image
C      compress:   compression flag: 0=don't; 1=do
C  Returns: 0 on success, -1 on failure with DFerror set
C  Users:       HDF HLL (high-level library) users, utilities, other routines
C  Invokes: process
C  Remarks: none
C----------------------------------------------------------------------------


      integer function dfufptoimage(hdim,vdim,max,min,hscale,vscale,
     *           data, palette,outfile,ctmethod,hres,vres,compress)

      integer       hdim, vdim
      real          max, min, hscale, vscale, data
      character*(*) palette
      character*(*) outfile
      integer       ctmethod, hres, vres, compress

      dfufptoimage = 
     *             duif2i(hdim,vdim,max,min,hscale,vscale,data,palette,
     *              outfile,ctmethod,hres,vres,compress, len(outfile))        
      return
      end

