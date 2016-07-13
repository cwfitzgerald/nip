#pragma once

#include "utilmacro.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

namespace nip {
	namespace util {
		template <class Func, class... Args>
		ALWAYS_INLINE auto bench_func(Func f, Args&&... a) {
			auto start = std::chrono::high_resolution_clock::now();
			auto x     = f(std::forward<Args>(a)...);
			auto end   = std::chrono::high_resolution_clock::now();
			return std::make_pair(
			    x, std::chrono::duration_cast<std::chrono::nanoseconds>(end - start));
		}

		template <class Func, class... Args>
		ALWAYS_INLINE auto bench_func_void(Func f, Args&&... a) {
			auto start = std::chrono::high_resolution_clock::now();
			f(std::forward<Args>(a)...);
			auto end = std::chrono::high_resolution_clock::now();
			return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
		}

		ALWAYS_INLINE std::string print_time(std::chrono::nanoseconds& dur) {
			double ns = dur.count();
			std::ostringstream ss;
			if (ns < 1'000) {
				ss << std::fixed << std::setprecision(0) << ns << " ns";
			}
			else if (ns < 1'000'000) {
				ss << std::fixed << std::setprecision(2) << ns / 1'000 << " μs";
			}
			else if (ns < 1'000'000'000) {
				ss << std::fixed << std::setprecision(3) << ns / 1'000'000 << " ms";
			}
			else {
				ss << std::fixed << std::setprecision(3) << ns / 1'000'000'000 << " sec";
			}
			return ss.str();
		}

		ALWAYS_INLINE std::string special_sanitize(char in) {
			using namespace std::string_literals;

			switch (in) {
				case '\a':
					return "\\a"s;
				case '\b':
					return "\\b"s;
				case '\f':
					return "\\f"s;
				case '\n':
					return "\\n"s;
				case '\r':
					return "\\r"s;
				case '\t':
					return "\\t"s;
				case '\v':
					return "\\v"s;
				case '\0':
					return "\\0"s;
				default:
					return std::string("") + in;
			}
		}

		ALWAYS_INLINE std::string special_sanitize(std::string& in) {
			std::ostringstream newstr;
			for (auto i : in) {
				newstr << special_sanitize(i);
			}
			return newstr.str();
		}
	}

	namespace color {
		enum Color_t {
			RESET,
			FG_BLACK,
			FG_RED,
			FG_GREEN,
			FG_YELLOW,
			FG_BLUE,
			FG_MAGENTA,
			FG_CYAN,
			FG_WHITE,
			FG_RESET,
			BG_BLACK,
			BG_RED,
			BG_GREEN,
			BG_YELLOW,
			BG_BLUE,
			BG_MAGENTA,
			BG_CYAN,
			BG_WHITE,
			BG_RESET
		};

		enum bold_t : bool { PLAIN = false, BOLD = true };

		ALWAYS_INLINE std::string print(Color_t c, bold_t bold = PLAIN) {
			std::string color("\033[");
			switch (c) {
				case RESET:
					color += '0';
					break;
				case FG_BLACK:
					color += '3';
					color += '0';
					break;
				case FG_RED:
					color += '3';
					color += '1';
					break;
				case FG_GREEN:
					color += '3';
					color += '2';
					break;
				case FG_YELLOW:
					color += '3';
					color += '3';
					break;
				case FG_BLUE:
					color += '3';
					color += '4';
					break;
				case FG_MAGENTA:
					color += '3';
					color += '5';
					break;
				case FG_CYAN:
					color += '3';
					color += '6';
					break;
				case FG_WHITE:
					color += '3';
					color += '7';
					break;
				case FG_RESET:
					color += '3';
					color += '9';
					break;
				case BG_BLACK:
					color += '4';
					color += '0';
					break;
				case BG_RED:
					color += '4';
					color += '1';
					break;
				case BG_GREEN:
					color += '4';
					color += '2';
					break;
				case BG_YELLOW:
					color += '4';
					color += '3';
					break;
				case BG_BLUE:
					color += '4';
					color += '4';
					break;
				case BG_MAGENTA:
					color += '4';
					color += '5';
					break;
				case BG_CYAN:
					color += '4';
					color += '6';
					break;
				case BG_WHITE:
					color += '4';
					color += '7';
					break;
				case BG_RESET:
					color += '4';
					color += '9';
					break;
			}
			if (bold) {
				color += ';';
				color += '1';
			}
			color += 'm';

			return color;
		};
	};
}