#pragma once

#include "../utilmacro.hpp"

#include <iosfwd>
#include <string>
#include <vector>

namespace nip {
	namespace error {
		enum _Error_Type : uint8_t { NOTE, WARNING, ERROR, FATAL_ERROR };

		struct _Error {
			_Error(_Error_Type type_i, const char* msg_i, size_t ll = 0, size_t lc = 0,
			       bool hl = false)
			    : msg(msg_i), loc_line(ll), loc_char(lc), has_location(hl), type(type_i) {}
			std::string msg;
			size_t loc_line;
			size_t loc_char;
			bool has_location;
			_Error_Type type;
		};

		class Error_Handler {
		  public:
			ALWAYS_INLINE void add_error(_Error_Type type, std::string& msg, size_t ll = 0,
			                             size_t lc = 0, bool hl = false);
			ALWAYS_INLINE void add_error(_Error_Type type, const char* msg, size_t ll = 0,
			                             size_t lc = 0, bool hl = false);
			void print_errors(std::ostream&);

			Error_Handler(){};
			Error_Handler(std::vector<std::string>&& source) : source_file(source){};
			void set_source(std::vector<std::string>&& source) {
				source_file = source;
			};

		  private:
			void sort();

			std::vector<_Error> error_list;
			std::vector<std::string> source_file;

			bool has_note    = false;
			bool has_warning = false;
			bool has_error   = false;
			bool has_fatal   = false;
			bool sorted      = false;
		};
	}
}

ALWAYS_INLINE void nip::error::Error_Handler::add_error(_Error_Type type, std::string& msg,
                                                        size_t ll, size_t lc, bool hl) {
	add_error(type, msg.data(), ll, lc, hl);
}

ALWAYS_INLINE void nip::error::Error_Handler::add_error(_Error_Type type, const char* msg,
                                                        size_t ll, size_t lc, bool hl) {
	error_list.emplace_back(type, msg, ll, lc, hl);
	switch (type) {
		case NOTE:
			has_note = true;
			break;
		case WARNING:
			has_warning = true;
			break;
		case ERROR:
			has_error = true;
			break;
		case FATAL_ERROR:
			has_fatal = true;
			break;
	}
}
