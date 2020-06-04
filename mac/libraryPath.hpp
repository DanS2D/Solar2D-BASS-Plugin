//  LibraryPath.hpp

#ifndef library_hpp
#define library_hpp

#include <string>

using namespace std;

class LibraryPath
{
public:
    static string Get(lua_State *L, const char* fileName);
};

#endif /* library_hpp */
