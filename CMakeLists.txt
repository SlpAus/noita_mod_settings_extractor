cmake_minimum_required(VERSION 3.15)

project(MyProject C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(fastlz_lib STATIC ext/FastLZ/fastlz.c)

target_include_directories(fastlz_lib PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/FastLZ"
)

set(SOURCE_FILES
    src/main.cpp
    src/formatter.cpp
    src/parser.cpp
)

add_executable(extractor ${SOURCE_FILES})

target_link_options(extractor PRIVATE -static)

target_link_libraries(extractor PRIVATE fastlz_lib)