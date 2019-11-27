del *.o *.a *.exe
i686-w64-mingw32-gcc -D_LIB -DGLEW_STATIC -I../include -c glew.c 
i686-w64-mingw32-ar rcv libglew32.a *.o
move /Y libglew32.a ../lib/
i686-w64-mingw32-gcc -DGLEW_STATIC -DGLUT_NO_LIB_PRAGMA -DGLUT_BUILDING_LIB -I../include -o glewinfo.exe glewinfo.c  -L../lib -lglew32 -lglut32 -lopengl32 -lGdi32
move /Y glewinfo.exe ../bin/
i686-w64-mingw32-gcc -DGLEW_STATIC -DGLUT_NO_LIB_PRAGMA -DGLUT_BUILDING_LIB -I../include -o visualinfo.exe visualinfo.c  -L../lib -lglew32 -lglut32 -lglu32 -lopengl32 -lGdi32
move /Y visualinfo.exe ../bin/
del *.o *.a *.exe

