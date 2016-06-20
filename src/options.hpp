#pragma once

#include <iosfwd>

namespace nip {
	struct Options {
		std::istream* program_stream;
		std::ostream* output_stream;
		std::ostream* error_stream;
	};
}

extern nip::Options opt;