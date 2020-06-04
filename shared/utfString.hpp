//
//  utfString.hpp
//  plugin_bass
//
//  Created by Danny on 04/06/2020.
//

#ifndef utfString_hpp
#define utfString_hpp

#include <string>

using namespace std;

class UTFString
{
	public:
		#ifdef _WIN32
			static wstring Convert(const string& utf8);
		#else
			static string Convert(string utf8);
		#endif
};

#endif /* utfString_hpp */
