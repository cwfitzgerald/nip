#include "errorhandler.hpp"
#include "../util.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

void nip::error::Error_Handler::print_errors(std::ostream& out) {
	sort();

	for (_Error e : error_list) {
		out << nip::color::print(nip::color::FG_WHITE, nip::color::BOLD) << e.loc_line << ":"
		    << e.loc_char << ": ";
		switch (e.type) {
			case NOTE:
				out << nip::color::print(nip::color::FG_CYAN, nip::color::BOLD) << "note: ";
				break;
			case WARNING:
				out << nip::color::print(nip::color::FG_MAGENTA, nip::color::BOLD) << "warning: ";
				break;
			case ERROR:
			case FATAL_ERROR:
				out << nip::color::print(nip::color::FG_RED, nip::color::BOLD) << "error: ";
				break;
		}
		out << nip::color::print(nip::color::RESET) << e.msg << "\n";
	}
}

void nip::error::Error_Handler::sort() {
	if (!sorted) {
		std::sort(error_list.begin(), error_list.end(), [](auto left, auto right) {
			return (left.loc_line < right.loc_line ||
			        (left.loc_line == right.loc_line && left.loc_char < right.loc_char));
		});
	}
}