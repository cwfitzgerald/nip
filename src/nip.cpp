#include "nip.hpp"
#include "options.hpp"

#include <fstream>
#include <iostream>

#include <chrono>

nip::Options opt;

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cerr << "Invalid amount of arguments.\n";
		return 1;
	}

	std::ifstream in_file(argv[1]);
	if (!in_file) {
		std::cerr << "Unable to open file " << argv[1] << ".\n";
		return 1;
	}

	opt.program_stream = &in_file;

	auto start  = std::chrono::high_resolution_clock::now();
	auto tokens = nip::tokenizer(*opt.program_stream);
	auto end    = std::chrono::high_resolution_clock::now() - start;
	std::cout << "Time to tokenize = " << end.count() / 1000 << "μs\n";
	token_printer(tokens, std::cout);
}