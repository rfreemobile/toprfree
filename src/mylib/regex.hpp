#pragma once
#ifndef INCLUDE_src_mylib_regex_hpp
#define INCLUDE_src_mylib_regex_hpp

#include <regex>
#include <string>

std::regex make_regex_C(const std::string & expr); ///< creates std::regex, but in C-locale

#endif
