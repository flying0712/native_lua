file(GLOB SRC_FILE_LIST "*.cpp")
add_executable(native_lua ${SRC_FILE_LIST})
TARGET_LINK_LIBRARIES(native_lua lua tolua)