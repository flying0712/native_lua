#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lua.hpp>
#include "helper.hpp"
#include <math.h>
#include <ctime>
#include <vector>
#include "LuaCallback.hpp"
#include <tolua++.h>

void debug_log(const char *fmt, ...)
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

void test_interact(){
    char buff[256];
    int error;
    lua_State *L = luaL_newstate();  /* opens Lua */

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
    lua_State* L = luaL_newstate();
    lua_pushboolean(L,1);
    lua_pushnumber(L,10);
    lua_pushnil(L);
    
    //lua会保存string copy
    char *hello = new char[6];
    strcpy(hello,"hello");
    lua_pushstring(L,hello);
    delete[] hello;
    Helper::stackDump(L);// (底-4)true,10,nil,'hello'(顶-1)

    /*将栈底的true复制到顶部*/
    lua_pushvalue(L,-4);
    Helper::stackDump(L);//(1)true,10,(3)nil,'hello',true(5,-1)

    /*栈顶元素(true)移动到底部第三个元素(nil)，nil会被删掉*/
    lua_replace(L,3);
    Helper::stackDump(L);// true,10,true,'hello'

    lua_settop(L,6);
    Helper::stackDump(L);//true,10,true,'hello'(-3),nil,nil(-1)
    lua_remove(L,-3); //true,10,true,nil,nil

    /* set top to -5 */
    lua_settop(L,-5); //true

    lua_pushstring(L,"elements");
    //query stack elements
    /* s不可以被修改, l是s的长度 */
    const char* s = lua_tostring(L, -1);
    size_t l = lua_strlen(L, -1);
    printf("query top = %s len = %ld\n",s,l);

    lua_close(L);

}

/* read width and height from a lua file */
void test_window_size(){
    lua_State* L = Helper::loadLua("scripts/window_size.lua");

    /*取出lua的值，并压人stack*/
    lua_getglobal(L, "width");
    lua_getglobal(L, "height");

    if (!lua_isnumber(L, -2)){
        Helper::handleError(L, "`width' should be a number\n");
    }

    if (!lua_isnumber(L, -1)){
        Helper::handleError(L, "`height' should be a number\n");
    }
    /* 使用栈顶的方式索引，无论堆栈之前的是否是空的，代码都有效
     * 尽量使用栈顶来索引
     */
    int width = (int)lua_tonumber(L, -2); //仅仅读,不影响堆栈
    int height = (int)lua_tonumber(L, -1);
    lua_close(L);

    debug_log("height = %d width = %d",width,height);
}

/* read and write table */
void test_table() {
    #define MAX_COLOR 255
    lua_State* L = Helper::loadLua("scripts/window_display.lua");
    /* push background to top */
    lua_getglobal(L, "background");

    if (!lua_istable(L, -1))
        Helper::handleError(L, "`background' is not a valid color table");

    lua_pushstring(L, "r");     /* push key 'r'*/
    lua_gettable(L, -2);        /* get background.r and push to top */ 
    int r = (int)(lua_tonumber(L, -1) * MAX_COLOR); /* read r */
    lua_pop(L, 1); /* remove r from top */
    
    debug_log("background.r = %d",r);

    /*set r to 0.1 */
    lua_pushstring(L, "r");     /* push key 'r' */
    lua_pushnumber(L,0.1);      /* push value '0.1' */
    lua_settable(L, -3);        /* set background.r = 0.1 */

    /* read again */
    lua_pushstring(L, "r");
    lua_gettable(L, -2);
    r = (int)(lua_tonumber(L, -1) * MAX_COLOR);
    lua_pop(L, 1);
    debug_log("background.r = %d",r);
    
}

void test_create_table(){
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    /* op: my_table = {} */
    lua_pushstring(L, "my_table");
    lua_newtable(L);
    /* register 'my_table = {}' to lua pre-defined table */
    lua_rawset(L,LUA_REGISTRYINDEX);

    /* op: my_table.name = 'lua' */
    lua_pushstring(L,"my_table");
    lua_rawget(L,LUA_REGISTRYINDEX);
    lua_pushstring(L,"name"); 
    lua_pushstring(L,"lua"); /* table(-3), name, lua */
    lua_rawset(L,-3); /* table */

    /* op: my_table.name */
    lua_pushstring(L,"name"); /* table */
    lua_rawget(L,-2); /* table 'name' */
    debug_log("read value = %s", lua_tostring(L,-1));

}

