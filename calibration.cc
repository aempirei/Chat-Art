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
typedef uint8_t rgb_t[3];
typedef size_t pos_t[2];
typedef uint32_t code_t;

///////
// ANSI
///////

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

//////
// PNM
//////

struct pnm {
	size_t width = 0;
	size_t height = 0;
	rgb_t *data = NULL;

	pnm(FILE *fp);
	~pnm();

	bool isloaded();
	rgb_t *pixel(size_t line, size_t column);
	const rgb_t *pixel(size_t line, size_t column) const;
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

rgb_t *pnm::pixel(size_t line, size_t column) {
	return &data[line * width + column];
}

const rgb_t *pnm::pixel(size_t line, size_t column) const {
	return &data[line * width + column];
}

///////
// TILE
///////

struct tile {
	const pnm& source;
	pos_t size;
	pos_t position;
	tile(const pnm& source, const pos_t tile_sz, const pos_t tile_pos);
};

tile::tile(const pnm& source, const pos_t tile_sz, const pos_t tile_pos) : source(source) {
	memcpy(size, tile_sz, sizeof(pos_t));
	memcpy(position, tile_pos, sizeof(pos_t));
}

typedef std::list<tile> tiles;

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

static const code_t prefix_code = 0xA55FACED;
static const code_t suffix_code = 0x1DEADFED;

static const size_t code_sz = sizeof(code_t) * 8;

#define BIT_ISSET(n,b)	(((n) & (1 << (b))) != 0)

void display_code(code_t code) {
	for(size_t bit_n = 0; bit_n < code_sz; bit_n++) {
		std::string bgcolor = ansi::bg(BIT_ISSET(code, bit_n) ? ansi::white : ansi::black);
		std::cout << bgcolor << ' ';
	}
	std::cout << ansi::clear << std::endl;
}

static const size_t calibration_lines = 8;

bool rgb_equal(const rgb_t c1, const rgb_t c2) {
	return c1[0] == c2[0] && c1[1] == c2[1] && c1[2] == c2[2];
}

bool try_code(const pnm& snapshot, code_t code, size_t x0, size_t y, size_t dx) {

	const rgb_t *c[2] = { NULL, NULL };

	for(size_t column = 0; column < code_sz; column++) {

		size_t x = x0 + column * dx;

		if(x >= snapshot.width)
			return false;

		const rgb_t *pixel = snapshot.pixel(y, x);

		int bit = BIT_ISSET(code, column);

		if(c[bit] == NULL) {

			c[bit] = pixel;

			if(c[0] != NULL && c[1] != NULL && rgb_equal(*c[0], *c[1]))
				return false;

		} else if(!rgb_equal(*pixel, *c[bit])) { 
			return false;
		}
	}

	return true;
}

bool find_code(const pnm& snapshot, size_t y0, code_t code, pos_t pos, size_t *tile_width) {

	size_t max_tile_width = snapshot.width / code_sz;

	for(size_t dx = max_tile_width; dx > 0; dx--) {

		size_t tile_columns = snapshot.width / dx;
		size_t x_offset = snapshot.width % dx;

		for(size_t column = 0; column + code_sz - 1 < tile_columns; column++) {

			for(size_t y = y0; y < snapshot.height; y++) {

				size_t x0 = column * dx + x_offset;

				if(try_code(snapshot, code, x0, y, dx)) {


					while(try_code(snapshot, code, x0 - 1, y, dx))
						x0--;

					pos[0] = x0;
					pos[1] = y;

					*tile_width = dx;

					return true;
				}
			}
		}
	}

	return false;
}

void display_calibration() {

	//
	// calibration text is 8 lines and 32 columns
	//

	///////////
	// preamble

	display_code(prefix_code);

	///////
	// font

	std::cout << ansi::fg(ansi::white) << ansi::bg(ansi::black);

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

void process_tiles(const pnm& snapshot, const pos_t tile_origin, const pos_t tile_sz) {

	tiles tiles;

	for(size_t tile_line = 0; tile_line < calibration_lines; tile_line++) {
		for(size_t tile_column = 0; tile_column < code_sz; tile_column++) {

			pos_t tile_pos = { tile_column * tile_sz[0] + tile_origin[0], tile_line * tile_sz[1] + tile_origin[1] };

			tiles.push_back(tile(snapshot, tile_sz, tile_pos));
		}
	}

	for(tiles::const_iterator iter = tiles.begin(); iter != tiles.end(); iter++) {
		// find colors
		// find fonts
		// calibrate

	}

	// output calibration configuration
}


void process_calibration() {

	pnm snapshot(stdin);

	if(snapshot.isloaded()) {

		pos_t prefix_pos;
		pos_t suffix_pos;

		size_t prefix_tile_width;
		size_t suffix_tile_width;

		if(	find_code(snapshot, 0, prefix_code, prefix_pos, &prefix_tile_width) &&
			find_code(snapshot, prefix_pos[1] + 1, suffix_code, suffix_pos, &suffix_tile_width))
		{

			size_t hn = suffix_pos[1] - prefix_pos[1];

			if(	(prefix_tile_width == suffix_tile_width) &&
				(prefix_pos[0] == suffix_pos[0]) &&
				(hn % (calibration_lines - 1) == 0))
			{

				pos_t tile_sz = { prefix_tile_width, hn / (calibration_lines - 1) };

				process_tiles(snapshot, prefix_pos, tile_sz);
			}
		}
	}
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
