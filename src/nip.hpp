#pragma once

#include "options.hpp"
#include "token.hpp"

#include <iosfwd>
#include <vector>

namespace nip {
	nip::Options argument_parser(int argc, char* argv[]);
	std::vector<nip::Token_t> tokenizer(std::istream&);
	void token_printer(std::vector<nip::Token_t>&, std::ostream&);
}