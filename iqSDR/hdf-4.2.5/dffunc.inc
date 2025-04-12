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
C $Id: dffunc.inc 5001 2007-12-13 07:39:20Z epourmal $
C
C     dffunc.inc
C
C     Declarations of return values for HDF SDS functions
C
      integer  DFSDadddata,		dsadata
      external DFSDadddata,		dsadata
      integer  DFSDclear,		dsclear
      external DFSDclear,		dsclear
      integer  DFSDsetdimscale,		dssdisc
      external DFSDsetdimscale,		dssdisc
      integer  DFSDendslice,		dseslc
      external DFSDendslice,		dseslc
      integer  DFSDgetNT,		dsgnt
      external DFSDgetNT,		dsgnt
      integer  DFSDgetdata,		dsgdata
      external DFSDgetdata,		dsgdata
      integer  DFSDgetdatalen,		dsgdaln
      external DFSDgetdatalen,		dsgdaln
      integer  DFSDgetdatastrs,		dsgdast
      external DFSDgetdatastrs,		dsgdast
      integer  DFSDgetdimlen,		dsgdiln
      external DFSDgetdimlen,		dsgdiln
      integer  DFSDgetdims,		dsgdims
      external DFSDgetdims,		dsgdims
      integer  DFSDgetdimscale,		dsgdisc
      external DFSDgetdimscale,		dsgdisc
      integer  DFSDgetdimstrs,		dsgdist
      external DFSDgetdimstrs,		dsgdist
      integer  DFSDgetrange,		dsgrang
      external DFSDgetrange,		dsgrang
      integer  DFSDgetslice,		dsgslc
      external DFSDgetslice,		dsgslc
      integer  DFSDlastref,		dslref
      external DFSDlastref,		dslref
      integer  DFSDnumber,		dsnum
      external DFSDnumber,		dsnum
      integer  DFSDputdata,		dspdata
      external DFSDputdata,		dspdata
      integer  DFSDputslice,		dspslc
      external DFSDputslice,		dspslc
      integer  DFSDreadref,		dsrref
      external DFSDreadref,		dsrref
      integer  DFSDrestart,		dsfirst
      external DFSDrestart,		dsfirst
      integer  DFSDsetNT,		dssnt
      external DFSDsetNT,		dssnt
      integer  DFSDsetdatastrs,		dssdast
      external DFSDsetdatastrs,		dssdast
      integer  DFSDsetdims,		dssdims
      external DFSDsetdims,		dssdims
      integer  DFSDsetdimstrs,		dssdist
      external DFSDsetdimstrs,		dssdist
      integer  DFSDsetlengths,		dsslens
      external DFSDsetlengths,		dsslens
      integer  DFSDsetrange,		dssrang
      external DFSDsetrange,		dssrang
      integer  DFSDstartslice,		dssslc
      external DFSDstartslice,		dssslc
      integer  dsgcal
      external dsgcal
      integer  dsp32sd
      external dsp32sd
      integer  dsscal
      external dsscal
      integer  dseslab
      external dseslab
      integer  dsrslab
      external dsrslab
      integer  dssslab
      external dssslab
      integer  dswslab
      external dswslab
      integer  dsgfill
      external dsgfill
      integer  dssfill
      external dssfill
      integer  dswref
      external dswref
C
C     Declarations of return values for HDF Annotation functions
C
      integer  DFANputlabel,		daplab
      external DFANputlabel,		daplab
      integer  DFANputdesc,		dapdesc
      external DFANputdesc,		dapdesc
      integer  DFANgetlablen,		dagllen
      external DFANgetlablen,		dagllen
      integer  DFANgetlabel,		daglab
      external DFANgetlabel,		daglab
      integer  DFANgetdesclen,		dagdlen
      external DFANgetdesclen,		dagdlen
      integer  DFANgetdesc,		dagdesc
      external DFANgetdesc,		dagdesc
      integer  DFANlablist,		dallist
      external DFANlablist,		dallist
      integer  DFANaddfid,		daafid
      external DFANaddfid,		daafid
      integer  DFANaddfds,		daafds
      external DFANaddfds,		daafds
      integer  DFANgetfidlen,		dagfidl
      external DFANgetfidlen,		dagfidl
      integer  DFANgetfid,		dagfid
      external DFANgetfid,		dagfid
      integer  DFANgetfdslen,		dagfdsl
      external DFANgetfdslen,		dagfdsl
      integer  DFANgetfds,		dagfds
      external DFANgetfds,		dagfds
      integer  DFANlastref,		dalref
      external DFANlastref,		dalref
      integer  daclear
      external daclear
