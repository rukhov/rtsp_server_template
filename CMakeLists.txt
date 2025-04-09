cmake_minimum_required(VERSION 3.26)

project(rtsp_server_sample VERSION 1.0.0 LANGUAGES CXX C)

include(./cmake/FindGStreamer.cmake)
include(./cmake/FindGstRtspServer.cmake)
find_package(unofficial-live555 REQUIRED)
find_package(OpenCV REQUIRED)

set(TARGET live555-test)

add_executable(${TARGET})

target_link_libraries(${TARGET} 
    #PRIVATE opencv_core opencv_videoio opencv_highgui
    #PRIVATE unofficial::liveMedia unofficial::groupsock unofficial::BasicUsageEnvironment unofficial::UsageEnvironment
    #PRIVATE OpenSSL::SSL
    PRIVATE GStreamer::GStreamer GstRtspServer
    PRIVATE /home/rukhov/projects/github/rtsp_server_sample/build/vcpkg_installed/x64-linux/lib/libx264.a
)

target_sources(${TARGET} PRIVATE
    #main-x264.cpp
    #testH264VideoStreamer.cpp
    main-gstream.cpp
)