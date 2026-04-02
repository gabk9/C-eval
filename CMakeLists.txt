cmake_minimum_required(VERSION 3.10)
project(shell C)

set(SOURCES
    main.c 
    includes/utils.c
    includes/s_math.c
    includes/eval.c
)

include_directories(includes)

add_executable(main ${SOURCES})

if(WIN32)
    set_target_properties(main PROPERTIES SUFFIX ".exe")
    
    if(MINGW)
    endif()
    
    if(MSVC)
        target_compile_definitions(main PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
else()
    target_link_libraries(main m)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic)
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(main PRIVATE /W4)
endif()