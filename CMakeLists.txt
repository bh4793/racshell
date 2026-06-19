
cmake_minimum_required(VERSION 3.24)

project(racshell C CXX)

set(CXX_STANDARD 17)

add_subdirectory(sear)

add_library(
    racshell

    racshell/lib/formatters/text_formatter.cpp
    racshell/lib/formatters/json_formatter.cpp
)

target_include_directories(
    racshell PUBLIC
    racshell
    racshell/lib
    externals/argparse/
    externals/nlohmann/
)

# Link racshell with the sear library target defined in sear/CMakeLists.txt
target_link_libraries(racshell PUBLIC sear)

add_executable(listuser "racshell/listuser.cpp")
target_link_libraries(listuser racshell -static)

add_executable(listgrp "racshell/listgrp.cpp")
target_link_libraries(listgrp racshell -static)

add_executable(adduser "racshell/adduser.cpp")
target_link_libraries(adduser racshell -static)

add_executable(deleteuser "racshell/deleteuser.cpp")
target_link_libraries(deleteuser racshell -static)

add_executable(alteruser "racshell/alteruser.cpp")
target_link_libraries(alteruser racshell -static)