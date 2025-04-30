set(TARGET test_streamer)

set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_STANDARD 23)

find_package(lunasvg CONFIG REQUIRED)

add_executable(${TARGET} 
    main-gstream.cpp
    frame_generator.cpp
)

target_precompile_headers(${TARGET} PRIVATE stdafx.h)

#message("==================== GSTREAMER_STATIC_LINKING: ${GSTREAMER_STATIC_LINKING}")

target_link_libraries(${TARGET} 
    PRIVATE 
        lib_rtsp_streamer
        lunasvg::lunasvg
)
