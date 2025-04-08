cmake_minimum_required(VERSION 3.26)

project(live555-test VERSION 1.0.0 LANGUAGES CXX C)

#include(./FindGStreamer.cmake)
find_package(unofficial-live555 REQUIRED)
find_package(OpenCV REQUIRED)

set(TARGET live555-test)

add_executable(${TARGET})

target_link_libraries(${TARGET} 
    #PRIVATE opencv_core opencv_videoio opencv_highgui
    PRIVATE unofficial::liveMedia unofficial::groupsock unofficial::BasicUsageEnvironment unofficial::UsageEnvironment
    PRIVATE OpenSSL::SSL
    PRIVATE /home/rukhov/projects/arena/live555test/build/vcpkg_installed/x64-linux/lib/libx264.a
 )

target_sources(${TARGET} PRIVATE
    main-x264.cpp
)