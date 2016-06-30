#include "token.hpp"
#include "nip.hpp"
#include "util.hpp"
#include "utilmacro.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

using namespace std::string_literals;

std::unordered_map<std::string, nip::TokenType_t> keyword_map = {
    //
    {"about", nip::KEY_ABOUT},
    {"call", nip::KEY_CALL},
    {"case", nip::KEY_CASE},
    {"define", nip::KEY_DEFINE},
    {"do", nip::KEY_DO},
    {"docs", nip::KEY_DOCS},
    {"elif", nip::KEY_ELIF},
    {"else", nip::KEY_ELSE},
    {"if", nip::KEY_IF},
    {"instance", nip::KEY_INSTANCE},
    {"intrinsic", nip::KEY_INTRIN},
    {"jump", nip::KEY_JUMP},
    {"left", nip::KEY_LEFT},
    {"match", nip::KEY_MATCH},
    {"operator", nip::KEY_OP},
    {"permission", nip::KEY_PERMIT},
    {"return", nip::KEY_RET},
    {"right", nip::KEY_RIGHT},
    {"synonym", nip::KEY_SYNONYM},
    {"trait", nip::KEY_TRAIT},
    {"type", nip::KEY_TYPE},
    {"vocab", nip::KEY_VOCAB},
    {"with", nip::KEY_WITH}
    //
};

ALWAYS_INLINE bool char_is_in(const char* array, char c) {
	for (size_t i = 0; array[i] != '\0'; i++) {
		if (array[i] == c) {
			return true;
		}
	}
	return false;
}

