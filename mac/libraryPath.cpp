//  LibraryPath.cpp

#include "CoronaAssert.h"
#include "CoronaEvent.h"
#include "CoronaLua.h"
#include "CoronaLibrary.h"
#include "libraryPath.hpp"

using namespace std;

static bool IsSimulator(lua_State *L)
{
    lua_getglobal(L, "system");
    lua_getfield(L, -1, "getInfo");
    lua_pushstring(L, "environment");
    lua_call(L, 1, 1);

    const char* environment = lua_tostring(L, -1);
    return (strcmp(environment, "simulator") == 0);
}

static const char* GetHomePath(lua_State *L)
{
    lua_getglobal(L, "os");
    lua_getfield(L, -1, "getenv");
    lua_pushstring(L, "HOME");
    lua_call(L, 1, 1);
    
    return lua_tostring(L, -1);
}

static const char* GetResourcePath(lua_State *L)
{
    lua_getglobal(L, "system");
    lua_getfield(L, -1, "pathForFile");
    lua_pushnil(L);
    lua_call(L, 1, 1);

    return lua_tostring(L, -1);
}

string LibraryPath::Get(lua_State *L, const char* fileName)
{
    string pluginPath;
    
    if (IsSimulator(L))
    {
        const char *homePath = GetHomePath(L);
        pluginPath.append(homePath);
        pluginPath.append("/Library/Application Support/Corona/Simulator/Plugins/");
    }
    else
    {
        const char *resourcePath = GetResourcePath(L);
        string pluginFolder;

        pluginFolder.append(resourcePath);
        pluginFolder = pluginFolder.substr(1, pluginFolder.find("/Resources"));
        pluginPath.append("/");
        pluginPath.append(pluginFolder);
        pluginPath.append("Plugins/");
    }
    
    pluginPath.append(fileName);

    return pluginPath;
}
