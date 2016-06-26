#pragma once

#include <cstddef>

namespace nip {
	enum TokenType_t {
		// Symbols //
		NUL,           // Reserved
		WHITESPACE,    // Spaces or tabs
		INDENT,        // Spaces or tabs
		DEDENT,        // Spaces or tabs
		NEWLINE,       // \n
		DOUBLE_SLASH,  // //
		LEFT_BRACKET,  // {
		RIGHT_BRACKET, // }
		LEFT_SQUARE,   // [
		RIGHT_SQUARE,  // ]
		LEFT_PAREN,    // (
		RIGHT_PAREN,   // )
		ARROW,         // ->
		PLUS,          // +
		MINUS,         // -
		COMMA,         // ,
		DOT,           // .
		TRIPLE_DOT,    // ...
		COLON,         // :
		DOUBLE_COLON,  // ::
		SEMI_COLON,    // ;
		LEFT_CARROT,   // <
		RIGHT_CARROT,  // >

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

	// If more information is in an array somewhere, the address tells the compiler where to find it
	// in the relvent table
	struct Token_t {
		Token_t() = default;
		Token_t(TokenType_t t, size_t ln = 0, size_t cn = 0, size_t add = 0)
		    : type(t), linenum(ln), charnum(cn), address(add){};
		TokenType_t type = NUL; // Says the type of the token
		size_t linenum;
		size_t charnum;
		size_t address;
	};
}
