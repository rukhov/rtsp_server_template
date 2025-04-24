set(TARGET_NAME lib_rtsp_streamer)

set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS Off)

include(FindPkgConfig)

#pkg_check_modules(GLIB REQUIRED glib-2.0)

pkg_check_modules(GStreamer 
    REQUIRED 
        gstreamer-1.0 gstreamer-base-1.0 
        gstreamer-plugins-base-1.0 gstreamer-plugins-bad-1.0 gstreamer-codecparsers-1.0 gstreamer-controller-1.0 gstreamer-insertbin-1.0
        gstreamer-transcoder-1.0
        gstreamer-video-1.0 gstreamer-rtsp-1.0
)
#pkg_check_modules(GStreamer REQUIRED gstreamer-1.0 gstreamer-plugins-base-1.0 gstreamer-plugins-good-1.0 gstreamer-plugins-bad-1.0)
#pkg_check_modules(GStreamerFull REQUIRED gstreamer-full-1.0)
#pkg_check_modules(GStreamerBase REQUIRED gstreamer-plugins-base-1.0)
#pkg_check_modules(GStreamerGood REQUIRED gstreamer-plugins-good-1.0)
#pkg_check_modules(GStreamerBad REQUIRED gstreamer-plugins-bad-1.0)
#pkg_check_modules(GstAlphaColor REQUIRED gstalphacolor)
#pkg_check_modules(GstApetag REQUIRED gstapetag)
pkg_check_modules(GstRtspServer REQUIRED gstreamer-rtsp-server-1.0)

add_library(${TARGET_NAME} STATIC)

target_sources(${TARGET_NAME} PRIVATE
    src/GstCustomVideoSrc.cpp
    src/RtspStreamer.cpp
    src/CustomMediaFactory.cpp
)

target_precompile_headers(${TARGET_NAME} PRIVATE src/stdafx.h)

target_include_directories(${TARGET_NAME} 
    PRIVATE ${GStreamer_INCLUDE_DIRS} 
    #PRIVATE ${GstRtspServer_INCLUDE_DIRS}
#    PRIVATE ${GSTVideoParsersBad_INCLUDE_DIRS}
    PRIVATE include/rtsp_streamer
    INTERFACE include
    INTERFACE ${GStreamer_INCLUDE_DIRS} 
)

#message("==================== GStreamer_LIBRARIES: ${GStreamer_LIBRARIES}")

if (${GSTREAMER_STATIC_LINKING})

    list(REMOVE_ITEM GStreamerFull_LINK_LIBRARIES "gstreamer-full-1.0")
    target_compile_options( ${TARGET_NAME} 
        PRIVATE -DGSTREAMER_STATIC_LINKING=1
        INTERFACE
            ${GStreamer_CFLAGS} ${GStreamer_CFLAGS_OTHER}
            ${GstRtspServer_CFLAGS} ${GstRtspServer_CFLAGS_OTHER}
    )
    target_link_libraries(${TARGET_NAME} 
        INTERFACE 
            #${GStreamer_LIBRARIES}
            #${GstRtspServer_LIBRARIES} 
            #${GstRtspServer_LINK_LIBRARIES}
            #${GStreamerFull_LINK_LIBRARIES} 
            ${GStreamerFull_LINK_LIBRARIES} ${GStreamerFull_LINK_LIBRARIES}
            ${GstRtspServer_LINK_LIBRARIES} 
    )
    target_link_options(${TARGET_NAME}
        INTERFACE 
            ${GStreamer_LDFLAGS} ${GStreamer_LDFLAGS_OTHER}
            ${GstRtspServer_LDFLAGS} ${GstRtspServer_LDFLAGS_OTHER}
    ) 
else()

    target_compile_options( ${TARGET_NAME} 
    PRIVATE -DGSTREAMER_STATIC_LINKING=0
    INTERFACE
        ${GStreamer_CFLAGS} ${GStreamer_CFLAGS_OTHER}
        ${GstRtspServer_CFLAGS} ${GstRtspServer_CFLAGS_OTHER}
    )

    if(UNIX)
        target_link_libraries(${TARGET_NAME} 
            INTERFACE 
                ${GStreamer_LIBRARIES}
                ${GstRtspServer_LIBRARIES} 
        )
    elseif(WIN32)
        target_link_libraries(${TARGET_NAME} 
            INTERFACE 
                ${GStreamer_LINK_LIBRARIES}
                ${GstRtspServer_LINK_LIBRARIES} 
        )
    endif()
    target_link_options(${TARGET_NAME}
        INTERFACE 
    ) 
endif()
