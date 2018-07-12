cmake_minimum_required(VERSION 3.5)

include(./sub/rule.cmake)

message(STATUS "TEST_VAR: $ENV{TEST_VAR}")

execute_process(COMMAND ls ${CMAKE_CURRENT_SOURCE_DIR}/setup.bash)
execute_process(COMMAND source ${CMAKE_CURRENT_SOURCE_DIR}/setup.bash)

message(STATUS "TEST_VAR: $ENV{TEST_VAR}")

message(STATUS "CMAKE_MODULE_PATH $ENV{CMAKE_MODULE_PATH}")
message(STATUS "CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH}")
message(STATUS "PATH $ENV{PATH}")

find_package(OpenCL REQUIRED)
message(STATUS "OpenCL_INCLUDE_DIRS: ${OpenCL_INCLUDE_DIRS}")

find_package(OpenCV 2.4.8 EXACT REQUIRED)
#find_package(OpenCV)
message(STATUS "\nOpenCV_VERSION: ${OpenCV_VERSION}")
message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBRARY_DIRS: ${OpenCV_LIBRARY_DIRS}")
message(STATUS "OpenCV_LIBRARIES: ${OpenCV_LIBS}")


message(STATUS "BOOST_ROOT: ${BOOST_ROOT}")
set(BOOST_ROOT /usr)
find_package(Boost 1.54.0  EXACT REQUIRED COMPONENTS date_time filesystem system)
message(STATUS "\nBoost_VERSION: ${Boost_VERSION}")
message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")

set(BOOST_ROOT /usr/local)
message(STATUS "BOOST_ROOT: ${BOOST_ROOT}")
message(STATUS "BOOST_ROOT: $ENV{BOOST_ROOT}")
find_package(Boost 1.60.0  EXACT REQUIRED COMPONENTS date_time filesystem system)
message(STATUS "\nBoost_VERSION: ${Boost_VERSION}")
message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")

find_package(PkgConfig)
set(ENV{PKG_CONFIG_PATH} /usr/local/lib/pkgconfig)
#pkg_check_modules(AVFORMAT REQUIRED libavformat>=58.10.100)
pkg_search_module(AVFORMAT58 REQUIRED libavformat=58.10.100)
message(STATUS "\nAVFORMAT_VERSION: ${AVFORMAT58_VERSION}")
message(STATUS "AVFORMAT_INCLUDE_DIRS: ${AVFORMAT58_INCLUDE_DIRS}")
message(STATUS "AVFORMAT_LIBRARY_DIRS: ${AVFORMAT58_LIBRARY_DIRS}")
message(STATUS "AVFORMAT_LIBRARIES: ${AVFORMAT58_LIBRARIES}")

#pkg_check_modules(AVFORMAT REQUIRED libavformat>=58.10.100)
set(ENV{PKG_CONFIG_PATH} /usr/lib/x86_64-linux-gnu/pkgconfig)
message(STATUS "PKG_CONFIG_PATH: $ENV{PKG_CONFIG_PATH}")
pkg_search_module(AVFORMAT54 REQUIRED libavformat=54.20.4)
message(STATUS "\nAVFORMAT_VERSION: ${AVFORMAT54_VERSION}")
message(STATUS "AVFORMAT_INCLUDE_DIRS: ${AVFORMAT54_INCLUDE_DIRS}")
message(STATUS "AVFORMAT_LIBRARY_DIRS: ${AVFORMAT54_LIBRARY_DIRS}")
message(STATUS "AVFORMAT_LIBRARIES: ${AVFORMAT54_LIBRARIES}")

find_package(urdfdom REQUIRED)
message(STATUS "\nurdfdom_INCLUDE_DIRS: ${urdfdom_INCLUDE_DIRS}")

set(TEST_FILE "log.txt")
add_custom_command(OUTPUT  ${TEST_FILE}
  COMMAND echo "Generating log.txt file..."
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_FILE} ${TEST_FILE}
  COMMENT  "This is a test"
 )
add_custom_target(Test1 ALL DEPENDS ${TEST_FILE})

add_custom_command(TARGET Test1
  POST_BUILD#PRE_BUILD 
  COMMAND echo "executing a fake command"
  COMMENT "This command will be executed before building target Test1"
)
