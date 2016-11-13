#ifndef HELPER_HPP
#define HELPER_HPP

#include "lua.hpp"

class Helper {
public:
    /*打印luaState的栈信息*/
    static void stackDump(lua_State* L);
};

#endif