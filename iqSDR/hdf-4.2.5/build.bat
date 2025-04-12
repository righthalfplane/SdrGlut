del *.o *.a
i686-w64-mingw32-gcc -I../include -D_WINDOWS -DINTEL86 -DWIN32 -DDOS_FS -D_HDFLIB_ -c ^
atom.c     bitvect.c  cdeflate.c  cnbit.c    cnone.c     crle.c      cskphuff.c  cszip.c     df24.c  ^
dfan.c     dfcomp.c    dfconv.c   dfgr.c      dfgroup.c   dfimcomp.c  dfjpeg.c   dfkconv.c ^
dfkcray.c  dfkfuji.c  dfknat.c    dfkswap.c  dfkvms.c    dfp.c  dfr8.c dfrle.c ^
dfsd.c     dfstubs.c  dfunjpeg.c  dfutil.c    dfutilf.c  dynarray.c ^
glist.c    hbitio.c   hblocks.c   hbuffer.c  hchunks.c   hcomp.c     hcompri.c   hdfalloc.c  herr.c  ^
hextelt.c  hfile.c    hfiledd.c   hkit.c     mcache.c    mfan.c    ^
mfgr.c       mstdio.c    tbbt.c     vattr.c     vconv.c     vg.c        vgp.c ^
vhi.c      vio.c      vparse.c    vrw.c      vsfld.c
i686-w64-mingw32-ar rcv libhdf32.a *.o
move /Y libhdf32.a ../lib/
del *.o *.a
 


