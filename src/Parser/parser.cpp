#include "parser.hpp"

#include "../Error/errorhandler.hpp"
#include "../utilmacro.hpp"
#include <exception>

Parse_Fatal_Error_t Parse_Fatal_Error;

void nip::parse::Parser::parse(std::vector<nip::Token_t>& tokens) {
	start = tokens.begin();
	end   = tokens.end();
	program();
}

ALWAYS_INLINE void nip::parse::Parser::next_sym() {
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
			if (accept(KEY_VOCAB)) {
				vocabulary();
			}
			else {
				element();
			}
		}
	}
	catch (Parse_Fatal_Error_t e) {
		errhdlr.print_errors(*opt.error_stream);
		return;
	}
}

void nip::parse::Parser::vocabulary() {
	if (accept(IDENTIFIER)) {
		if (accept(SEMI_COLON)) {
			return;
		}
		else {
			block_start();
			while (!is(RIGHT_BRACKET, DEDENT)) {
				element();
			}
			block_end();
		}
	}
	else {
		error("expected indentifier");
	}
}

void nip::parse::Parser::element() {
	;
}

void nip::parse::Parser::block_start() {
	if (accept(LEFT_BRACKET)) {
		block_type.push_back(BRACKETS);
		while (accept(NEWLINE))
			continue;
	}
	else if (accept(COLON)) {
		block_type.push_back(INDENTATION);
		newlines();
		if (accept(INDENT)) {
			return;
		}
		else {
			error("expected indent");
		}
	}
	else {
		error("expected the start of a block");
	}
}

void nip::parse::Parser::block_end() {
	if (block_type.back() == INDENTATION) {
		if (accept(DEDENT)) {
			block_type.pop_back();
		}
		else {
			error("expected dedent");
		}
	}
	else if (block_type.back() == BRACKETS) {
		if (accept(RIGHT_BRACKET)) {
			block_type.pop_back();
		}
		else {
			error("expected right bracket");
		}
	}
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
