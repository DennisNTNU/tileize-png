cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(tileize-png VERSION 0.1)

#####################
# ueye capture tool #
#####################

# Executable source files
set(execsrc
    lodepng/lodepng.cpp
    src/main.cpp
    )

# Executable include directories, relative to CMakeLists.txt file
set(execinc
    lodepng
    src
    )

# Executable libraries to link
set(execlib
    m
    )

# Additional executable compile options
set(execopt
    -Wall
    -Wextra
    -Werror
    -O3
    )


add_executable(tileize-png ${execsrc})
target_include_directories(tileize-png PUBLIC ${execinc})
target_link_libraries(tileize-png PUBLIC ${execlib})
target_compile_options(tileize-png PUBLIC ${execopt})