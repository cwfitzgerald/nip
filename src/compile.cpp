#include "nip.hpp"
#include "util.hpp"

#include <chrono>
#include <iostream>
#include <tuple>

void nip::compiler::compile() {
	std::vector<nip::Token_t> tokens;
	std::chrono::nanoseconds time;
	std::tie(tokens, time) = nip::util::bench_func([&] { return tokenizer(); });
	*opt.error_stream << "Time to tokenize = " << nip::util::print_time(time) << '\n';

	token_printer(tokens, *opt.output_stream);

	time = nip::util::bench_func_void([&] { return parser.parse(tokens, token_caches); });
	*opt.error_stream << "Time to parse    = " << nip::util::print_time(time) << '\n';

	parser.print_metadata_functor_info();
}
