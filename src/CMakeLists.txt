
add_subdirectory(lib_rtsp_streamer)
if(BUILD_EXAMPLES)
    add_subdirectory(test_streamer)
    add_subdirectory(simple_example)
endif()