C
C     Declarations of return values for HDF Raster Image functions
C
      integer  DFR8setpalette,		d8spal
      external DFR8setpalette,		d8spal
      integer  DFR8putimage,		d8pimg
      external DFR8putimage,		d8pimg
      integer  DFR8addimage,		d8aimg
      external DFR8addimage,		d8aimg
      integer  DFR8getdims,		d8gdims
      external DFR8getdims,		d8gdims
      integer  DFR8getimage,		d8gimg
      external DFR8getimage,		d8gimg
      integer  DFR8readref,		d8rref
      external DFR8readref,		d8rref
      integer  DFR8writeref,		d8wref
      external DFR8writeref,		d8wref
      integer  DFR8restart,		d8first
      external DFR8restart,		d8first
      integer  DFR8nimages,             d8nims
      external DFR8nimages,             d8nims
      integer  DFR8lastref,             d8lref
      external DFR8lastref,             d8lref
      integer  DFR8scompress,           d8scomp
      external DFR8scompress,           d8scomp
      integer  DFR8sjpeg,               d8sjpeg
      external DFR8sjpeg,               d8sjpeg

      integer  DF24setil,		d2setil
      external DF24setil,		d2setil
      integer  DF24addimage,		d2aimg
      external DF24addimage,		d2aimg
      integer  DF24putimage,		d2pimg
      external DF24putimage,		d2pimg
      integer  DF24getimage,		d2gimg
      external DF24getimage,		d2gimg
      integer  DF24getdims,		d2gdims
      external DF24getdims,		d2gdims
      integer  DF24setdims,		d2sdims
      external DF24setdims,		d2sdims
      integer  DF24readref,		d2rref
      external DF24readref,		d2rref
      integer  DF24restart,		d2first
      external DF24restart,		d2first
      integer  DF24reqil,		d2reqil
      external DF24reqil,		d2reqil
      integer  			d2lref
      external 			d2lref
      integer  DF24scompress,           d2scomp
      external DF24scompress,           d2scomp
      integer  DF24sjpeg,               d2sjpeg
      external DF24sjpeg,               d2sjpeg

      integer  DFPaddpal,		dpapal
      external DFPaddpal,		dpapal
      integer  DFPgetpal,		dpgpal
      external DFPgetpal,		dpgpal
      integer  DFPputpal,		dpppal
      external DFPputpal,		dpppal
      integer  DFPnpals,		dpnpals
      external DFPnpals,		dpnpals
      integer  DFPwriteref,		dpwref
      external DFPwriteref,		dpwref
      integer  DFPreadref,		dprref
      external DFPreadref,		dprref
      integer  DFPrestart,		dprest
      external DFPrestart,		dprest
      integer  DFPlastref,		dplref
      external DFPlastref,		dplref

C
C     Declarations of return values for HDF Raster Image functions
C
      integer  DFopen
      external DFopen
      integer  DFclose
      external DFclose
      integer  DFsfind
      external DFsfind
      integer  DFfind
      external DFfind
      integer  DFget
      external DFget
      integer  DFput
      external DFput
      integer  DFaccess
      external DFaccess
      integer  DFread
      external DFread
      integer  DFwrite
      external DFwrite
      integer  DFseek
      external DFseek
      integer  DFupdate
      external DFupdate
      integer  DFdup
      external DFdup
      integer  DFdel
      external DFdel
      integer  DFerrno
      external DFerrno
      integer  DFishdf
      external DFishdf
      integer  DFnewref
      external DFnewref
      integer  DFnumber
      external DFnumber
      integer  DFstat
      external DFstat

C
C     Declarations of return values for HDF Vgroup functions
C
      integer  vfadtr 
      external vfadtr 
      integer  vfainfo
      external vfainfo
      integer  vfatch
      external vfatch
      integer  vfdtch
      external vfdtch
      integer  vfdtr
      external vfdtr
      integer  vfend
      external vfend
      integer  vfents
      external vfents
      integer  vffdatt
      external vffdatt
      integer  vffloc
      external vffloc
      integer  vfgnatt
      external vfgnatt
      integer  vfgcatt
      external vfgcatt
      integer  vfgcls
      external vfgcls
      integer  vfgid
      external vfgid
      integer  vfgnam
      external vfgnam
      integer  vfgnxt
      external vfgnxt
      integer  vfgttr
      external vfgttr
      integer  vfgttrs
      external vfgttrs
      integer  vfgver
      external vfgver
      integer  vfind
      external vfind
      integer  vfinq
      external vfinq
      integer  vfinqtr
      external vfinqtr
      integer  vfinsrt
      external vfinsrt
      integer  vfisvg
      external vfisvg
      integer  vfisvs
      external vfisvs
      integer  vflone
      external vflone
      integer  vfnatts
      external vfnatts
      integer  vfntr
      external vfntr
      integer  vfsnatt
      external vfsnatt
      integer  vfscatt
      external vfscatt
      integer  vfscls
      external vfscls
      integer  vfsnam
      external vfsnam
      integer  vfstart
      external vfstart

