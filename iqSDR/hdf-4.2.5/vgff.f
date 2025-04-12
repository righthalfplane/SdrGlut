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
C $Id: vgff.f 4932 2007-09-07 17:17:23Z bmribler $ 
C     
c **************************************************************************
c *
c * vgFf.f
c * Part of the HDF VSet interface.
c *
c * Contains fortran routines callable from fortran programs.
c *
c **************************************************************************


c   ============================================================
c	 VGROUP ROUTINES
c	============================================================

c	attachs to a vgroup 	
c	related: Vattach--vatchc--VFATCH

      integer function vfatch(f, vgid, accesstype)
      integer     f, vgid
      character*1 accesstype
      integer     vatchc

      vfatch = vatchc (f, vgid, accesstype)
      end

c	------------------------------------------------------------
c	detaches from a vgroup
c	related: Vdetach--vdtchc--VFDTCH

      integer function vfdtch (vg)
      integer     vg
      integer   vdtchc

      vfdtch = vdtchc (vg)
      end

c	------------------------------------------------------------
c	general inquiry on a vgroup
c	related: Vgetname--vgnamc--VFGNAM

      integer function vfgnam (vg, vgname)
      integer         vg
      character*(*)   vgname
      integer         vgnamc

      vfgnam = vgnamc (vg, vgname)
      end
c	------------------------------------------------------------
c	get the class name of a vgroup
c	related: Vgetclass--vgclsc--VFGCLS

      integer function vfgcls (vg, vgclass)

      integer   vg
      character*(*)   vgclass
      integer       vgclsc

      vfgcls = vgclsc  (vg, vgclass)
      end
c   ------------------------------------------------------------
c	general inquiry on a vgroup
c	related: Vinquire--vinqc--VFINQ

      integer   function    vfinq (vg, nentries, vgname)
      integer   vg, nentries
      character*(*)   vgname
      integer   vinqc

      vfinq = vinqc (vg, nentries, vgname)
      end

c   ------------------------------------------------------------
c	gets the id of the next vgroup in the file
c	related: Vgetid--vgidc--VFGID

      integer   function    vfgid (f, vgid)
      integer     f, vgid
      integer     vgidc

      vfgid = vgidc (f, vgid)
      end

c	------------------------------------------------------------
c	gets the id of the next entry in the vgroup
c	related: Vgetnext--vgnxtc--VFGNXT

      integer   function    vfgnxt (vg, id)
      integer   vg, id
      integer   vgnxtc

      vfgnxt = vgnxtc (vg, id)
      end


c	------------------------------------------------------------
c	sets the name of the vgroup
c	related: Vsetname--vsnamc--VFSNAM

      integer function vfsnam (vg, vgname)
      integer        vg
      character*(*)  vgname
      integer        vsnamc

      vfsnam = vsnamc (vg, vgname, len(vgname))
      end
c   ------------------------------------------------------------
c	sets the class name of the vgroup
c	related: Vsetclass--vsclsc--VFSCLS

      integer function vfscls (vg, vgclass)
      integer       vg
      character*(*) vgclass
      integer       vsclsc

      vfscls = vsclsc  (vg, vgclass, len(vgclass))
      end

c	------------------------------------------------------------
c	inserts a vset entity (ie vgroup or vdata) into the given vgroup
c	related: Vinsert--vinsrtc--VFINSRT

      integer   function vfinsrt (vg, velt)
      integer   vg, velt
      integer   vinsrtc

      vfinsrt = vinsrtc (vg, velt)
      end

c     ------------------------------------------------------------
c     tests if an id in a vgroup is a vgroup
c     related: Visvg--visvgc--vfisvg

      integer function vfisvg (vg, id)
      integer vg, id
      integer visvgc

      vfisvg = visvgc (vg, id)
      end

c     ------------------------------------------------------------
c     tests if an id in a vgroup is a vdata
c     related: Visvs--visvsc--vfisvs

      integer   function vfisvs (vg, id)
      integer   vg, id
      integer   visvsc

      vfisvs = visvsc (vg, id)
      end



c     ============================================================
c      VDATA ROUTINES
c     ============================================================

