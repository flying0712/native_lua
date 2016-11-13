#include "helper.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctime>
#include <vector>

static void debug_log(const char *fmt, ...)
{
    std::time_t t = std::time(nullptr);
    char time_buf[100];
    std::strftime(time_buf, sizeof(time_buf), "%D %T", std::gmtime(&t));
    
    va_list args1;
    va_start(args1, fmt);
    va_list args2;
    va_copy(args2, args1);

    //trick to get the size of args1 with fmt
    int len = 1+vsnprintf(NULL, 0, fmt, args1);
    std::vector<char> buf(len);
    va_end(args1);

    //fmt args2 to buf
    vsnprintf(buf.data(), buf.size(), fmt, args2);
    va_end(args2);

    printf("%s [debug]: %s\n", time_buf, buf.data());
}

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

/* 
 * 通用的函数调用
 *  call_va("f", "dd>d", x, y, &z);
 * 'd'->double 'i'->integer 's'->strings 
 * '>' 参数和结果的分隔符
 */
void Helper::call_lua_func(lua_State* L,const char *func, const char *sig, ...) {
    va_list vl;

    int narg,nres; /*number of arguments and results*/
    va_start(vl, sig);
    lua_getglobal(L, func); /* get function */

    /* push arguments */
    narg = 0;
    bool break_while = false;
    while (*sig) { /* push arguments */
        switch (*sig++) {
            case 'd': /* double argument */
                lua_pushnumber(L, va_arg(vl, double));
                break;
            case 'i': /* int argument */
                lua_pushnumber(L, va_arg(vl, int));
                break;
            case 's': /* string argument */
                lua_pushstring(L, va_arg(vl, char *));
                break;
            case '>':
                break_while = true;
                break;
            default:
                handleError(L, "invalid option (%c)", *(sig - 1));
        }
        if(break_while){
            break;
        }
        narg++;
        luaL_checkstack(L, 1, "too many arguments");
    }

    /* do the call */
    nres = strlen(sig); /* number of expected results */
    debug_log("narg = %d nres = %d", narg, nres);
    if (lua_pcall(L, narg, nres, 0) != 0) /* do the call */
        Helper::handleError(L, "error running function `%s': %s", func, lua_tostring(L, -1));
    
    /* retrieve results */
    nres = -nres; /* stack index of first result */
    debug_log("nres = %d",nres);
    while (*sig) { /* get results */
        switch (*sig++) {
            case 'd': /* double result */ 
            {
                if (!lua_isnumber(L, nres)){
                    Helper::handleError(L, "wrong result type");
                }
                // *(va_arg(vl, double *)) = lua_tonumber(L, nres);
                break;
            }
            case 'i': /* int result */
            {
                if (!lua_isnumber(L, nres))
                    Helper::handleError(L, "wrong result type");
                int test_num = (int)lua_tonumber(L, nres);
                *va_arg(vl, int *) = (int)lua_tonumber(L, nres);
                break;
            }
            case 's': /* string result */
                if (!lua_isstring(L, nres))
                    Helper::handleError(L, "wrong result type");
                // *va_arg(vl, const char **) = lua_tostring(L, nres);
                break;
                
            default:
                Helper::handleError(L, "invalid option (%c)", *(sig - 1));
        }
        nres++;
    }
    va_end(vl);
}
