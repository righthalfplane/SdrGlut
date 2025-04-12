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
C $Id: vgpff.f 4995 2007-11-05 21:34:07Z swegner $ 
C     
c **************************************************************************
c *
c * vgpff.f, based on vgff.f,v 1.20
c * Part of the HDF VSet interface. For Fortran PowerStation 
c *
c * Contains fortran routines callable from fortran programs.
c *
c **************************************************************************


c   ============================================================
c	 VGROUP ROUTINES
c	============================================================

c	attachs to a vgroup 	
c	related: Vattach--vatchc--VFATCH

      integer function vfatch(f, vgref, accesstype)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfatch
	!MS$endif
      integer     f, vgref
      character*1 accesstype
C      integer     vatchc
      INTERFACE
        INTEGER FUNCTION vatchc(f, vgref, accesstype)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VATCHC' :: vatchc
	    !DEC$ ATTRIBUTES reference :: accesstype
          integer f, vgref
          character*1 accesstype
        END FUNCTION vatchc
      END INTERFACE

      vfatch = vatchc (f, vgref, accesstype)
      end

c	------------------------------------------------------------
c	detaches from a vgroup
c	related: Vdetach--vdtchc--VFDTCH

      integer function vfdtch (vgid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfdtch
	!MS$endif
      integer     vgid
c      integer   vdtchc
      INTERFACE
        INTEGER FUNCTION vdtchc(vgid)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VDTCHC' :: vdtchc
          integer vgid
        END FUNCTION vdtchc
      END INTERFACE

      vfdtch = vdtchc (vgid)
      end

c	------------------------------------------------------------
c	general inquiry on a vgroup
c	related: Vgetname--vgnamc--VFGNAM

      integer function vfgnam (vg, vgname)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfgnam 
	!MS$endif
      integer         vg
      character*(*)   vgname
c      integer         vgnamc
      INTERFACE
        INTEGER FUNCTION vgnamc(vg, vgname)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VGNAMC' :: vgnamc
	    !DEC$ ATTRIBUTES reference :: vgname
          integer vg
          character*(*) vgname
        END FUNCTION vgnamc
      END INTERFACE

      vfgnam = vgnamc (vg, vgname)
      end
c	------------------------------------------------------------
c	get the class name of a vgroup
c	related: Vgetclass--vgclsc--VFGCLS

      integer function vfgcls (vg, vgclass)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfgcls
	!MS$endif

      integer			vg
      character*(*)	vgclass
c      integer       vgclsc
      INTERFACE
        INTEGER FUNCTION vgclsc(vg, vgclass)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VGCLSC' :: vgclsc
	    !DEC$ ATTRIBUTES reference :: vgclass
          integer vg
          character*(*) vgclass
        END FUNCTION vgclsc
      END INTERFACE

      vfgcls = vgclsc  (vg, vgclass)
      end
c   ------------------------------------------------------------
c	general inquiry on a vgroup
c	related: Vinquire--vinqc--VFINQ

      integer   function    vfinq (vg, nentries, vgname)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfinq
	!MS$endif
      integer			vg, nentries
      character*(*)	vgname
c      integer			vinqc
      INTERFACE
        INTEGER FUNCTION vinqc(vg, nentries, vgname)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VINQC' :: vinqc
	    !DEC$ ATTRIBUTES reference :: vgname
          integer vg, nentries
          character*(*) vgname
        END FUNCTION vinqc
      END INTERFACE

      vfinq = vinqc (vg, nentries, vgname)
      end

c   ------------------------------------------------------------
c	gets the id of the next vgroup in the file
c	related: Vgetid--vgidc--VFGID

      integer   function    vfgid (f, vgref)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfgid
	!MS$endif
      integer		f, vgref
c      integer		vgidc
      INTERFACE
        INTEGER FUNCTION vgidc(f, vgref)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VGIDC' :: vgidc
          integer f, vgref 
        END FUNCTION vgidc
      END INTERFACE

      vfgid = vgidc (f, vgref)
      end

c	------------------------------------------------------------
c	gets the ref of the next vgroup or vdata entry in the vgroup
c	related: Vgetnext--vgnxtc--VFGNXT

      integer   function    vfgnxt (vg, vref)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfgnxt
	!MS$endif
      integer		vg, vref
c      integer		vgnxtc
      INTERFACE
        INTEGER FUNCTION vgnxtc(vg, vref)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VGNXTC' :: vgnxtc
          integer vg, vref
        END FUNCTION vgnxtc
      END INTERFACE

      vfgnxt = vgnxtc (vg, vref)
      end

c	------------------------------------------------------------
c	sets the name of the vgroup
c	related: Vsetname--vsnamc--VFSNAM

      integer function vfsnam (vg, vgname)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfsnam
	!MS$endif
      integer			vg
      character*(*)	vgname
c      integer       vsnamc
      INTERFACE
        INTEGER FUNCTION vsnamc(vg, vgname, nmlen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSNAMC' :: vsnamc
	    !DEC$ ATTRIBUTES reference :: vgname
          integer vg, nmlen
          character*(*) vgname
        END FUNCTION vsnamc
      END INTERFACE

      vfsnam = vsnamc (vg, vgname, len(vgname))
      end
c   ------------------------------------------------------------
c	sets the class name of the vgroup
c	related: Vsetclass--vsclsc--VFSCLS

      integer function vfscls (vg, vgclass)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfscls
	!MS$endif
      integer			vg
      character*(*)	vgclass
c      integer      vsclsc
      INTERFACE
        INTEGER FUNCTION vsclsc(vg, vgclass, clslen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSCLSC' :: vsclsc
	    !DEC$ ATTRIBUTES reference :: vgclass
          integer vg, clslen
          character*(*) vgclass
        END FUNCTION vsclsc
      END INTERFACE

      vfscls = vsclsc(vg, vgclass, len(vgclass))
      end

c	------------------------------------------------------------
c	inserts a vset entity (ie vgroup or vdata) into the given vgroup
c	related: Vinsert--vinsrtc--VFINSRT

      integer   function vfinsrt (vg, vid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfinsrt
	!MS$endif
      integer		vg, vid
c      integer		vinsrtc
      INTERFACE
        INTEGER FUNCTION vinsrtc(vg, vid)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VINSRTC' :: vinsrtc
          integer vg, vid
        END FUNCTION vinsrtc
      END INTERFACE

      vfinsrt = vinsrtc (vg, vid)
      end

c     ------------------------------------------------------------
c     tests if an id in a vgroup is a vgroup
c     related: Visvg--visvgc--vfisvg

      integer	function	vfisvg (vg, id) 									
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfisvg
	!MS$endif
      integer		vg, id
c      integer		visvgc
      INTERFACE
         INTEGER FUNCTION visvgc(vg, id)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VISVGC' :: visvgc
          integer vg, id
        END FUNCTION visvgc
      END INTERFACE

      vfisvg = visvgc (vg, id)
      end

c     ------------------------------------------------------------
c     tests if an id in a vgroup is a vdata
c     related: Visvs--visvsc--vfisvs

      integer	function	vfisvs (vg, id)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfisvs
	!MS$endif
      integer		vg, id
c      integer		visvsc
      INTERFACE
         INTEGER FUNCTION visvsc(vg, id)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VISVSC' :: visvsc
          integer vg, id
        END FUNCTION visvsc
      END INTERFACE

      vfisvs = visvsc (vg, id)
      end

c     ------------------------------------------------------------
c     start Vset interface
C     related:  Vstart -- vfistart -- vfstart

      integer function vfstart(file_id)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfstart
	!MS$endif
      integer file_id
C      integer vfistart
      INTERFACE 
        INTEGER FUNCTION vfistart(file_id)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFISTART' :: vfistart
          integer file_id
        END FUNCTION vfistart
      END INTERFACE
   
      vfstart = vfistart(file_id)
      end

c     ------------------------------------------------------------
c     end Vset interface
C     related:  Vend -- vfiend -- vfend

      integer function vfend(file_id)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfend
	!MS$endif
      integer file_id
C      integer vfiend
      INTERFACE 
        INTEGER FUNCTION vfiend(file_id)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFIEND' :: vfiend
          integer file_id
        END FUNCTION vfiend
      END INTERFACE
   
      vfend = vfiend(file_id)
      end

c     ============================================================
c      VDATA ROUTINES
c     ============================================================

c     attach to a vdata
c     related: VSattach--vsatchc--vfatch

      integer	function	vsfatch (f, vsid, accesstype)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfatch
	!MS$endif
      integer		f, vsid
      character*1	accesstype
c      integer		vsatchc
      INTERFACE
         INTEGER FUNCTION vsatchc(f, vsid, accesstype)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSATCHC' :: vsatchc
	    !DEC$ ATTRIBUTES reference :: accesstype
          integer f, vsid
          character*1   accesstype
        END FUNCTION vsatchc
      END INTERFACE

      vsfatch = vsatchc (f, vsid, accesstype)
      end

c     ------------------------------------------------------------
c     detach from a vdata
c     related: VSdetach--vsdtchc--vfdtch

      integer function vsfdtch (vs)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfdtch
	!MS$endif
      integer		vs
c      integer       vsdtchc
      INTERFACE
         INTEGER FUNCTION vsdtchc(vs)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSDTCHC' :: vsdtchc
          integer vs
        END FUNCTION vsdtchc
      END INTERFACE

      vsfdtch = vsdtchc (vs)
      end

c     ------------------------------------------------------------
c     get the ref # of a vdata
c     related: VSQueryref -- vsiqref -- vsqref

      integer function vsqref(vsid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsqref
	!MS$endif
      integer vsid
C     integer vsiqref
      INTERFACE
        INTEGER FUNCTION vsiqref(vsid)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSIQREF' :: vsiqref
          integer vsid
        END FUNCTION vsiqref
      END INTERFACE 

      vsqref = vsiqref(vsid)
      end
c     ------------------------------------------------------------
c     get the tag of a vdata
c     related: VSQuerytag -- vsiqtag -- vsqtag

      integer function vsqtag(vsid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsqtag
	!MS$endif
      integer vsid
C     integer vsiqtag
      INTERFACE
        INTEGER FUNCTION vsiqtag(vsid)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSIQTAG' :: vsiqtag
          integer vsid
        END FUNCTION vsiqtag
      END INTERFACE

      vsqtag = vsiqtag(vsid)
      end

c     ------------------------------------------------------------
c     get the ver # of a vdata
c     related: VSgetversion -- vsigver -- vsgver

      integer function vsgver(vsid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsgver
	!MS$endif
      integer vsid
C     integer vsigver
      INTERFACE
        INTEGER FUNCTION vsigver(vsid)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSIGVER' :: vsigver
          integer vsid
        END FUNCTION vsigver
      END INTERFACE

      vsgver = vsigver(vsid)
      end
c     ------------------------------------------------------------
c     seeks to a given element position in a vadata
c     related: VSseek--vsseekc--vsfseek

      integer	function	vsfseek (vs, eltpos )
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfseek
	!MS$endif
      integer		vs, eltpos
c      integer		vsseekc
      INTERFACE
        INTEGER FUNCTION vsseekc(vs, eltpos)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSSEEKC' :: vsseekc
          integer vs, eltpos
        END FUNCTION vsseekc
      END INTERFACE

      vsfseek = vsseekc (vs, eltpos)
      end

c     ------------------------------------------------------------
c     gets the name of a vdata
c     related: VSgetname--vsgnamc--vsfgnam

      integer function vsfgnam (vs, vsname)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfgnam
	!MS$endif
      integer			vs
      character*(*)	vsname
c      integer       vsgnamc
      INTERFACE
        INTEGER FUNCTION vsgnamc(vs, vsname, nmlen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSGNAMC' :: vsgnamc
	    !DEC$ ATTRIBUTES reference :: vsname
          integer vs, nmlen
          character*(*) vsname
        END FUNCTION vsgnamc
      END INTERFACE

      vsfgnam = vsgnamc (vs, vsname, len(vsname))
      end

c     ------------------------------------------------------------
c     get the class name of a vdata
c     related: VSgetclass--vsgclsc--vsfgcls

      integer function vsfgcls (vs, vsclass)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfgcls
	!MS$endif
      integer			vs
      character*(*)	vsclass
c      integer       vsgclsc
      INTERFACE
        INTEGER FUNCTION vsgclsc(vs, vsclass, clslen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSGCLSC' :: vsgclsc
	    !DEC$ ATTRIBUTES reference :: vsclass
          integer vs, clslen
          character*(*) vsclass
        END FUNCTION vsgclsc
      END INTERFACE

      vsfgcls = vsgclsc  (vs, vsclass, len(vsclass))
      end

c     ------------------------------------------------------------
c     general inquiry on a vdata
c     related: VSinquire--vsinqc--vsfinq

      integer function vsfinq (vs,nvs,ilace,fields,vsize,vsname) 
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfinq
	!MS$endif
      integer			vs, nvs, ilace, vsize
      character*(*)	fields, vsname
c      integer			vsinqc
      INTERFACE
        INTEGER FUNCTION vsinqc(vs,nvs,il,flds,vsize, vsname,
     +                          fldlen, nmlen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSINQC' :: vsinqc
	    !DEC$ ATTRIBUTES reference :: vsname, flds
          integer vs,nvs,il,vsize, fldlen, nmlen
          character*(*) vsname, flds
        END FUNCTION vsinqc
      END INTERFACE

      vsfinq = vsinqc (vs, nvs, ilace, fields, vsize, vsname,
     +             len(fields), len(vsname))
      end

c     ------------------------------------------------------------
c     tests if given fields exist in the vdata
c     related: VSfexist--vsfexc--vsfex

      integer	function vsfex (vs, fields)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfex
	!MS$endif
      integer			vs
      character*(*)	fields
c      integer			vsfexc
      INTERFACE
        INTEGER FUNCTION vsfexc(vs, fields, fldslen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSFEXC' :: vsfexc
	    !DEC$ ATTRIBUTES reference :: fields
          integer vs, fldslen
          character*(*) fields
        END FUNCTION vsfexc
      END INTERFACE

      vsfex = vsfexc (vs, fields, len(fields))
      end

c     ------------------------------------------------------------
c     looks for a Vdata with a given name
c     related: VSfind--vsfind--vsffnd

      integer	function vsffnd (vs, name)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsffnd
	!MS$endif
      integer			vs
      character*(*)	name
c      integer vsfndc
      INTERFACE
        INTEGER FUNCTION vsfndc(vs, name, nmlen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSFNDC' :: vsfndc
	    !DEC$ ATTRIBUTES reference :: name
          integer vs, nmlen
          character*(*) name
        END FUNCTION vsfndc
      END INTERFACE

      vsffnd = vsfndc (vs, name, len(name))
      end

c  ------------------------------------------------------------
c     gets the id of the next vdata from the file
c     related: VSgetid--vsgidc--vsfgid

      integer  function vsfgid (f, vsid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfgid
	!MS$endif
      integer     f, vsid
c      integer     vsgidc
      INTERFACE
        INTEGER FUNCTION vsgidc(f, vsid)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSGIDC' :: vsgidc
          integer f, vsid
        END FUNCTION vsgidc
      END INTERFACE

      vsfgid = vsgidc (f, vsid)
      end

c  ------------------------------------------------------------
c     remove the vdata with id from the file
c     related: VSdelete--vsdltc--vsfdlte

      integer  function vsfdlte (f, vsid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfdlte
	!MS$endif
      integer     f, vsid
c      integer     vsdltc
      INTERFACE
        INTEGER FUNCTION vsdltc(f, vsid)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSDLTC' :: vsdltc
          integer f, vsid
        END FUNCTION vsdltc
      END INTERFACE

      vsfdlte = vsdltc (f, vsid)
      end

c     ------------------------------------------------------------
c     sets the name of a vdata
c     related: VSsetname--vssnamc--vsfsnam

      integer function vsfsnam (vs, vsname)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfsnam
	!MS$endif
      integer			vs
      character*(*)	vsname
c       integer       vssnamc
      INTERFACE
        INTEGER FUNCTION vssnamc(vs, vsname, nmlen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSSNAMC' :: vssnamc
	    !DEC$ ATTRIBUTES reference :: vsname
          integer vs, nmlen
          character*(*) vsname
        END FUNCTION vssnamc
      END INTERFACE

      vsfsnam = vssnamc (vs, vsname, len(vsname))
      end

c     ------------------------------------------------------------
c     set the class name of a vdata
c     related: VSsetclass--vssclsc--vsfscls

      integer function vsfscls (vs, vsclass)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfscls
	!MS$endif
      integer			vs
      character*(*)	vsclass
c      integer vssclsc
      INTERFACE
        INTEGER FUNCTION vssclsc(vs, vsclass, clslen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSSCLSC' :: vssclsc
	    !DEC$ ATTRIBUTES reference :: vsclass
          integer vs, clslen
          character*(*) vsclass
        END FUNCTION vssclsc
      END INTERFACE

      vsfscls = vssclsc  (vs, vsclass, len(vsclass))
      end

c     ------------------------------------------------------------
c     sets the fields in a vdata for reading or writing
c     related: VSsetfields--vssfldc--vsfsfld

      integer	function	vsfsfld (vs, fields)		
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfsfld
	!MS$endif
      integer			vs
      character*(*)	fields
c      integer		vssfldc
      INTERFACE
        INTEGER FUNCTION vssfldc(vs, fields, fldlen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSSFLDC' :: vssfldc
	    !DEC$ ATTRIBUTES reference :: fields
          integer vs, fldlen
          character*(*) fields
        END FUNCTION vssfldc
      END INTERFACE

      vsfsfld = vssfldc (vs, fields, len(fields))
      end

c     ------------------------------------------------------------
c     sets the file interlace of a vdata
c     related: VSsetinterlace--vssintc--vsfsint

      integer 	function vsfsint (vs, interlace)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfsint
	!MS$endif
      integer 		vs, interlace
c      integer		vssintc
      INTERFACE
        INTEGER FUNCTION vssintc(vs, interlace)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSSINTC' :: vssintc
          integer vs, interlace
        END FUNCTION vssintc
      END INTERFACE

      vsfsint = vssintc (vs, interlace)
      end

c     ------------------------------------------------------------
c     defines a new field to be used in the vdata
c     related: VSfdefine--vsfdefc--vsffdef

      integer	function	vsffdef (vs, field, localtype, order)	
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsffdef
	!MS$endif

      integer			vs, localtype, order
      character*(*)	field
c      integer			vsfdefc
      INTERFACE
        INTEGER FUNCTION vsfdefc(vs, field, localtype, order, fldlen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSFDEFC' :: vsfdefc
	    !DEC$ ATTRIBUTES reference :: field
          integer vs, localtype, order, fldlen
          character*(*) field
        END FUNCTION vsfdefc
      END INTERFACE

      vsffdef = vsfdefc ( vs, field, localtype, order, len(field))

      end

c     ------------------------------------------------------------
C     this function should be replaced by vsfrdc and vsfrd
c     reads from a vdata 
c     related: VSread--vsreadc--vsfread

      integer	function	vsfread (vs, buf, nelts , interlace)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfread
	!MS$endif
      integer			vs, nelts , interlace
      character*(*)	buf
c      integer			vsreadc
      INTERFACE
        INTEGER FUNCTION vsreadc(vs, buf, nelts, interlace)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSREADC' :: vsreadc
	    !DEC$ ATTRIBUTES reference :: buf
          integer vs, nelts, interlace
          character*(*) buf
        END FUNCTION vsreadc
      END INTERFACE

      vsfread = vsreadc (vs, buf, nelts, interlace)
      end

c     ------------------------------------------------------------
c     reads records from a vdata into a char buffer
c     related: VSread -- vsfirdc -- vsfrdc

      integer   function        vsfrdc (vsid, buf, nelts , interlace)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfrdc
	!MS$endif
      integer                   vsid, nelts , interlace
      character*(*)     buf
c      integer          vsfirdc
      INTERFACE 
        INTEGER FUNCTION vsfirdc(vsid, buf, nelts, interlace)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSFIRDC' :: vsfirdc
	    !DEC$ ATTRIBUTES reference :: buf
          integer vsid, nelts, interlace
		  character*(*)  buf
        END FUNCTION vsfirdc
      END INTERFACE

      vsfrdc = vsfirdc (vsid, buf, nelts, interlace)
      end

c     ------------------------------------------------------------
c     reads records from a vdata into a numeric buffer
c     related: VSread--vsfird--vsfrd

      integer   function        vsfrd (vsid, buf, nelts , interlace)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfrd
	!MS$endif
      integer                   vsid, nelts , interlace
      integer     buf(*)
C      integer                   vsfird
      INTERFACE 
        INTEGER FUNCTION vsfird(vsid, buf, nelts, interlace)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSFIRD' :: vsfird
          integer vsid, nelts, interlace
          integer buf(*)
        END FUNCTION vsfird
      END INTERFACE

      vsfrd = vsfird (vsid, buf, nelts, interlace)
      end

c     ------------------------------------------------------------
c     writes records to a vdata from a char buffer
c     related: VSwrite -- vsfiwrc -- vsfwrtc

      integer   function        vsfwrtc(vsid, buf, nelts, interlace)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfwrtc
	!MS$endif
      integer                   vsid, nelts , interlace
      character*(*)     buf
c      integer          vsfiwrc
      INTERFACE
        INTEGER FUNCTION vsfiwrc(vsid, buf, nelts, interlace)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSFIWRC' :: vsfiwrc
	    !DEC$ ATTRIBUTES reference :: buf
          integer vsid, nelts, interlace
		  character*(*) buf
        END FUNCTION vsfiwrc
      END INTERFACE

      vsfwrtc = vsfiwrc (vsid, buf, nelts, interlace)
      end

c     ------------------------------------------------------------
c     writes records to a vdata from a numeric buffer
c     related: VSwrite -- vsfiwr -- vsfwrt

      integer   function        vsfwrt(vsid, buf, nelts, interlace)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfwrt
	!MS$endif
      integer         vsid, nelts , interlace, buf(*)
c      integer          vsfiwr
      INTERFACE
        INTEGER FUNCTION vsfiwr(vsid, buf, nelts, interlace)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSFIWR' :: vsfiwr
          integer vsid, nelts, interlace, buf(*)
        END FUNCTION vsfiwr
      END INTERFACE

      vsfwrt = vsfiwr (vsid, buf, nelts, interlace)
      end

c     ------------------------------------------------------------
c     This function should replaced by vsfwrtc and vsfwrt
c     writes to a vdata
c     related: VSwrite--vswritc--vsfwrit

      integer	function	vsfwrit (vs, buf, nelts, interlace)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfwrit
	!MS$endif

      integer			vs, nelts, interlace
      character*(*)		buf	
C      integer			vswritc
      INTERFACE
        INTEGER FUNCTION vswritc(vsid, buf, nelts, interlace)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSWRITC' :: vswritc
	    !DEC$ ATTRIBUTES reference :: buf
          integer vsid, nelts, interlace
          character*(*)     buf
        END FUNCTION vswritc
      END INTERFACE

      vsfwrit = vswritc (vs, buf, nelts, interlace)
      end

C----------------------------------------------------------------------
C     Name: vsfsextf
C     Purpose:  call vssextfc to store data in an external file
C     Inputs:   vid: vdata id
C               fname: name of external file
C               offset: Number of bytes from the beginning of the
C                       external file to where the data starts
C     Returns: 0 on success, FAIL on failure with error set
C     Users:    Fortran stub routine
C     Related:  VSsetexternalfile--vssextfc--vsfsextf
C----------------------------------------------------------------------

      integer function vsfsextf(vid, fname, offset)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfsextf
	!MS$endif

      character*(*) fname

      integer       vid, offset
c      integer       vssextfc
      INTERFACE
        INTEGER FUNCTION vssextfc(vid, fname, offset, nmlen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSSEXTFC' :: vssextfc
	    !DEC$ ATTRIBUTES reference :: fname
          integer vid, offset, nmlen
          character*(*) fname
        END FUNCTION vssextfc
      END INTERFACE

      vsfsextf = vssextfc(vid, fname, offset, len(fname))
      return
      end

c     ===========================================
c     VDATA FIELD MANIPULATION ROUTINES
c     ===========================================
C-----------------------------------------------------------
C     vfnflds -- number of fields in a vdata
C     related: VFnfields -- vfinflds -- vfnflds

      integer function vfnflds(vsid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfnflds
	!MS$endif
      integer vsid
C      integer vfinflds
      INTERFACE 
        INTEGER FUNCTION vfinflds(vsid)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFINFLDS' :: vfinflds
          integer vsid
        END FUNCTION vfinflds
      END INTERFACE

      vfnflds = vfinflds(vsid)
      end

C-----------------------------------------------------------
C     vffname -- name of a field in a vdata
C     related: VFfieldname -- vfifnm -- vffname

      integer function vffname(vsid, findex, fname)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vffname
	!MS$endif
      integer vsid, findex
      character*(*) fname
C      integer vfifnm
      INTERFACE 
        INTEGER FUNCTION vfifnm(vsid, findex, fname)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFIFNM' :: vfifnm
	    !DEC$ ATTRIBUTES reference :: fname
          integer vsid, findex
          character*(*) fname
        END FUNCTION vfifnm
      END INTERFACE

      vffname = vfifnm(vsid, findex, fname)
      end

C-----------------------------------------------------------
C     vfftype -- type of a field in a vdata
C     related: VFfieldtype -- vfiftp -- vfftype

      integer function vfftype(vsid, findex)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfftype
	!MS$endif
      integer vsid, findex
C      integer vfifnm
      INTERFACE 
        INTEGER FUNCTION vfiftp(vsid, findex)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFIFTP' :: vfiftp
          integer vsid, findex
        END FUNCTION vfiftp
      END INTERFACE

      vfftype = vfiftp(vsid, findex)
      end

C-----------------------------------------------------------
C     vffisiz -- HDF size of a field in a vdata
C     related: VFfieldisize -- vfifisz -- vffisiz

      integer function vffisiz(vsid, findex)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vffisiz
	!MS$endif
      integer vsid, findex
C      integer vfifisz
      INTERFACE 
        INTEGER FUNCTION vfifisz(vsid, findex)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFIFISZ' :: vfifisz
          integer vsid, findex
        END FUNCTION vfifisz
      END INTERFACE

      vffisiz = vfifisz(vsid, findex)
      end

C-----------------------------------------------------------
C     vffesiz -- memory size of a field in a vdata
C     related: VFfieldesize -- vfifesz -- vffesiz

      integer function vffesiz(vsid, findex)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vffesiz
	!MS$endif
      integer vsid, findex
C      integer vfifesz
      INTERFACE 
        INTEGER FUNCTION vfifesz(vsid, findex)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFIFESZ' :: vfifesz
          integer vsid, findex
        END FUNCTION vfifesz
      END INTERFACE

      vffesiz = vfifesz(vsid, findex)
      end

C-----------------------------------------------------------
C     vffordr -- order of a field in a vdata
C     related: VFfieldorder -- vfifodr -- vffordr

      integer function vffordr(vsid, findex)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vffordr
	!MS$endif
      integer vsid, findex
C      integer vfifodr
      INTERFACE 
        INTEGER FUNCTION vfifodr(vsid, findex)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFIFODR' :: vfifodr
          integer vsid, findex
        END FUNCTION vfifodr
      END INTERFACE

      vffordr = vfifodr(vsid, findex)
      end

c     ===========================================
c     MISCELLANEOUS USEFUL VDATA INQUIRY ROUTINES
c     ===========================================
c
c     undocumented


c     gets the interlace of a vdata
c     related: VSgetinterlace--vsgintc--vsfgint

      integer 	function vsfgint (vs)					
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfgint
	!MS$endif
      integer 		vs
c      integer		vsgintc
      INTERFACE
        INTEGER FUNCTION vsgintc(vs)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSGINTC' :: vsgintc
          integer vs
        END FUNCTION vsgintc
      END INTERFACE

      vsfgint = vsgintc (vs)
      end

c     ------------------------------------------------------------
c     gets the number of elements in a vdata
c     related: VSelts--vseltsc--vsfelts

      integer 	function vsfelts (vs)			
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfelts
	!MS$endif
      integer  vs	
c      integer	vseltsc
      INTERFACE
        INTEGER FUNCTION vseltsc(vs)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSELTSC' :: vseltsc
          integer vs
        END FUNCTION vseltsc
      END INTERFACE

      vsfelts = vseltsc (vs)
      end

c     ------------------------------------------------------------
c     gets the fields in the vdata
c     related: VSgetfields--vsgfldc--vsfgfld

      integer	function vsfgfld (vs, fields)	
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfgfld
	!MS$endif
      integer			vs
      character*(*) 	fields
c      integer			vsgfldc
      INTERFACE
        INTEGER FUNCTION vsgfldc(vs, fields)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSGFLDC' :: vsgfldc
	    !DEC$ ATTRIBUTES reference :: fields
          integer vs
          character*(*)   fields
        END FUNCTION vsgfldc
      END INTERFACE

      vsfgfld = vsgfldc (vs, fields)
      end

c     ------------------------------------------------------------
c 	determines the (machine) size of the given fields
c     related: VSsizeof--vssizc--vsfsiz

      integer	function vsfsiz (vs, fields)		
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfsiz 
	!MS$endif
      integer			vs
      character*(*) 	fields
c      integer			vssizc
      INTERFACE
        INTEGER FUNCTION vssizc(vs, fields, fldslen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSSIZC' :: vssizc
	    !DEC$ ATTRIBUTES reference :: fields
          integer vs, fldslen
          character*(*) fields
        END FUNCTION vssizc
      END INTERFACE

      vsfsiz = vssizc (vs, fields, len(fields))
      end

c     ------------------------------------------------------------
c     determines the no of entries in a vgroup
c     related: Ventries--ventsc--vfents

      integer 	function	vfents (f, vgid)		
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfents
	!MS$endif
      integer	f, vgid
c      integer	ventsc 
      INTERFACE
        INTEGER FUNCTION ventsc(f, vgid)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VENTSC' :: ventsc
          integer f, vgid
        END FUNCTION ventsc
      END INTERFACE

      vfents = ventsc (f, vgid)
      end

c     ------------------------------------------------------------
c     gets the refs of all lone vgroups in the file
c     related: Vlone--vlonec--vflone

      integer function vflone (f, idarray, asize)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vflone 
	!MS$endif
      integer	f, asize
      integer	idarray(*) 
c      integer	vlonec
      INTERFACE
        INTEGER FUNCTION vlonec(f, idarray, asize)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VLONEC' :: vlonec
          integer f, idarray(*), asize
        END FUNCTION vlonec
      END INTERFACE

      vflone = vlonec  (f, idarray, asize)
      end

c     ------------------------------------------------------------
c     gets the refs of all lone vdatas in the file
c     related: VSlone--vslonec--vsflone

      integer function vsflone (f, idarray, asize)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsflone
	!MS$endif
      integer   f, asize
      integer   idarray(*)
c      integer	vslonec
      INTERFACE
        INTEGER FUNCTION vslonec(f, idarray, asize)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSLONEC' :: vslonec
          integer f, idarray(*), asize
        END FUNCTION vslonec
      END INTERFACE

      vsflone = vslonec  (f, idarray, asize)
      end

c     ------------------------------------------------------------
c     gets the ref # of a vgroup for a given name
c     related: Vfind--vfindc--VFIND

      integer function vfind(f,name)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfind
	!MS$endif
      integer		f
      character*(*)  name
C      integer vfindc 	
      INTERFACE
        INTEGER FUNCTION vfindc(f, name, namelen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFINDC' :: vfindc
	    !DEC$ ATTRIBUTES reference :: name
          integer f, namelen
          character*(*) name
        END FUNCTION vfindc
      END INTERFACE

      vfind = vfindc (f, name, len(name))
      end

c     ------------------------------------------------------------
c     gets the ref # of a vgroup for a given class
c     related: Vfindclass--vfndclsc--VFNDCLS

      integer function vfndcls(f,class)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfndcls
	!MS$endif
      integer		f
      character*(*)  class
c      integer vfndclsc 	
      INTERFACE
        INTEGER FUNCTION vfndclsc(f, class, clslen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFNDCLSC' :: vfndclsc
	    !DEC$ ATTRIBUTES reference :: class
          integer f, clslen
          character*(*) class
        END FUNCTION vfndclsc
      END INTERFACE

      vfndcls = vfndclsc (f, class, len(class))
      end

c     ------------------------------------------------------------
c     store a simple dataset in a vdata
c     related: VHstoredata--vhsdc--vhfsd

      integer function vhfsd(f,field,buf,n,dtype,vsname,vsclass)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vhfsd
	!MS$endif
      integer		f
      character*(*)  field
      integer		buf(*)	
      integer		n, dtype
      character*(*)  vsname, vsclass
c      integer vhsdc 	
      INTERFACE
        INTEGER FUNCTION vhsdc(f,field,buf,n,dtype,vsname,vsclass,
     +                         fldlen, nmlen, clslen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VHSDC' :: vhsdc
	    !DEC$ ATTRIBUTES reference :: field, vsname, vsclass
          integer f, buf(*),n,dtype,fldlen,nmlen, clslen
          character*(*) field,vsname,vsclass
        END FUNCTION vhsdc
      END INTERFACE

      vhfsd = vhsdc (f, field, buf, n, dtype, vsname, vsclass,
     1            len(field), len(vsname), len(vsclass))
      end

c     ------------------------------------------------------------
c     store an aggregate dataset in a vadata
c     related: VHstoredatam--vhsdmc--vhfsdm

      integer function vhfsdm (f,field,buf,n,dtype,vsname,vsclass,order)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vhfsdm
	!MS$endif
      integer			f
      character*(*)  field
      integer			buf(*)	
      integer			n, dtype, order
      character*(*)  vsname, vsclass
c      integer vhsdmc 		
      INTERFACE
        INTEGER FUNCTION vhsdmc(f,field,buf,n,dtype,vsname,vsclass,
     +                         order, fldlen, nmlen, clslen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VHSDMC' :: vhsdmc
	    !DEC$ ATTRIBUTES reference :: field,vsname,vsclass
          integer f, buf(*),n,dtype,order, fldlen,nmlen, clslen
          character*(*) field,vsname,vsclass
        END FUNCTION vhsdmc
      END INTERFACE

      vhfsdm = vhsdmc (f, field, buf, n, dtype, vsname,vsclass, order,
     1             len(field), len(vsname), len(vsclass))
      end

c     ------------------------------------------------------------
c     store a simple char dataset in a vdata
c     related: VHstoredata--vhscdc--vhfscd

      integer function vhfscd(f,field,cbuf,n,dtype,vsname,vsclass)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vhfscd
	!MS$endif
      integer           f
      character*(*)  field
      character      cbuf(*)
      integer           n, dtype
      character*(*)  vsname, vsclass
c      integer vhscdc
      INTERFACE
        INTEGER FUNCTION vhscdc(f,field,cbuf,n,dtype,vsname,vsclass,
     +                         fldlen, nmlen, clslen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VHSCDC' :: vhscdc
	    !DEC$ ATTRIBUTES reference :: field,vsname,vsclass,cbuf
          integer f, n,dtype,fldlen,nmlen, clslen
          character*(*) field,vsname,vsclass
          character cbuf(*)
        END FUNCTION vhscdc
      END INTERFACE

      vhfscd = vhscdc(f,field,cbuf,n,dtype,vsname,vsclass,
     1            len(field), len(vsname), len(vsclass))
      end

c     ------------------------------------------------------------
c     store an aggregate char dataset in a vadata
c     related: VHstoredatam--vhscdmc--vhfscdm

      integer function vhfscdm (f,field,cbuf,n,dtype,vsname,
     1            vsclass,order)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vhfscdm
	!MS$endif
      integer                   f
      character*(*)  field
      character              cbuf(*)
      integer                   n, dtype, order
      character*(*)  vsname, vsclass
c      integer vhscdmc
      INTERFACE
        INTEGER FUNCTION vhscdmc(f,field,cbuf,n,dtype,vsname,vsclass,
     +                         order, fldlen, nmlen, clslen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VHSCDMC' :: vhscdmc
	    !DEC$ ATTRIBUTES reference :: field,vsname,vsclass, cbuf
          integer f, n,dtype,order, fldlen,nmlen, clslen
          character*(*) field,vsname,vsclass
          character   cbuf(*)
        END FUNCTION vhscdmc
      END INTERFACE

      vhfscdm = vhscdmc (f, field, cbuf, n, dtype, vsname,
     1          vsclass, order, len(field), len(vsname), 
     1          len(vsclass))
      end

c     ------------------------------------------------------------
c     make a new vgroup given several tag/ref pairs
c     related: VHmakegroup--vhmkgpc--vhfmkgp

      integer function vhfmkgp(f,tagarray,refarray,n,vgname,vgclass)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vhfmkgp
	!MS$endif
      integer		f, n
      integer		tagarray(*), refarray(*)
      character*(*)  vgname, vgclass
c      integer 		vhmkgpc 	
      INTERFACE
        INTEGER FUNCTION vhmkgpc(f,tagarray, refarray,n,vgname,
     +                         vgclass, nmlen, clslen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VHMKGPC' :: vhmkgpc
	    !DEC$ ATTRIBUTES reference :: vgname,vgclass
          integer f, tagarray(*), refarray(*),n,nmlen,clslen
          character*(*) vgname,vgclass
        END FUNCTION vhmkgpc
      END INTERFACE

      vhfmkgp = vhmkgpc (f, tagarray, refarray , n, vgname, vgclass,
     1                len(vgname), len(vgclass))
      end

c     ============================================================
c     More vgroup routines

c     locate a field in a vdata that belongs to this VGROUP	
c     related: Vflocate--vflocc--vffloc

      integer function vffloc  (vg, field)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vffloc
	!MS$endif
      integer			vg
      character*(*)	field
c      integer 			vflocc
      INTERFACE
        INTEGER FUNCTION vflocc(vg, field, fldlen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFLOCC' :: vflocc
	    !DEC$ ATTRIBUTES reference :: field
          integer vg, fldlen
          character*(*) field
        END FUNCTION vflocc
      END INTERFACE

      vffloc = vflocc (vg, field, len(field))
      end

c     ------------------------------------------------------------
c     tests if a tag/ref pair is in a vgroup.
c     related: Vinqtagref--vinqtrc--vfinqtr

      integer function vfinqtr  (vg, tag, ref)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfinqtr
	!MS$endif
      integer		vg, tag, ref
c      integer		vinqtrc
      INTERFACE
        INTEGER FUNCTION vinqtrc(vg, tag,ref)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VINQTRC' :: vinqtrc
          integer vg, tag,ref
        END FUNCTION vinqtrc
      END INTERFACE

      vfinqtr = vinqtrc (vg, tag, ref)
      end

c     ------------------------------------------------------------
c     gets the number of tag/refs stored in a vgroup
c     related: Vntagrefs -- vntrc -- vfntr

      integer function vfntr (vg)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfntr
	!MS$endif
      integer		vg
c      integer vntrc 
      INTERFACE
        INTEGER FUNCTION vntrc(vg)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VNTRC' :: vntrc
          integer vg
        END FUNCTION vntrc
      END INTERFACE

      vfntr = vntrc (vg)
      end
c     ------------------------------------------------------------
c     gets the number of tags of a given type in a vgroup
c     related: Vnrefs -- vfirefs -- vnrefs

      integer function vnrefs(vgid, tag)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vnrefs
	!MS$endif
      integer           vgid, tag
C      integer vfirefs
      INTERFACE
        INTEGER FUNCTION vfirefs(vgid, tag)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFIREFS' :: vfirefs
          integer vgid, tag
        END FUNCTION vfirefs
      END INTERFACE
      
      vnrefs = vfirefs(vgid, tag)
      end

c     ------------------------------------------------------------
c     gets the reference number of this vgroup
c     related: VQueryref -- vfiqref -- vqref

      integer function vqref(vgid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vqref
	!MS$endif
      integer           vgid
C      integer vfiqref
      INTERFACE
        INTEGER FUNCTION vfiqref(vgid)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFIQREF' :: vfiqref
          integer vgid
        END FUNCTION vfiqref
      END INTERFACE

      vqref = vfiqref(vgid)
      end

c     ------------------------------------------------------------
c     gets the tag of this vgroup
c     related: VQuerytag -- vfiqtag -- vqtag

      integer function vqtag(vgid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vqtag
	!MS$endif
      integer           vgid
C      integer vfiqtag
      INTERFACE
        INTEGER FUNCTION vfiqtag(vgid)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VFIQTAG' :: vfiqtag
          integer vgid
        END FUNCTION vfiqtag
      END INTERFACE

      vqtag = vfiqtag(vgid)
      end


c     ------------------------------------------------------------
c     returns all the tag/ref pairs in a vgroup
c     related: Vgettagrefs--vgttrsc--vfgttrs

      integer function vfgttrs (vg, tagarray, refarray, n)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfgttrs
	!MS$endif
      integer		vg, n
      integer		tagarray(*), refarray(*)
c      integer		vgttrsc
      INTERFACE
        INTEGER FUNCTION vgttrsc(vg, tagarray,refarray, n)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VGTTRSC' :: vgttrsc
          integer vg, tagarray(*),refarray(*)
        END FUNCTION vgttrsc
      END INTERFACE

      vfgttrs = vgttrsc (vg, tagarray, refarray, n)
      end
c     ------------------------------------------------------------
c     returns a specified tag/ref pair in a vgroup
c     related: Vgettagref--vgttrc--vfgttr

      integer function vfgttr (vg, which, tag, ref)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfgttr
	!MS$endif
      integer		vg, which
      integer		tag, ref
c      integer		vgttrc
      INTERFACE
        INTEGER FUNCTION vgttrc(vg, which, tag,ref)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VGTTRC' :: vgttrc
          integer vg, which, tag,ref
        END FUNCTION vgttrc
      END INTERFACE

      vfgttr = vgttrc (vg, which, tag, ref)
      end

c     ------------------------------------------------------------
c     add a tag/ref pair to a vgroup
c     related: Vaddtagref--vadtrc--vfadtr

      integer function vfadtr	( vg, tag, ref)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfadtr
	!MS$endif
      integer		vg, tag, ref
c      integer vadtrc				
      INTERFACE
        INTEGER FUNCTION vadtrc(vg, tag,ref)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VADTRC' :: vadtrc
          integer vg, tag,ref
        END FUNCTION vadtrc
      END INTERFACE

      vfadtr = vadtrc  ( vg, tag, ref)
      end

c     ------------------------------------------------------------
c     specific inquiry on a vdata, gets number of records
c     related: VSQuerycount -- vsiqnelt -- vsqfnelt

      integer function vsqfnelt(vsid, nelts)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsqfnelt
	!MS$endif
      integer                   vsid, nelts
c      integer                   vsiqnelt
      INTERFACE 
        INTEGER FUNCTION vsiqnelt(vsid, nelts)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSIQNELT' :: vsiqnelt
          integer vsid, nelts
        END FUNCTION vsiqnelt
      END INTERFACE
      vsqfnelt = vsiqnelt (vsid,nelts)
      end

c     ------------------------------------------------------------
c     specific inquiry on a vdata, gets interlace
c     related: VSQueryinterlace -- vsiqintr -- vsqfintr

      integer function vsqfintr(vsid,interlace)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsqfintr
	!MS$endif
      integer                   vsid, interlace
c      integer                   vsiqintr
      INTERFACE
        INTEGER FUNCTION vsiqintr(vsid, interlace)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSIQINTR' :: vsiqintr
          integer vsid, interlace
        END FUNCTION vsiqintr
      END INTERFACE

      vsqfintr = vsiqintr (vsid,interlace)
      end

c     ------------------------------------------------------------
c     specific inquiry on a vdata, gets record size on local machine
c     related: VSQueryvsize -- vsiqvsz -- vsqfvsiz 

      integer function vsqfvsiz(vsid,size)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsqfvsiz
	!MS$endif
      integer                   vsid, size
c      integer                   vsiqvsz
      INTERFACE 
        INTEGER FUNCTION vsiqvsz(vsid, size)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSIQVSZ' :: vsiqvsz
          integer vsid, size
        END FUNCTION vsiqvsz
      END INTERFACE

      vsqfvsiz = vsiqvsz (vsid,size)
      end

c     ------------------------------------------------------------
c     specific inquiry on a vdata, gets fields
c     related: VSQueryfields--vsqfldsc--vsqfflds

      integer function vsqfflds (vs,fields) 
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsqfflds
	!MS$endif
      integer			vs
      character*(*)	fields
c      integer			vsqfldsc
      INTERFACE
        INTEGER FUNCTION vsqfldsc(vs, fields, fldslen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSQFLDSC' :: vsqfldsc
	    !DEC$ ATTRIBUTES reference :: fields
          integer vs, fldslen
          character*(*) fields
        END FUNCTION vsqfldsc
      END INTERFACE

      vsqfflds = vsqfldsc (vs,fields, len(fields))
      end

c     ------------------------------------------------------------
c     specific inquiry on a vdata, gets vdata name
c     related: VSQueryname--vsqnamec--vsqfname

      integer function vsqfname (vs,name) 
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsqfname
	!MS$endif
      integer			vs
      character*(*)	name
c      integer			vsqnamec
      INTERFACE
        INTEGER FUNCTION vsqnamec(vs, name, nmlen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSQNAMEC' :: vsqnamec
	    !DEC$ ATTRIBUTES reference :: name
          integer vs, nmlen
          character*(*) name
        END FUNCTION vsqnamec
      END INTERFACE

      vsqfname = vsqnamec (vs,name, len(name))
      end

c     ============================================================
c     pack values of a numeric field into data buf
c     related: VSfpack--vsfncpk--vsfnpak
c     Use ' ' (blank char) for buflds if the the data buf contains
c         all fields of the vdata
c     In Fortran paktype = HDF_VSPACK(0) for packing
c                          HDF_VSUNPACK(1) for unpacking

      integer function vsfnpak(vs,packtype,buflds,buf,bufsz,
     +                        nrecs,pckfld,fldbuf)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfnpak
	!MS$endif
      integer vs, bufsz, nrecs, packtype
      integer buf, fldbuf
      character*(*) buflds, pckfld
c      integer vsfncpk
      INTERFACE
        INTEGER FUNCTION vsfncpk(vs,ptype,buflds,buf,bufsz,
     +               nrecs,pckfld, fldbuf, bfldslen,pfldlen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSFNCPK' :: vsfncpk
	    !DEC$ ATTRIBUTES reference :: buflds, pckfld
          integer vs,ptype,buf, bufsz,nrecs,fldbuf,bfldslen
          integer pfldlen
          character*(*) buflds, pckfld
        END FUNCTION vsfncpk
      END INTERFACE
      
      vsfnpak = vsfncpk(vs,packtype,buflds,buf,bufsz,nrecs,pckfld,
     +                 fldbuf, len(buflds), len(pckfld)) 
      end

c     ============================================================
c     pack values of a char field into data buf
c     related: VSfpack--vsfccpk--vsfcpak
c     Use ' ' (blank char) for buflds if the the data buf contains
c         all fields of the vdata
c     In Fortran paktype = HDF_VSPACK(0) for packing
c                          HDF_VSUNPACK(1) for unpacking

      integer function vsfcpak(vs,packtype,buflds,buf,bufsz,
     +                        nrecs,pckfld,fldbuf)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfcpak
	!MS$endif
      integer vs, packtype, bufsz, nrecs
      integer buf
      character*(*) buflds, pckfld, fldbuf
c      integer vsfccpk
      INTERFACE
        INTEGER FUNCTION vsfccpk(vs,ptype,buflds,buf,bufsz,
     +               nrecs,pckfld, fldbuf, bfldslen,pfldlen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSFCCPK' :: vsfccpk
	    !DEC$ ATTRIBUTES reference :: buflds,fldbuf,  pckfld
          integer vs,ptype,buf, bufsz,nrecs,bfldslen
          integer pfldlen
          character*(*) buflds,fldbuf,  pckfld
        END FUNCTION vsfccpk
      END INTERFACE
      
      vsfcpak = vsfccpk(vs,packtype,buflds,buf,bufsz,nrecs,pckfld,
     +                 fldbuf, len(buflds), len(pckfld)) 
      end
c  
c     ------------------------------------------------------------
c     Delete a tag/ref pair in a vgroup.
c     related: vfdtr()-->vdtrc()-->Vdeletetagref()

      integer function vfdtr	( vg, tag, ref)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vfdtr
	!MS$endif
      integer		vg, tag, ref
C      integer vdtrc				
      INTERFACE
        INTEGER FUNCTION vdtrc(vg, tag, ref)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VDTRC' :: vdtrc
          integer vg, tag, ref
        END FUNCTION vdtrc
      END INTERFACE

      vfdtr = vdtrc  ( vg, tag, ref)
      end
C-------------------------------------------------------------------------
C        Name:      vsffcls 
C        Purpose:   looks in the file, finds the vdata 
C                   with the specified class 
C                   and returns the ref of the vdata with class vdclass 
C        Inputs:    id       -  file ID
C                   vdclass  -  class of vdata to find  
C        Returns:   returns 0 if not found, or error. Otherwise, returns
C                   the vdata's ref number (a positive integer)
C        Calls:     vcffcls (C stub for VSfindclass function)
C        Users:     HDF Fortran programmers
C-------------------------------------------------------------------------

         INTEGER function vsffcls(id, vdclass)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsffcls
	!MS$endif
C
         INTEGER id
         CHARACTER*(*) vdclass
C         INTEGER vscfcls 
      INTERFACE
        INTEGER FUNCTION vscfcls(id, vdclass, vdclasslen)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VSCFCLS' :: vscfcls
	    !DEC$ ATTRIBUTES reference :: vdclass
          integer id, vdclasslen
          character*(*) vdclass
        END FUNCTION vscfcls
      END INTERFACE
C
         vsffcls = vscfcls(id,vdclass, len(vdclass)) 

C
         return 
         end
C-------------------------------------------------------------------------
C        Name:      vdelete
C        Purpose:   deletes vgroup from the file
C        Inputs:    id       -  file ID
C                   vgid     -  vgroup identifier
C        Returns:   returns 0 if successfull, -1 on error
C        Calls:     vdeletec (C stub for Vdelete function)
C        Users:     HDF Fortran programmers
C-------------------------------------------------------------------------
         INTEGER function vdelete(id, vgid)
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vdelete
	!MS$endif
C
         INTEGER id, vgid
 
      INTERFACE
        INTEGER FUNCTION vdeletec(id, vgid)
	    !MS$ATTRIBUTES C,reference,decorate,alias:'VDELETEC' :: vdeletec
          integer id, vgid
        END FUNCTION vdeletec
      END INTERFACE
C
         vdelete = vdeletec(id, vgid)
 
C
         return
         end
 

C-------------------------------------------------------------------------
C        Name:      vsfsetblsz
C        Purpose:   sets the block size of the linked-block element
C        Inputs:    id       -  vdata identifier
C                   block_size - size of each block
C        Returns:   returns 0 if succeeds and -1 if fails
C        Calls:     vscsetblsz (C stub for VSsetblocksize function)
C        Users:     HDF Fortran programmers
C-------------------------------------------------------------------------

         INTEGER function vsfsetblsz(id, block_size)
C
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfsetblsz
	!MS$endif
         INTEGER id, block_size
C
      INTERFACE
        INTEGER FUNCTION vscsetblsz(id, block_size)
	   !MS$ATTRIBUTES C,reference,decorate,alias:'VSCSETBLSZ' :: vscsetblsz
          integer id, block_size
        END FUNCTION vscsetblsz
      END INTERFACE
         vsfsetblsz = vscsetblsz(id, block_size) 
         return 
         end

C-------------------------------------------------------------------------
C        Name:      vsfsetnmbl
C        Purpose:   sets the number of blocks for a linked-block element
C        Inputs:    id       -  vdata identifier
C                   num_blocks - number of blocks to be used for the linked-block
C                                elements
C        Returns:   returns 0 if succeeds and -1 if fails
C        Calls:     vscsetnmbl (C stub for VSsetnumblocks function)
C        Users:     HDF Fortran programmers
C-------------------------------------------------------------------------

         INTEGER function vsfsetnmbl(id, num_blocks)
C
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfsetnmbl
	!MS$endif
         INTEGER id, num_blocks
      INTERFACE
        INTEGER FUNCTION vscsetnmbl(id, num_blocks)
	   !MS$ATTRIBUTES C,reference,decorate,alias:'VSCSETNMBL' :: vscsetnmbl
          integer id, num_blocks
        END FUNCTION vscsetnmbl
      END INTERFACE
C
         vsfsetnmbl = vscsetnmbl(id, num_blocks) 
         return 
         end

C-------------------------------------------------------------------------
C        Name:      vsfgetblinfo
C        Purpose:   retrieves the block size and the number of blocks
C                   of a linked-block element.
C        Inputs:    id       -  vdata identifier
C        Outputs:   block_size - the linked-block size
C                   num_blocks - number of blocks the element has
C        Returns:   returns 0 if succeeds and -1 if fails
C        Calls:     vscgetblinfo (C stub for VSgetblockinfo function)
C        Users:     HDF Fortran programmers
C-------------------------------------------------------------------------

         INTEGER function vsfgetblinfo(id, block_size, num_blocks)
C
	!MS$if defined(BUILD_HDF_DLL)
	!MS$attributes dllexport :: vsfgetblinfo
	!MS$endif
         INTEGER id, num_blocks, block_size
C
      INTERFACE
        INTEGER FUNCTION vscgblinfo(id, block_size, num_blocks)
	   !MS$ATTRIBUTES C,reference,decorate,alias:'VSCGBLINFO' :: vscgblinfo
          integer id, block_size, num_blocks
        END FUNCTION vscgblinfo
      END INTERFACE
         vsfgetblinfo = vscgblinfo(id, block_size, num_blocks) 
         return 
         end
