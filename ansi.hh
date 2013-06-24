#ifndef ANSI_HH
#define ANSI_HH

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <ios>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <list>

namespace ansi {

	std::string esc(const std::list<int>&, char);
	std::string esc(int, char);

	const std::string start("\033[");
	const std::string bold(esc(1, 'm'));
	const std::string clear(esc(0, 'm'));

	enum color_index { black, red, green, yellow, blue, magenta, cyan, white, first_color = black, last_color = white };

	const char *color_name[] = { "black","red","green","yellow","blue","magenta","cyan","white" };

	std::string esc(const std::list<int>& args, char mode) {

		std::stringstream ss;
		auto iter = args.begin();

		ss << start;

		if(iter != args.end()) {
			ss << *iter;

			for(iter++; iter != args.end(); iter++) {
				ss << ';' << *iter;

			}
		}

		ss << mode;

		return ss.str();
	}

	std::string esc(int d, char mode) {
		std::stringstream ss;
		ss << start << d << mode;
		return ss.str();
	}

	std::string bg(int d) {
		return esc(d + 40, 'm');
	}

	std::string fg(int d) {
		return esc(d + 30, 'm');
	}

}

#endif
