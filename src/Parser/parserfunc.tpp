#pragma once

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

ALWAYS_INLINE void nip::parse::Parser::next_sym() {
	last_token_data = (*current).address;
	if (current != end) {
		cur_symbol = *++current;
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
