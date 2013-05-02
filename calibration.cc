// calibration
// Calibrate your terminal for use with Chat-Art tools.

// Copyright(c) 2013 by Christopher Abad | 20 GOTO 10

// email: aempirei@gmail.com aempirei@256.bz
// http://www.256.bz/ http://www.twentygoto10.com/
// git: git@github.com:aempirei/Chat-Art.git
// aim: ambientempire

#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <algorithm>
#include <ios>
#include <map>
#include <deque>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <list>
#include <set>
#include <math.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

typedef std::list<int> ints;

namespace ansi {

	std::string start("\033[");

	std::string esc(const ints& args, char mode) {

		std::stringstream ss;
		ints::const_iterator iter = args.begin();

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

	const std::string bold = esc(1, 'm');
	const std::string clear = esc(0, 'm');

	enum color_index { black, red, green, yellow, blue, magenta, cyan, white, first_color = black, last_color = white };
}

void help(const char *prog) {
	std::cerr << "\nusage: " << prog << " [options]\n\n";
	std::cerr << "either display the calibration pattern to save a snapshot of, or process a snapshot.\n";
	std::cerr << "after displaying the calibration in your terminal, save a snapshot of the pattern.\n"; 
	std::cerr << "this can usually be done via the PRINT SCREEN button. afterward, convert it to the\n";
	std::cerr << "PNM file format.  netpbm, djpeg or imagemagick can usually do this. finally, pipe\n";
	std::cerr << "the PNM file into this program again using the '-c' option\n\n";
	std::cerr << "\t-d   display the calibration pattern to stdout\n";
	std::cerr << "\t-p   process calibration pattern image snapshot from stdin\n";
	std::cerr << "\t-h   show this help\n\n";
	std::cerr << "report bugs to <aempirei@gmail.com>\n\n";
}

std::string pad(int n) {
	std::stringstream ss;
	ss << ansi::clear << ansi::bg(ansi::black) << std::setw(n) << ' ' << std::setw(0) << ansi::clear;
	return ss.str();
}
std::string edge = pad(2);

ints prefix_sequence = {5,4,1,7,4,5,5};
ints suffix_sequence = {5,1,2,4,3};

void display_code_sequence(const ints& preamble) {
	
	size_t pattern_sz = 10;

	for(ints::const_iterator iter = preamble.begin(); iter != preamble.end(); iter++)
		pattern_sz += *iter;

	std::cout << pad(pattern_sz) << std::endl;

	std::cout << edge << ansi::bg(ansi::white) << std::setw(1) << ' ';

	for(ints::const_iterator iter = preamble.begin(); iter != preamble.end(); iter++) {
		std::cout << ansi::bg(ansi::black) << std::setw(*iter) << ' ';
		std::cout << ansi::bg(ansi::white) << std::setw(1) << ' ';
	}
	std::cout << edge << ansi::clear << std::endl;

	std::cout << pad(pattern_sz) << ansi::clear << std::endl;
}

void display_calibration() {

	///////////
	// preamble

	display_code_sequence(prefix_sequence);

	///////
	// font

	std::cout << edge << ansi::bg(ansi::black) << "ABCDEFGHIJKLMNOPQRSTUVWXYZ" << edge << std::endl;
	std::cout << edge << ansi::bg(ansi::black) << "abcdefghijklmnopqrstuvwxyz" << edge << std::endl;
	std::cout << edge << ansi::bg(ansi::black) << "0123456789" << edge << std::endl;

	////////////////////
	// background colors

	std::cout << edge;

	for(int i = ansi::first_color; i <= ansi::last_color; i++)
		std::cout << ansi::bg(i) << ' ';

	std::cout << edge << std::endl;

	////////////////////
	// foreground colors

	std::cout << edge << ansi::bg(ansi::black);

	for(int i = ansi::first_color; i <= ansi::last_color; i++)
		std::cout << ansi::fg(i) << 'X';

	std::cout << edge << std::endl;

	/////////////////////////
	// bold foreground colors

	std::cout << edge << ansi::bg(ansi::black);

	for(int i = ansi::first_color; i <= ansi::last_color; i++)
		std::cout << ansi::bold << ansi::fg(i) << 'X';

	std::cout << edge << std::endl;

	///////////
	// preamble

	display_code_sequence(suffix_sequence);

}

void process_calibration() {
	// read pnm file
	// find preamble
	// find registration
	// find colors
	// find fonts
	// calibrate
	// output calibration configuration
}

int main(int argc, char **argv) {

	int display = 0;
	int process = 0;
	int opt;

	while ((opt = getopt(argc, argv, "dph")) != -1) {
		switch (opt) {
			case 'd': display = 1; break;
			case 'p': process = 1; break;
			case 'h':
			default:
				  help(basename(*argv));
				  exit(EXIT_SUCCESS);
		}
	}

	if(display == process) {
		std::cerr << '\n' << ansi::bold;
		std::cerr << "error: please provide exactly one mode of operation, either -d or -p";
		std::cerr << ansi::clear << '\n';
		help(basename(*argv));
		exit(EXIT_SUCCESS);
	}

	if(display)
		display_calibration();

	if(process)
		process_calibration();


	exit(EXIT_SUCCESS);
}
