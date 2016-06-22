#pragma once

#include "utilmacro.hpp"
#include <sstream>

ALWAYS_INLINE std::string special_sanitize(std::string& in) {
	std::ostringstream newstr;
	for (auto i : in) {
		switch (i) {
			case '\a':
				newstr << "\\a";
				break;
			case '\b':
				newstr << "\\b";
				break;
			case '\f':
				newstr << "\\f";
				break;
			case '\n':
				newstr << "\\n";
				break;
			case '\r':
				newstr << "\\r";
				break;
			case '\t':
				newstr << "\\t";
				break;
			case '\v':
				newstr << "\\v";
				break;
			default:
				newstr << i;
				break;
		}
	}

	return newstr.str();
}