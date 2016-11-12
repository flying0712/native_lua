#include <stdio.h>
#include <string.h>
#include <lua.hpp>

// extern "C" {

// #include <lua.h>
// #include <lauxlib.h>
// #include <lualib.h>

// }

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
    error = luaL_loadbuffer(L, buff, strlen(buff), "line") || lua_pcall(L, 0, 0, 0);
    if (error) {
      fprintf(stderr, "%s", lua_tostring(L, -1));
      lua_pop(L, 1);/* pop error message from the stack */ 
    }
  }
  lua_close(L);
}

int main (void)
{
  test_interact();
  return 0;
}
