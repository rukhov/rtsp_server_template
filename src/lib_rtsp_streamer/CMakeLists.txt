set(TARGET_NAME lib_rtsp_streamer)

set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS Off)

include(FindPkgConfig)

pkg_check_modules(GStreamer REQUIRED gstreamer-1.0)
pkg_check_modules(GStreamerFull REQUIRED gstreamer-full-1.0)
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
    PRIVATE ${GSTVideoParsersBad_INCLUDE_DIRS}
    PRIVATE include/rtsp_streamer
    INTERFACE include
    INTERFACE ${GStreamer_INCLUDE_DIRS} 
)

#message("==================== GStreamerPluginsBase_LINK_LIBRARIES: ${GStreamerPluginsBase_LINK_LIBRARIES}")

if (${GSTREAMER_STATIC_LINKING})
    list(REMOVE_ITEM GStreamerFull_LINK_LIBRARIES "gstreamer-full-1.0")
    target_link_libraries(${TARGET_NAME} 
        INTERFACE 
            ${GStreamer_LINK_LIBRARIES} 
            ${GStreamerFull_LINK_LIBRARIES} 
            ${GstRtspServer_LINK_LIBRARIES} 
    )
    target_compile_options( ${TARGET_NAME} PRIVATE -DGSTREAMER_STATIC_LINKING=1)
else()

    target_link_libraries(${TARGET_NAME} 
        INTERFACE 
            ${GStreamer_LIBRARIES}
            ${GstRtspServer_LIBRARIES} 
    )
    target_compile_options( ${TARGET_NAME} PRIVATE -DGSTREAMER_STATIC_LINKING=0)
endif()
