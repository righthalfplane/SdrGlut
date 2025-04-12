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
C $Id: mfgrff.f 4932 2007-09-07 17:17:23Z bmribler $
C
C------------------------------------------------------------------------------
C File:     mfgrff.f
C Purpose:  Fortran stubs for Fortran GR routines
C Invokes:  mfgrf.c
C Contents: 
C   dsgdims:        get dimensions of next SDG
C Remarks: none
C------------------------------------------------------------------------------



C------------------------------------------------------------------------------
C Name: mgcreat
C Purpose:  Create a new raster image
C Inputs:   
C       grid: GR ID of interface to create image in
C       name: name of raster image
C       ncomp: number of components per pixel
C       nt: number-type of each component
C       il: interlace scheme to use
C       dimsizes[2]: dimensions of the image to create
C Returns: RI ID on success, -1 on failure
C Users:    HDF Fortran programmers
C Invokes: mgicreat
C------------------------------------------------------------------------------

      integer function mgcreat(grid, name, ncomp, nt, il, dimsizes)
      character*(*) name
      integer grid, ncomp, nt, il, dimsizes, mgicreat

      mgcreat = mgicreat(grid, name, ncomp, nt, il, dimsizes,
     +                                              len(name))
      return
      end

C------------------------------------------------------------------------------
C Name: mgn2ndx
C Purpose:  Map the name of a raster image to an index in the file
C Inputs:   
C       grid: GR ID of interface to create image in
C       name: name of raster image
C Returns: index of image on success, -1 on failure
C Users:    HDF Fortran programmers
C Invokes: mgin2ndx
C------------------------------------------------------------------------------

      integer function mgn2ndx(grid, name)
      character*(*) name
      integer grid, mgin2ndx

      mgn2ndx = mgin2ndx(grid, name, len(name))
      return
      end

C------------------------------------------------------------------------------
C Name: mgsxfil
C Purpose:  Convert a standard image into an external image
C Inputs:   
C       riid: RI ID of image to move
C       filename: filename of file to move image into
C       offset: offset in file to move image to
C Returns: SUCCEED/FAIL
C Users:    HDF Fortran programmers
C Invokes: mgisxfil
C-------------------------------------------------------------

      integer function mgsxfil(riid, filename, offset)
      character*(*) filename
      integer riid, mgisxfil, offset

      mgsxfil = mgisxfil(riid, filename, offset, len(filename))
      return
      end

C-------------------------------------------------------------
C Name: mgscatt
C Purpose:  Add a char type attribute to a raster image
C Inputs:   
C       riid: RI ID of image
C       name: the name of the attribute
C       nt: the number-type of the attribute
C       count: the number of values in the attribute
C       data: the data for the attribute
C Returns: SUCCEED/FAIL
C Users:    HDF Fortran programmers
C Invokes: mgiscatt
C-------------------------------------------------------------

      integer function mgscatt(riid, name, nt, count, data)
      character*(*) name
      character*(*) data
      integer riid, mgiscatt, nt, count

      mgscatt = mgiscatt(riid, name, nt, count, data, len(name))
      return
      end

C-------------------------------------------------------------
C Name: mgsnatt
C Purpose:  Add a numeric attribute to a raster image
C Inputs:   
C       riid: RI ID of image
C       name: the name of the attribute
C       nt: the number-type of the attribute
C       count: the number of values in the attribute
C       data: the data for the attribute
C Returns: SUCCEED/FAIL
C Users:    HDF Fortran programmers
C Invokes: mgisattr
C-------------------------------------------------------------

      integer function mgsnatt(riid, name, nt, count, data)
      character*(*) name
      integer data
      integer riid, mgisattr, nt, count

      mgsnatt = mgisattr(riid, name, nt, count, data, len(name))
      return
      end

C-------------------------------------------------------------
C Name: mgsattr
C Purpose:  Add an attribute to a raster image
C Inputs:   
C       riid: RI ID of image
C       name: the name of the attribute
C       nt: the number-type of the attribute
C       count: the number of values in the attribute
C       data: the data for the attribute
C Returns: SUCCEED/FAIL
C Users:    HDF Fortran programmers
C Invokes: mgisattr
C-------------------------------------------------------------

      integer function mgsattr(riid, name, nt, count, data)
      character*(*) name
      character*(*) data
      integer riid, mgisattr, nt, count

      mgsattr = mgisattr(riid, name, nt, count, data, len(name))
      return
      end
