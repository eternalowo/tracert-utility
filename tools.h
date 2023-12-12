#ifndef TOOLS_H_
#define TOOLS_H_

#include <string>
#include <stdexcept>
#include <iostream>

namespace tools {

	bool is_convertible_to_int(const char* str);

	void show_usage(const char* path);

} // namespace tools

#endif