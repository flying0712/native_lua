#include "helper.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctime>
#include <vector>

void Helper::stackDump(lua_State* L){    
    int i;
    int top = lua_gettop(L);
    for(i=1; i<=top; i++){
        int t = lua_type(L,i);
        switch(t){
            case LUA_TSTRING:{
                printf("'%s'",lua_tostring(L,i));
                break;
            }
            case LUA_TBOOLEAN:{
                printf(lua_toboolean(L,i)?"true":"false");
                break;
            }
            case LUA_TNUMBER:{
                printf("%g",lua_tonumber(L,i));
                break;
            }
            default:{
                printf("%s",lua_typename(L,t));
                break;
            }
        }
        printf(",");
    }
    printf("\n");
}

void Helper::handleError(lua_State *L, const char *fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
    lua_close(L);
    exit(-1);
}

lua_State* Helper::loadLua(const char* filename){
    // const char *filename = "scripts/window_size.lua";
    lua_State *L = lua_open();
    luaL_openlibs(L);

    if (luaL_loadfile(L, filename) || lua_pcall(L, 0, 0, 0)){
        handleError(L, "cannot run configuration file: %s", lua_tostring(L, -1));
    }
    return L;
}