C---------------------------------------------------------------
C Name: mgfndat
C Purpose:  Locate an attribute for a raster image
C Inputs:   
C       riid: RI ID of image
C       name: the name of the attribute
C Returns: SUCCEED/FAIL
C Users:    HDF Fortran programmers
C Invokes: mgifndat
C------------------------------------------------------------------------------

      integer function mgfndat(riid, name)
      character*(*) name
      integer riid, mgifndat

      mgfndat = mgifndat(riid, name, len(name))
      return
      end

C-------------------------------------------------------------------------
C        Name:      mggichnk
C        Purpose:   get Info on GR 
C        Inputs:    riid       - access ID to GR
C        Outputs:   dim_length - chunk dimensions (if any)
C                   comp_type: 
C                               -1 - GR is non-chunked
C                                0 - GR is chunked without compression
C                                1 - GR is chunked and compressed 
C        Returns:   0 on success, -1 on failure
C        Calls:     mgcgichnk (C stub for GRgetchunkinfo function)
C-------------------------------------------------------------------------

         INTEGER function mggichnk(riid, dim_length, comp_type)

         INTEGER riid, dim_length(*), comp_type
         INTEGER mgcgichnk 


         mggichnk = mgcgichnk(riid, dim_length, comp_type)
         return
         end

           
C-------------------------------------------------------------------------
C        Name:      mgrcchnk
C        Purpose:   read the specified chunk of CHARACTER data to the GR 
C        Inputs:    riid      - access ID to GR
C                   start     - chunk coordinates 
C                   char_data - buffer the data will be read into  
C        Returns:   0 on success, -1 on failure
C        Calls:     mgcrcchnk (C stub for GRreadchunk function)
C        Users:     HDF Fortran programmers
C-------------------------------------------------------------------------

         INTEGER function mgrcchnk(riid, start, char_data)

         INTEGER riid, start(*)
         CHARACTER*(*) char_data(*)
         INTEGER mgcrcchnk 

         mgrcchnk = mgcrcchnk(riid, start, char_data) 

         return 
         end

           
C-------------------------------------------------------------------------
C        Name:      mgrchnk
C        Purpose:   read the specified chunk of NUMERIC data to the GR 
C        Inputs:    riid     - access ID to GR
C                   start    - chunk coordinates 
C                   num_data - buffer the  numeric data will be read into  
C        Returns:   0 on success, -1 on failure
C        Calls:     mgcrchnk (C stub for GRreadchunk function)
C        Users:     HDF Fortran programmers
C-------------------------------------------------------------------------

         INTEGER function mgrchnk(riid, start, num_data)

         INTEGER riid, start(*), num_data(*)
         INTEGER mgcrchnk 

         mgrchnk = mgcrchnk(riid, start, num_data) 

         return 
         end

           
C-------------------------------------------------------------------------
C        Name:      mgscchnk
C        Purpose:   set the maximum number of chunks to cache
C        Inputs:    riid     - access ID to GR
C                   maxcache - max number of chunks to cache 
C                   flags    - flags =0, HDF_CACHEALL
C                              Currently only 0 can be passed.
C        Returns:   0 on success, -1 on failure
C        Calls:     mgcscchnk (C stub for GRsetchunkcache function)
C        Users:     HDF Fortran programmers
C-------------------------------------------------------------------------

         INTEGER function mgscchnk(riid, maxcache, flags)
C
         INTEGER riid, maxcache, flags 
         INTEGER mgcscchnk 
C
         mgscchnk = mgcscchnk(riid, maxcache, flags) 
C
         return 
         end

           

C-------------------------------------------------------------------------
C        Name:      mgschnk
C        Purpose:   make the GR a chunked GR 
C        Inputs:    riid       - access ID to GR
C                   dim_length - chunk dimensions
C                   comp_type  - type of compression         
C                   comp_prm   - compression parameters array
C                   Huffman:   comp_prm(1) = skphuff_skp_size
C                   GZIP:      comp_prm(1) = deflate_level       
C                                      
C        Returns:   0 on success, -1 on failure
C        Calls:     mgcschnk (C stub for GRsetchunk function)
C-------------------------------------------------------------------------

         INTEGER function mgschnk(riid, dim_length, comp_type,
     .                            comp_prm)

         INTEGER riid, dim_length(*), comp_type, comp_prm(*)
         INTEGER mgcschnk 
         mgschnk = mgcschnk(riid, dim_length, comp_type,
     .                      comp_prm)
         return
         end

           
