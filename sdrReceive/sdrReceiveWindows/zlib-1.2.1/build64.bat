del *.o *.a
x86_64-w64-mingw32-gcc -m64 -I../include -c ^
adler32.c  compress.c  crc32.c  deflate.c gzio.c  infback.c  inffast.c  inflate.c  ^
inftrees.c  minigzip.c  trees.c  uncompr.c  zutil.c
 x86_64-w64-mingw32-ar rcv libzlib64.a *.o
 move /Y libzlib64.a ../lib/
 del *.o *.a
 


