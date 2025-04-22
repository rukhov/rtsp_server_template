set(TARGET test_streamer)

set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_STANDARD 23)

include(FindPkgConfig)

add_executable(${TARGET} main-gstream.cpp)

target_precompile_headers(${TARGET} PRIVATE stdafx.h)

#message("==================== GSTREAMER_STATIC_LINKING: ${GSTREAMER_STATIC_LINKING}")

target_link_libraries(${TARGET} 
    PRIVATE 
        lib_rtsp_streamer
)
