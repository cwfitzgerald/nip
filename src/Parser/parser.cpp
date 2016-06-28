#include "parser.hpp"

#include "../Error/errorhandler.hpp"
#include "../utilmacro.hpp"
#include <exception>

Parse_Fatal_Error_t Parse_Fatal_Error;

void nip::parse::Parser::parse(const std::vector<nip::Token_t>& tokens) {
	start = tokens.begin();
	end   = tokens.end();
	program();
}

ALWAYS_INLINE void nip::parse::Parser::next_sym() {
	last_token_data = (*start).address;
	if (++start != end) {
		cur_symbol = *start;
	}
	else {
		cur_symbol = nip::Token_t(NUL);
	}
}

ALWAYS_INLINE void nip::parse::Parser::error(const char* msg, nip::error::_Error_Type et) {
	errhdlr.add_error(et, msg, cur_symbol.linenum, cur_symbol.charnum, true);
	if (et == nip::error::FATAL_ERROR) {
		throw Parse_Fatal_Error;
	}
}

ALWAYS_INLINE bool nip::parse::Parser::expect(nip::TokenType_t tt, const char* msg,
                                              nip::error::_Error_Type et) {
	if (accept(tt)) {
		return true;
	}
	error(msg, et);
	return false;
}

ALWAYS_INLINE void nip::parse::Parser::newlines() {
	while (accept(NEWLINE))
		continue;
}

void nip::parse::Parser::program() {
	try {
		while (!accept(NUL)) {
			element();
		}
	}
	catch (Parse_Fatal_Error_t e) {
		errhdlr.print_errors(*opt.error_stream);
		return;
	}
}

void nip::parse::Parser::element() {
	if (accept(KEY_TRAIT)) {
		trait_declaration();
	}
	else if (accept(KEY_INTRIN)) {
		intrinsic_declaration();
	}
	else if (accept(KEY_DEFINE)) {
		function_definition();
	}
	else if (accept(KEY_INSTANCE)) {
		instance_definition();
	}
	else if (accept(KEY_PERMIT)) {
		permission_definition();
	}
	else if (accept(KEY_ABOUT)) {
		about_section();
	}
	else if (accept(KEY_TYPE)) {
		if (accept(KEY_SYNONYM)) {
			type_synonym();
		}
		else {
			type_definition();
		}
	}
	else if (accept(KEY_VOCAB)) {
		if (accept(KEY_SYNONYM)) {
			vocabulary_synonym();
		}
		else {
			vocabulary_definition();
		}
	}
	else if (accept(KEY_IMPORT)) {
		import_element();
	}
	else if (accept(KEY_EXPORT)) {
		// export_element(); // Not implimented
		error("export isn't a supported language feature");
	}
	else {
		term();
	}
	endofstatement();
	newlines();
}

void nip::parse::Parser::term() {}

void nip::parse::Parser::import_element() {
	block_start();
	while (!is(DEDENT, RIGHT_BRACKET)) {
		import_name();
	}
	block_end();
}

void nip::parse::Parser::export_element() {
	// Not a current language feature
}

void nip::parse::Parser::trait_declaration() {
	if (expect(IDENTIFIER, "expected identifier")) {
		trait_arguments();
		signature();
	}
}

void nip::parse::Parser::intrinsic_declaration() {
	if (expect(IDENTIFIER, "expected identifier")) {
		signature();
	}
}

void nip::parse::Parser::function_definition() {
	if (expect(IDENTIFIER, "expected identifier")) {
		signature();
		generic_block();
	}
}

void nip::parse::Parser::instance_definition() {
	if (expect(IDENTIFIER, "expected identifier")) {
		signature();
		generic_block();
	}
}

void nip::parse::Parser::permission_definition() {
	if (expect(IDENTIFIER, "expected identifier")) {
		signature();
		generic_block();
	}
}

void nip::parse::Parser::type_definition() {
	if (accept(IDENTIFIER)) {
		block_start();
		while (accept(KEY_CASE)) {
			type_name();
			newlines();
		}
	}
	else {
		error("expected identifier");
	}
}

