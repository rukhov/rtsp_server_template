cmake_minimum_required(VERSION 3.28)

project(rtsp_server_sample VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS Off)

include(FindPkgConfig)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GStreamer REQUIRED gstreamer-1.0)
pkg_check_modules(GstRtspServer REQUIRED gstreamer-rtsp-server-1.0)

find_package(unofficial-live555 REQUIRED)
find_package(OpenCV REQUIRED)

set(TARGET rtsp_server_sample)

add_executable(${TARGET})

target_include_directories(${TARGET} 
    PRIVATE ${GStreamer_INCLUDE_DIRS}
    PRIVATE ${GstRtspServer_INCLUDE_DIRS}
)

target_link_libraries(${TARGET} 
    #PRIVATE opencv_core opencv_videoio opencv_highgui
    #PRIVATE unofficial::liveMedia unofficial::groupsock unofficial::BasicUsageEnvironment unofficial::UsageEnvironment
    #PRIVATE OpenSSL::SSL
    #PRIVATE GStreamer::GStreamer GstRtspServer
    PRIVATE /home/rukhov/projects/github/rtsp_server_sample/build/vcpkg_installed/x64-linux/lib/libx264.a
    PRIVATE ${GStreamer_LIBRARIES}
    PRIVATE ${GstRtspServer_LIBRARIES}
)

target_sources(${TARGET} PRIVATE
    #main-x264.cpp
    #testH264VideoStreamer.cpp
    main-gstream.cpp
    gst-color-bars.cpp
    GstColorBarsVideoSrc.cpp
)