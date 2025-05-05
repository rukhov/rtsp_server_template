cmake_minimum_required(VERSION 3.28)

project(rtsp_server_template VERSION 1.0.0 LANGUAGES CXX C)

option(BUILD_EXAMPLES "Build examples" ON)

set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS Off)

set(GSTREAMER_STATIC_LINKING OFF)

if(UNIX)
#    add_compile_options( $<$<CONFIG:Debug>:-g3 -O0> )
    if( CMAKE_BUILD_TYPE EQUAL "Debug" )
        add_compile_options( -g3 -O0 )
    endif()
elseif(WIN32)
    add_compile_definitions("UNICODE")
    add_compile_definitions("_UNICODE")
    add_compile_definitions("BOOST_USE_WINAPI_VERSION=0x0602")
    add_compile_definitions("_WIN32_WINNT=0x0602")
    add_compile_definitions("BOOST_PROCESS_VERSION=2")
endif()

add_subdirectory(src)
