add_executable(native_lua helper.cpp main.cpp)
TARGET_LINK_LIBRARIES(native_lua lua tolua)