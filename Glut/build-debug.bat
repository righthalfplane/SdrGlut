rm *.o *.a
i686-w64-mingw32-gcc -DWIN32 -D_WINDOWS -D_USRDLL -g -O0 -c *.c -I../include
i686-w64-mingw32-ar rcv libglut32-debug.a *.o
mv libglut32-debug.a ../lib/
rm *.o *.a