c     attach to a vdata
c     related: VSattach--vsatchc--vfatch

      integer function vsfatch (f, vsid, accesstype)
      integer     f, vsid
      character*1 accesstype
      integer     vsatchc

      vsfatch = vsatchc (f, vsid, accesstype)
      end

c     ------------------------------------------------------------
c     detach from a vdata
c     related: VSdetach--vsdtchc--vfdtch

      integer function vsfdtch (vs)
      integer vs
      integer vsdtchc

      vsfdtch = vsdtchc (vs)
      end

c     ------------------------------------------------------------
c     seeks to a given element position in a vadata
c     related: VSseek--vsseekc--vsfseek

      integer   function vsfseek (vs, eltpos )
      integer   vs, eltpos
      integer   vsseekc

      vsfseek = vsseekc (vs, eltpos)
      end

c     ------------------------------------------------------------
c     gets the name of a vdata
c     related: VSgetname--vsgnamc--vsfgnam

      integer function vsfgnam (vs, vsname)
      integer       vs
      character*(*) vsname
      integer       vsgnamc

      vsfgnam = vsgnamc (vs, vsname, len(vsname))
      end

c     ------------------------------------------------------------
c     get the class name of a vdata
c     related: VSgetclass--vsgclsc--vsfgcls

      integer function vsfgcls (vs, vsclass)
      integer       vs
      character*(*) vsclass
      integer       vsgclsc

      vsfgcls = vsgclsc  (vs, vsclass, len(vsclass))
      end

c     ------------------------------------------------------------
c     general inquiry on a vdata
c     related: VSinquire--vsinqc--vsfinq

      integer function vsfinq (vs,nvs,ilace,fields,vsize,vsname) 
      integer        vs, nvs, ilace, vsize
      character*(*)  fields, vsname
      integer        vsinqc

      vsfinq = vsinqc (vs, nvs, ilace, fields, vsize, vsname,
     +             len(fields), len(vsname))
      end

c     ------------------------------------------------------------
c     tests if given fields exist in the vdata
c     related: VSfexist--vsfexc--vsfex

      integer   function vsfex (vs, fields)
      integer   vs
      character*(*) fields
      integer       vsfexc

      vsfex = vsfexc (vs, fields, len(fields))
      end

c     ------------------------------------------------------------
c     looks for a Vdata with a given name
c     related: VSfind--vsfind--vsffnd

      integer function vsffnd (vs, name)
      integer       vs
      character*(*) name
      integer vsfndc

      vsffnd = vsfndc (vs, name, len(name))
      end

c  ------------------------------------------------------------
c     gets the id of the next vdata from the file
c     related: VSgetid--vsgidc--vsfgid

      integer  function vsfgid (f, vsid)
      integer  f, vsid
      integer  vsgidc

      vsfgid = vsgidc (f, vsid)
      end

c  ------------------------------------------------------------
c     remove the vdata with id from the file
c     related: VSdelete--vsdltc--vsfdlte

      integer  function vsfdlte (f, vsid)
      integer     f, vsid
      integer     vsdltc

      vsfdlte = vsdltc (f, vsid)
      end

c     ------------------------------------------------------------
c     sets the name of a vdata
c     related: VSsetname--vssnamc--vsfsnam

      integer function vsfsnam (vs, vsname)
      integer       vs
      character*(*) vsname
      integer       vssnamc

      vsfsnam = vssnamc (vs, vsname, len(vsname))
      end

c     ------------------------------------------------------------
c     set the class name of a vdata
c     related: VSsetclass--vssclsc--vsfscls

      integer function vsfscls (vs, vsclass)
      integer       vs
      character*(*) vsclass
      integer vssclsc

      vsfscls = vssclsc  (vs, vsclass, len(vsclass))
      end

c     ------------------------------------------------------------
c     sets the fields in a vdata for reading or writing
c     related: VSsetfields--vssfldc--vsfsfld

      integer function vsfsfld (vs, fields)
      integer   vs
      character*(*)  fields
      integer        vssfldc

      vsfsfld = vssfldc (vs, fields, len(fields))
      end

