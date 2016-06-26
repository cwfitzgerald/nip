#pragma once

#include "Error/errorhandler.hpp"
#include "options.hpp"
#include "token.hpp"

#include <iosfwd>
#include <vector>

namespace nip {
	class compiler {
	  private:
		std::vector<int64_t> token_int_cache;
		std::vector<double> token_float_cache;
		std::vector<std::string> token_identifier_cache;

		nip::Options opt;
		nip::error::Error_Handler errhdlr;

		std::vector<nip::Token_t> tokenizer();
		void token_printer(std::vector<nip::Token_t>&, std::ostream&);

	  public:
		compiler(nip::Options& o) : opt(o){};
		void argument_parser(int argc, char* argv[]);
		void compile();
	};
}
