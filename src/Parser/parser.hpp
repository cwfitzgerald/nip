#pragma once

#include "../Error/errorhandler.hpp"
#include "../options.hpp"
#include "../token.hpp"
#include "../utilmacro.hpp"

#include <iosfwd>
#include <vector>

namespace nip::parse {
	class Parser {
	  public:
		Parser(nip::error::Error_Handler& e, nip::Options& o, std::ostream& err)
		    : errhdlr(e), opt(o), err_stream(err), cur_symbol(NUL){};
		void parse(std::vector<nip::Token_t>&);

	  private:
		nip::error::Error_Handler& errhdlr;
		nip::Options& opt;
		std::ostream& err_stream;
		nip::Token_t cur_symbol;

		std::vector<nip::Token_t>::iterator start, end;

		enum blocktype_t : bool { INDENTATION, BRACKETS };
		std::vector<blocktype_t> block_type;

		ALWAYS_INLINE void next_sym();
		ALWAYS_INLINE void error(const char* msg,
		                         nip::error::_Error_Type et = nip::error::FATAL_ERROR);
		template <class... Args>
		ALWAYS_INLINE bool is(nip::TokenType_t, nip::TokenType_t, Args...);
		ALWAYS_INLINE bool is(nip::TokenType_t);
		template <class... Args>
		ALWAYS_INLINE bool accept(nip::TokenType_t, nip::TokenType_t, Args...);
		ALWAYS_INLINE bool accept(nip::TokenType_t);
		ALWAYS_INLINE bool expect(nip::TokenType_t tt, const char* msg, nip::error::_Error_Type et);

		void program();
		void vocabulary();

		// Elements
		void element();
		void declaration_element();
		void definition_element();
		void metadata_element();
		void synonym_element();
		void import_element();
		void export_element();
		void term();

		// Declarations
		void trait_declaration();
		void intrinsic_declaration();

		// Definitions
		void word_definition();
		void instance_definition();
		void permission_definition();

		// Metadata
		void metadata_field();

		// Synonym
		void word_synonym();
		void type_synonym();
		void vocabulary_synonym();

		// Imports
		void import_name();

		// Terms
		void literal_term();
		void word_term();
		void section_term();
		void group_term();
		void vector_term();
		void lambda_term();
		void match_term();
		void if_term();
		void do_term();
		void block_term();
		void with_term();

		// General stuff
		void block_start();
		void block_end();
		void signature();
		void endofstatement();
		void newlines();
	};
}

class Parse_Fatal_Error_t : public std::exception {
  public:
	const char* what() const throw() {
		return "There was a parser error\n";
	}
};

template <class... Args>
ALWAYS_INLINE bool nip::parse::Parser::accept(nip::TokenType_t tt1, nip::TokenType_t tt2,
                                              Args... a) {
	if (cur_symbol.type == tt1) {
		next_sym();
		return true;
	}
	else {
		return accept(tt2, a...);
	}
}

ALWAYS_INLINE bool nip::parse::Parser::accept(nip::TokenType_t tt) {
	if (cur_symbol.type == NUL && tt == NUL) {
		error("unexpected end of file");
		return false;
	}
	else if (cur_symbol.type == tt) {
		next_sym();
		return true;
	}
	else {
		return false;
	}
}

template <class... Args>
ALWAYS_INLINE bool nip::parse::Parser::is(nip::TokenType_t tt1, nip::TokenType_t tt2, Args... a) {
	if (cur_symbol.type == tt1) {
		return true;
	}
	else {
		return is(tt2, a...);
	}
}

ALWAYS_INLINE bool nip::parse::Parser::is(nip::TokenType_t tt) {
	return cur_symbol.type == tt;
}
