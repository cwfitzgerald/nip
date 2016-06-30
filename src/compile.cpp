#include "nip.hpp"

#include <chrono>

void nip::compiler::compile() {
	auto start  = std::chrono::high_resolution_clock::now();
	auto tokens = tokenizer();
	auto end    = std::chrono::high_resolution_clock::now() - start;
	*opt.error_stream << "Time to tokenize = " << end.count() / 1000 << "μs\n";
	token_printer(tokens, *opt.output_stream);
	errhdlr.print_errors(*opt.error_stream);
}
