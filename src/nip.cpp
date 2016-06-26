#include "nip.hpp"
#include "options.hpp"

#include <fstream>
#include <iostream>

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

	nip::Options opt;

	opt.program_stream = &in_file;
	opt.output_stream  = &std::cout;
	opt.error_stream   = &std::cerr;

	nip::compiler comp(opt);
	comp.compile();
}