/* call a function `f' defined in Lua */
void test_call_function() {
    double x = 2;
    double y = 10;

    lua_State* L = Helper::loadLua("scripts/call_me.lua");
    /* push functions and arguments */ 
    lua_getglobal(L,"f"); /*function to be called*/
    lua_pushnumber(L, x); /* push 1st argument */ 
    lua_pushnumber(L, y); /* push 2nd argument */
    debug_log("before call: size = %d",lua_gettop(L));

    /* do the call (2 arguments, 1 result) */
    if (lua_pcall(L, 2, 1, 0) != 0)
        Helper::handleError(L, "error running function `f': %s", lua_tostring(L, -1));
    
    debug_log("after call size = %d",lua_gettop(L));

    /* retrieve result */
    if (!lua_isnumber(L, -1))
        Helper::handleError(L, "function `f' must return a number");
    double z = lua_tonumber(L, -1);
    lua_pop(L, 1); /* pop returned value */ 

    debug_log("z = %f",z);
}

void test_common_call(){
    lua_State* L = Helper::loadLua("scripts/call_me.lua");
    int rz;
    Helper::call_lua_func(L, "f", "ii>i", 11, 31, &rz);
    debug_log("rz = %d",rz);

    lua_close(L);
}


/*调用C函数*/

/*
typedef int (*lua_CFunction) (lua_State *L);
在Lua中注册的函数都必须是lua_CFunction类型
函数在将返回值入栈之前不需要清理栈，函数返回之后，lua自动清除栈中返回结果以下的所有内容
*/
static int l_sin (lua_State *L) {
    debug_log("l_sin>>top = %d",lua_gettop(L));
    // double d = lua_tonumber(L, 1); /* get argument */
    double d = luaL_checknumber(L, 1); /* get argument with check */
    lua_pushnumber(L, sin(d)); /* push result */ 
    debug_log("l_sin>>top = %d",lua_gettop(L));
    return 1; /* number of results */
}

void test_lua_call_sin(){
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    /* push l_sin to top, stack == 1 */
    lua_pushcfunction(L, l_sin);
    debug_log("top = %d",lua_gettop(L));

    /* set top value to 'mysin', stack == 0 */
    lua_setglobal(L, "mysin");
    debug_log("top = %d",lua_gettop(L));

    lua_pushstring(L,"sentinel");
    debug_log("test-> top = %d",lua_gettop(L));

    /*inovke mysin in a lua script*/
    const char* filename = "scripts/call_c_fun.lua";
    if (luaL_loadfile(L, filename) || lua_pcall(L, 0, 0, 0)){
        Helper::handleError(L, "cannot run file: %s", lua_tostring(L, -1));
    }

    /* lua auto clear func call stack */
    debug_log("top value = %s",lua_tostring(L, -1));
    lua_close(L);
}

void tolua_mooli_registerCallback(lua_State* tolua_S){

}

int lua_register_callback(lua_State* L)
{
    int argc = 0;
    int hid = (int)tolua_tonumber(L, 1, 0);
    int handler = LuaCallback::addLuaFunction(L, 2);
    printf("%d\n", handler);
    LuaCallback::get()->registerLuaHandler(hid, handler);

    return 0;//无返回值
}

void test_register_lua_function(){
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    lua_getglobal(L, "_G");
    tolua_open(L);
    tolua_module(L,"mo",0);
    tolua_beginmodule(L,"mo");
        tolua_function(L,"registerCallback",lua_register_callback);
    tolua_endmodule(L);

    /*run script*/
    const char* filename = "scripts/register_callback.lua";
    if (luaL_loadfile(L, filename) || lua_pcall(L, 0, 0, 0)){
        Helper::handleError(L, "cannot run file: %s", lua_tostring(L, -1));
    }
    LuaCallback::get()->invoke(L,1001);
    
    lua_close(L);

}

int main (void)
{
    test_register_lua_function();
    // test_interact();
    // test_stack_api();
    // debug_log("Logging, %d %d %d", 1, 2, 3);
    // test_window_size();
    // test_table();
    // test_call_function();
    // test_common_call();
    test_create_table();
    // test_lua_call_sin();
    return 0;
}
