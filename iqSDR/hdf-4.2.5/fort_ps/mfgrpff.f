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
C $Id: mfgrpff.f 4995 2007-11-05 21:34:07Z swegner $
C
C------------------------------------------------------------------------------
C File:     mfgrpff.f
C Purpose:  Fortran stubs for Fortran PowerStation GR routines
C Invokes:  mfgrpf.c
C Contents: 
C Remarks: none
C------------------------------------------------------------------
C----------------------------------------------------------------
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
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgcreat 
	!MS$endif
      character*(*) name
      integer grid, ncomp, nt, il, dimsizes
C     integer  mgicreat
      INTERFACE 
         INTEGER FUNCTION mgicreat(grid,name,ncomp,nt,il,dimsizes,
     +                             nmlen)
  	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGICREAT' :: mgicreat
	      !DEC$ ATTRIBUTES reference :: name
            integer grid, ncomp,nt,il,dimsizes, nmlen
            character*(*) name
         END FUNCTION mgicreat
      END INTERFACE
 
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
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgn2ndx
	!MS$endif
      character*(*) name
      integer grid
C     integer mgin2ndx
      INTERFACE
        INTEGER FUNCTION mgin2ndx(grid,name,nmlen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIN2NDX' :: mgin2ndx
	    !DEC$ ATTRIBUTES reference :: name
          integer grid, nmlen
          character*(*) name
        END FUNCTION mgin2ndx
      END INTERFACE

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
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgsxfil
	!MS$endif
      character*(*) filename
      integer riid, offset
C      integer mgisxfil
      INTERFACE
        INTEGER FUNCTION mgisxfil(riid,filename,offset, nmlen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGISXFIL' :: mgisxfil
	    !DEC$ ATTRIBUTES reference :: filename
          integer riid, offset, nmlen
          character*(*) filename
        END FUNCTION mgisxfil
      END INTERFACE

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
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgscatt
	!MS$endif
      character*(*) name
      character*(*) data
      integer riid, nt, count
C      integer mgiscatt
      INTERFACE
        INTEGER FUNCTION mgiscatt(riid,name,nt,count,data,
     +                             nmlen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGISCATT' :: mgiscatt
	    !DEC$ ATTRIBUTES reference :: name, data
          integer riid,nt,count, nmlen
          character*(*) name, data
         END FUNCTION mgiscatt
      END INTERFACE

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
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgsnatt
	!MS$endif
      character*(*) name
      integer data
      integer riid, nt, count
C      integer mgisattr
      INTERFACE
        INTEGER FUNCTION mgisattr(riid,name,nt,count,data,
     +                             nmlen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGISATTR' :: mgisattr
	    !DEC$ ATTRIBUTES reference :: name
          integer riid, nt,count,data, nmlen
          character*(*) name
        END FUNCTION mgisattr
      END INTERFACE

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

C      integer function mgsattr(riid, name, nt, count, data)
C	!MS$if defined(BUILD_HDF_DLL)
C	!MS$attributes dllexport :: mgsattr
C	!MS$endif
C      character*(*) name
C      character*(*) data
C      integer riid, nt, count
C      integer mgisattr
C      INTERFACE
C        INTEGER FUNCTION mgisattr(riid,name,nt,count,data,
C     +                             nmlen)
C	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGISATTR' :: mgisattr
C  	     !DEC$ ATTRIBUTES reference :: name, data
C          integer riid, nt,count, nmlen
C          character*(*) name, data
C        END FUNCTION mgisattr
C      END INTERFACE

C      mgsattr = mgisattr(riid, name, nt, count, data, len(name))
C      return
C      end
C---------------------------------------------------------------
C Name: mgfndat
C Purpose:  Locate an attribute for a raster image
C Inputs:   
C       riid: RI ID of image
C       name: the name of the attribute
C Returns: SUCCEED/FAIL
C Users:    HDF Fortran programmers
C Invokes: mgifndat
C----------------------------------------------------------------

      integer function mgfndat(riid, name)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgfndat
	!MS$endif
      character*(*) name
      integer riid
C      integer mgifndat
      INTERFACE
        INTEGER FUNCTION mgifndat(riid,name,nmlen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIFNDAT' :: mgifndat
	    !DEC$ ATTRIBUTES reference :: name
          integer riid, nmlen
          character*(*) name
        END FUNCTION mgifndat
      END INTERFACE

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
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mggichnk
	!MS$endif

         INTEGER riid, dim_length(*), comp_type
C         INTEGER mgcgichnk 
      INTERFACE
        INTEGER FUNCTION mgcgichnk(riid, dim_length, comp_type)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGCGICHNK' :: mgcgichnk
          integer riid, dim_length(*), comp_type
        END FUNCTION mgcgichnk
      END INTERFACE

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
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgrcchnk
	!MS$endif
         INTEGER riid, start(*)
         CHARACTER*(*) char_data(*)
C         INTEGER mgcrcchnk 
      INTERFACE
        INTEGER FUNCTION mgcrcchnk(riid, start, char_data)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGCRCCHNK' :: mgcrcchnk
          integer riid, start(*)
	    CHARACTER*(*) char_data(*)
        END FUNCTION mgcrcchnk
      END INTERFACE
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
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgrchnk
	!MS$endif
         INTEGER riid, start(*), num_data(*)
C         INTEGER mgcrchnk 
      INTERFACE
        INTEGER FUNCTION mgcrchnk(riid, start, num_data)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGCRCHNK' :: mgcrchnk
          integer riid, start(*), num_data(*)
        END FUNCTION mgcrchnk
      END INTERFACE
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
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgscchnk
	!MS$endif
C
         INTEGER riid, maxcache, flags 
C         INTEGER mgcscchnk 
      INTERFACE
        INTEGER FUNCTION mgcscchnk(riid,name,nmlen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGCSCCHNK' :: mgcscchnk
          integer riid, maxcache, flags
        END FUNCTION mgcscchnk
      END INTERFACE
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
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgschnk
	!MS$endif

         INTEGER riid, dim_length(*), comp_type, comp_prm(*)
C         INTEGER mgcschnk 
      INTERFACE
        INTEGER FUNCTION mgcschnk(riid, dim_length, comp_type, comp_prm)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGCSCHNK' :: mgcschnk
          integer riid, dim_length(*), comp_type, comp_prm(*)
        END FUNCTION mgcschnk
      END INTERFACE

         mgschnk = mgcschnk(riid, dim_length, comp_type,
     .                      comp_prm)
         return
         end

C---------------------------------------------------------------
      integer function mgstart(fid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgstart
	!MS$endif
        integer fid
C        integer mgistrt
      INTERFACE
        INTEGER FUNCTION mgistrt(fid)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGISTRT' :: mgistrt
          integer fid
        END FUNCTION mgistrt
      END INTERFACE
     
      mgstart = mgistrt(fid)
      return
      end
C------------------------------------------------------------------
     
      integer function mgfinfo(grid,datasets,attrs)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgfinfo
	!MS$endif
        integer grid, datasets,attrs
C        integer mgifinf
      INTERFACE
        INTEGER FUNCTION mgifinf(grid,datasets,attrs)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIFINF' :: mgifinf
          integer grid,datasets,attrs
        END FUNCTION mgifinf
      END INTERFACE
     
      mgfinfo = mgifinf(grid, datasets, attrs)
      return
      end
C------------------------------------------------------------------
     
      integer function mgend(grid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgend
	!MS$endif
        integer grid
C        integer mgiend
      INTERFACE
        INTEGER FUNCTION mgiend(grid)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIEND' :: mgiend
          integer grid
        END FUNCTION mgiend
      END INTERFACE
     
      mgend = mgiend(grid)
      return
      end
C------------------------------------------------------------------
     
      integer function mgselct(grid,index)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgselct
	!MS$endif
        integer grid, index
C        integer mgislct
      INTERFACE
        INTEGER FUNCTION mgislct(grid,index)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGISLCT' :: mgislct
          integer grid,index
        END FUNCTION mgislct
      END INTERFACE
     
      mgselct = mgislct(grid, index)
      return
      end
C------------------------------------------------------------------
     
      integer function mggiinf(riid,name,ncomp,nt,il,dimsizes,attrs)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mggiinf
	!MS$endif
        integer riid, ncomp,nt,il,dimsizes,attrs
        character*(*) name
C        integer mgigiinf
      INTERFACE
        INTEGER FUNCTION mgigiinf(riid,name,ncomp,nt,il,
     +                            dimsizes,attrs)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIGIINF' :: mgigiinf
	    !DEC$ ATTRIBUTES reference :: name
          integer riid,ncomp,nt,il,dimsizes,attrs
          character*(*) name
        END FUNCTION mgigiinf
      END INTERFACE
     
      mggiinf = mgigiinf(riid,name,ncomp,nt,il,dimsizes,attrs)
      return
      end
C------------------------------------------------------------------
    
      integer function mgwcimg(riid,start,stride,count,cdata)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgwcimg
	!MS$endif
        integer riid, start,stride,count
        character*(*) cdata
C        integer mgiwcim
      INTERFACE
        INTEGER FUNCTION mgiwcim(riid,start,stride,count,
     +                            cdata)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIWCIM' :: mgiwcim
	    !DEC$ ATTRIBUTES reference :: cdata
          integer riid,start,stride,count
          character*(*) cdata
        END FUNCTION mgiwcim
      END INTERFACE

      mgwcimg = mgiwcim(riid,start,stride,count,cdata)
      return
      end
C------------------------------------------------------------------
    
      integer function mgwrimg(riid,start,stride,count,data)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgwrimg
	!MS$endif
        integer riid, start,stride,count,data
C        integer mgiwimg
      INTERFACE
        INTEGER FUNCTION mgiwimg(riid,start,stride,count,
     +                            data)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIWIMG' :: mgiwimg
          integer riid,start,stride,count,data
        END FUNCTION mgiwimg
      END INTERFACE

      mgwrimg = mgiwimg(riid,start,stride,count,data)
      return
      end

C------------------------------------------------------------------
    
      integer function mgrcimg(riid,start,stride,count,cdata)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgrcimg 
	!MS$endif
        integer riid, start,stride,count
        character*(*) cdata
C        integer mgircim
      INTERFACE
        INTEGER FUNCTION mgircim(riid,start,stride,count,
     +                            cdata)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIRCIM' :: mgircim
	    !DEC$ ATTRIBUTES reference :: cdata
          integer riid,start,stride,count
          character*(*) cdata
        END FUNCTION mgircim
      END INTERFACE

      mgrcimg = mgircim(riid,start,stride,count,cdata)
      return
      end
C------------------------------------------------------------------
    
      integer function mgrdimg(riid,start,stride,count,data)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgrdimg
	!MS$endif
        integer riid, start,stride,count,data
C        integer mgirimg
      INTERFACE
        INTEGER FUNCTION mgirimg(riid,start,stride,count,
     +                            data)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIRIMG' :: mgirimg
          integer riid,start,stride,count,data
        END FUNCTION mgirimg
      END INTERFACE

      mgrdimg = mgirimg(riid,start,stride,count,data)
      return
      end

C------------------------------------------------------------------
    
      integer function mgendac(riid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgendac
	!MS$endif
        integer riid
C        integer mgiendac
      INTERFACE
        INTEGER FUNCTION mgiendac(riid)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIENDAC' :: mgiendac
          integer riid
        END FUNCTION mgiendac
      END INTERFACE

      mgendac = mgiendac(riid)
      return
      end
C------------------------------------------------------------------
   
      integer function mgid2rf(riid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgid2rf
	!MS$endif
        integer riid
C        integer mgiid2r
      INTERFACE
        INTEGER FUNCTION mgiid2r(riid)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIID2R' :: mgiid2r
          integer riid
        END FUNCTION mgiid2r
      END INTERFACE

      mgid2rf = mgiid2r(riid)
      return
      end
C------------------------------------------------------------------
   
      integer function mgr2idx(grid, ref)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgr2idx
	!MS$endif
        integer grid, ref
C        integer mgir2dx
      INTERFACE
        INTEGER FUNCTION mgir2dx(grid,ref)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIR2DX' :: mgir2dx
          integer grid, ref
        END FUNCTION mgir2dx
      END INTERFACE

      mgr2idx = mgir2dx(grid, ref)
      return
      end
C------------------------------------------------------------------
   
      integer function mgrltil(riid, il)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgrltil
	!MS$endif
        integer riid, il
C        integer mgiltil
      INTERFACE
        INTEGER FUNCTION mgiltil(riid, il)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGILTIL' :: mgiltil
          integer riid, il
        END FUNCTION mgiltil
      END INTERFACE

      mgrltil = mgiltil(riid, il)
      return
      end
C------------------------------------------------------------------
   
      integer function mgrimil(riid, il)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgrimil
	!MS$endif
        integer riid, il
C        integer mgiimil
      INTERFACE
        INTEGER FUNCTION mgiimil(riid, il)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIIMIL' :: mgiimil
          integer riid, il
        END FUNCTION mgiimil
      END INTERFACE

      mgrimil = mgiimil(riid, il)
      return
      end
C------------------------------------------------------------------
   
      integer function mggltid(riid, lut_index)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mggltid
	!MS$endif
        integer riid, lut_index
C        integer mgiglid
      INTERFACE
        INTEGER FUNCTION mgiglid(riid, lut_index)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIGLID' :: mgiglid
          integer riid, lut_index
        END FUNCTION mgiglid
      END INTERFACE

      mggltid = mgiglid(riid, lut_index)
      return
      end
C------------------------------------------------------------------
   
      integer function mgglinf(lutid,ncomp,nt,il,nentries)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgglinf
	!MS$endif
        integer lutid,ncomp,nt,il,nentries
C        integer mgiglinf
      INTERFACE
        INTEGER FUNCTION mgiglinf(lutid,ncomp,nt,il,nentries)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIGLINF' :: mgiglinf
          integer lutid, ncomp, nt, il, nentries
        END FUNCTION mgiglinf
      END INTERFACE

      mgglinf = mgiglinf(lutid,ncomp,nt,il,nentries)
      return
      end
C------------------------------------------------------------------
   
      integer function mgwclut(lutid,ncomp,nt,il,nentries,cdata)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgwclut
	!MS$endif
        integer lutid,ncomp,nt,il,nentries
        character*(*) cdata
C        integer mgiwclt
      INTERFACE
        INTEGER FUNCTION mgiwclt(lutid,ncomp,nt,il,nentries,cdata)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIWCLT' :: mgiwclt
	    !DEC$ ATTRIBUTES reference :: cdata
          integer lutid, ncomp, nt, il, nentries
          character*(*) cdata
        END FUNCTION mgiwclt
      END INTERFACE

      mgwclut = mgiwclt(lutid,ncomp,nt,il,nentries,cdata)
      return
      end
C------------------------------------------------------------------
   
      integer function mgwrlut(lutid,ncomp,nt,il,nentries,data)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgwrlut
	!MS$endif
        integer lutid,ncomp,nt,il,nentries, data
C        integer mgiwrlt
      INTERFACE
        INTEGER FUNCTION mgiwrlt(lutid,ncomp,nt,il,nentries,data)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIWRLT' :: mgiwrlt
          integer lutid, ncomp, nt, il, nentries, data
        END FUNCTION mgiwrlt
      END INTERFACE

      mgwrlut = mgiwrlt(lutid,ncomp,nt,il,nentries,data)
      return
      end
C------------------------------------------------------------------
   
      integer function mgrclut(lutid,cdata)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgrclut
	!MS$endif
        integer lutid
        character*(*) cdata
C        integer mgirclt
      INTERFACE
        INTEGER FUNCTION mgirclt(lutid,cdata)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIRCLT' :: mgirclt
	    !DEC$ ATTRIBUTES reference :: cdata
          integer lutid
          character*(*) cdata
        END FUNCTION mgirclt
      END INTERFACE

      mgrclut = mgirclt(lutid,cdata)
      return
      end
C------------------------------------------------------------------
   
      integer function mgrdlut(lutid,data)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgrdlut
	!MS$endif
        integer lutid, data
C        integer mgirdlt
      INTERFACE
        INTEGER FUNCTION mgirdlt(lutid,data)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIRDLT' :: mgirdlt
          integer lutid, data
        END FUNCTION mgirdlt
      END INTERFACE

      mgrdlut = mgirdlt(lutid,data)
      return
      end
C------------------------------------------------------------------
   
      integer function mgsactp(riid,acctype)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgsactp
	!MS$endif
        integer riid, acctype
C        integer mgiactp
      INTERFACE
        INTEGER FUNCTION mgiactp(riid,acctype)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIACTP' :: mgiactp
          integer riid,acctype
        END FUNCTION mgiactp
      END INTERFACE

      mgsactp = mgiactp(riid,acctype)
      return
      end
C------------------------------------------------------------------

      integer function mgatinf(riid,index,name,nt,count)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgatinf
	!MS$endif
        integer riid, index,nt,count
        character*(*) name
C        integer mgiainf
      INTERFACE
        INTEGER FUNCTION mgiainf(riid,index,name,nt,count)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIAINF' :: mgiainf
	    !DEC$ ATTRIBUTES reference :: name
          integer riid,index,nt,count
          character*(*) name
        END FUNCTION mgiainf
      END INTERFACE

      mgatinf = mgiainf(riid,index,name,nt,count)
      return
      end

C-------------------------------------------------------------
C Name: mggcatt
C Purpose:  Get a char type attribute of a raster image
C Inputs:   
C       riid: RI ID of image
C       index: the index of the attribute
C       cdata: buffer to hold the data of the attribute
C Returns: SUCCEED/FAIL
C Users:    HDF Fortran programmers
C Invokes: mgigcat
C-------------------------------------------------------------

      integer function mggcatt(riid, index, cdata)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mggcatt
	!MS$endif
      integer riid, index
      character*(*) cdata
C      integer mgigcat
      INTERFACE
        INTEGER FUNCTION mgigcat(riid,index,cdata)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIGCAT' :: mgigcat
	    !DEC$ ATTRIBUTES reference :: cdata
          integer riid,index
          character*(*)  cdata
         END FUNCTION mgigcat
      END INTERFACE

      mggcatt = mgigcat(riid, index, cdata)
      return
      end

C-------------------------------------------------------------
C Name: mggnatt
C Purpose:  Get a numeric attribute of a raster image
C Inputs:   
C       riid: RI ID of image
C       index: the index of the attribute
C       data: the data of the attribute
C Returns: SUCCEED/FAIL
C Users:    HDF Fortran programmers
C Invokes: mgignat
C-------------------------------------------------------------

      integer function mggnatt(riid, index, data)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mggnatt
	!MS$endif
      integer data
      integer riid, index
C      integer mgignat
      INTERFACE
        INTEGER FUNCTION mgignat(riid,index,data)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIGNAT' :: mgignat
          integer riid, index,data
        END FUNCTION mgignat
      END INTERFACE

      mggnatt = mgignat(riid, index, data)
      return
      end

C-------------------------------------------------------------
C Name: mggatt
C Purpose:  Get an attribute of a raster image
C Inputs:   
C       riid: RI ID of image
C       index: the index of the attribute
C       data: the data for the attribute
C Returns: SUCCEED/FAIL
C Users:    HDF Fortran programmers
C Invokes: mgisattr
C-------------------------------------------------------------
      integer function mggattr(riid, index, data)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mggattr
	!MS$endif
      integer data
      integer riid, index
C      integer mgigatt
      INTERFACE
        INTEGER FUNCTION mgigatt(riid,index,data)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGIGATT' :: mgigatt
          integer riid, index,data
        END FUNCTION mgigatt
      END INTERFACE

      mggattr = mgigatt(riid, index, data)
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
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgwcchnk
	!MS$endif
         INTEGER riid, start(*) 
         CHARACTER*(*) char_data(*)
C         INTEGER mgcwcchnk 
      INTERFACE
        INTEGER FUNCTION mgcwcchnk(riid,start,char_data)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGCWCCHNK' :: mgcwcchnk
          INTEGER riid, start(*)
	    CHARACTER*(*) char_data(*)
        END FUNCTION mgcwcchnk
      END INTERFACE
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
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgwchnk
	!MS$endif
         INTEGER riid, start(*), num_data(*)
C         INTEGER mymgcwchnk 
	INTERFACE
        INTEGER FUNCTION mgcwchnk(riid,start,num_data)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGCWCHNK' :: mgcwchnk
          INTEGER riid, start(*), num_data(*)
        END FUNCTION mgcwchnk
      END INTERFACE
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
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mgscompress
	!MS$endif

         INTEGER riid, comp_type, comp_prm
C         INTEGER mgcscompress 
      INTERFACE
        INTEGER FUNCTION mgcscompress(riid, comp_type, comp_prm)
	 !MS$ATTRIBUTES C,reference,decorate,alias:'MGCSCOMPRESS'::mgcscompress
          integer riid, comp_type, comp_prm
        END FUNCTION mgcscompress
      END INTERFACE

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
C                            COMP_CODE_JPEG    = 6
C                   comp_prm  - compression parameters array
C                   Huffman:   comp_prm(1) = skphuff_skp_size
C                   GZIP:      comp_prm(1) = deflate_level       
C        Returns:   0 on success, -1 on failure
C        Calls:     mgcgcompress (C stub for GRgetcompress function)
C-------------------------------------------------------------------------

         INTEGER function mggcompress(riid, comp_type, comp_prm)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mggcompress
	!MS$endif

         INTEGER riid, comp_type, comp_prm(1)
      INTERFACE
        INTEGER FUNCTION mgcgcompress(riid, comp_type, comp_prm)
	 !MS$ATTRIBUTES C,reference,decorate,alias:'MGCGCOMPRESS'::mgcgcompress
          integer riid, comp_type, comp_prm(1)
        END FUNCTION mgcgcompress
      END INTERFACE

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
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: mggnluts
	!MS$endif

         INTEGER riid
      INTERFACE
        INTEGER FUNCTION mgcgnluts(riid) 
	    !MS$ATTRIBUTES C,reference,decorate,alias:'MGCGNLUTS'::mgcgnluts
          integer riid
        END FUNCTION mgcgnluts
      END INTERFACE

         mggnluts = mgcgnluts(riid)
         return
         end

           