c     ------------------------------------------------------------
c     sets the file interlace of a vdata
c     related: VSsetinterlace--vssintc--vsfsint

      integer   function vsfsint (vs, interlace)
      integer   vs, interlace
      integer   vssintc

      vsfsint = vssintc (vs, interlace)
      end

c     ------------------------------------------------------------
c     defines a new field to be used in the vdata
c     related: VSfdefine--vsfdefc--vsffdef

      integer function vsffdef (vs, field, localtype, order)

      integer        vs, localtype, order
      character*(*)  field
      integer        vsfdefc

      vsffdef = vsfdefc ( vs, field, localtype, order, len(field))

      end

c     ------------------------------------------------------------
c     reads from a vdata
c     related: VSread--vsreadc--vsfread

      integer function vsfread (vs, buf, nelts , interlace)

      integer    vs, nelts , interlace
      character*(*)   buf
      integer    vsreadc

      vsfread = vsreadc (vs, buf, nelts, interlace)
      end

c     ------------------------------------------------------------
c     writes to a vdata
c     related: VSwrite--vswritc--vsfwrit

      integer function vsfwrit (vs, buf, nelts, interlace)

      integer       vs, nelts, interlace
      character*(*) buf(*)
      integer       vswritc

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

      character*(*) fname
      integer       vid, offset, vssextfc

      vsfsextf = vssextfc(vid, fname, offset, len(fname))
      return
      end

c     ===========================================
c     MISCELLANEOUS USEFUL VDATA INQUIRY ROUTINES
c     ===========================================
c
c     undocumented


c     gets the interlace of a vdata
c     related: VSgetinterlace--vsgintc--vsfgint

      integer   function vsfgint (vs)
      integer   vs
      integer   vsgintc

      vsfgint = vsgintc (vs)
      end

c     ------------------------------------------------------------
c     gets the number of elements in a vdata
c     related: VSelts--vseltsc--vsfelts

      integer function vsfelts (vs)
      integer vs
      integer vseltsc

      vsfelts = vseltsc (vs)
      end

c     ------------------------------------------------------------
c     gets the fields in the vdata
c     related: VSgetfields--vsgfldc--vsfgfld

      integer function vsfgfld (vs, fields)
      integer vs
      character*(*) fields
      integer  vsgfldc

      vsfgfld = vsgfldc (vs, fields)
      end

c     ------------------------------------------------------------
c 	determines the (machine) size of the given fields
c     related: VSsizeof--vssizc--vsfsiz

      integer function vsfsiz (vs, fields)
      integer vs
      character*(*) fields
      integer vssizc

      vsfsiz = vssizc (vs, fields, len(fields))
      end

c     ------------------------------------------------------------
c     determines the no of entries in a vgroup
c     related: Ventries--ventsc--vfents

      integer function vfents (f, vgid)
      integer f, vgid
      integer ventsc 

      vfents = ventsc (f, vgid)
      end

c     ------------------------------------------------------------
c     gets the refs of all lone vgroups in the file
c     related: Vlone--vlonec--vflone

      integer function vflone (f, idarray, asize)
      integer f
      integer idarray(*)
      integer asize
      integer vlonec

      vflone = vlonec  (f, idarray, asize)
      end

c     ------------------------------------------------------------
c     gets the refs of all lone vdatas in the file
c     related: VSlone--vslonec--vsflone

      integer function vsflone (f, idarray, asize)
      integer f
      integer idarray(*)
      integer asize
      integer vslonec

      vsflone = vslonec  (f, idarray, asize)
      end

c     ------------------------------------------------------------
c     gets the ref # of a vgroup for a given name
c     related: Vfind--vfindc--VFIND

      integer function vfind(f,name)
      integer f
      character*(*)  name
      integer vfindc

      vfind = vfindc (f, name, len(name))
      end

c     ------------------------------------------------------------
c     gets the ref # of a vgroup for a given class
c     related: Vfindclass--vfndclsc--VFNDCLS

      integer function vfndcls(f,class)
      integer f
      character*(*)  class
      integer vfndclsc

      vfndcls = vfndclsc (f, class, len(class))
      end

