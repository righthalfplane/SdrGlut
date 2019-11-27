del *.o *.a *.exe
x86_64-w64-mingw32-gcc -m64 -D_LIB -DGLEW_STATIC -I../include -c glew.c 
x86_64-w64-mingw32-ar rcv libglew64.a *.o
move /Y libglew64.a ../lib/
x86_64-w64-mingw32-gcc -m64  -DGLEW_STATIC -DGLUT_NO_LIB_PRAGMA -DGLUT_BUILDING_LIB -I../include -o glewinfo64.exe glewinfo.c  -L../lib -lglew64 -lglut64 -lopengl32 -lGdi32
move /Y glewinfo64.exe ../bin/
x86_64-w64-mingw32-gcc -m64  -DGLEW_STATIC -DGLUT_NO_LIB_PRAGMA -DGLUT_BUILDING_LIB -I../include -o visualinfo64.exe visualinfo.c  -L../lib -lglew64 -lglut64 -lglu32 -lopengl32 -lGdi32
move /Y visualinfo64.exe ../bin/
del *.o *.a *.exe

