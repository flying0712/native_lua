#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lua.hpp>
#include "helper.hpp"
#include <ctime>
#include <vector>



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
    lua_State* L = Helper::loadLua("scripts/window_display.lua");

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

void test_table() {
    // #define MAX_COLOR 255
    // lua_State* L = Helper::loadLua("scripts/window_display.lua");
    // lua_getglobal(L, "background");

    // lua_pushstring(L, key);
    // lua_gettable(L, -2); /* get background[key] */ 
    // result = (int)(lua_tonumber(L, -1) * MAX_COLOR);

    // lua_pop(L, 1); /* remove number */

    // return result;
}

int main (void)
{
    // test_interact();
    // test_stack_api();
    // debug_log("Logging, %d %d %d", 1, 2, 3);
    // test_window_size();
    return 0;
}
