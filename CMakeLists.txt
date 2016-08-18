cmake_minimum_required (VERSION 2.6)

project (PoolWatcher)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

#
# cmake -DCMAKE_BUILD_TYPE=Debug ..
# cmake -DCMAKE_BUILD_TYPE=Release ..
#
message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
IF(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Debug build.")
  set(CMAKE_CXX_FLAGS "-g -O0 -Wall")
ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
  message("Release build.")
  set(CMAKE_CXX_FLAGS "-O2 -Wall")
ELSE()
  message("Some other build type.")
ENDIF()


SET(CMAKE_CXX_COMPILER "g++")
SET(CMAKE_C_COMPILER   "gcc")
SET(CMAKE_CXX_COMPILER_ARG1 "-std=c++0x")


find_package(Glog)
if(NOT GLOG_FOUND)
  message(FATAL_ERROR "Glog not found!")
endif(NOT GLOG_FOUND)

find_package(LibEvent)
if(NOT LibEvent_FOUND)
  message(FATAL_ERROR "libevent2 not found!")
endif(NOT LibEvent_FOUND)

execute_process(COMMAND mysql_config --libs_r OUTPUT_VARIABLE MYSQL_LIB OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND mysql_config --include OUTPUT_VARIABLE MYSQL_INCLUDE OUTPUT_STRIP_TRAILING_WHITESPACE)

include_directories(src test ${GLOG_INCLUDE_DIRS} ${LIBEVENT_INCLUDE_DIR} ${MYSQL_INCLUDE})
set(THRID_LIBRARIES ${GLOG_LIBRARIES} ${LIBEVENT_LIB} ${MYSQL_LIB})

file(GLOB LIB_SOURCES src/*.cc)
add_library(btcpoolwatcher STATIC ${LIB_SOURCES})

#file(GLOB_RECURSE TEST_SOURCES test/*.cc)
#add_executable(unittest ${TEST_SOURCES})
#target_link_libraries(unittest btcpoolwatcher ${THRID_LIBRARIES})

file(GLOB_RECURSE WATCHER_SOURCES src/watcher/*.cc)
add_executable(poolwatcher ${WATCHER_SOURCES})
target_link_libraries(poolwatcher btcpoolwatcher ${THRID_LIBRARIES})
