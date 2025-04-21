set(TARGET_NAME lib_rtsp_streamer)

set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS Off)

include(FindPkgConfig)

pkg_check_modules(GStreamer REQUIRED gstreamer-1.0)
pkg_check_modules(GStreamerVideo REQUIRED gstreamer-video-1.0)
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

target_link_libraries(${TARGET_NAME} 
    PRIVATE ${GStreamer_LINK_LIBRARIES}
    PRIVATE ${GStreamerVideo_LINK_LIBRARIES}
    PRIVATE ${GstRtspServer_LINK_LIBRARIES}
)
