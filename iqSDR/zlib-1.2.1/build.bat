del *.o *.a
i686-w64-mingw32-gcc -I../include -c ^
adler32.c  compress.c  crc32.c  deflate.c gzio.c  infback.c  inffast.c  inflate.c  ^
inftrees.c  minigzip.c  trees.c  uncompr.c  zutil.c
 i686-w64-mingw32-ar rcv libzlib32.a *.o
 move /Y libzlib32.a ../lib/
 del *.o *.a
 


