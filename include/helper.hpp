#ifndef HELPER_HPP
#define HELPER_HPP

#include "lua.hpp"

class Helper {
public:
    /*打印luaState的栈信息*/
    static void stackDump(lua_State* L);
    static void handleError(lua_State *L, const char *fmt, ...);
    static lua_State* loadLua(const char* filename);
    static void call_lua_func(lua_State* L,const char *func, const char *sig, ...);
};

#endif