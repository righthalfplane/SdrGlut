i686-w64-mingw32-g++ -g -o FireBall.exe ^
    doJPG.o DrawPalette.o Geom.o ^
	Getpalette.o LoadFiles.o ^
	LoadTemperature.o mThreadPC.o ReDrawVolumetric.o ^
	Scene.o ShowGLSL.o Utilities.o ^
	ViewFactorTests.o WriteJpeg.o CreateBackDrop.o  ^
	DoubleRecurse.o KdTree.o KdTreeTrace.o   ^
	Shade.o Sphere.o Tools.o ReadGeom.o ReadSTLFiles.o ^
	Tri.o Triangle.o TriN2.o  ^
	DrawFindPointHistory.o GridPlot.o StlLoadFile.o  ^
	BatchFile.o Blocks.o CExpress.o CExpressDummy.o FFT.o  ^
	File8.o FileManager.o LoadJPG.o Utilities2.o initPIO.o  ^
	uFiles.o uFilesBatch3d.o uFilesBatch3d01.o uFilesUtilities.o   ^
	BackgroundQ.o BatchExpression.o uFilesBatchSds2d.o uFilesBatchStl.o  ^
	KdTemperatures.o cMalloc.o uFilesBatchCanScan.o stlAux.o  ^
	fbTriangle.o fbObjects.o SceneBatch.o SceneList.o  ^
    algebra3.o arcball.o glui.o   ^
	glui_add_controls.o glui_bitmap_img_data.o   ^
	glui_bitmaps.o glui_button.o   ^
	glui_checkbox.o glui_column.o   ^
	glui_commandline.o glui_control.o   ^
	glui_edittext.o glui_filebrowser.o   ^
	glui_list.o glui_listbox.o   ^
	glui_mouse_iaction.o glui_node.o   ^
	glui_panel.o glui_radio.o   ^
	glui_rollout.o glui_rotation.o   ^
	glui_scrollbar.o glui_separator.o   ^
	glui_spinner.o glui_statictext.o   ^
	glui_string.o glui_textbox.o   ^
	glui_translation.o	glui_tree.o   ^
	glui_treepanel.o	glui_window.o   ^
	quaternion.o	ppm.o   ^
	DialogFileOpen.o DialogFireBall.o QuitDialog.o   ^
	DialogRangeData.o	DialogView.o DialogSave.o PlotAttributes.o   ^
	 lineAttributes.o pointTimeHistory.o   ^
	main.o DialogViewFactorTests.o StlOpenDialog.o   sceneDisplay.o ^
	DialogPreferences.o DialogOpacity.o DialogRecipe.o OpenWindows.o ^
	CFluence.o CKdTree.o CTemperature.o  ^
    CGemoFile.o CStlFile.o CWindow.o ^
    CGLSL.o CKdTreeTemp.o CVolumetric.o ^
	 -L../lib -lrecipe32 -lhdf32 -ljpeg32 -lzlib32 -lglut32 -lglew32 -lopengl32 -lwinmm -lglu32 -lGdi32 -lgfortran -static
	
