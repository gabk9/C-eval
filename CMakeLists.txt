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
        target_compile_definitions(ceval PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
else()
    target_link_libraries(ceval m)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ceval PRIVATE -Wall -Wextra -Wpedantic)
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(ceval PRIVATE /W4)
endif()
