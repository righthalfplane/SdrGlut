C***********************************************************
C * NCSA HDF
C   *
C * Software Development Group
C   *
C * National Center for Supercomputing Applications
C   *
C * University of Illinois at Urbana-Champaign
C   *
C * 605 E. Springfield, Champaign IL 61820
C   *
C *
C   *
C * For conditions of distribution and use, see the accompanying
C   *
C * hdf/COPYING file.
C   *
C *
C   *
C***************************************************
C
C $Id: vattrpff.f 4995 2007-11-05 21:34:07Z swegner $
C
C **************************************************
C *
C * vattrpff.f -- based upon vattrff.f,v 1.5
C * Vset attribute Fortran routines for Fortran PowerStation
C *
C *************************************************

C --------------- vsffidx ------------------------
C    vsffidx -- find index of a named field in a vdata
C    VSfindex -- vsfcfdx -- vsffidx
C
       integer function vsffidx(vsid, fldnm, findex) 
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsffidx
	!MS$endif
       integer vsid
       character*(*) fldnm
       integer findex
C       integer vsfcfdx
       INTERFACE
         INTEGER FUNCTION vsfcfdx(vsid,fldnm,findex,fldnmlen)
 	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSFCFDX' :: vsfcfdx
	     !DEC$ ATTRIBUTES reference :: fldnm
           integer vsid, findex, fldnmlen
           character*(*) fldnm
         END FUNCTION vsfcfdx
       END INTERFACE

       vsffidx = vsfcfdx(vsid, fldnm, findex, 
     +                  len(fldnm))
       end
C --------------- vsfsnat -------------------------
C    vsfsnat -- set a numeric attribute for a vdata 
C              or a field of a vdata
C    VSsetattr -- vsfcsat -- vsfsnat
C
       integer function vsfsnat(vsid, findex,attrnm,dtype,
     +                        count, values)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfsnat
	!MS$endif
       integer vsid, findex, dtype, count
       character*(*) attrnm
       integer  values
C       integer vsfcsat
       INTERFACE
         INTEGER FUNCTION vsfcsat(vsid,findex,attrnm,dtype,count,
     +                            values, attnmlen)
 	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSFCSAT' :: vsfcsat
	     !DEC$ ATTRIBUTES reference :: attrnm
           integer vsid, findex, dtype,count,values, attnmlen
           character*(*) attrnm
         END FUNCTION vsfcsat
       END INTERFACE
     
       vsfsnat = vsfcsat(vsid, findex, attrnm, dtype,
     +                count, values, len(attrnm))
       end
C --------------- vsfscat -------------------------
C    vsfscat -- set a char type attribute for a vdata 
C              or a field of a vdata
C    VSsetattr -- vsfcsca -- vsfscat
C
       integer function vsfscat(vsid, findex,attrnm,dtype,
     +                      count, values)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfscat
	!MS$endif
       integer vsid, findex, dtype, count
       character*(*) attrnm
       character*(*) values
C       integer vsfcsca
       INTERFACE
         INTEGER FUNCTION vsfcsca(vsid, findex, attrnm,dtype,
     +                            count, values, attnmlen)
         !MS$ATTRIBUTES C,reference,decorate,alias:'VSFCSCA' :: vsfcsca
	   !DEC$ ATTRIBUTES reference :: attrnm, values
         integer vsid, findex, dtype,count,attnmlen
         character*(*) attrnm, values
         END FUNCTION vsfcsca
       END INTERFACE
       vsfscat = vsfcsca(vsid, findex, attrnm, dtype,
     +         count, values, len(attrnm))
       end
