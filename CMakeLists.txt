########################################################################
# Build Soapy SDR support module for NetSDR
########################################################################
# cmake -G"Visual Studio 15 2017 Win64" ../
cmake_minimum_required(VERSION 3.1)

project(SoapyNetSDR)

set(CMAKE_CXX_STANDARD 11)

if(CMAKE_COMPILER_IS_GNUCXX)

    #disable warnings for unused parameters
    add_definitions(-Wno-unused-parameter)

endif(CMAKE_COMPILER_IS_GNUCXX)

find_package(SoapySDR CONFIG)
if (NOT SoapySDR_FOUND)
    message(FATAL_ERROR "Soapy SDR development files not found...")
endif ()

#find_package(RtAudio CONFIG)
#if (NOT RtAudio_FOUND)
#   message(FATAL_ERROR "RtAudio SDR development files not #found...")
#endif ()

SET (sdrglut_sources
    ${sdrglut_sources}
	MainSource/main.cpp 
	MainSource/QuitDialog.cpp 
	MainSource/DialogSave.cpp 
    glui-routines/algebra3.cpp
    glui-routines/arcball.cpp
    glui-routines/glui.cpp 
	glui-routines/glui_add_controls.cpp 
	glui-routines/glui_bitmap_img_data.cpp 
	glui-routines/glui_bitmaps.cpp 
	glui-routines/glui_button.cpp 
	glui-routines/glui_checkbox.cpp
	glui-routines/glui_column.cpp 
	glui-routines/glui_commandline.cpp 
	glui-routines/glui_control.cpp 
	glui-routines/glui_edittext.cpp 
	glui-routines/glui_filebrowser.cpp 
	glui-routines/glui_list.cpp 
	glui-routines/glui_listbox.cpp 
	glui-routines/glui_mouse_iaction.cpp 
	glui-routines/glui_node.cpp 
	glui-routines/glui_panel.cpp
	glui-routines/glui_radio.cpp 
	glui-routines/glui_rollout.cpp 
	glui-routines/glui_rotation.cpp 
	glui-routines/glui_scrollbar.cpp 
	glui-routines/glui_separator.cpp 
	glui-routines/glui_spinner.cpp 
	glui-routines/glui_statictext.cpp 
	glui-routines/glui_string.cpp 
	glui-routines/glui_textbox.cpp 
	glui-routines/glui_translation.cpp	
	glui-routines/glui_tree.cpp 
	glui-routines/glui_treepanel.cpp	
	glui-routines/glui_window.cpp 
	glui-routines/quaternion.cpp	
	glui-routines/tools/ppm.cpp 
	Classes/CLines.cpp 
	Classes/CLInesPlotLabels.cpp 
	Classes/ClinesAttributes.cpp 
	Classes/ClinesPlotAttributes.cpp 
	Radio/smeter.cpp 
	Classes/CLinesPlotBox.cpp 
	Radio/Radio.cpp 
	Radio/DialogRadio.cpp 
	Radio/RaceFastsdr.cpp 
	MainSource/printInfo.cpp 
	Digital/Poly.cpp 
	Digital/BatchFile.cpp 
	MainSource/eMemory.cpp 
	Radio/send.cpp
	MainSource/DialogFileOpen.cpp 
	SdrFile/SdrFile.cpp 
    MainSource/DialogSave.cpp 
    SdrFile/DialogSdrFile.cpp
    MainSource/DialogFolder.cpp
	MainSource/PlayIQ.cpp 
	MainSource/RecordIQ.cpp 
	MainSource/printInfo.cpp
	Classes/Transmit.cpp
	MainSource/WriteToGLUIWindow.cpp 
	Digital/Poly.cpp
	Digital/BatchFile.cpp 
	MainSource/eMemory.cpp 
	Radio/send.cpp
	CExpress/Utilities2.c
	MainSource/Utilities.c
	MainSource/Scene.c
	DrawFindPointHistory/GridPlot.c
	MainSource/WriteToGLUIWindow.cpp
	Classes/CWindow.cpp
	MainSource/sceneDisplay.c
	MainSource/BackgroundQ.c
	MainSource/mThread.cpp
	MainSource/Getpalette.c
	MainSource/SceneList.c
	MainSource/cMalloc.c
	MainSource/audiolib.cpp
	CExpress/FFT2.c
)

SET (sdrglut_headers
    ${sdrglut_headers}
    glui-routines/include/GL/glui.h
    KdTree-routines/Defs.h
    CExpress/SetFrameData.h
)


#network libraries
if (UNIX)   
    if (APPLE)
        add_definitions(-Wno-deprecated)
	    link_directories(/opt/X11/lib /usr/local/lib)
	    add_executable(sdrglut.x ${sdrglut_sources} ${sdrglut_headers})
	    target_include_directories(sdrglut.x PUBLIC include CExpress KdTree-routines glui-routines glui-routines/include MainSource Classes Digital Radio DrawFindPointHistory SdrFile /usr/local/include)
	    target_link_libraries(sdrglut.x PRIVATE  glut gl glu liquid rtaudio SoapySDR "-framework OpenAL")
	else (APPLE)
	    #add_definitions(-Wno-deprecated)
	    #link_directories(/opt/X11/lib)
	    add_executable(sdrglut.x ${sdrglut_sources} ${sdrglut_headers})
	    target_include_directories(sdrglut.x PUBLIC include CExpress KdTree-routines glui-routines glui-routines/include MainSource Classes Digital Radio DrawFindPointHistory SdrFile)
	    target_link_libraries(sdrglut.x PRIVATE  glut GL GLU liquid rtaudio SoapySDR openal pthread)

	endif (APPLE)
else (UNIX)
	link_directories(/opt/X11/lib "C:/Program Files/PothosSDR/lib" "C:/Program Files/RtAudio/lib" "freeglut-3.2.1/lib" "C:/Program Files (x86)/OpenAL 1.1 SDK/libs/Win64")
	add_executable(sdrglut.x ${sdrglut_sources} ${sdrglut_headers})
	target_include_directories(sdrglut.x PUBLIC include CExpress KdTree-routines glui-routines glui-routines/include MainSource Classes Digital Radio DrawFindPointHistory SdrFile)
	include_directories("C:/Program Files/RtAudio/include" "C:/Program Files/PothosSDR/include" "C:/Program Files (x86)/OpenAL 1.1 SDK/include")
	link_directories("C:/Program Files/RtAudio/lib"  "C:/Program Files/PothosSDR/lib")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
    target_link_libraries(sdrglut.x PRIVATE ws2_32 libliquid rtaudio SoapySDR freeglutd OpenAL32)

endif (UNIX)
