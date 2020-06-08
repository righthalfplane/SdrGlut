del *.o
i686-w64-mingw32-gcc -g -DGLEW_STATIC -D_WIN32 -DGLUT_NO_LIB_PRAGMA -DGLUT_BUILDING_LIB -D_MSC_VER -c -I./DrawFindPointHistory  -I./mShowOpenGL -I./KdTree-routines -I./glui-routines/include -I./CExpress -I../include -I./Classes ^
    ./mShowOpenGL/doJPG.c ./mShowOpenGL/DrawPalette.c ./mShowOpenGL/Geom.c ^
	./mShowOpenGL/Getpalette.c ./mShowOpenGL/LoadFiles.c ./mShowOpenGL/sceneDisplay.c  ^
	./mShowOpenGL/LoadTemperature.c ./mShowOpenGL/mThreadPC.c ./mShowOpenGL/ReDrawVolumetric.c  ^
	./mShowOpenGL/Scene.c ./mShowOpenGL/ShowGLSL.c ./mShowOpenGL/Utilities.c  ^
	./mShowOpenGL/ViewFactorTests.c ./mShowOpenGL/WriteJpeg.c  ./mShowOpenGL/CreateBackDrop.c  ^
	./KdTree-routines/DoubleRecurse.c ./KdTree-routines/KdTree.c ./KdTree-routines/KdTreeTrace.c   ^
	./KdTree-routines/Shade.c ./KdTree-routines/Sphere.c ./KdTree-routines/Tools.c ./mShowOpenGL/ReadGeom.c ./mShowOpenGL/ReadSTLFiles.c  ^
	./KdTree-routines/Tri.c ./KdTree-routines/Triangle.c ./KdTree-routines/TriN2.c  ^
	./DrawFindPointHistory/DrawFindPointHistory.c DrawFindPointHistory/GridPlot.c ./mShowOpenGL/StlLoadFile.c  ^
	./CExpress/BatchFile.c ./CExpress/Blocks.c ./CExpress/CExpress.c ./CExpress/CExpressDummy.c ./CExpress/FFT.c  ^
	./CExpress/File8.c ./CExpress/FileManager.c ./CExpress/LoadJPG.c ./CExpress/Utilities2.c ./CExpress/initPIO.c  ^
	./CExpress/uFiles.c ./CExpress/uFilesBatch3d.c ./CExpress/uFilesBatch3d01.c ./CExpress/uFilesUtilities.c  ^
	./mShowOpenGL/BackgroundQ.c ./CExpress/BatchExpression.c ./CExpress/uFilesBatchSds2d.c ./CExpress/uFilesBatchStl.c  ^
	./CExpress/KdTemperatures.c ./mShowOpenGL/cMalloc.c ./CExpress/uFilesBatchCanScan.c ./CExpress/stlAux.c  ^
	./KdTree-routines/fbTriangle.c ./KdTree-routines/fbObjects.c ./mShowOpenGL/SceneBatch.c ./mShowOpenGL/SceneList.c
i686-w64-mingw32-g++ -g -DGLEW_STATIC -D_WIN32 -DGLUT_NO_LIB_PRAGMA -DGLUT_BUILDING_LIB -c -I./DrawFindPointHistory  -I./mShowOpenGL -I./KdTree-routines -I./glui-routines/include -I./CExpress -I../include -I./Classes ^
    ./glui-routines/algebra3.cpp ./glui-routines/arcball.cpp ./glui-routines/glui.cpp ^
	./glui-routines/glui_add_controls.cpp ./glui-routines/glui_bitmap_img_data.cpp  ^
	./glui-routines/glui_bitmaps.cpp ./glui-routines/glui_button.cpp  ^
	./glui-routines/glui_checkbox.cpp ./glui-routines/glui_column.cpp  ^
	./glui-routines/glui_commandline.cpp ./glui-routines/glui_control.cpp  ^
	./glui-routines/glui_edittext.cpp ./glui-routines/glui_filebrowser.cpp  ^
	./glui-routines/glui_list.cpp ./glui-routines/glui_listbox.cpp  ^
	./glui-routines/glui_mouse_iaction.cpp ./glui-routines/glui_node.cpp  ^
	./glui-routines/glui_panel.cpp ./glui-routines/glui_radio.cpp  ^
	./glui-routines/glui_rollout.cpp ./glui-routines/glui_rotation.cpp  ^
	./glui-routines/glui_scrollbar.cpp ./glui-routines/glui_separator.cpp  ^
	./glui-routines/glui_spinner.cpp ./glui-routines/glui_statictext.cpp  ^
	./glui-routines/glui_string.cpp ./glui-routines/glui_textbox.cpp  ^
	./glui-routines/glui_translation.cpp	./glui-routines/glui_tree.cpp  ^
	./glui-routines/glui_treepanel.cpp	./glui-routines/glui_window.cpp  ^
	./glui-routines/quaternion.cpp	./glui-routines/tools/ppm.cpp  ^
	./mShowOpenGL/DialogFileOpen.cpp ./mShowOpenGL/DialogFireBall.cpp ./mShowOpenGL/QuitDialog.cpp ./DrawFindPointHistory/PlotAttributes.cpp  ^
	./mShowOpenGL/DialogRangeData.cpp	./mShowOpenGL/DialogView.cpp ./mShowOpenGL/DialogSave.cpp ./mShowOpenGL/DialogViewFactorTests.cpp  ^
	./mShowOpenGL/main.cpp ./DrawFindPointHistory/lineAttributes.cpp ./DrawFindPointHistory/pointTimeHistory.cpp ./mShowOpenGL/StlOpenDialog.cpp  ^
	./mShowOpenGL/DialogPreferences.cpp ./mShowOpenGL/DialogOpacity.cpp ./mShowOpenGL/DialogRecipe.cpp ./mShowOpenGL/OpenWindows.cpp ^
	./Classes/CFluence.cpp ./Classes/CKdTree.cpp ./Classes/CTemperature.cpp  ^
    ./Classes/CGemoFile.cpp ./Classes/CStlFile.cpp ./Classes/CWindow.cpp ^
    ./Classes/CGLSL.cpp ./Classes/CKdTreeTemp.cpp ./Classes/CVolumetric.cpp
