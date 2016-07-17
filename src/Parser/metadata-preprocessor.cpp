#include "parser.hpp"

#include <algorithm>
#include <functional>
#include <string>

// This part of the parser processes all the about sections, allowing the actual
// parser to know if a function call is pre or postfix. This also grabs and stores
// help information on the functions for later use.
void nip::parse::Parser::metadata_preprocessor() {
	metadata_scan_abouts(0);
	current = start; // Reset the current iterator back to normal
	// metadata_scan_functors( 0);
	current = start; // Reset the current iterator back to normal
}

template <class Callback_t, class... Args>
void nip::parse::Parser::metadata_vocab(Callback_t callback, Args&&... a) {
	expect(IDENTIFIER, "expected identifier");
	auto& tmp_name = token_caches.identifier[last_token_data];
	if (accept(COLON, LEFT_BRACKET)) {
		current_qualified_name.emplace_back(tmp_name);
		accept(NEWLINE);
		expect(INDENT, "expected indent");
		callback(std::forward<Args>(a)...);
		current_qualified_name.pop_back();
	}
}

Functor_Pre_Info_t& nip::parse::Parser::metadata_get_functor_info(std::vector<std::string>& name,
                                                                  bool about) {
	auto found_element =
	    std::find_if(functor_pre_info.begin(), functor_pre_info.end(), [&name](auto a) -> bool {
		    if (a.name.size() == name.size()) {
			    for (size_t i = 0; i < a.name.size(); i++) {
				    if (a.name[i] != name[i]) {
					    return false;
				    }
			    }
			    return true;
		    }
		    return false;
		});

	if (found_element == functor_pre_info.end()) {
		functor_pre_info.emplace_back();
		functor_pre_info.back().name = name;
		return functor_pre_info.back();
	}
	if (about) {
		return (*found_element);
	}
	else {
		if (found_element->declared) {
			error("function redeclairation");
			return (*found_element);
		}
		else {
			return (*found_element);
		}
	}
}

std::pair<size_t, size_t> nip::parse::Parser::metadata_parse_functor_args() {
	size_t before    = 0;
	size_t after     = 0;
	size_t arg_count = 0;
	while (!accept(RIGHT_PAREN)) {
		if (is(NUL)) {
			error("Unexpected end of file");
		}
		if (!is(ARROW) && arg_count == 0) {
			arg_count = 1;
		}
		if (accept(LEFT_PAREN)) {
			metadata_parse_functor_args();
		}
		else if (accept(COMMA)) {
			arg_count++;
		}
		else if (accept(ARROW)) {
			before    = arg_count;
			arg_count = 0;
		}
		else {
			next_sym();
		}
	}
	after = arg_count;
	return std::make_pair(before, after);
}

size_t nip::parse::Parser::metadata_parse_trait_args() {
	size_t arg_count = 0;
	while (!accept(RIGHT_CARROT)) {
		if (arg_count == 0) {
			arg_count = 1;
		}
		if (accept(COMMA)) {
			arg_count++;
		}
		else {
			next_sym();
		}
	}
	return arg_count;
}

void nip::parse::Parser::metadata_scan_abouts(size_t start_indent) {
	size_t indent_level = start_indent;
	while (!is(NUL) && indent_level >= start_indent) {
		if (accept(KEY_VOCAB)) {
			metadata_vocab([&] { metadata_scan_abouts(indent_level + 1); });
		}
		else if (accept(KEY_ABOUT)) {
			expect(IDENTIFIER, "expected identifier");

			// Create current name
			auto name = current_qualified_name;
			name.push_back(token_caches.identifier[last_token_data]);

			Functor_Pre_Info_t& function = metadata_get_functor_info(name, true);
			expect(COLON, "expected colon");
			expect(NEWLINE, "expected newline");
			expect(INDENT, "expected indent");
			while (accept(IDENTIFIER)) {
				std::vector<std::string>& right_term =
				    function.about_pairs[token_caches.identifier[last_token_data]];
				expect(COLON, "expected colon");
				accept(NEWLINE);
				bool indented = accept(INDENT);
				while (!accept(NEWLINE)) {
					switch (cur_symbol.type) {
						case IDENTIFIER:
							right_term.emplace_back(token_caches.identifier[cur_symbol.address]);
							break;
						case LIT_FLOAT:
							right_term.emplace_back(
							    std::to_string(token_caches.floating_pt[cur_symbol.address]));
							break;
						case LIT_INT:
							right_term.emplace_back(
							    std::to_string(token_caches.integer[cur_symbol.address]));
							break;
						default:
							error("unexpected token");
							break;
					}
					next_sym();
				}
				if (indented) {
					expect(DEDENT, "expected dedent");
				}
			}
		}
		else if (accept(INDENT)) {
			indent_level++;
		}
		else if (accept(DEDENT)) {
			indent_level--;
		}
		else {
			next_sym();
		}
	}
	std::cerr << functor_pre_info.size() << '\n';
}

void nip::parse::Parser::metadata_scan_functors(size_t start_indent) {

	size_t indent_level = start_indent;
	while (!is(NUL) && indent_level >= start_indent) {
		if (accept(KEY_VOCAB)) {
			metadata_vocab([&] { metadata_scan_functors(indent_level + 1); });
		}
		else if (accept(KEY_DEFINE)) {
			expect(IDENTIFIER, "expected identifier");

			// Create current name
			// TODO: FIX
			auto name = current_qualified_name;
			name.push_back(token_caches.identifier[last_token_data]);

			// Record function information into an appropriate struct.
			Functor_Pre_Info_t& function = metadata_get_functor_info(name, false);
			expect(LEFT_PAREN, "expected left paren");
			function.argument_count = metadata_parse_functor_args();
		}
		else if (accept(KEY_TRAIT)) {
			expect(IDENTIFIER, "expected identifier");

			// Create current name
			// TODO: FIX
			auto name = current_qualified_name;
			name.push_back(token_caches.identifier[last_token_data]);

			// Record trait information into an appropriate struct.
			Functor_Pre_Info_t& function = metadata_get_functor_info(name, false);
			expect(LEFT_CARROT, "expected left carrot");
			function.trait_argument_count = metadata_parse_trait_args();
			expect(LEFT_PAREN, "expected left paren");
			function.argument_count = metadata_parse_functor_args();
		}
		else if (accept(INDENT)) {
			indent_level++;
		}
		else if (accept(DEDENT)) {
			indent_level--;
		}
		else {
			next_sym();
		}
	}

	std::cerr << functor_pre_info.size() << '\n';
}
