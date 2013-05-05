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

typedef uint8_t rgb_t[3];

class pnm {
	public:
		pnm(FILE *fp);
		~pnm();
		bool isloaded();
		size_t width = 0;
		size_t height = 0;
		rgb_t *data = NULL;
		rgb_t *pixel(size_t row, size_t col);
};

pnm::pnm(FILE *fp) {

	const unsigned int expected_mv = 255;

	unsigned int w;
	unsigned int h;
	unsigned int mv;

	if(fscanf(fp, "P6 %u %u %u", &w, &h, &mv) == 3) {

		int ch = fgetc(fp);

		if(isspace(ch) && mv == expected_mv) {

			data = new rgb_t[w * h];

			if(fread(data, sizeof(rgb_t), w * h, fp) == w * h) {
				width = w;
				height = h;
			} else {
				delete data;
				data = NULL;
			}
		}
	}
}

bool pnm::isloaded() {
	return data != NULL && width > 0 && height > 0;
}

pnm::~pnm() {
	if(data != NULL)
		delete data;
}

rgb_t *pnm::pixel(size_t row, size_t col) {
	return &data[row * width + col];
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

typedef uint32_t code_t;

code_t prefix_code = 0xA55FACED;
code_t suffix_code = 0x1DEADFED;
size_t code_sz = sizeof(code_t) * 8;

#define BIT_ISSET(n,b)	(((n) & (1 << (b))) != 0)

void display_code(code_t code) {
	for(size_t bit = 0; bit < code_sz; bit++) {
		std::string bgcolor = ansi::bg(BIT_ISSET(code, bit) ? ansi::white : ansi::black);
		std::cout << bgcolor << ' ';
	}
	std::cout << ansi::clear << std::endl;
}

#undef BIT_ISSET

void display_calibration() {

	///////////
	// preamble

	display_code(prefix_code);

	///////
	// font

	std::cout << ansi::fg(ansi::white) << ansi::bg(ansi::black) ;

	std::cout << "ABCDEFGHIJKLMNOPQRSTUVWXYZ" << std::endl;
	std::cout << "abcdefghijklmnopqrstuvwxyz" << std::endl;
	std::cout << "0123456789" << std::endl;

	////////////////////
	// background colors

	for(int i = ansi::first_color; i <= ansi::last_color; i++)
		std::cout << ansi::bg(i) << ' ';

	std::cout << std::endl;

	////////////////////
	// foreground colors

	std::cout << ansi::bg(ansi::black);

	for(int i = ansi::first_color; i <= ansi::last_color; i++)
		std::cout << ansi::fg(i) << 'X';

	std::cout << std::endl;

	/////////////////////////
	// bold foreground colors

	std::cout << ansi::bg(ansi::black);

	for(int i = ansi::first_color; i <= ansi::last_color; i++)
		std::cout << ansi::bold << ansi::fg(i) << 'X';

	std::cout << std::endl;

	///////////
	// preamble

	display_code(suffix_code);

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
