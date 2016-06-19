#pragma once

#include <iosfwd>

struct NipOptions {
	std::istream* program_stream;
	std::ostream* output_stream;
	std::ostream* error_stream;
};

extern NipOptions opt;