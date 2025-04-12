# ****************************************************************************
# * NCSA HDF                                                                 *
# * Software Development Group                                               *
# * National Center for Supercomputing Applications                          *
# * University of Illinois at Urbana-Champaign                               *
# * 605 E. Springfield, Champaign IL 61820                                   *
# *                                                                          *
# * For conditions of distribution and use, see the accompanying             *
# * hdf/COPYING file.                                                        *
# *                                                                          *
# ***************************************************************************
#
# $Id: src.inc 4445 2003-12-10 21:15:54Z epourmal $
#
# ##################################################################
#
#           Part of MAKEFILE for creating the HDF library, libdf.a,
#
# This file contains the definitions of all the source code and headers
# used in the actual makefile.  They are located here for ease of maintainence.
#
# ##################################################################
#
#

# Header files not included for copying to 'include' directory during installation
# These are private to the library and users do need them
HDRS_NO= glist.h mcache.h hqueue.h

HDRS=$(H_HDRS) $(DF_HDRS) $(DFAN_HDRS) $(DFRI_HDRS) $(DFSD_HDRS) $(V_HDRS) \
     $(MFAN_HDRS) $(MFGR_HDRS)

H_HDRS=hfile.h hdf.h hdfi.h herr.h hproto.h hcomp.h hconv.h hbitio.h \
       tbbt.h crle.h cnbit.h mstdio.h cnone.h cskphuff.h cdeflate.h \
       hlimits.h hntdefs.h htags.h hkit.h hcompi.h bitvect.h atom.h \
       dynarray.h linklist.h hchunks.h vattr.h cszip.h

DF_HDRS=df.h dfi.h dfstubs.h dfufp2i.h
DFAN_HDRS=dfan.h
DFRI_HDRS=dfrig.h dfgr.h
DFSD_HDRS=dfsd.h
V_HDRS=vg.h
MFAN_HDRS=mfan.h
MFGR_HDRS=mfgr.h

CSRCS = $(HCSRCS) $(VCSRCS) $(DFCSRCS) $(MFCSRCS)
HCSRCS=hfile.c herr.c hblocks.c hbuffer.c hcompri.c hextelt.c hkit.c hdfalloc.c \
    hcomp.c hbitio.c tbbt.c hdfalloc.c hcomp.c crle.c cnbit.c mstdio.c cnone.c \
    cskphuff.c cdeflate.c bitvect.c atom.c hfiledd.c dynarray.c linklist.c \
    glist.c hchunks.c mcache.c cszip.c

VCSRCS=vg.c vrw.c vio.c vparse.c vconv.c vsfld.c vgp.c vhi.c vattr.c

DFCSRCS=$(DFG_CSRCS) $(DFAN_CSRCS) $(DFSD_CSRCS) $(DFRI_CSRCS)
DFG_CSRCS=dfgroup.c dfconv.c dfufp2i.c dfutil.c dfstubs.c dfknat.c dfkswap.c \
          dfkcray.c dfkvms.c dfkconv.c dfkfuji.c 
DFAN_CSRCS=dfan.c
DFSD_CSRCS=dfsd.c
DFRI_CSRCS=dfr8.c dfcomp.c dfrle.c dfimcomp.c dfjpeg.c dfunjpeg.c dfp.c \
           dfgr.c df24.c

MFCSRCS= $(MFAN_CSRCS) $(MFGR_CSRCS)
MFAN_CSRCS=mfan.c
MFGR_CSRCS=mfgr.c

COBJS = $(HCOBJS) $(VCOBJS) $(DFCOBJS) $(MFCOBJS)
HCOBJS=hfile.o herr.o hblocks.o hbuffer.o hcompri.o hextelt.o hkit.o hdfalloc.o \
    hcomp.o hbitio.o tbbt.o crle.o cnbit.o mstdio.o cnone.o cskphuff.o \
    cdeflate.o bitvect.o atom.o hfiledd.o dynarray.o linklist.o glist.o \
    hchunks.o mcache.o cszip.o