c     ------------------------------------------------------------
c     store a simple dataset in a vdata
c     related: VHstoredata--vhsdc--vhfsd

      integer function vhfsd(f,field,buf,n,dtype,vsname,vsclass)
      integer f
      character*(*)  field
      integer buf(*)
      integer n, dtype
      character*(*)  vsname, vsclass
      integer vhsdc

      vhfsd = vhsdc (f, field, buf, n, dtype, vsname, vsclass,
     1            len(field), len(vsname), len(vsclass))
      end

c     ------------------------------------------------------------
c     store an aggregate dataset in a vadata
c     related: VHstoredatam--vhsdmc--vhfsdm

      integer function vhfsdm (f,field,buf,n,dtype,vsname,vsclass,order)
      integer f
      character*(*)  field
      integer        buf(*)
      integer        n, dtype, order
      character*(*)  vsname, vsclass
      integer vhsdmc

      vhfsdm = vhsdmc (f, field, buf, n, dtype, vsname,vsclass, order,
     1             len(field), len(vsname), len(vsclass))
      end

c     ------------------------------------------------------------
c     store a simple char dataset in a vdata
c     related: VHstoredata--vhscdc--vhfscd

      integer function vhfscd(f,field,cbuf,n,dtype,vsname,vsclass)
      integer        f
      character*(*)  field
      character      cbuf(*)
      integer        n, dtype
      character*(*)  vsname, vsclass
      integer vhscdc

      vhfscd = vhscdc(f,field,cbuf,n,dtype,vsname,vsclass,
     1            len(field), len(vsname), len(vsclass))
      end

c     ------------------------------------------------------------
c     store an aggregate char dataset in a vadata
c     related: VHstoredatam--vhscdmc--vhfscdm

      integer function vhfscdm (f,field,cbuf,n,dtype,vsname,
     1            vsclass,order)
      integer        f
      character*(*)  field
      character      cbuf(*)
      integer        n, dtype, order
      character*(*)  vsname, vsclass
      integer vhscdmc

      vhfscdm = vhscdmc (f, field, cbuf, n, dtype, vsname,
     1          vsclass, order, len(field), len(vsname), 
     1          len(vsclass))
      end

c     ------------------------------------------------------------
c     make a new vgroup given several tag/ref pairs
c     related: VHmakegroup--vhmkgpc--vhfmkgp

      integer function vhfmkgp(f,tagarray,refarray,n,vgname,vgclass)
      integer f, n
      integer tagarray(*), refarray(*)
      character*(*)  vgname, vgclass
      integer vhmkgpc 

      vhfmkgp = vhmkgpc (f, tagarray, refarray , n, vgname, vgclass,
     1                len(vgname), len(vgclass))
      end

c     ============================================================
c     More vgroup routines

c     locate a field in a vdata that belongs to this VGROUP	
c     related: Vflocate--vffloc--vflocc

      integer function vffloc  (vg, field)
      integer vg
      character*(*) field
      integer vflocc
      vffloc = vflocc (vg, field, len(field))
      end

c     ------------------------------------------------------------
c     tests if a tag/ref pair is in a vgroup.
c     related: Vinqtagref--vinqtrc--vfinqtr

      integer function vfinqtr  (vg, tag, ref)
      integer vg, tag, ref
      integer vinqtrc
      vfinqtr = vinqtrc (vg, tag, ref)
      end

c     ------------------------------------------------------------
c     gets the number of tag/refs stored in a vgroup
c     related: Velts--veltsc--vfelts

      integer function vfntr (vg)
      integer vg
      integer vntrc 
      vfntr = vntrc (vg)
      end

c     ------------------------------------------------------------
c     returns all the tag/ref pairs in a vgroup
c     related: Vgettagrefs--vgttrsc--vfgttrs

      integer function vfgttrs (vg, tagarray, refarray, n)
      integer vg, n
      integer tagarray(*), refarray(*)
      integer vgttrsc

      vfgttrs = vgttrsc (vg, tagarray, refarray, n)
      end
c     ------------------------------------------------------------
c     returns a specified tag/ref pair in a vgroup
c     related: Vgettagref--vgttrc--vfgttr

      integer function vfgttr (vg, which, tag, ref)
      integer vg, which
      integer tag, ref
      integer vgttrc

      vfgttr = vgttrc (vg, which, tag, ref)
      end

