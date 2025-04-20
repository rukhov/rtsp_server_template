set(TARGET_NAME lib_rtsp_streamer)

set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS Off)

include(FindPkgConfig)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GStreamer REQUIRED gstreamer-1.0)
pkg_check_modules(GstRtspServer REQUIRED gstreamer-rtsp-server-1.0)

add_library(${TARGET_NAME} STATIC)

target_sources(${TARGET_NAME} PRIVATE
    src/GstCustomVideoSrc.cpp
    src/RtspStreamer.cpp
)

target_precompile_headers(${TARGET_NAME} PRIVATE src/stdafx.h)

target_include_directories(${TARGET_NAME} 
    PRIVATE ${GStreamer_INCLUDE_DIRS} 
    PRIVATE ${GstRtspServer_INCLUDE_DIRS}
    PRIVATE include/rtsp_streamer
    INTERFACE include
    INTERFACE ${GStreamer_INCLUDE_DIRS} 
)

target_link_directories(${TARGET_NAME} 
    PRIVATE ${GStreamer_LIBRARY_DIRS} 
    PRIVATE ${GstRtspServer_LIBRARY_DIRS}
)

message("============= GStreamer_LIBRARY_DIRS: ${GStreamer_LIBRARY_DIRS}")
message("============= GstRtspServer_LIBRARY_DIRS: ${GstRtspServer_LIBRARY_DIRS}")
message("============= GStreamer_LIBRARIES: ${GStreamer_LIBRARIES}")
message("============= GstRtspServer_LIBRARIES: ${GstRtspServer_LIBRARIES}")

target_link_libraries(${TARGET_NAME} 
    PRIVATE ${GStreamer_LIBRARIES}
    PRIVATE ${GstRtspServer_LIBRARIES}
    PRIVATE opencv_videoio opencv_imgcodecs opencv_imgproc
)
