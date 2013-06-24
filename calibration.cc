// calibration
// Calibrate your terminal for use with Chat-Art tools.

// Copyright(c) 2013 by Christopher Abad | 20 GOTO 10

// email: aempirei@gmail.com aempirei@256.bz
// http://www.256.bz/ http://www.twentygoto10.com/
// git: git@github.com:aempirei/Chat-Art.git
// aim: ambientempire

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <ios>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <list>
#include <map>
#include <vector>

#include <geometry.hh>
#include <ansi.hh>
#include <tile.hh>
#include <pnm.hh>
#include <rgb.hh>

typedef std::list<int> ints;
typedef std::map<uint8_t, int> histogram;
typedef std::vector<tile> tiles;

struct config {
	tiles bg_colors;
	tiles fg_colors;
	tiles symbols;
};

typedef uint32_t code_t;

static const code_t prefix_code = 0xA55FACED;
static const code_t suffix_code = 0x1DEADFED;

static const int code_sz = sizeof(code_t) * 8;

static const int calibration_lines = 8;

#define BIT_ISSET(n,b)	(((n) & (1 << (b))) != 0)

bool try_code(const pnm& snapshot, code_t code, int x0, int y, int dx) {

	const rgb_t *c[2] = { NULL, NULL };

	for(int column = 0; column < code_sz; column++) {

		size_t x = x0 + column * dx;

		if(x >= snapshot.width)
			return false;

		const rgb_t *pixel = snapshot.pixel(y, x);

		int bit = BIT_ISSET(code, column);

		if(c[bit] == NULL) {

			c[bit] = pixel;

			if(c[0] != NULL && c[1] != NULL && rgb::equals(*c[0], *c[1]))
				return false;

		} else if(!rgb::equals(*pixel, *c[bit])) { 
			return false;
		}
	}

	return true;
}

