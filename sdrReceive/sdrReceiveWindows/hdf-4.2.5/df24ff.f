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
C $Id: df24ff.f 4932 2007-09-07 17:17:23Z bmribler $
C
C------------------------------------------------------------------------------
C File:     df24Ff.f
C Purpose:  read and write 24-bit raster images
C Invokes:  df24F.c
C Contents: 
C   d2gdims:      - get dimensions of image
C   d2gimg:       - read in image
C   d2aimg:       - write out image
C   d2rref:       - set reference number to be used for next get
C   DF24getdims:  - get dimensions of image
C   DF24getimage: - read in image
C   DF24getimage: - write out image
C   DF24readref:  - set reference number to be used for next get
C
C Remarks:  A RIG specifies attributes associated with an image - lookup table, 
C           dimension, compression, color compensation etc.
C----------------------------------------------------------------------------*/



C------------------------------------------------------------------------------
C Name: d2gdims
C Purpose:  get dimensions of next image RIG
C Inputs:   name: name of HDF file
C           xdim, ydim: locations for returning x,y dimensions
C           il: location for returning interlace of image in file
C Returns: 0 on success, -1 on failure with DFerror set
C           *pxdim, *pydim, *pil set on success
C Users:    HDF HLL (high-level library) users, utilities, other routines
C Invokes: d2igdim
C Remarks: none
C----------------------------------------------------------------------------*/

      integer function d2gdims(name, xdim, ydim, il)
      character*(*) name
      integer xdim, ydim, il, d2igdim

      d2gdims = d2igdim(name, xdim, ydim, il, len(name))
      return
      end


C------------------------------------------------------------------------------
C Name: d2gimg
C Purpose:  get image from next RIG
C Inputs:   name: name of HDF file
C           image: pointer to space to return image
C           xdim, ydim: dimensions of space to return image
C Returns: 0 on success, -1 on failure with DFerror set
C Users:    HDF HLL (high-level library) users, utilities, other routines
C Invokes: d2igimg
C Remarks: space is assumed to be xdim * ydim * 3 bytes
C----------------------------------------------------------------------------*/

      integer function d2gimg(name, image, xdim, ydim)
      character*(*) name, image
      integer xdim, ydim, d2igimg

      d2gimg = d2igimg(name, image, xdim, ydim, len(name))
      return
      end


C------------------------------------------------------------------------------
C Name: d2aimg
C Purpose:  Write out image
C Inputs:   filename: name of HDF file
C           image: image to write
C           xdim, ydim: dimensions of array image
C Returns: 0 on success, -1 on failure with DFerror set
C Users:    HDF HLL (high-level library) users, utilities, other routines
C Invokes: d2iaimg
C Remarks: array image is assumed to be xdim * ydim * ncomps bytes
C----------------------------------------------------------------------------*/

      integer function d2aimg(name, image, xdim, ydim)
      character*(*) name, image
      integer xdim, ydim, d2iaimg

      d2aimg = d2iaimg(name, image, xdim, ydim, len(name), 0)
      return
      end

      integer function d2pimg(name, image, xdim, ydim)
      character*(*) name, image
      integer xdim, ydim, d2iaimg

      d2pimg = d2iaimg(name, image, xdim, ydim, len(name), 1)
      return
      end

C------------------------------------------------------------------------------
C Name: d2rref
C Purpose:  
C Inputs:   filename: name of HDF file
C           ref:      reference number to be used for next get
C Returns: 0 on success, -1 on failure with DFerror set
C Users:    HDF HLL (high-level library) users, utilities, other routines
C Invokes: d2irref
C Remarks: 
C----------------------------------------------------------------------------*/

      integer function d2rref(name, ref)
      character*(*) name
      integer ref

      d2rref = d2irref(name, ref, len(name))
      return
      end


C------------------------------------------------------------------------------
C Name: d2nimg
C Purpose:  Determine the number of 24-bit raster images in a file.
C Inputs:   filename: name of HDF file
C Returns: # of images on success, -1 on failure with error stack set
C Users:    HDF HLL (high-level library) users, utilities, other routines
C Invokes: d2inimg
C Remarks: 
C----------------------------------------------------------------------------*/

      integer function d2nimg(name)
      character*(*) name

      d2nimg = d2inimg(name, len(name))
      return
      end


CEND7MAX


C------------------------------------------------------------------------------
C Name: df24getdims
C Purpose:  get dimensions of next image RIG
C Inputs:   name: name of HDF file
C           xdim, ydim: locations for returning x,y dimensions
C           il: location for returning interlace of image in file
C Returns: 0 on success, -1 on failure with DFerror set
C           *pxdim, *pydim, *pil set on success
C Users:    HDF HLL (high-level library) users, utilities, other routines
C Invokes: d2igdim
C Remarks: none
C----------------------------------------------------------------------------*/

      integer function df24getdims(name, xdim, ydim, il)
      character*(*) name
      integer xdim, ydim, il, d2igdim

      df24getdims = d2igdim(name, xdim, ydim, il, len(name))
      return
      end


C------------------------------------------------------------------------------
C Name: df24getimage
C Purpose:  get image from next RIG
C Inputs:   name: name of HDF file
C           image: pointer to space to return image
C           xdim, ydim: dimensions of space to return image
C Returns: 0 on success, -1 on failure with DFerror set
C Users:    HDF HLL (high-level library) users, utilities, other routines
C Invokes: d2igimg
C Remarks: space is assumed to be xdim * ydim * 3 bytes
C----------------------------------------------------------------------------*/

      integer function df24getimage(name, image, xdim, ydim)
      character*(*) name, image
      integer xdim, ydim, d2igimg

      df24getimage = d2igimg(name, image, xdim, ydim, len(name))
      return
      end


C------------------------------------------------------------------------------
C Name: df24addimage
C Purpose:  Write out image
C Inputs:   filename: name of HDF file
C           image: image to write
C           xdim, ydim: dimensions of array image
C Returns: 0 on success, -1 on failure with DFerror set
C Users:    HDF HLL (high-level library) users, utilities, other routines
C Invokes: d2iaimg
C Remarks: array image is assumed to be xdim * ydim * ncomps bytes
C----------------------------------------------------------------------------*/

      integer function df24addimage(name, image, xdim, ydim)
      character*(*) name, image
      integer xdim, ydim, d2iaimg

      df24addimage = d2iaimg(name, image, xdim, ydim, len(name), 0)
      return
      end

      integer function df24putimage(name, image, xdim, ydim)
      character*(*) name, image
      integer xdim, ydim, d2iaimg

      df24putimage = d2iaimg(name, image, xdim, ydim, len(name), 1)
      return
      end


C------------------------------------------------------------------------------
C Name: df24readref
C Purpose:  Write out image
C Inputs:   filename: name of HDF file
C           ref:      reference number to be used for next get
C Returns: 0 on success, -1 on failure with DFerror set
C Users:    HDF HLL (high-level library) users, utilities, other routines
C Invokes: d2irref
C Remarks: array image is assumed to be xdim * ydim * ncomps bytes
C----------------------------------------------------------------------------*/

      integer function df24readref(name, ref)
      character*(*) name
      integer ref

      df24readref = d2irref(name, ref, len(name))
      return
      end