void nip::parse::Parser::about_section() {
	if (accept(IDENTIFIER)) {
		block_start();
		while (is(DEDENT, RIGHT_BRACKET)) {
			metadata_field();
		}
		block_end();
	}
	else {
		error("expected identifier");
	}
}

void nip::parse::Parser::metadata_field() {
	if (accept(KEY_DOCS)) {
		if (expect(LIT_STRING, "expected string literal")) {
			; // Add documentation string
		}
	}
	else if (accept(KEY_OP)) {
		if (accept(KEY_LEFT)) {
			if (accept(LIT_INT)) {
				; // Add left-assosiative operator with appropriate precidence.
			}
		}
		else if (accept(KEY_RIGHT)) {
			if (accept(LIT_INT)) {
				; // Add right-assosiative operator with appropriate precidence.
			}
		}
		else {
			// Add general data
		}
	}
	else if (accept(IDENTIFIER)) {
		newlines();
	}
	else {
		error("expected identifier");
	}
}

void nip::parse::Parser::word_synonym() {}

void nip::parse::Parser::type_synonym() {}

void nip::parse::Parser::vocabulary_synonym() {}

void nip::parse::Parser::import_name() {
	if (is(IDENTIFIER)) {
		qualified_name();
	}
	else if (accept(KEY_TYPE)) {
		qualified_name();
	}
	else if (accept(KEY_VOCAB)) {
		qualified_name();
	}
	else {
		error("expected qualified name, \"vocab\" or \"type\"");
	}
}

void nip::parse::Parser::vocabulary_definition() {
	if (accept(IDENTIFIER)) {
		if (accept(SEMI_COLON)) {
			return;
		}
		else {
			generic_block();
		}
	}
	else {
		error("expected indentifier");
	}
}

ALWAYS_INLINE void nip::parse::Parser::generic_block() {
	block_start();
	while (!is(RIGHT_BRACKET, DEDENT)) {
		element();
	}
	block_end();
}

void nip::parse::Parser::block_start() {
	if (accept(LEFT_BRACKET)) {
		block_type.push_back(BRACKETS);
		newlines();
	}
	else if (accept(COLON)) {
		block_type.push_back(INDENTATION);
		newlines();
		if (expect(INDENT, "expected indent")) {
			return;
		}
	}
	else {
		error("expected the start of a block");
	}
}

void nip::parse::Parser::block_end() {
	if (block_type.back() == INDENTATION) {
		if (expect(DEDENT, "expected dedent")) {
			block_type.pop_back();
		}
	}
	else if (block_type.back() == BRACKETS) {
		if (expect(RIGHT_BRACKET, "expected right bracket")) {
			block_type.pop_back();
		}
	}
}

void nip::parse::Parser::qualified_name() {
	do {
		if (expect(IDENTIFIER, "expected identifier")) {
			;
		}
	} while (accept(DOUBLE_COLON));
}

void nip::parse::Parser::trait_arguments() {
	do {
		if (expect(IDENTIFIER, "expected identifier")) {
			;
		}
	} while (accept(COMMA));
	accept(RIGHT_CARROT);
}

void nip::parse::Parser::type_name() {
	qualified_name();
	if (accept(LEFT_CARROT)) {
		trait_arguments();
	}
	if (accept(LEFT_PAREN)) {
		do {
			expect(IDENTIFIER, "expected identifier");
		} while (accept(COMMA));
		expect(RIGHT_PAREN);
	}
}

void nip::parse::Parser::signature() {
	expect(LEFT_PAREN, "expected (");
	do {
		expect(IDENTIFIER, "expected identifier");
	} while (accept(COMMA));
	expect(RIGHT_PAREN, "expected )");
	expect(ARROW, "expected ->");
}

void nip::parse::Parser::endofstatement() {
	if (accept(NEWLINE)) {
		return;
	}
	else if (accept(SEMI_COLON)) {
		return;
	}
	else {
		error("end of statement expected", nip::error::FATAL_ERROR);
	}
}