C-------------------------------------------------------------------------
C        Name:      mgwcchnk
C        Purpose:   write the specified chunk of CHARACTER data to the GR 
C        Inputs:    riid      - access ID to GR
C                   start     - chunk coordinates 
C                   char_data - buffer containing  data to be written  
C        Returns:   0 on success, -1 on failure
C        Calls:     mgcwcchnk (C stub for GRwritechunk function)
C        Users:     HDF Fortran programmers
C-------------------------------------------------------------------------

         INTEGER function mgwcchnk(riid, start, char_data)

         INTEGER riid, start(*) 
         CHARACTER*(*) char_data(*)
         INTEGER mgcwcchnk 

         mgwcchnk = mgcwcchnk(riid, start, char_data) 

         return 
         end

C-------------------------------------------------------------------------
C        Name:      mgwchnk
C        Purpose:   write the specified chunk of NUMERIC data to the GR 
C        Inputs:    riid     - access ID to GR
C                   start    - chunk coordinates 
C                   num_data - buffer containing data to be written  
C        Returns:   0 on success, -1 on failure
C        Calls:     mgcwchnk (C stub for GRwritechunk function)
C        Users:     HDF Fortran programmers
C-------------------------------------------------------------------------

         INTEGER function mgwchnk(riid, start, num_data)

         INTEGER riid, start(*), num_data(*)
         INTEGER mgcwchnk 

         mgwchnk = mgcwchnk(riid, start, num_data) 

         return 
         end

           
C-------------------------------------------------------------------------
C        Name:      mgscompress
C        Purpose:   compress GR 
C        Inputs:    riid      - access ID to GR
C                   comp_type - type of compression
C                   supports the following compression types:
C                            ( see hcomp.h  file) 
C                            COMP_CODE_NONE = 0
C                            COMP_CODE_RLE =1
C                            COMP_CODE_SKPHUFF = 3
C                            COMP_CODE_DEFLATE = 4
C                            COMP_CODE_JPEG    = 7 
C                   comp_prm  - compression parameters array
C                   Huffman:   comp_prm(1) = skphuff_skp_size
C                   GZIP:      comp_prm(1) = deflate_level       
C                   JPEG:      comp_prm(1) = quality ( value from 0 to 100)
C                              comp_prm(2) = baseline (compatibility flag 0 or 1)
C        Returns:   0 on success, -1 on failure
C        Calls:     mgcscompress (C stub for GRsetcompress function)
C-------------------------------------------------------------------------

         INTEGER function mgscompress(riid, comp_type, comp_prm)

         INTEGER riid, comp_type
         INTEGER mgcscompress 

         mgscompress = mgcscompress(riid, comp_type,
     .                              comp_prm)
         return
         end

C-------------------------------------------------------------------------
C        Name:      mggcompress
C        Purpose:   gets compression information about GR image
C        Inputs:    riid      - access ID to GR
C        Outputs:  comp_type - type of compression
C                   supports the following compression types:
C                            ( see hcomp.h  file) 
C                            COMP_CODE_NONE = 0
C                            COMP_CODE_RLE =1
C                            COMP_CODE_SKPHUFF = 3
C                            COMP_CODE_DEFLATE = 4
C                            COMP_CODE_JPEG    = 7 
C                   comp_prm  - compression parameters array
C                   Huffman:   comp_prm(1) = skphuff_skp_size
C                   GZIP:      comp_prm(1) = deflate_level       
C        Returns:   0 on success, -1 on failure
C        Calls:     mgcgcompress (C stub for GRgetcompress function)
C-------------------------------------------------------------------------

         INTEGER function mggcompress(riid, comp_type, comp_prm)

         INTEGER riid, comp_type, comp_prm(1)
         INTEGER mgcgcompress 

         mggcompress = mgcgcompress(riid, comp_type,
     .                              comp_prm)
         return
         end
C-------------------------------------------------------------------------
C        Name:      mggnluts
C        Purpose:   gets number of palettes (LUTs) for an image
C        Inputs:    riid      - access ID to GR
C        Returns:   Number of palettes on success (0 or 1), -1 on failure
C        Calls:     mgcgnluts (C stub for GRgetnluts function)
C-------------------------------------------------------------------------

         INTEGER function mggnluts(riid)

         INTEGER riid
         INTEGER mgcgnluts 

         mggnluts = mgcgnluts(riid)
         return
         end

           
           
