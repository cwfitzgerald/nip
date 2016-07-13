#pragma once

#include "Error/errorhandler.hpp"
#include "Parser/parser.hpp"
#include "options.hpp"
#include "token.hpp"

#include <iosfwd>
#include <vector>

namespace nip {
	class compiler {
	  private:
		Token_Cache_t token_caches;

		nip::Options opt;
		nip::error::Error_Handler errhdlr;

		nip::parse::Parser parser;

		std::vector<nip::Token_t> tokenizer();
		void token_printer(std::vector<nip::Token_t>&, std::ostream&);

	  public:
		compiler(nip::Options& o) : opt(o), parser(errhdlr, opt, *opt.error_stream){};
		void argument_parser(int argc, char* argv[]);
		void compile();
	};
}
