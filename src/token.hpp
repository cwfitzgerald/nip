#pragma once

enum TokenType {
	// Symbols //////
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
	TRIPLE_DOT     // ...
	COLON,         // :
	DOUBLE_COLON,  // ::
	LEFT_CARROT,   // <
	RIGHT_CARROT,  // >
	SINGLE_QUOTE,  // '
	DOUBLE_QUOTE,  // "
	TD_QUOTE,      // """

	// Identifiers //
	IDENTIFIER, // Custom identifier

	// Keywords
	KEY_TYPE,     // type
	KEY_INTRIN,   // intrinsic
	KEY_VOCAB,    // vocab
	KEY_DEFINE,   // define
	KEY_TRAIT,    // trait
	KEY_INSTANCE, // instance
	KEY_MATCH,    // match
	KEY_CASE,     // case
	KEY_DO,       // do
	KEY_IF,       // if
	KEY_ELSE,     // else
	KEY_DATA      // data
};