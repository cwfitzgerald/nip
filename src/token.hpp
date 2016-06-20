#pragma once

#include <stddef.h>

namespace nip {
	enum TokenType_t {
		// Symbols //
		NUL,           // Reserved
		WHITESPACE,    // Spaces or tabs
		INDENT,        // Spaces or tabs
		DEDENT,        // Spaces or tabs
		NEWLINE,       // /n
		LEFT_BRACKET,  // {
		RIGHT_BRACKET, // }
		LEFT_SQUARE,   // [
		RIGHT_SQUARE,  // ]
		LEFT_PAREN,    // (
		RIGHT_PAREN,   // )
		ARROW,         // ->
		COMMA,         // ,
		DOT,           // .
		TRIPLE_DOT,    // ...
		COLON,         // :
		DOUBLE_COLON,  // ::
		LEFT_CARROT,   // <
		RIGHT_CARROT,  // >
		SINGLE_QUOTE,  // '
		DOUBLE_QUOTE,  // "
		TD_QUOTE,      // """

		// User defined stuff //
		IDENTIFIER, // foo in foo(->):
		LIT_INT,    // 3
		LIT_FLOAT,  // 3.14159
		LIT_STRING, // "FOO BAR!"

		// Keywords //
		KEY_ABOUT,    // about
		KEY_CALL,     // call
		KEY_CASE,     // case
		KEY_DEFINE,   // define
		KEY_DO,       // do
		KEY_DOCS,     // docs
		KEY_ELIF,     // elif
		KEY_ELSE,     // else
		KEY_IF,       // if
		KEY_INSTANCE, // instance
		KEY_INTRIN,   // intrinsic
		KEY_JUMP,     // jump
		KEY_LEFT,     // left
		KEY_MATCH,    // match
		KEY_OP,       // operator
		KEY_PERM,     // permission
		KEY_RET,      // return
		KEY_RIGHT,    // right
		KEY_SYNONYM,  // synonym
		KEY_TRAIT,    // trait
		KEY_TYPE,     // type
		KEY_VOCAB,    // vocab
		KEY_WITH      // with
	};

	// If more information is in an array somewhere, location contextually tells the compiler where
	// to find it, and the address says what entry in that table it can find the extra information
	struct Token_t {
		TokenType_t type; // Says the type of the token
		size_t location;
		size_t address;
	};
}