std::vector<nip::Token_t> nip::compiler::tokenizer() {
	std::vector<nip::Token_t> token_list;

	std::stringstream file;

	char curchar     = '\0';
	size_t curline   = 1;
	size_t curcolumn = 0;
	size_t curindent = 0;
	std::vector<size_t> curindentlevels;
	enum : uint8_t { UNSET, SPACE, TAB } indent_type = UNSET;
	bool afternewline = true;
	bool skipswitch   = false;
	bool skipnumber   = false;

	// Utility functions
	auto is_whitespace = [](char c) { return (c == ' ') || (c == '\n') || (c == '\t'); };
	auto is_numeric    = [](char c) { return (('0' <= c && c <= '9') || c == '.' || c == '-'); };
	auto is_number     = [](char c) { return (('0' <= c && c <= '9') || c == '.'); };
	auto is_digit      = [](char c) { return ('0' <= c && c <= '9'); };
	auto is_terminal   = [](char c) { return char_is_in(" \t\n/{}[]()-+,.:;<>", c); };
	auto is_letter     = [&](char c) { return !is_terminal(c) && !is_numeric(c); };
	auto advance_char  = [&curchar, &curcolumn, &file]() {
		curchar = file.get();
		curcolumn++;
		return bool(file);
	};
	auto return_char = [&curchar, &curline, &curcolumn, &file]() {
		file.unget();
		if (file.peek() == '\n') {
			curline--;
		}
		else {
			curcolumn--;
		}
	};

	auto escaper = [this, curline, curcolumn](char c) {
		char out = '\0';
		switch (c) {
			case 'a':
				out = '\a';
				break;
			case 'b':
				out = '\b';
				break;
			case 'f':
				out = '\f';
				break;
			case 'n':
				out = '\n';
				break;
			case 'r':
				out = '\r';
				break;
			case 't':
				out = '\t';
				break;
			case 'v':
				out = '\v';
				break;
			case '\0':
				out = '\0';
				break;
			case '\\':
				out = '\\';
				break;
			case '\'':
				out = '\'';
				break;
			case '\"':
				out = '\"';
				break;
			case '\?':
				out = '\?';
				break;
			default: {
				errhdlr.add_error(error::ERROR, ("improper escape code \\"s + c).data(), curline,
				                  curcolumn, true);
				break;
			}
		}
		return out;
	};

	// String capture function
	auto str_advance = [&](bool single_quote) -> std::string {
		std::string output;
		bool valid   = true;
		bool escaped = false;
		do {
			advance_char();
			if (escaped) {
				output += escaper(curchar);
				escaped = false;
			}
			else if (curchar == '\\') {
				escaped = true;
			}
			else if (single_quote && curchar == '\n') {
				// TODO: Throw error
				valid = false;
			}
			else if (curchar == '\n') {
				output += curchar;
				curline++;
				curchar = 0;
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

	{
		// Preprocess away any lines with only comments, and load the
		// source file into an array for the error handler
		std::vector<std::string> source_cache;

		while (*opt.program_stream) {
			std::string tmp;
			std::getline(*opt.program_stream, tmp);
			size_t index       = 0;
			size_t lines_total = source_cache.size() + 1;
			while (index < tmp.size() && is_whitespace(tmp[index])) {
				index++;
			}
			if (index + 1 < tmp.size() && tmp[index] == '/' && tmp[index + 1] == '/') {
				lines_total += 1;
				file << '\n';
			}
			else if (index < tmp.size() && tmp[index] == '/' && tmp[index + 1] == '*') {
				char c = '\0';

				while (*opt.program_stream) {
					if (opt.program_stream->get(c) && c == '\n') {
						lines_total++;
						file << '\n';
					}
					else if (c == '*' && opt.program_stream->peek() == '/') {
						break;
					}
				}
				file << "\n";
			}
			else {
				file << tmp << '\n';
			}
			source_cache.push_back(std::move(tmp));
		}
		errhdlr.set_source(std::move(source_cache));
	}

	while (advance_char()) {
		// This pushes INDENTs and DEDENTs to update to the current level of indentation. This
		// also
		// checks to make sure that the indentation is consistant.
		auto update_indentation = [&curindent, &curindentlevels, &token_list, &afternewline,
		                           &curline, &curcolumn, this](size_t ic) {
			if (ic > curindent) {
				curindentlevels.push_back(ic);
				token_list.emplace_back(INDENT, curline, curcolumn);
			}
			else if (ic < curindent) {
				while (curindentlevels.size() && curindentlevels.back() != ic) {
					curindentlevels.pop_back();
					token_list.emplace_back(DEDENT, curline, curcolumn);
				}
				if (curindentlevels.size() == 0) {
					errhdlr.add_error(error::ERROR, "mismatched indentation, inconsistant levels",
					                  curline, curcolumn, true);
				}
			}
			curindent    = ic;
			afternewline = false;
		};

		// Preprocess away any lines with just a comment in them to prevent indentation mess ups
		if (curchar == '\n') {
			afternewline = true;
			curline++;
			curcolumn = 0;
			if (token_list.size() && token_list.back().type != NEWLINE) {
				token_list.emplace_back(NEWLINE, curline, curcolumn);
			}
			continue;
		}

		else if (curchar == '\t') {
			size_t ic = 1;
			if (afternewline) {
				// Check for indent consistancy
				if (indent_type == TAB || indent_type == UNSET) {
					// If consistant, find all the tabs
					while (file.peek() == ' ' && advance_char()) {
						ic++;
					}
				}
				else if (indent_type == SPACE) {
					errhdlr.add_error(error::FATAL_ERROR,
					                  "expected tab-based indentation, found spaces", curline,
					                  curcolumn, true);
					return token_list;
				}
				if (indent_type == UNSET) {
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
					errhdlr.add_error(error::FATAL_ERROR,
					                  "expected space-based indentation, found tabs", curline,
					                  curcolumn, true);
					return token_list;
				}
				if (indent_type == UNSET) {
					indent_type = SPACE;
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
					token_list.emplace_back(DEDENT, curline, curcolumn);
				}
			}
		}

		afternewline = false;

		if (!skipswitch) {
			switch (curchar) {
				// Find // comments, than eat the whole line afterwards
				// If it's not a comment, skip, and parse as an
				case '/':
					if (file.peek() == '/') {
						// Eat the line
						while (file.peek() != '\n' && advance_char()) {
							continue;
						}
					}
					else if (file.peek() == '*') {
						while (advance_char()) {
							if (curchar == '*' && file.peek() == '/') {
								break;
							}
						}
					}
					else {
						skipswitch = true;
						return_char();
					}
					continue;

				// Find any arrows ->
				case '-':
					if (file.peek() == '>') {
						advance_char();
						token_list.emplace_back(ARROW, curline, curcolumn - 1);
					}
					else {
						skipswitch = true;
						return_char();
					}
					continue;

				// Find : and ::
				case ':':
					if (file.peek() == ':') {
						advance_char();
						token_list.emplace_back(DOUBLE_COLON, curline, curcolumn);
					}
					else {
						token_list.emplace_back(COLON, curline, curcolumn);
					}
					continue;

				// Find . and ...
				case '.':
					if (file.get() == '.' && file.peek() == '.') {
						token_list.emplace_back(TRIPLE_DOT, curline, curcolumn);
						file.unget();
						advance_char();
						advance_char();
					}
					else {
						file.unget();
						token_list.emplace_back(DOT, curline, curcolumn);
					}
					continue;

				// Find {}
				case '{':
					token_list.emplace_back(LEFT_BRACKET, curline, curcolumn);
					continue;
				case '}':
					token_list.emplace_back(RIGHT_BRACKET, curline, curcolumn);
					continue;

				// Find []
				case '[':
					token_list.emplace_back(LEFT_SQUARE, curline, curcolumn);
					continue;
				case ']':
					token_list.emplace_back(RIGHT_SQUARE, curline, curcolumn);
					continue;

				// Find ()
				case '(':
					token_list.emplace_back(LEFT_PAREN, curline, curcolumn);
					continue;
				case ')':
					token_list.emplace_back(RIGHT_PAREN, curline, curcolumn);
					continue;

				// Find ,
				case ',':
					token_list.emplace_back(COMMA, curline, curcolumn);
					continue;

				// Find <>, <, and >
				case '<':
					if (file.peek() == '>') {
						token_identifier_cache.push_back("<>");
						token_list.emplace_back(IDENTIFIER, curline, curcolumn,
						                        token_identifier_cache.size() - 1);
						advance_char();
					}
					else {
						token_list.emplace_back(LEFT_CARROT, curline, curcolumn);
					}
					continue;
				case '>':
					token_list.emplace_back(RIGHT_CARROT, curline, curcolumn);
					continue;

				// Find +
				case '+':
					token_list.emplace_back(PLUS, curline, curcolumn);
					continue;

				// Find ;
				case ';':
					token_list.emplace_back(SEMI_COLON, curline, curcolumn);
					continue;

				// Deal with character literals
				case '\'': {
					char c = '\0';
					advance_char();
					if (curchar == '\\') {
						advance_char();
						c = escaper(curchar);
					}
					else {
						c = curchar;
					}
					token_list.emplace_back(LIT_CHAR, curline, curcolumn, c);
				}

				// Deal with string literals, both "blah" and """blah"""
				case '"': {
					bool single      = true;
					size_t startline = curline;
					size_t startcol  = curcolumn;
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
					token_list.emplace_back(LIT_STRING, startline, startcol,
					                        token_identifier_cache.size() - 1);
					continue;
				}
			}
		}
		else {
			skipswitch = false;
		}

		if (!skipnumber) {
			if (is_numeric(curchar)) {
				size_t startcol = curcolumn;
				bool negitive   = false;
				if (curchar == '-') {
					if (is_number(file.peek())) {
						negitive = true;
					}
					else {
						skipnumber = true;
						return_char();
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
					token_list.emplace_back(LIT_FLOAT, curline, startcol,
					                        token_float_cache.size() - 1);
				}
				else {
					if (negitive) {
						int_number *= -1;
					}
					token_int_cache.push_back(int_number);
					token_list.emplace_back(LIT_INT, curline, startcol, token_int_cache.size() - 1);
				}
				continue;
			}
		}
		else {
			skipnumber = false;
		}

		if (is_letter(curchar)) {
			size_t startcol = curcolumn;
			std::string str;
			str += curchar;
			while (!char_is_in(" \t\n{}[]()-+,.:;", file.peek()) && advance_char()) {
				str += curchar;
			}
			// Look up string in the map, and if it can find it, set the token appropriately.
			TokenType_t tt;
			auto itt = keyword_map.find(str);
			if (itt != keyword_map.end()) {
				tt = (*itt).second;
			}
			else {
				tt = IDENTIFIER;
			}
			if (tt == IDENTIFIER) {
				token_identifier_cache.push_back(std::move(str));
				token_list.emplace_back(IDENTIFIER, curline, startcol,
				                        token_identifier_cache.size() - 1);
			}
			else {
				token_list.emplace_back(tt, curline, startcol);
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
void nip::compiler::token_printer(std::vector<nip::Token_t>& tklist, std::ostream& out) {
	size_t length = tklist.size();
	if (length == 0) {
		return;
	}
	size_t token_num_digits = std::ceil(std::log10(length));
	size_t line_num_digits  = std::ceil(std::log10(tklist.back().linenum));
	size_t char_num_digits  = std::ceil(
	    std::log10((*std::max_element(tklist.begin(), tklist.end(), [](auto left, auto right) {
		               return (left.charnum < right.charnum);
		           })).charnum));

	size_t i = 1;
	for (auto t : tklist) {
		out << "Line: " << std::setfill('0') << std::setw(line_num_digits) << t.linenum << " | ";
		out << "Char: " << std::setfill('0') << std::setw(char_num_digits) << t.charnum << " | ";
		out << std::setfill('0') << std::setw(token_num_digits) << i++ << ": ";
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
			case SEMI_COLON:
				out << "SEMI_COLON"
				    << " | ;";
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
			case LIT_CHAR:
				out << "LIT_CHAR"
				    << " | " << special_sanitize(static_cast<char>(t.address));
				break;
			case LIT_STRING:
				out << "LIT_STRING"
				    << " | " << special_sanitize(token_identifier_cache[t.address]);
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
			case KEY_PERMIT:
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
