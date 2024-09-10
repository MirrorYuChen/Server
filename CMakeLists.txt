cmake_minimum_required(VERSION 3.1...3.29)

project(Server LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

set(INC_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/source)
set(3RD_DIR ${CMAKE_CURRENT_LIST_DIR}/3rdLibs)

option(BUILD_SHARED_LIBS "Build Shared library" ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release")
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include_directories(
  ${INC_DIR}
  ${SRC_DIR}
  ${3RD_DIR}/spdlog
)

file(GLOB_RECURSE SRC_FILES
  ${SRC_DIR}/*.h
  ${SRC_DIR}/*.cc
)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${SRC_FILES})

set(OBJECTS_TO_LINK "")
find_package(Threads REQUIRED)
list(APPEND OBJECTS_TO_LINK Threads::Threads)

if (BUILD_SHARED_LIBS)
  add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
else (BUILD_SHARED_LIBS)
  add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
endif (BUILD_SHARED_LIBS)
target_link_libraries(${PROJECT_NAME} ${OBJECTS_TO_LINK})

set(TEST_LIST
  TestLogger
)

foreach(TEST ${TEST_LIST})
  add_executable(${TEST} test/${TEST}.cc)
  target_link_libraries(${TEST} ${PROJECT_NAME})
endforeach(TEST ${TEST_LIST})
