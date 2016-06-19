#include "options.hpp"

#include <fstream>
#include <iostream>

NipOptions opt;

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
}