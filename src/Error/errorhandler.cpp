#include "errorhandler.hpp"
#include "../util.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

void nip::error::Error_Handler::print_errors(std::ostream& out) {
	sort();

	for (_Error e : error_list) {
		out << color::print(color::FG_WHITE, color::BOLD) << e.loc_line << ":" << e.loc_char
		    << ": ";
		switch (e.type) {
			case NOTE:
				out << color::print(color::FG_CYAN, color::BOLD) << "note: ";
				break;
			case WARNING:
				out << color::print(color::FG_MAGENTA, color::BOLD) << "warning: ";
				break;
			case ERROR:
			case FATAL_ERROR:
				out << color::print(color::FG_RED, color::BOLD) << "error: ";
				break;
		}
		out << color::print(color::RESET) << e.msg << "\n";
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