C
C     Declarations of return values for HDF high level Vdata/Vgroup
C     functions
C
      integer  vhfmkgp
      external vhfmkgp
      integer  vhfscd
      external vhfscd
      integer  vhfsd
      external vhfsd
      integer  vhfscdm
      external vhfscdm
      integer  vhfsdm
      external vhfsdm

C
C     Declarations of return values for HDF Vdata functions
C
      integer  vsfainf
      external vsfainf
      integer  vsfatch
      external vsfatch
      integer  vsfcpak
      external vsfcpak
      integer  vsfdlte
      external vsfdlte
      integer  vsfdtch
      external vsfdtch
      integer  vsfelts
      external vsfelts
      integer  vsfex
      external vsfex
      integer  vsffdef
      external vsffdef
      integer  vsffdat
      external vsffdat
      integer  vsffidx
      external vsffidx
      integer  vsffnas
      external vsffnas
      integer  vsffnd
      external vsffnd
      integer  vsfgnat
      external vsfgnat
      integer  vsfgcat
      external vsfgcat
      integer  vsfgcls
      external vsfgcls
      integer  vsfgfld
      external vsfgfld
      integer  vsfgid
      external vsfgid
      integer  vsfgint
      external vsfgint
      integer  vsfgnam
      external vsfgnam
      integer  vsfinq
      external vsfinq
      integer  vsfisat
      external vsfisat
      integer  vsflone
      external vsflone
      integer  vsfnats
      external vsfnats
      integer  vsfndc
      external vsfndc
      integer  vsfnpak
      external vsfnpak
      integer  vsfrd
      external vsfrd
      integer  vsfrdc
      external vsfrdc
      integer  vsfread
      external vsfread
      integer  vsfsnat
      external vsfsnat
      integer  vsfscat
      external vsfscat
      integer  vsfscls
      external vsfscls
      integer  vsffcls
      external vsffcls
      integer  vsfseek
      external vsfseek
      integer  vsfsextf
      external vsfsextf
      integer  vsfsfld
      external vsfsfld
      integer  vsfsint
      external vsfsint
      integer  vsfsiz
      external vsfsiz
      integer  vsfsnam
      external vsfsnam
      integer  vsfwrt
      external vsfwrt
      integer  vsfwrtc
      external vsfwrtc
      integer  vsfwrit
      external vsfwrit
      external vsfsetblsz
      integer  vsfsetblsz
      external vsfsetnmbl 
      integer  vsfsetnmbl
      external vsfgetblinfo
      integer  vsfgetblinfo
C
C     Declarations of return values for HDF Vdata Field Functions
C
      external vffesiz
      integer  vffesiz 
      external vffisiz
      integer  vffisiz
      external vffname
      integer  vffname
      external vffordr
      integer  vffordr
      external vfftype
      integer  vfftype
      external vfnflds
      integer  vfnflds
C
C     Declarations of return values for HDF Vdata Query functions
C
      integer  vsqfnelt
      external vsqfnelt
      integer  vsqfintr
      external vsqfintr
      integer  vsqfflds
      external vsqfflds
      integer  vsqfvsiz
      external vsqfvsiz
      integer  vsqfname
      external vsqfname

C
C     Declarations of return values for HDF low level H functions
C
      integer  hclose
      external hclose
      integer  heprnt
      external heprnt
      integer  hnumber
      external hnumber
      integer  hopen
      external hopen
      integer  hxscdir
      external hxscdir
      integer  hxsdir
      external hxsdir
      integer  hddontatexit
      external hddontatexit
      integer  hglibver
      external hglibver
      integer  hgfilver
      external hgfilver
      integer  hishdff
      external hishdff
      integer  hestringf
      external hestringf
      integer  heprntf
      external heprntf
      integer  hconfinf
      external hconfinf
      

