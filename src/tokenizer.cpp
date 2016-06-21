#include "token.hpp"
#include "nip.hpp"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#ifdef _MSC_VER
#define ALWAYS_INLINE __forceinline
#elif defined __GNUC__
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#endif

std::vector<int64_t> token_int_cache;
std::vector<double> token_float_cache;
std::vector<std::string> token_identifier_cache;

ALWAYS_INLINE bool char_is_in(const char* array, char c) {
	for (size_t i = 0; array[i] != '\0'; i++) {
		if (array[i] == c) {
			return true;
		}
	}
	return false;
}

std::vector<nip::Token_t> nip::tokenizer(std::istream& file) {
	std::vector<nip::Token_t> token_list;

	char curchar     = '\0';
	size_t curline   = 0;
	size_t curcolumn = 0;
	size_t curindent = 0;
	std::vector<size_t> curindentlevels;
	enum : uint8_t { UNSET, SPACE, TAB } indent_type = UNSET;
	bool afternewline = true;
	bool skipswitch   = false;
	bool skipnumber   = false;

	// Utility functions
	auto is_whitespace = [](char c) { return (c == ' ') || (c == '\n') || (c == '\t'); };
	auto is_letter     = [](char c) { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'); };
	auto is_numeric    = [](char c) { return (('0' <= c && c <= '9') || c == '.' || c == '-'); };
	auto is_number     = [](char c) { return (('0' <= c && c <= '9') || c == '.'); };
	auto is_digit      = [](char c) { return ('0' <= c && c <= '9'); };
	auto is_terminal   = [](char c) { return char_is_in(" \t\n/{}[]()-+,.:<>", c); };
	auto advance_char  = [&curchar, &curcolumn, &file]() {
		curchar = file.get();
		curcolumn++;
		return !!file;
	};

	// String capture function
	auto str_advance = [&](bool single_quote) -> std::string {
		std::string output;
		bool valid   = true;
		bool escaped = false;
		do {
			advance_char();
			if (escaped) {
				output += curchar;
				escaped = false;
			}
			else if (curchar == '\\') {
				escaped = true;
			}
			else if (single_quote && curchar == '\n') {
				// TODO: Throw error
				valid = false;
			}
			else if (curchar == '"') {
				if (single_quote) {
					valid = false;
				}
				else {
					if (file.get() == '"' && file.peek() == '"') {
						valid = false;
						file.unget();
						advance_char();
						advance_char();
					}
				}
			}
			else {
				output += curchar;
			}
		} while (valid);

		return output;
	};

	while (advance_char()) {
		// This pushes INDENTs and DEDENTs to update to the current level of indentation. This also
		// checks to make sure that the indentation is consistant.
		auto update_indentation = [&curindent, &curindentlevels, &token_list,
		                           &afternewline](size_t ic) {
			if (ic > curindent) {
				curindentlevels.push_back(ic);
				token_list.emplace_back(INDENT);
			}
			else if (ic < curindent) {
				while (curindentlevels.size() && curindentlevels.back() != ic) {
					curindentlevels.pop_back();
					token_list.emplace_back(DEDENT);
				}
				if (curindentlevels.size() == 0) {
					std::cerr << "Mismatched indentation, inconsistant levels\n";
				}
			}
			curindent    = ic;
			afternewline = false;
		};

		if (curchar == '\n') {
			afternewline = true;
			curline++;
			curcolumn = 0;
			token_list.emplace_back(NEWLINE);
		}

		else if (curchar == '\t') {
			size_t ic = 1;
			if (afternewline) {
				// Check for indent consistancy
				if (indent_type == TAB) {
					// If consistant, find all the tabs
					while (advance_char() && curchar == '\t') {
						ic++;
					}
				}
				else if (indent_type == SPACE) {
					std::cerr << "Mismatched indentation, expected tab, found space\n";
					return token_list;
				}
				else if (indent_type == UNSET) {
					indent_type = TAB;
				}

				// Update indentation level
				update_indentation(ic);
			}
			continue;
		}

		else if (curchar == ' ') {
			size_t ic = 1;
			if (afternewline) {
				// Check for indent consistancy
				if (indent_type == SPACE || indent_type == UNSET) {
					// If consistant, find all the spaces
					while (file.peek() == ' ' && advance_char()) {
						ic++;
					}

					indent_type = SPACE;
				}
				else if (indent_type == TAB) {
					std::cerr << "Mismatched indentation, expected space, found tab\n";
					return token_list;
				}

				// Update indentation level
				update_indentation(ic);
			}
			continue;
		}

		else if (afternewline && !is_whitespace(curchar)) {
			if (curindent > 0) {
				curindent = 0;
				while (curindentlevels.size()) {
					curindentlevels.pop_back();
					token_list.emplace_back(DEDENT);
				}
			}
			afternewline = false;
		}

		if (!skipswitch) {
			switch (curchar) {
				// Find // comments, than eat the whole line afterwards
				// If it's not a comment, skip, and parse as an
				case '/':
					if (file.peek() == '/') {
						// Eat the line
						while (file.peek() == '\n' && advance_char()) {
							continue;
						}
					}
					else {
						skipswitch = true;
						file.unget();
					}
					continue;

				// Find any arrows ->
				case '-':
					if (file.peek() == '>') {
						advance_char();
						token_list.emplace_back(ARROW);
					}
					else {
						skipswitch = true;
						file.unget();
					}
					continue;

				// Find : and ::
				case ':':
					if (file.peek() == ':') {
						advance_char();
						token_list.emplace_back(DOUBLE_COLON);
					}
					else {
						token_list.emplace_back(COLON);
					}
					continue;

				// Find . and ...
				case '.':
					if (file.get() == '.' && file.peek() == '.') {
						token_list.emplace_back(TRIPLE_DOT);
						file.unget();
						advance_char();
						advance_char();
					}
					else {
						file.unget();
						token_list.emplace_back(DOT);
					}
					continue;

				// Find {}
				case '{':
					token_list.emplace_back(LEFT_BRACKET);
					continue;
				case '}':
					token_list.emplace_back(RIGHT_BRACKET);
					continue;

				// Find []
				case '[':
					token_list.emplace_back(LEFT_SQUARE);
					continue;
				case ']':
					token_list.emplace_back(RIGHT_SQUARE);
					continue;

				// Find ()
				case '(':
					token_list.emplace_back(LEFT_PAREN);
					continue;
				case ')':
					token_list.emplace_back(RIGHT_PAREN);
					continue;

				// Find ,
				case ',':
					token_list.emplace_back(COMMA);
					continue;

				// Find <>
				case '<':
					token_list.emplace_back(LEFT_CARROT);
					continue;
				case '>':
					token_list.emplace_back(RIGHT_CARROT);
					continue;

				// Find +
				case '+':
					token_list.emplace_back(PLUS);
					continue;

				// Deal with string literals, both "blah" and """blah"""
				case '"': {
					bool single = true;
					if (file.get() == '"' && file.peek() == '"') {
						file.unget();
						advance_char();
						advance_char();
						single = false;
					}
					else {
						file.unget();
					}
					std::string str = str_advance(single);
					token_identifier_cache.push_back(std::move(str));
					token_list.emplace_back(LIT_STRING, token_identifier_cache.size() - 1);
					continue;
				}
			}
		}
		else {
			skipswitch = false;
		}

		if (!skipnumber) {
			if (is_numeric(curchar)) {
				bool negitive = false;
				if (curchar == '-') {
					if (is_number(file.peek())) {
						negitive = true;
					}
					else {
						skipnumber = true;
						file.unget();
						continue;
					}
				}
				int64_t int_number = 0;
				if (is_digit(curchar)) {
					int_number = curchar - '0';
				}
				while (is_digit(file.peek()) && advance_char()) {
					int_number = int_number * 10 + (curchar - '0');
				}
				if (file.peek() == '.') {
					advance_char();
					double float_number   = int_number;
					int64_t decimal_place = -1;
					while (is_digit(file.peek()) && advance_char()) {
						double num = curchar - '0';
						float_number += num * std::pow(10.0L, decimal_place--);
					}
					if (file.peek() == 'e' && advance_char()) {
						int64_t exponent = 0;
						bool neg_exp     = false;
						if (file.peek() == '-' && advance_char()) {
							neg_exp = true;
						}
						while (is_digit(file.peek()) && advance_char()) {
							exponent = exponent * 10 + (curchar - '0');
						}
						if (neg_exp) {
							exponent *= -1;
						}
						float_number *= std::pow(10.0L, (double) exponent);
					}
					if (negitive) {
						float_number *= -1.0;
					}
					token_float_cache.push_back(float_number);
					token_list.emplace_back(LIT_FLOAT, token_float_cache.size() - 1);
				}
				else {
					if (negitive) {
						int_number *= -1;
					}
					token_int_cache.push_back(int_number);
					token_list.emplace_back(LIT_INT, token_int_cache.size() - 1);
				}
				continue;
			}
		}
		else {
			skipnumber = false;
		}

		if (is_letter(curchar)) {
			std::string str;
			str += curchar;
			while (!is_terminal(file.peek()) && advance_char()) {
				str += curchar;
			}
			TokenType_t tt;
			if (str == "about") {
				tt = KEY_ABOUT;
			}
			else if (str == "call") {
				tt = KEY_CALL;
			}
			else if (str == "case") {
				tt = KEY_CASE;
			}
			else if (str == "define") {
				tt = KEY_DEFINE;
			}
			else if (str == "do") {
				tt = KEY_DO;
			}
			else if (str == "docs") {
				tt = KEY_DOCS;
			}
			else if (str == "elif") {
				tt = KEY_ELIF;
			}
			else if (str == "else") {
				tt = KEY_ELSE;
			}
			else if (str == "if") {
				tt = KEY_IF;
			}
			else if (str == "instance") {
				tt = KEY_INSTANCE;
			}
			else if (str == "intrinsic") {
				tt = KEY_INTRIN;
			}
			else if (str == "jump") {
				tt = KEY_JUMP;
			}
			else if (str == "left") {
				tt = KEY_LEFT;
			}
			else if (str == "match") {
				tt = KEY_MATCH;
			}
			else if (str == "operator") {
				tt = KEY_OP;
			}
			else if (str == "permission") {
				tt = KEY_PERM;
			}
			else if (str == "return") {
				tt = KEY_RET;
			}
			else if (str == "right") {
				tt = KEY_RIGHT;
			}
			else if (str == "synonym") {
				tt = KEY_SYNONYM;
			}
			else if (str == "trait") {
				tt = KEY_TRAIT;
			}
			else if (str == "type") {
				tt = KEY_TYPE;
			}
			else if (str == "vocab") {
				tt = KEY_VOCAB;
			}
			else if (str == "with") {
				tt = KEY_WITH;
			}
			else {
				tt = IDENTIFIER;
			}
			if (tt == IDENTIFIER) {
				token_identifier_cache.push_back(std::move(str));
				token_list.emplace_back(IDENTIFIER, token_identifier_cache.size() - 1);
			}
			else {
				token_list.emplace_back(tt);
			}
		}
	}

	return token_list;
};

// The format used is the following:
// XXXXX: 13_wide_name_ | <text equivilant>
// ex.
// 00000:        KEY_IF | if
// 00001:    LEFT_PAREN | (
// 00002:    IDENTIFIER | foo
// 00003:          PLUS | +
// 00004:    IDENTIFIER | bar
// 00005:         MINUS | -
// 00006:    IDENTIFIER | blah
// 00007:   RIGHT_PAREN | )
// 00008:         COLON | :
// 00009:       NEWLINE | \n
// 00010:        INDENT |
// 00011:    IDENTIFIER | true
// 00012:       NEWLINE | \n
// 00013:        DEDENT |
// 00014:       NEWLINE | \n
void nip::token_printer(std::vector<nip::Token_t>& tklist, std::ostream& out) {
	size_t length = tklist.size();
	size_t digits = std::ceil(std::log10(length));

	size_t i = 0;
	for (auto t : tklist) {
		out << std::setfill('0') << std::setw(digits) << i++ << ": ";
		out << std::setfill(' ') << std::setw(13);
		switch (t.type) {
			case NUL:
				out << "NUL"
				    << " | ";
				break;
			case WHITESPACE:
				out << "WHITESPACE"
				    << " | ";
				break;
			case INDENT:
				out << "INDENT"
				    << " | ";
				break;
			case DEDENT:
				out << "DEDENT"
				    << " | ";
				break;
			case NEWLINE:
				out << "NEWLINE"
				    << " | \\n";
				break;
			case DOUBLE_SLASH:
				out << "DOUBLE_SLASH"
				    << " | //";
				break;
			case LEFT_BRACKET:
				out << "LEFT_BRACKET"
				    << " | {";
				break;
			case RIGHT_BRACKET:
				out << "RIGHT_BRACKET"
				    << " | }";
				break;
			case LEFT_SQUARE:
				out << "LEFT_SQUARE"
				    << " | [";
				break;
			case RIGHT_SQUARE:
				out << "RIGHT_SQUARE"
				    << " | ]";
				break;
			case LEFT_PAREN:
				out << "LEFT_PAREN"
				    << " | (";
				break;
			case RIGHT_PAREN:
				out << "RIGHT_PAREN"
				    << " | )";
				break;
			case ARROW:
				out << "ARROW"
				    << " | ->";
				break;
			case PLUS:
				out << "PLUS"
				    << " | +";
				break;
			case MINUS:
				out << "MINUS"
				    << " | -";
				break;
			case COMMA:
				out << "COMMA"
				    << " | ,";
				break;
			case DOT:
				out << "DOT"
				    << " | .";
				break;
			case TRIPLE_DOT:
				out << "TRIPLE_DOT"
				    << " | ...";
				break;
			case COLON:
				out << "COLON"
				    << " | :";
				break;
			case DOUBLE_COLON:
				out << "DOUBLE_COLON"
				    << " | ::";
				break;
			case LEFT_CARROT:
				out << "LEFT_CARROT"
				    << " | <";
				break;
			case RIGHT_CARROT:
				out << "RIGHT_CARROT"
				    << " | >";
				break;
			case IDENTIFIER:
				out << "IDENTIFIER"
				    << " | " << token_identifier_cache[t.address];
				break;
			case LIT_INT:
				out << "LIT_INT"
				    << " | " << token_int_cache[t.address];
				break;
			case LIT_FLOAT:
				out << "LIT_FLOAT"
				    << " | " << token_float_cache[t.address];
				break;
			case LIT_STRING:
				out << "LIT_STRING"
				    << " | " << token_identifier_cache[t.address];
				break;
			case KEY_ABOUT:
				out << "KEY_ABOUT"
				    << " | about";
				break;
			case KEY_CALL:
				out << "KEY_CALL"
				    << " | call";
				break;
			case KEY_CASE:
				out << "KEY_CASE"
				    << " | case";
				break;
			case KEY_DEFINE:
				out << "KEY_DEFINE"
				    << " | define";
				break;
			case KEY_DO:
				out << "KEY_DO"
				    << " | do";
				break;
			case KEY_DOCS:
				out << "KEY_DOCS"
				    << " | docs";
				break;
			case KEY_ELIF:
				out << "KEY_ELIF"
				    << " | elif";
				break;
			case KEY_ELSE:
				out << "KEY_ELSE"
				    << " | else";
				break;
			case KEY_IF:
				out << "KEY_IF"
				    << " | if";
				break;
			case KEY_INSTANCE:
				out << "KEY_INSTANCE"
				    << " | instance";
				break;
			case KEY_INTRIN:
				out << "KEY_INTRIN"
				    << " | intrinsic";
				break;
			case KEY_JUMP:
				out << "KEY_JUMP"
				    << " | jump";
				break;
			case KEY_LEFT:
				out << "KEY_LEFT"
				    << " | left";
				break;
			case KEY_MATCH:
				out << "KEY_MATCH"
				    << " | match";
				break;
			case KEY_OP:
				out << "KEY_OP"
				    << " | operator";
				break;
			case KEY_PERM:
				out << "KEY_PERM"
				    << " | permission";
				break;
			case KEY_RET:
				out << "KEY_RET"
				    << " | return";
				break;
			case KEY_RIGHT:
				out << "KEY_RIGHT"
				    << " | right";
				break;
			case KEY_SYNONYM:
				out << "KEY_SYNONYM"
				    << " | synonym";
				break;
			case KEY_TRAIT:
				out << "KEY_TRAIT"
				    << " | trait";
				break;
			case KEY_TYPE:
				out << "KEY_TYPE"
				    << " | type";
				break;
			case KEY_VOCAB:
				out << "KEY_VOCAB"
				    << " | vocab";
				break;
			case KEY_WITH:
				out << "KEY_WITH"
				    << " | with";
				break;

			default:
				out << "UNKNOWN"
				    << " | ";
		}
		out << "\n";
	}
}
