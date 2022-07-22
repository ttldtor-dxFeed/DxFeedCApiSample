cmake_minimum_required(VERSION 3.10)
project(DxFeedCApiSample)

set(CMAKE_CXX_STANDARD 11)

add_executable(DxFeedCApiSample main.cpp)

add_definitions(-DFMT_HEADER_ONLY=1)

target_include_directories(DxFeedCApiSample PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/dxFeedCApi-8.6.3/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/fmt-9.0.0/include>)
target_link_libraries(DxFeedCApiSample debug ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/dxFeedCApi-8.6.3/lib/libDXFeedd_64.so
        optimized ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/dxFeedCApi-8.6.3/lib/libDXFeed_64.so)

# We add these linker options so that it looks for libraries in the current directory
set_target_properties(DxFeedCApiSample PROPERTIES LINK_FLAGS "-Wl,-rpath,./")

add_custom_command(TARGET DxFeedCApiSample POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/thirdparty/dxFeedCApi-8.6.3/lib/libDXFeed_64.so
        ${CMAKE_CURRENT_BINARY_DIR}/libDXFeed_64.so)

add_custom_command(TARGET DxFeedCApiSample POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/thirdparty/dxFeedCApi-8.6.3/lib/libDXFeedd_64.so
        ${CMAKE_CURRENT_BINARY_DIR}/libDXFeedd_64.so)
