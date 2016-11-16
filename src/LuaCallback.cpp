#include "LuaCallback.hpp"


#define TOLUA_REFID_FUNCTION_MAPPING "mooli_func"

int LuaCallback::s_function_ref_id = 100001;
bool LuaCallback::openLuaFunc = false;

LuaCallback* LuaCallback::get(){
    static LuaCallback instance;
    return &instance;
}

void LuaCallback::openFunctionStoreTable(lua_State* L){
    openLuaFunc = true;
    lua_pushstring(L, TOLUA_REFID_FUNCTION_MAPPING);
    lua_newtable(L);
    lua_rawset(L, LUA_REGISTRYINDEX);
}

int LuaCallback::addLuaFunction(lua_State* L,int lo){
    if(!openLuaFunc){
        openFunctionStoreTable(L);
    }
    if (!lua_isfunction(L, lo)) return 0;

    s_function_ref_id++;

    lua_pushstring(L, TOLUA_REFID_FUNCTION_MAPPING);
    lua_rawget(L, LUA_REGISTRYINDEX);                           /* stack: fun ... refid_fun */
    lua_pushinteger(L, s_function_ref_id);                      /* stack: fun ... refid_fun refid */
    lua_pushvalue(L, lo);                                       /* stack: fun ... refid_fun refid fun */

    lua_rawset(L, -3);                  /* refid_fun[refid] = fun, stack: fun ... refid_ptr */
    lua_pop(L, 1);                                              /* stack: fun ... */

    return s_function_ref_id;
}

void LuaCallback::getLuaFunction(lua_State* L, int refid)
{
    lua_pushstring(L, TOLUA_REFID_FUNCTION_MAPPING);
    lua_rawget(L, LUA_REGISTRYINDEX);                           /* stack: ... refid_fun */
    lua_pushinteger(L, refid);                                  /* stack: ... refid_fun refid */
    lua_rawget(L, -2);                                          /* stack: ... refid_fun fun */
    lua_remove(L, -2);                                          /* stack: ... fun */
}


void LuaCallback::registerLuaHandler(int handlerID, int luaFunctionID){
    handlers[handlerID]=luaFunctionID;
}

void LuaCallback::invoke(lua_State* L, int handlerID){
    int luaId = handlers[handlerID];
    getLuaFunction(L,luaId);//push lua function to stack
    lua_pcall(L, 0, 0, 0);
}

