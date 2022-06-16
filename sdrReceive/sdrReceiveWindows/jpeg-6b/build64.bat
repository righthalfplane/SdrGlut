del *.o *.a
x86_64-w64-mingw32-gcc -m64 -I../include -c ^
 jcapimin.c  jcinit.c    jcphuff.c   jdatadst.c  jdinput.c   jdpostct.c  jfdctint.c  jquant1.c   rdjpgcom.c  wrbmp.c ^
 cdjpeg.c    jcapistd.c  jcmainct.c  jcprepct.c  jdatasrc.c  jdmainct.c  jdsample.c  jidctflt.c  jquant2.c   rdppm.c     wrgif.c ^
 cjpeg.c     jccoefct.c  jcmarker.c  jcsample.c  jdcoefct.c  jdmarker.c  jdtrans.c   jidctfst.c  jmemmgr.c   jutils.c    rdrle.c     wrjpgcom.c ^
 ckconfig.c  jccolor.c   jcmaster.c  jctrans.c   jdcolor.c   jdmaster.c  jerror.c    jidctint.c  jmemname.c  rdbmp.c     rdswitch.c  wrppm.c ^
 djpeg.c     jcdctmgr.c  jcomapi.c   jdapimin.c  jddctmgr.c  jdmerge.c   jfdctflt.c  jidctred.c  jmemnobs.c  rdcolmap.c  rdtarga.c   wrrle.c ^
 jchuff.c    jcparam.c   jdapistd.c  jdhuff.c    jdphuff.c   jfdctfst.c  jmemansi.c  jpegtran.c  rdgif.c     transupp.c  wrtarga.c
 x86_64-w64-mingw32-ar rcv libjpeg64.a *.o
 move /Y libjpeg64.a ../lib/
 del *.o *.a
 
