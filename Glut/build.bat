del *.o *.a
i686-w64-mingw32-gcc -g -DWIN32 -D_WINDOWS -D_USRDLL -c *.c -I../include
i686-w64-mingw32-ar rcv libglut32.a *.o
move /Y libglut32.a ../lib/
del *.o *.a

