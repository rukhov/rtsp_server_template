cmake_minimum_required(VERSION 3.28)

project(rtsp_server_sample VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS Off)

if(UNIX)
#    add_compile_options( $<$<CONFIG:Debug>:-g3 -O0> )
    if( CMAKE_BUILD_TYPE EQUAL "Debug" )
        add_compile_options( -g3 -O0 )
    endif()
elseif(WIN32)
    add_compile_definitions("UNICODE")
    add_compile_definitions("_UNICODE")
    add_compile_definitions("BOOST_USE_WINAPI_VERSION=0x0602")
    add_compile_definitions("_WIN32_WINNT=0x0602")
    add_compile_definitions("BOOST_PROCESS_VERSION=2")
endif()

include(FindPkgConfig)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GStreamer REQUIRED gstreamer-1.0)
pkg_check_modules(GstRtspServer REQUIRED gstreamer-rtsp-server-1.0)

find_package(OpenCV REQUIRED)

set(TARGET rtsp_server_sample)

add_executable(${TARGET})

target_precompile_headers(${TARGET} PRIVATE stdafx.h)

target_include_directories(${TARGET} 
    PRIVATE ${GStreamer_INCLUDE_DIRS}
    PRIVATE ${GstRtspServer_INCLUDE_DIRS}
)

target_link_libraries(${TARGET} 
    PRIVATE ${GStreamer_LIBRARIES}
    PRIVATE ${GstRtspServer_LIBRARIES}
    PRIVATE opencv_videoio opencv_imgcodecs opencv_imgproc
)

target_sources(${TARGET} PRIVATE
    main-gstream.cpp
    picture_gen.cpp
    GstCustomVideoSrc.cpp
)