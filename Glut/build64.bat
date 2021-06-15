del *.o *.a
x86_64-w64-mingw32-gcc -m64  -DWIN32 -D_WINDOWS -D_USRDLL -c *.c -I../include
x86_64-w64-mingw32-ar rcv libglut64.a *.o
move /Y libglut64.a ../lib/
del *.o *.a

