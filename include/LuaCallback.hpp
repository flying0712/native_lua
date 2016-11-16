#ifndef LUA_CALLBCK_HPP
#define LUA_CALLBCK_HPP

#include <string>
#include <map>
#include <lua.hpp>

class LuaCallback
{
public:
    static LuaCallback* get();
    static int addLuaFunction(lua_State* L,int idx);
    static void getLuaFunction(lua_State* L,int idx);
    void registerLuaHandler(int handlerID, int luaFunctionID);
    void invoke(lua_State* L, int handlerID);
private:
    static bool openLuaFunc;
    static int s_function_ref_id;
    static void openFunctionStoreTable(lua_State* L);
    std::map<int,int> handlers;
};

#endif