
cmake_minimum_required(VERSION 3.24)

project(racshell C CXX)

set(CXX_STANDARD 17)

add_subdirectory(sear)

add_library(
    racshell

    racshell/listuser.cpp
)

target_include_directories(
    racshell PUBLIC
    racshell
    externals/argparse/
    externals/nlohmann/
)

# Link racshell with the sear library target defined in sear/CMakeLists.txt
target_link_libraries(racshell PUBLIC sear)

add_executable(listuser "racshell/listuser.cpp")
target_link_libraries(listuser racshell -static)