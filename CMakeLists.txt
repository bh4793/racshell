
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

add_executable(listgroup "racshell/listgroup.cpp")
target_link_libraries(listgroup racshell -static)

add_executable(adduser "racshell/adduser.cpp")
target_link_libraries(adduser racshell -static)

add_executable(addgroup "racshell/addgroup.cpp")
target_link_libraries(addgroup racshell -static)

add_executable(deleteuser "racshell/deleteuser.cpp")
target_link_libraries(deleteuser racshell -static)

add_executable(deletegroup "racshell/deletegroup.cpp")
target_link_libraries(deletegroup racshell -static)

add_executable(altuser "racshell/altuser.cpp")
target_link_libraries(altuser racshell -static)

add_executable(altgroup "racshell/altgroup.cpp")
target_link_libraries(altgroup racshell -static)

add_executable(connect "racshell/connect.cpp")
target_link_libraries(connect racshell -static)

add_executable(remove "racshell/remove.cpp")
target_link_libraries(remove racshell -static)

add_executable(adddataset "racshell/adddataset.cpp")
target_link_libraries(adddataset racshell -static)

add_executable(deletedataset "racshell/deletedataset.cpp")
target_link_libraries(deletedataset racshell -static)

add_executable(listdataset "racshell/listdataset.cpp")
target_link_libraries(listdataset racshell -static)

add_executable(addresource "racshell/addresource.cpp")
target_link_libraries(addresource racshell -static)

add_custom_target(
    racshell_commands
    DEPENDS
    listuser
    listgroup
    adduser
    addgroup
    deleteuser
    deletegroup
    altuser
    altgroup
    connect
    remove
    adddataset
    deletedataset
    listdataset
    addresource
)