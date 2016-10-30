#include "JsonChecker_Constexpr.hpp"
#include <iostream>
#include <cstring>

int main(int argc, const char * argv[] ) {

	if (argc > 1) {
		const auto ok = operator""_jsonchecker(argv[1], std::strlen(argv[1]));
		std::cout << (ok? "Parse OK\n": "FAIL parsing on:\n") << argv[1] << std::endl;
		return ok?0:1;
	}

	std::cout << "usage: " << argv[0] << " <json_string> " << std::endl;
	return 1;
}
