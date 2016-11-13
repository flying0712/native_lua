#include <stdio.h>
#include <string.h>
#include <lua.hpp>
#include "helper.hpp"


void error (lua_State *L, const char *fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    vfprintf(stderr, argp);
    va_end(argp);
    lua_close(L);
    exit(EXIT_FAILURE);
}
void test_interact(){
    char buff[256];
    int error;
    lua_State *L = lua_open();  /* opens Lua */

    /* 
    luaopen_base(L);
    luaopen_table(L);
    luaopen_io(L);
    luaopen_string(L);
    luaopen_math(L);
    PANIC: unprotected error in call to Lua API (no calling environment)
    http://www.cnblogs.com/cappuccino/p/3274596.html
    */
    /* lua新版本使用下面的方法，如果使用上面的方法，会报上面的错误 */

    luaL_openlibs(L);
    while (fgets(buff, sizeof(buff), stdin) != NULL) {
      /* 如果没有错误，loadbuffer操作将buff编译为chunk，然后压人栈 */
      /* pcall会将chunk从栈中弹出并在保护模式下运行 */
      /* 如果有错误，都会有一条错误信息压人栈顶 */
      error = luaL_loadbuffer(L, buff, strlen(buff), "line") || lua_pcall(L, 0, 0, 0);
      if (error) {
        fprintf(stderr, "%s", lua_tostring(L, -1));//获得栈顶数据
        //弹出栈顶数据( lua_pop(L,n) --> Pops n elements from the stack )
        lua_pop(L, 1);/* pop error message from the stack */ 
      }
    }
    lua_close(L);
}

/**
* lua的栈顶index为：-1(-2,-3...栈底)
* lua的栈尾index为：1(2,3...栈顶)
* Lua代码的stack操作遵循严格LIFO(后进先出)，当你调用Lua时，它只会改变栈顶的部分。
*/
void test_stack_api(){
    lua_State*L = luaL_newstate();
    lua_pushboolean(L,1);
    lua_pushnumber(L,10);
    lua_pushnil(L);
    lua_pushstring(L,"hello");
    Helper::stackDump(L);// (底-4)true,10,nil,'hello'(顶-1)
    lua_pushvalue(L,-4);
    Helper::stackDump(L);//(1,-5)true,10,nil,'hello',true(5,-1)
    lua_replace(L,3);
    Helper::stackDump(L);// true,10,true,'hello'
    lua_settop(L,6);
    Helper::stackDump(L);//true,10,true,'hello'(-3),nil,nil(-1)
    lua_remove(L,-3); //true,10,true,nil,nil
    lua_settop(L,-5); //true
    lua_close(L);
}
int main (void)
{
    //test_interact();
    test_stack_api();
    return 0;
}