C ------------- vsfnats ------------------------
C     vsfnats -- get total number of attributes of a vdata and
C             its fields
C     VSnattrs -- vsfcnats -- vsfnats
C
       integer function vsfnats(vsid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfnats
	!MS$endif
       integer vsid
C       integer vsfcnats
       INTERFACE 
         INTEGER FUNCTION vsfcnats(vsid)
 	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSFCNATS' :: vsfcnats
           integer vsid
         END FUNCTION vsfcnats
       END INTERFACE

       vsfnats = vsfcnats(vsid)
       end       
C ------------- vsffnas ------------------------
C     vsfnats -- get total number of attributes of a vdata and
C             its fields
C     VSnattrs -- vsfcfnas -- vsffnas
C
       integer function vsffnas(vsid, findex)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsffnas
	!MS$endif
       integer vsid, findex
C       integer vsfcfnas
       INTERFACE
         INTEGER FUNCTION vsfcfnas(vsid, findex)
 	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSFCFNAS' :: vsfcfnas
           integer vsid, findex
         END FUNCTION vsfcfnas
       END INTERFACE

       vsffnas = vsfcfnas(vsid, findex)
       end

C --------------- vsffdat -------------------------
C    vsffdat -- find attribute of a vdata of a field
C               of the vdata
C    VSfindattr -- vsfcfda -- vsffdat
C
       integer function vsffdat(vsid, findex, attrnm)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsffdat
	!MS$endif
       integer vsid, findex
       character*(*) attrnm
C       integer vsfcfda
       INTERFACE
         INTEGER FUNCTION vsfcfda(vsid, findex, attrnm, nmlen)
 	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSFCFDA' :: vsfcfda
	     !DEC$ ATTRIBUTES reference :: attrnm
           integer vsid, findex, nmlen
           character*(*) attrnm
         END FUNCTION vsfcfda
       END INTERFACE

       vsffdat = vsfcfda(vsid, findex, attrnm, 
     +                   len(attrnm))
       end 

C --------------- vsfainf -------------------------
C    vsffdat -- get info of an attribute 
C    VSattrinfo -- vsfcainf -- vsfainf
C
       integer function vsfainf(vsid, findex, aindex, attrnm,
     +                   dtype,count,size)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfainf
	!MS$endif
       integer vsid, findex, aindex, dtype, count, size
       character*(*) attrnm
C       integer vsfcainf
       INTERFACE
         INTEGER FUNCTION vsfcainf(vsid, findex, aindex, attrnm,
     +                   dtype, count, size, nmlen)
 	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSFCAINF' :: vsfcainf
	     !DEC$ ATTRIBUTES reference :: attrnm
           integer vsid, findex, aindex, dtype, count, size, nmlen
           character*(*) attrnm
         END FUNCTION vsfcainf
       END INTERFACE

       vsfainf = vsfcainf(vsid, findex, aindex, attrnm, dtype,
     +                   count, size, len(attrnm))
       end

C --------------- vsfgnat -------------------------
C    vsfgnat -- get values of a numeric attribute 
C    VSgetattr -- vsfcgna -- vsfgnat
C
       integer function vsfgnat(vsid, findex, aindex, values)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfgnat
	!MS$endif
       integer vsid, findex, aindex, values(*)
C       integer vsfcgna
       INTERFACE
         INTEGER FUNCTION vsfcgna(vsid, findex, aindex, values)
 	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSFCGNA' :: vsfcgna
           integer vsid, findex, aindex, values(*)
         END FUNCTION vsfcgna
       END INTERFACE

       vsfgnat = vsfcgna(vsid, findex, aindex, values)
       end

C --------------- vsfgcat -------------------------
C    vsfgcat -- get values of a character type attribute 
C    VSgetattr -- vsfcgca -- vsfgcat
C
       integer function vsfgcat(vsid, findex, aindex, values)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfgcat
	!MS$endif
       integer vsid, findex, aindex
       character*(*) values
C       integer vsfcgca
       INTERFACE
         INTEGER FUNCTION vsfcgca(vsid, findex, aindex, values)
 	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSFCGCA' :: vsfcgca
	     !DEC$ ATTRIBUTES reference :: values
           integer vsid, findex, aindex
           character*(*) values
         END FUNCTION vsfcgca
       END INTERFACE

       vsfgcat = vsfcgca(vsid, findex, aindex, values)
       end

C --------------- vsfisat -------------------------
C    vsfisat --  test if a vdata is an attribute of other object
C    VSisattr -- vsfcisa -- vsfisat
C
       integer function vsfisat(vsid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfisat
	!MS$endif
       integer vsid
C       integer vsfcisa
       INTERFACE
         INTEGER FUNCTION vsfcisa(vsid)
 	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSFCISA' :: vsfcisa
           integer vsid
         END FUNCTION vsfcisa
       END INTERFACE

       vsfisat = vsfcisa(vsid)
       end

C --------------- vfsnatt -------------------------
C    vfsnatt -- set a numeric attr for a vgroup
C    Vsetattr -- vfcsatt -- vfsnatt 
C
       integer function vfsnatt(vgid, attrnm, dtype, 
     +                        count, values)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfsnatt
	!MS$endif
       integer vgid, dtype, count, values
       character*(*) attrnm
C       integer vfcsatt
       INTERFACE
         INTEGER FUNCTION vfcsatt(vgid, attrnm, dtype, count,
     +                       values, nmlen)
 	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFCSATT' :: vfcsatt
	     !DEC$ ATTRIBUTES reference :: attrnm
           integer vgid, dtype, count, values, nmlen
           character*(*)  attrnm
         END FUNCTION vfcsatt
       END INTERFACE

       vfsnatt = vfcsatt(vgid, attrnm, dtype,count,
     +                  values, len(attrnm))
       end
C -------------- vfscatt -------------------------
C    vfscatt -- set a char attr for a vgroup
C    Vsetattr -- vfcscat -- vfscatt
C
       integer function vfscatt(vgid, attrnm, dtype,
     +                        count, values)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfscatt
	!MS$endif
       integer vgid, dtype, count
       character*(*) attrnm, values
c       integer vfcscat
       INTERFACE
         INTEGER FUNCTION vfcscat(vgid, attrnm, dtype, count,
     +                       values, nmlen)
 	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFCSCAT' :: vfcscat
	     !DEC$ ATTRIBUTES reference :: attrnm, values
           integer vgid, dtype, count, nmlen
           character*(*)  attrnm, values
         END FUNCTION vfcscat
       END INTERFACE

       vfscatt = vfcscat(vgid, attrnm, dtype,count,
     +                  values, len(attrnm))
       end

C ------------- vfnatts ------------------------
C     vfnatts -- get total number of attributes of a vgroup 
C     Vnattrs -- vfcnats -- vfnatts
C
       integer function vfnatts(vgid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfnatts
	!MS$endif
       integer vgid
C       integer vfcnats
       INTERFACE 
         INTEGER FUNCTION vfcnats(vgid)
 	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFCNATS' :: vfcnats
           integer vgid
         END FUNCTION vfcnats
       END INTERFACE

       vfnatts = vfcnats(vgid)
       end       

C --------------- vffdatt ---------------------
C    vffdatt -- find an attribute of a vgroup
C    Vfindattr -- vfcfdat -- vffdatt
C
       integer function vffdatt(vg, attrnm)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vffdatt
	!MS$endif
       integer vg
       character*(*)   attrnm
C       integer vfcfdat
       INTERFACE
         INTEGER FUNCTION vfcfdat(vg, attrnm, nmlen)
 	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFCFDAT' :: vfcfdat
	     !DEC$ ATTRIBUTES reference :: attrnm
           integer vg, nmlen
           character*(*) attrnm
         END FUNCTION vfcfdat
       END INTERFACE

       vffdatt = vfcfdat(vg, attrnm, len(attrnm))
       end
C --------------- vfainfo -------------------------
C    vfainfo -- get info of an attribute 
C    Vattrinfo -- vfcainf -- vfainfo
C
       integer function vfainfo(vgid, aindex, attrnm,
     +                   dtype,count,size)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfainfo
	!MS$endif
       integer vgid, aindex, dtype, count, size
       character*(*) attrnm
C       integer vfcainf
       INTERFACE
         INTEGER FUNCTION vfcainf(vgid, aindex, attrnm,
     +                   dtype, count, size, nmlen)
 	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFCAINF' :: vfcainf
	     !DEC$ ATTRIBUTES reference :: attrnm
           integer vgid, aindex, dtype, count, size, nmlen
           character*(*) attrnm
         END FUNCTION vfcainf
       END INTERFACE

       vfainfo = vfcainf(vgid, aindex, attrnm, dtype,
     +                   count, size, len(attrnm))
       end

C --------------- vfgnatt -------------------------
C    vfgnatt -- get values of a numeric attribute 
C    Vgetattr -- vfcgnat -- vfgnatt
C
       integer function vfgnatt(vgid, aindex, values)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfgnatt
	!MS$endif
       integer vgid, aindex, values
C       integer vfcgnat
       INTERFACE
         INTEGER FUNCTION vfcgnat(vgid, aindex, values)
 	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFCGNAT' :: vfcgnat
           integer vgid, aindex, values
         END FUNCTION vfcgnat
       END INTERFACE

       vfgnatt = vfcgnat(vgid, aindex, values)
       end

C --------------- vfgcatt -------------------------
C    vfgcatt -- get values of a character type attribute 
C    Vgetattr -- vfcgcat -- vfgcatt
C
       integer function vfgcatt(vgid, aindex, values)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfgcatt
	!MS$endif
       integer vgid, aindex
       character*(*) values
C       integer vfcgcat
       INTERFACE
         INTEGER FUNCTION vfcgcat(vgid, aindex, values)
 	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFCGCAT' :: vfcgcat
	     !DEC$ ATTRIBUTES reference :: values
           integer vgid, aindex
           character*(*) values
         END FUNCTION vfcgcat
       END INTERFACE

       vfgcatt = vfcgcat(vgid, aindex, values)
       end
 
C --------------- vfgver -------------------------
C    vfgver -- get version number of a vgroup
C    Vgetversion -- vfcgver -- vfgver
C
       integer function vfgver(vgid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfgver
	!MS$endif
       integer vgid
C       integer vfcgver
       INTERFACE
         INTEGER FUNCTION vfcgver(vgid)
 	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFCGVER' :: vfcgver
           integer vgid
         END FUNCTION vfcgver
       END INTERFACE

       vfgver = vfcgver(vgid)
       end
