#pragma once

#include "../Error/errorhandler.hpp"
#include "../token.hpp"
#include "../utilmacro.hpp"

#include <iosfwd>
#include <vector>

namespace nip::parse {
	class Parser {
	  public:
		Parser(nip::error::Error_Handler& e, std::ostream& err)
		    : errhdlr(e), err_stream(err), cur_symbol(NUL){};
		void parse(std::vector<nip::Token_t>&);

	  private:
		nip::error::Error_Handler& errhdlr;
		std::ostream& err_stream;
		nip::Token_t cur_symbol;

		std::vector<nip::Token_t>::iterator start, end;

		ALWAYS_INLINE void next_sym();
		ALWAYS_INLINE bool accept(nip::TokenType_t);
		ALWAYS_INLINE bool expect(nip::TokenType_t tt, nip::error::_Error_Type et, const char* msg);

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
		void block();
		void signature();
	};
}
