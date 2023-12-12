#include "tools.h"

namespace tools {

	bool is_convertible_to_int(const char* str) {
		// checking if char[] is convertible to number 
		try {
			std::stoi(str);
			return true;
		}
		catch (const std::invalid_argument&) {
			return false;
		}
		catch (const std::out_of_range&) {
			return false;
		}
	}

	void show_usage(const char* path) {
		// reference information
		std::cout << std::endl << "Usage: " << path << " target_name [-d] [-h maximum_hops] [-w timeout]" << std::endl << std::endl;
		std::cout << "Parameters: " << std::endl;
		std::cout << "    -d             \tDo not resolve addresses to hostnames." << std::endl;
		std::cout << "    -h maximum_hops\tMaximum number of hops to search for target." << std::endl;
		std::cout << "    -w timeout\t\tTimeout in milliseconds to wait for each reply." << std::endl;
	};

} // namespace tools