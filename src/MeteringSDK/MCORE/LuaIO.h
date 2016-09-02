#ifndef MCORE_LUAIO_H
#define MCORE_LUAIO_H

#include <MCORE/MCOREDefs.h>

#if !M_NO_LUA_COOPERATIVE_IO

#ifdef __cplusplus
extern "C"
{
#endif

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#ifdef __cplusplus
}
#endif

int MLuaYieldAndSelect(int fd, int timeout, int write);
void MLuaYieldAndSleep(int timeout);

#endif

#endif
