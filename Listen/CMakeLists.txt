########################################################################
# Build Soapy SDR support module for NetSDR
########################################################################
# cmake -G"Visual Studio 15 2017 Win64" ../
cmake_minimum_required(VERSION 3.1)

project(SoapyNetSDR CXX)

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


if (UNIX)
    if(APPLE)
	   include_directories(/usr/local/include)
 	   link_directories(/usr/local/lib)
        add_executable(Listen.x listen.cpp mThread.cpp)
        target_link_libraries(Listen.x PRIVATE liquid rtaudio SoapySDR)
    else(APPLE)
 	    link_directories(/usr/local/lib)
        add_executable(Listen.x listen.cpp mThread.cpp)
        target_link_libraries(Listen.x PRIVATE liquid rtaudio SoapySDR pthread)
    endif(APPLE)
else (UNIX)
	include_directories("C:/Program Files/RtAudio/include" "C:/Program Files/PothosSDR/include")
	link_directories("C:/Program Files/RtAudio/lib"  "C:/Program Files/PothosSDR/lib" /usr/local/lib)
    add_executable(Listen.x listen.cpp mThread.cpp)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
    target_link_libraries(Listen.x PRIVATE ws2_32 libliquid rtaudio SoapySDR)
endif (UNIX)
