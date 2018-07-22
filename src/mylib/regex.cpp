
#include "mylib/regex.hpp"

std::regex make_regex_C(const std::string & expr) {
	std::regex ret;
	ret.imbue( std::locale::classic() ); // C locale
	ret = expr;
	return ret;
}

