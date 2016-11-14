cmake_minimum_required (VERSION 3.0)
project (native_lua)

message("${PROJECT_BINARY_DIR}\n${PROJECT_SOURCE_DIR}")

# include directories
include_directories(
   ${PROJECT_SOURCE_DIR}/lua/
   ${PROJECT_SOURCE_DIR}/tolua/
   ${PROJECT_SOURCE_DIR}/include/
)

# lib directories
LINK_DIRECTORIES(
  ${PROJECT_BINARY_DIR}/lib
)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# CMakeLists.txt should in sub dir
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(lua)
ADD_SUBDIRECTORY(tolua)

file(GLOB LUA_FILE_LIST "scripts/*.lua")
file(COPY ${LUA_FILE_LIST} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/scripts")

