#include "parser.hpp"

#include "../utilmacro.hpp"

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

ALWAYS_INLINE bool nip::parse::Parser::accept(nip::TokenType_t tt) {
	if (cur_symbol.type == tt) {
		next_sym();
		return true;
	}
	else {
		return false;
	}
}

ALWAYS_INLINE bool nip::parse::Parser::expect(nip::TokenType_t tt, nip::error::_Error_Type et,
                                              const char* msg) {
	if (accept(tt)) {
		return true;
	}
	errhdlr.add_error(et, msg, cur_symbol.linenum, cur_symbol.charnum, true);
	return false;
}

void nip::parse::Parser::program() {}
