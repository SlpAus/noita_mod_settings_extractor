cmake_minimum_required(VERSION 3.15)

project(NoitaModSettings C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_library(fastlz_lib STATIC ext/FastLZ/fastlz.c)
target_include_directories(fastlz_lib PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/FastLZ"
)

set(CORE_SOURCES
    src/processor.cpp 
    src/formatter.cpp
    src/parser.cpp
)

add_library(core_lib STATIC ${CORE_SOURCES})
target_link_libraries(core_lib PUBLIC fastlz_lib)
target_include_directories(core_lib PUBLIC src)

if(NOT EMSCRIPTEN)
    add_executable(extractor src/main.cpp)
    target_link_libraries(extractor PRIVATE core_lib)
    set_target_properties(extractor PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    if(MSVC)
        target_compile_options(extractor PRIVATE "/O2")
    else()
        target_compile_options(extractor PRIVATE "-O3")
        target_link_options(extractor PRIVATE
            "-static"
            "-s"
        )
    endif()
endif()

if(EMSCRIPTEN)
    add_executable(extractor_wasm src/wasm_binding.cpp)
    target_link_libraries(extractor_wasm PRIVATE core_lib)
    target_compile_options(extractor_wasm PRIVATE
        "-O3"
    )
    target_link_options(extractor_wasm PRIVATE
        "-O3"
        "-sWASM=1"
        "-sMODULARIZE=1"
        "-sEXPORT_NAME='NoitaModSettings'"
        "-lembind"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sSINGLE_FILE=0"
        "--no-entry"
        "-sFILESYSTEM=0"
        "-flto"
    )
    set_target_properties(extractor_wasm PROPERTIES SUFFIX ".js")
endif()