C
C     Decls of MGxxx functions for Fortran multi-file GR interface
C

      integer  mgstart
      external mgstart
      integer  mgfinfo
      external mgfinfo
      integer  mgend       
      external mgend       
      integer  mgcreat    
      external mgcreat    
      integer  mgselct     
      external mgselct     
      integer  mgn2ndx    
      external mgn2ndx    
      integer  mggiinf     
      external mggiinf     
      integer  mgwrimg     
      external mgwrimg     
      integer  mgrdimg     
      external mgrdimg     
      integer  mgendac     
      external mgendac     
      integer  mgid2rf     
      external mgid2rf     
      integer  mgr2idx     
      external mgr2idx     
      integer  mgrltil     
      external mgrltil     
      integer  mgrimil     
      external mgrimil     
      integer  mggltid     
      external mggltid     
      integer  mgglinf     
      external mgglinf     
      integer  mgwrlut     
      external mgwrlut     
      integer  mgrdlut     
      external mgrdlut     
      integer  mgsxfil    
      external mgsxfil    
      integer  mgsattr    
      external mgsattr    
      integer  mgatinf     
      external mgatinf     
      integer  mggattr     
      external mggattr     
      integer  mgfndat    
      external mgfndat    
      integer  mggichnk
      external mggichnk
      integer  mgscchnk
      external mgscchnk
      integer  mgschnk
      external mgschnk
      integer  mgwchnk
      external mgwchnk
      integer  mgwcchnk
      external mgwcchnk
      integer  mgrchnk
      external mgrchnk
      integer  mgrcchnk
      external mgrcchnk
      integer  mgscompress
      external mgscompress
      integer  mggcompress
      external mggcompress
      integer  mglt2rf
      external mglt2rf
      integer  mggnluts
      external mggnluts

C
C       Decls of ANxxx functions for Fortran multi-file AN interface
C
      integer  afstart
      external afstart
      integer  affileinfo
      external affileinfo
      integer  afend       
      external afend       
      integer  afcreate
      external afcreate
      integer  affcreate
      external affcreate
      integer  afselect
      external afselect
      integer  afnumann
      external afnumann
      integer  afannlist
      external afannlist
      integer  afannlen
      external afannlen
      integer  afwriteann
      external afwriteann
      integer  afreadann
      external afreadann
      integer  afendaccess
      external afendaccess
      integer  afgettagref
      external afgettagref
      integer  afidtagref
      external afidtagref
      integer  aftagrefid
      external aftagrefid
      integer  afatypetag
      external afatypetag
      integer  aftagatype
      external aftagatype


C
C       Decls of SFxxx functions for Fortran multi-file interface
C
      integer  sfstart
      external sfstart
      integer  sfn2index
      external sfn2index
      integer  sfcreate
      external sfcreate
      integer  sfsdmstr
      external sfsdmstr
      integer  sfsdmname
      external sfsdmname
      integer  sfsdtstr
      external sfsdtstr
      integer  sfgdtstr
      external sfgdtstr
      integer  sfgdmstr
      external sfgdmstr
      integer  sfginfo
      external sfginfo
      integer  sfgainfo
      external sfgainfo
      integer  sfgdinfo
      external sfgdinfo
      integer  sfsattr
      external sfsattr
      integer  sfsnatt
      external sfscatt
      integer  sfscatt
      integer  sffattr
      external sffattr
      integer  sfend
      external sfend
      integer  sfendacc
      external sfendacc
      integer  sffinfo
      external sffinfo
      integer  sfselect
      external sfselect
      integer  sfdimid
      external sfdimid
      integer  sfgcal
      external sfgcal
      integer  sfscal
      external sfscal
      integer  sfsdscale
      external sfsdscale
      integer  sfgdscale
      external sfgdscale
      integer  sfsfill
      external sfsfill
      integer  sfgfill
      external sfgfill
      integer  sfgrange
      external sfgrange
      integer  sfsrange
      external sfsrange
      integer  sfrattr
      external sfrattr
      integer  sfrnatt
      external sfrcatt
      integer  sfrcatt
      integer  sfrdata
      external sfrdata
      integer  sfwdata
      external sfwdata
      integer  sfsextf
      external sfsextf
      integer  sfsnbit
      external sfsnbit
      integer  sfsacct
      external sfsacct
      integer  sfid2ref
      external sfid2ref
      integer  sfiscvar
      external sfiscvar
      integer  sfref2index
      external sfref2index
      integer  sfsdmvc
      external sfsdmvc
      integer  sfisdmvc
      external sfisdmvc
      integer  sfgichnk
      external sfgichnk
      integer  sfrcchnk
      external sfrcchnk
      integer  sfrchnk
      external sfrchnk
      integer  sfwcchnk
      external sfwcchnk
      integer  sfwchnk
      external sfwchnk
      integer  sfscchnk
      external sfscchnk
      integer  sfschnk
      external sfschnk
      integer  sfscompress
      external sfscompress
      integer  sfgcompress
      external sfgcompress
      integer  sfisrcrd
      external sfisrcrd
      integer  sfsblsz
      external sfsblsz
      integer  sfchempty
      external sfchempty
      integer  sfwcdata
      external sfwcdata
      integer  sfrcdata
      external sfrcdata
      integer  sfgcfill
      external sfgcfill
      integer  sfscfill
      external sfscfill
      integer  sfsflmd
      external sfsflmd

C End of declarations

