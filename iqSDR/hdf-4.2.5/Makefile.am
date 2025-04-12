#############################################################################
##                          Library to build                               ##
#############################################################################

include $(top_srcdir)/config/commence.am

lib_LTLIBRARIES = libdf.la

FSOURCES = df24ff.f dfanf.c dfanff.f dff.c dfff.f dfpf.c dfpff.f dfr8f.c    \
           dfr8ff.f dfsdf.c dfsdff.f dfufp2iff.f dfutilf.c herrf.c hfilef.c  \
	   df24f.c dfufp2if.c\
           hfileff.f mfanf.c mfgrf.c mfgrff.f vattrf.c vattrff.f vgf.c vgff.f 
CSOURCES = atom.c bitvect.c cdeflate.c cnbit.c cnone.c crle.c cskphuff.c    \
           cszip.c df24.c dfan.c dfcomp.c dfconv.c dfgr.c dfgroup.c         \
           dfimcomp.c dfjpeg.c dfkconv.c dfkcray.c dfkfuji.c dfknat.c       \
           dfkswap.c dfkvms.c dfp.c dfr8.c dfrle.c dfsd.c dfstubs.c         \
           dfufp2i.c dfunjpeg.c dfutil.c dynarray.c glist.c hbitio.c        \
           hblocks.c hbuffer.c hchunks.c hcomp.c hcompri.c hdfalloc.c       \
           herr.c hextelt.c hfile.c hfiledd.c hkit.c linklist.c mcache.c    \
           mfan.c mfgr.c mstdio.c tbbt.c vattr.c vconv.c vg.c vgp.c vhi.c   \
           vio.c vparse.c vrw.c vsfld.c

CHEADERS = atom.h bitvect.h cdeflate.h cnbit.h cnone.h cskphuff.h crle.h    \
           cszip.h df.h dfan.h dfi.h dfgr.h dfrig.h dfsd.h dfstubs.h        \
           dfufp2i.h dynarray.h h4config.h hbitio.h hchunks.h hcomp.h       \
           hcompi.h hconv.h hdf.h hdfi.h herr.h hfile.h hkit.h hlimits.h    \
           hproto.h hntdefs.h htags.h linklist.h mfan.h mfgr.h mstdio.h     \
           tbbt.h vattr.h vg.h
FHEADERS = dffunc.f90 hdf.f90 dffunc.inc hdf.inc

if HDF_BUILD_FORTRAN
libdf_la_SOURCES = $(CSOURCES) $(FSOURCES)
include_HEADERS = $(CHEADERS) hproto_fortran.h $(FHEADERS)
else
libdf_la_SOURCES = $(CSOURCES)
include_HEADERS = $(CHEADERS)
endif

# The following is a workaround. Since Fortran is included in this 
# Makefile.am, automake will always try to use the Fortran linker, even when
# fortran has been disabled in configure. The Fortran linker gets confused 
# when shared libraries are enabled, so when Fortran is not enabled,
# this replaces the Fortran link macro with the C link macro.
if HDF_BUILD_FORTRAN
    # do nothing
else
    F77LINK = $(LINK)
endif

SUFFIXES = .inc .f90
.inc.f90:
	if test -f $<; then                                             \
	  sed -e 's/^[cC]/!/' -e 's/^     [^ ]/     \&/' < $< > $*.f90; \
	else                                                            \
	  sed -e 's/^[cC]/!/' -e 's/^     [^ ]/     \&/' < $(srcdir)/$< > $*.f90; \
	fi

#############################################################################
##                          And the cleanup                                ##
#############################################################################

DISTCLEANFILES = dffunc.f90 hdf.f90
