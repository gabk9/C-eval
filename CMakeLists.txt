cmake_minimum_required(VERSION 3.10)
project(shell C)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "Cannot compile in 32 bit or less architectures")
endif()

set(SOURCES
    main.c 
    includes/utils.c
    includes/s_math.c
    includes/eval.c
)

include_directories(includes)

add_executable(ceval ${SOURCES})

if(WIN32)
    if(MSVC)
        # Disable MSVC secure warnings and provide compatibility for GCC-style attributes and ssize_t
        target_compile_definitions(ceval PRIVATE _CRT_SECURE_NO_WARNINGS)
        # Force-include a small header that defines ssize_t and neutralizes __attribute__ on MSVC
        target_compile_options(ceval PRIVATE /FI"${CMAKE_SOURCE_DIR}\\includes\\msvc_compat.h")
    endif()
else()
    target_link_libraries(ceval m)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ceval PRIVATE -Wall -Wextra -Wpedantic)
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(ceval PRIVATE /W4)
endif()