bool find_code(const pnm& snapshot, int y0, code_t code, pos_t pos, int *tile_width) {

	size_t max_tile_width = snapshot.width / code_sz;

	for(int dx = max_tile_width; dx > 0; dx--) {

		size_t tile_columns = snapshot.width / dx;
		size_t x_offset = snapshot.width % dx;

		for(size_t column = 0; column + code_sz - 1 < tile_columns; column++) {

			for(size_t y = y0; y < snapshot.height; y++) {

				int x0 = column * dx + x_offset;

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

//////////
// PROCESS
//////////

void assign_tiles(tiles& ts, int line, geometry::mode_type mode, int base) {

	for(size_t column = 0; column < ts.size(); column++) {

		const auto& code = column;
		tile& t = ts[column];

		t.move(line, column);
		t.set_status(mode, code, base);
	}
}

void assign_symbolic_tiles(config& cfg, std::list<tiles> tiles_list) {

	histogram seen;

	for(auto ts : tiles_list)
		for(auto t : ts)
			if(!seen[t.geo.ratio]++)
				cfg.symbols.push_back(t);
}

void print_tiles(const tiles& ts, const char *title) {
	printf("#\n# %s\n#\n", title);
	for(auto t : ts) 
		printf("%s\n", t.geo.to_string().c_str());
}

void process_tiles(config& cfg, const pnm& snapshot, const pos_t size, const pos_t origin) {

	tile base(&snapshot, size, origin);

	tiles     solid_tiles( 8, base);
	tiles uppercase_tiles(26, base);
	tiles lowercase_tiles(26, base);
	tiles    number_tiles(10, base);

	cfg.bg_colors.resize(8, base);
	cfg.fg_colors.resize(8, base);

	assign_tiles(    solid_tiles, 1, geometry::SOLID,   0);

	assign_tiles(  cfg.bg_colors, 2, geometry::BGCOLOR, 0);
	assign_tiles(  cfg.fg_colors, 3, geometry::FGCOLOR, 1);

	assign_tiles(uppercase_tiles, 4, geometry::SYMBOL, 'A');
	assign_tiles(lowercase_tiles, 5, geometry::SYMBOL, 'a');
	assign_tiles(   number_tiles, 6, geometry::SYMBOL, '0');

	for(tile t : cfg.bg_colors) {
		t.geo.mode = geometry::FGCOLOR;
		cfg.fg_colors.push_back(t);
	}

	tile space_tile = solid_tiles.front();

	space_tile.set_status(geometry::SYMBOL, 0, ' ');

	assign_symbolic_tiles(cfg, {uppercase_tiles, lowercase_tiles, number_tiles, tiles(1, space_tile)});

	print_tiles(cfg.bg_colors, "background colors");
	print_tiles(cfg.fg_colors, "foreground colors");
	print_tiles(cfg.symbols  , "character symbols");
}

void process_calibration() {

	pnm snapshot(stdin);
	config cfg;

	if(snapshot.isloaded()) {

		pos_t prefix_pos;
		pos_t suffix_pos;

		int prefix_tile_width;
		int suffix_tile_width;

		if(	find_code(snapshot, 0, prefix_code, prefix_pos, &prefix_tile_width) &&
				find_code(snapshot, prefix_pos[1] + 1, suffix_code, suffix_pos, &suffix_tile_width))
		{

			int hn = suffix_pos[1] - prefix_pos[1];

			if(	(prefix_tile_width == suffix_tile_width) &&
					(prefix_pos[0] == suffix_pos[0]) &&
					(hn % (calibration_lines - 1) == 0))
			{

				pos_t tile_sz = { prefix_tile_width, hn / (calibration_lines - 1) };

				process_tiles(cfg, snapshot, tile_sz, prefix_pos);
			}
		}
	}
}

//////////
// DISPLAY
//////////

void display_code(code_t code) {
	for(int bit_n = 0; bit_n < code_sz; bit_n++) {
		std::string bgcolor = ansi::bg(BIT_ISSET(code, bit_n) ? ansi::white : ansi::black);
		std::cout << bgcolor << ' ';
	}
	std::cout << ansi::clear << std::endl;
}


void display_calibration() {

	/////////////////////////////////////////////
	// calibration text is 8 lines and 32 columns
	/////////////////////////////////////////////
	// 0.32 prefix code
	// 1.08 background colors
	// 2.08 foreground colors
	// 3.08 bold foreground colors
	// 4.26 uppercase alphabet
	// 5.26 lowercase alphabet
	// 6.10 numbers
	// 7.32 suffix code

	//////////////
	// prefix code

	display_code(prefix_code);

	////////////////////
	// background colors

	for(int i = ansi::first_color; i <= ansi::last_color; i++)
		std::cout << ansi::bg(i) << ' ';

	std::cout << std::endl;

	////////////////////
	// foreground colors

	std::cout << ansi::bg(ansi::black);

	for(int i = ansi::first_color; i <= ansi::last_color; i++)
		std::cout << ansi::fg(i) << '#';

	std::cout << std::endl;

	/////////////////////////
	// bold foreground colors

	std::cout << ansi::bg(ansi::black);

	for(int i = ansi::first_color; i <= ansi::last_color; i++)
		std::cout << ansi::bold << ansi::fg(i) << '#';

	std::cout << std::endl;

	///////
	// font

	std::cout << ansi::fg(ansi::white) << ansi::bg(ansi::black);

	std::cout << "ABCDEFGHIJKLMNOPQRSTUVWXYZ" << std::endl;
	std::cout << "abcdefghijklmnopqrstuvwxyz" << std::endl;
	std::cout << "0123456789" << std::endl;

	//////////////
	// suffix code

	display_code(suffix_code);

}

void help(const char *prog) {
	std::cerr << "\nusage: " << prog << " [options]\n\n";
	std::cerr << "either display the calibration pattern to save a snapshot of, or process a snapshot.\n";
	std::cerr << "after displaying the calibration in your terminal, save a snapshot of the pattern.\n"; 
	std::cerr << "this can usually be done via the PRINT SCREEN button. afterward, convert it to the\n";
	std::cerr << "PNM file format.  netpbm, djpeg or imagemagick can usually do this. finally, pipe\n";
	std::cerr << "the PNM file into this program again using the '-p' option and then redirect output\n";
	std::cerr << "to a configuration file.\n\n";
	std::cerr << "\t-d   display the calibration pattern to stdout\n";
	std::cerr << "\t-p   process calibration pattern image snapshot from stdin\n";
	std::cerr << "\t-h   show this help\n\n";
	std::cerr << "example: djpeg screenshot.jpg | " << prog << " -p > conf.d/ubuntu-mono-12.conf\n\n";
	std::cerr << "report bugs to <aempirei@256.bz>\n\n";
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
