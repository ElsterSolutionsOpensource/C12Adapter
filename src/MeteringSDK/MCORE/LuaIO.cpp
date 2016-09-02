#include "MCOREExtern.h"
#include "MException.h"
#include "LuaIO.h"

#if !M_NO_LUA_COOPERATIVE_IO

int MLuaYieldAndSelect(int fd, int timeout, int write)
{
   lua_State *L = lua_this();
   fd_set fs;
   FD_ZERO(&fs);
   FD_SET(fd, &fs);
   fd_set *readfs = !write ? &fs : 0;
   fd_set *writefs = write ? &fs : 0;
   timeval tv = { 0, 0 };
   int status = select(fd + 1, readfs, writefs, 0, &tv);

   if( status != 0 || timeout == 0 )
      return status;

   lua_newtable(L);
   lua_pushinteger(L, fd); lua_setfield(L, -2, "fd");
   lua_pushinteger(L, timeout); lua_setfield(L, -2, "timeout");

   if( write )
      lua_pushstring(L, "writing");
   else
      lua_pushstring(L, "reading");

   //printf("MLuaYieldAndSelect Yield fd=%p, timeout(ms)=%i\n", fd, timeout);

   int n = lua_yield(L, 2);

   //printf("MLuaYieldAndSelect Resume\n");
   //printf("  n=%i\n", n);

   lua_getfield(L, -n, "status");
   status = luaL_checkinteger(L, -1);
   lua_pop(L, n + 1);

   //printf("  status=%i\n", status);

   return status;
}

#endif