VCOBJS=vg.o vrw.o vio.o vparse.o vconv.o vsfld.o vgp.o vhi.o vattr.o

DFCOBJS=$(DFG_COBJS) $(DFAN_COBJS) $(DFSD_COBJS) $(DFRI_COBJS)
DFG_COBJS=dfgroup.o dfconv.o dfufp2i.o dfutil.o dfstubs.o dfknat.o dfkswap.o \
          dfkcray.o dfkvms.o dfkconv.o dfkfuji.o 
DFAN_COBJS=dfan.o
DFSD_COBJS=dfsd.o
DFRI_COBJS=dfr8.o dfcomp.o dfrle.o dfimcomp.o dfjpeg.o dfunjpeg.o dfp.o \
           dfgr.o df24.o

MFCOBJS= $(MFAN_COBJS) $(MFGR_COBJS)
MFAN_COBJS=mfan.o
MFGR_COBJS=mfgr.o

FSRCS = $(FCSRCS) $(FFSRCS)
FCSRCS=$(H_FCSRCS) $(V_FCSRCS) $(DF_FCSRCS) $(DFAN_FCSRCS) $(DFSD_FCSRCS) \
       $(DFRI_FCSRCS) $(MFAN_FCSRCS) $(MFGR_FCSRCS)
H_FCSRCS=hfilef.c herrf.c
V_FCSRCS=vgf.c vattrf.c
DF_FCSRCS=dff.c dfutilf.c
DFAN_FCSRCS=dfanf.c
DFSD_FCSRCS=dfsdf.c
DFRI_FCSRCS=dfr8f.c dfpf.c df24f.c
MFAN_FCSRCS=mfanf.c
MFGR_FCSRCS=mfgrf.c

FFSRCS=$(H_FFSRCS) $(V_FFSRCS) $(DF_FFSRCS) $(DFAN_FFSRCS) $(DFSD_FFSRCS) \
       $(DFRI_FFSRCS) $(MFAN_FFSRCS) $(MFGR_FFSRCS)
H_FFSRCS=hfileff.f 
V_FFSRCS=vgff.f vattrff.f
DF_FFSRCS=dfff.f dfufp2if.f
DFAN_FFSRCS=dfanff.f
DFSD_FFSRCS=dfsdff.f
DFRI_FFSRCS=df24ff.f dfpff.f dfr8ff.f
MFAN_FFSRCS=
MFGR_FFSRCS=mfgrff.f

FOBJS = $(FCOBJS) $(FFOBJS)

FCOBJS=$(H_FCOBJS) $(V_FCOBJS) $(DF_FCOBJS) $(DFAN_FCOBJS) $(DFSD_FCOBJS) \
       $(DFRI_FCOBJS) $(MFAN_FCOBJS) $(MFGR_FCOBJS)
H_FCOBJS=hfilef.o herrf.o 
V_FCOBJS=vgf.o vattrf.o
DF_FCOBJS=dff.o dfutilf.o
DFAN_FCOBJS=dfanf.o
DFSD_FCOBJS=dfsdf.o
DFRI_FCOBJS=dfr8f.o dfpf.o df24f.o
MFAN_FCOBJS=mfanf.o
MFGR_FCOBJS=mfgrf.o

FFOBJS=$(H_FFOBJS) $(V_FFOBJS) $(DF_FFOBJS) $(DFAN_FFOBJS) $(DFSD_FFOBJS) \
       $(DFRI_FFOBJS) $(MFAN_FFOBJS) $(MFGR_FFOBJS)
H_FFOBJS=hfileff.o 
V_FFOBJS=vgff.o vattrff.o
DF_FFOBJS=dfff.o dfufp2if.o
DFAN_FFOBJS=dfanff.o
DFSD_FFOBJS=dfsdff.o
DFRI_FFOBJS=df24ff.o dfpff.o dfr8ff.o
MFAN_FFOBJS=
MFGR_FFOBJS=mfgrff.o

F90INCS=dffunc.f90 hdf.f90
FINCS=dffunc.inc hdf.inc $(F90INCS)