c     ------------------------------------------------------------
c     add a tag/ref pair to a vgroup
c     related: Vaddtagref--vadtrc--vfadtr

      integer function vfadtr( vg, tag, ref)
      integer vg, tag, ref
      integer vadtrc

      vfadtr = vadtrc  ( vg, tag, ref)
      end

c     ------------------------------------------------------------
c     specific inquiry on a vdata, gets fields
c     related: VSQueryfields--vsqfldsc--vsqfflds

      integer function vsqfflds (vs,fields) 
      integer vs
      character*(*) fields
      integer       vsqfldsc

      vsqfflds = vsqfldsc (vs,fields, len(fields))
      end

c     ------------------------------------------------------------
c     specific inquiry on a vdata, gets vdata name
c     related: VSQueryname--vsqnamec--vsqfname

      integer function vsqfname (vs,name) 
      integer vs
      character*(*) name
      integer vsqnamec

      vsqfname = vsqnamec (vs,name, len(name))
      end

c     ============================================================
c     pack values of a numeric field into data buf
c     related: VSfpack--vsfncpk--vsfnpak
c     Use ' ' (blank char) for buflds if the the data buf contains
c         all fields of the vdata
c     paktype = _HDF_VSPACK(0) for packing
c               _HDF_VSUNPACK(1) for unpacking

      integer function vsfnpak(vs,packtype,buflds,buf,bufsz,
     +                        nrecs,pckfld,fldbuf)
      integer vs, bufsz, nrecs
      integer buf, fldbuf
      character*(*) buflds, pckfld
      integer vsfncpk
      
      vsfnpak = vsfncpk(vs,packtype,buflds,buf,bufsz,nrecs,pckfld,
     +                 fldbuf, len(buflds), len(pckfld)) 
      end

c     ============================================================
c     pack values of a char field into data buf
c     related: VSfpack--vsfccpk--vsfcpak
c     Use ' ' (blank char) for buflds if the the data buf contains
c         all fields of the vdata
c     paktype = _HDF_VSPACK(0) for packing
c               _HDF_VSUNPACK(1) for unpacking

      integer function vsfcpak(vs,packtype,buflds,buf,bufsz,
     +                        nrecs,pckfld,fldbuf)
      integer vs, bufsz, nrecs
      integer buf
      character*(*) buflds, pckfld, fldbuf
      integer vsfccpk
      
      vsfcpak = vsfccpk(vs,packtype,buflds,buf,bufsz,nrecs,pckfld,
     +                 fldbuf, len(buflds), len(pckfld)) 
      end
c  

c     ------------------------------------------------------------
c     Delete a tag/ref pair in a vgroup.
c     related: vfdtr()-->vdtrc()-->Vdeletetagref()

      integer function vfdtr( vg, tag, ref)
      integer vg, tag, ref
      integer vdtrc

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
C
         INTEGER id
         CHARACTER*(*) vdclass
         INTEGER vscfcls 
C
         vsffcls = vscfcls(id,vdclass, len(vdclass)) 

C
         return 
         end

C-------------------------------------------------------------------------
C        Name:      vffname 
C        Purpose:   returns the name of a vdata field 
C        Inputs:    vdata_id - vdata identofoer
C                   field_index - field index    
C        Output:    field_name           
C        Returns:   returns 0 on if successful and -1 otherwise  
C        Calls:     vffnamec (C stub for VDfieldname function)
C        Users:     HDF Fortran programmers
C-------------------------------------------------------------------------

         INTEGER function vffname(vdata_id, field_index, field_name)
         INTEGER vdata_id, field_index
         CHARACTER*(*) field_name
         INTEGER vffnamec 
         vffname = vffnamec(vdata_id, field_index, field_name,
     .                      len(field_name)) 
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
         INTEGER id, block_size
         INTEGER vscsetblsz
C
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
         INTEGER id, num_blocks
         INTEGER vscsetnmbl
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
         INTEGER id, num_blocks, block_size
         INTEGER vscgblinfo
C
         vsfgetblinfo = vscgblinfo(id, block_size, num_blocks) 
         return 
         end
