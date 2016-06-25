#pragma once

#include <iostream>

namespace nip {
	struct Options {
		std::istream* program_stream = nullptr;
		std::ostream* output_stream  = &std::cout;
		std::ostream* error_stream   = &std::cerr;
	};
}