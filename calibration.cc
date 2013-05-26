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

typedef std::list<int> ints;
typedef size_t pos_t[2];

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
// RGB
//////

typedef uint8_t rgb_t[3];
typedef uint32_t rgb_sum_t[3];
typedef std::map<uint32_t, long> histogram;

static uint32_t rgbtou32(const rgb_t v) {
	return ((uint32_t)v[0]<<16) | ((uint32_t)v[1]<<8) | (uint32_t)v[2];
}

static bool rgb_equal(const rgb_t c1, const rgb_t c2) {
	return c1[0] == c2[0] && c1[1] == c2[1] && c1[2] == c2[2];
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
	rgb_t *pixel(size_t y, size_t x) const;
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

rgb_t *pnm::pixel(size_t y, size_t x) const {
	return &data[y * width + x];
}

///////
// TILE
///////

struct tile {

	const pnm& source;

	pos_t size;
	pos_t pos;

	rgb_sum_t color = { 0 };

	tile(const pnm& source, const pos_t size, const pos_t pos);
	tile(const pnm& source, size_t line, size_t column, const pos_t size, const pos_t origin);
	tile(const pnm& source);

	const pos_t& move(ssize_t lines, ssize_t columns);

	const rgb_t& mean(rgb_t& v) const;
	const rgb_t& mode(rgb_t& v) const;
	const rgb_sum_t& sum(rgb_sum_t& v) const;

	rgb_t *pixel(size_t y, size_t x) const;
	rgb_t &at(size_t y, size_t x) const;

	size_t n() const;
	
	static const pos_t& position(pos_t& xy, size_t line, size_t column, const pos_t size, const pos_t origin);
};

tile::tile(const pnm& source) : source(source) {
}

tile::tile(const pnm& source, const pos_t size, const pos_t pos) : source(source) {
	memcpy(this->size, size, sizeof(pos_t));
	memcpy(this->pos, pos, sizeof(pos_t));
}

tile::tile(const pnm& source, size_t line, size_t column, const pos_t size, const pos_t origin) : source(source) {
	memcpy(this->size, size, sizeof(pos_t));
	tile::position(this->pos, line, column, size, origin);
}
const pos_t& tile::move(ssize_t lines, ssize_t columns) {
	pos[1] += lines * size[1];
	pos[0] += columns * size[0];
	return pos;
}

const rgb_t& tile::mean(rgb_t& v) const {
	rgb_sum_t vs;
	sum(vs);
	v[0] = (uint8_t)(vs[0] / n());
	v[1] = (uint8_t)(vs[1] / n());
	v[2] = (uint8_t)(vs[2] / n());
	return v;
}

const rgb_t& tile::mode(rgb_t& v) const {

	histogram h;

	for(size_t y = 0; y < size[1]; y++) {
		for(size_t x = 0; x < size[0]; x++) {
			uint32_t rgb32 = rgbtou32(*pixel(y,x));
			h[rgb32]++;
		}
	}

	return v;
}

const rgb_sum_t& tile::sum(rgb_sum_t& v) const {

	v[0] = v[1] = v[2] = 0;

	for(size_t y = 0; y < size[1]; y++) {
		for(size_t x = 0; x < size[0]; x++) {

			const rgb_t& w = *pixel(y,x);

			v[0] += w[0];
			v[1] += w[1];
			v[2] += w[2];
		}
	}

	return v;
}

rgb_t *tile::pixel(size_t y, size_t x) const {
	return source.pixel(pos[1] + y, pos[0] + x);
}

rgb_t &tile::at(size_t y, size_t x) const {
	return *source.pixel(pos[1] + y, pos[0] + x);
}


size_t tile::n() const {
	return size[0] * size[1];
}

const pos_t& tile::position(pos_t& xy, size_t line, size_t column, const pos_t size, const pos_t origin) {
	xy[0] = column * size[0] + origin[0];
	xy[1] = line * size[1] + origin[1];
	return xy;
}

typedef std::vector<tile> tiles;

///////
// CODE
///////

typedef uint32_t code_t;

static const code_t prefix_code = 0xA55FACED;
static const code_t suffix_code = 0x1DEADFED;

static const size_t code_sz = sizeof(code_t) * 8;

static const size_t calibration_lines = 8;

#define BIT_ISSET(n,b)	(((n) & (1 << (b))) != 0)

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

//////////
// PROCESS
//////////

void print_tile(const tile& t) {
	printf("#%02x %02x %02x / %d @ %d,%d\n", (int)t.color[0], (int)t.color[1], (int)t.color[2], (int)t.n(), (int)t.pos[0], (int)t.pos[1]);
}

const tiles& assign_tiles(tiles& tiles, size_t line) {

	for(size_t column = 0; column < tiles.size(); column++) {
		tile& t = tiles[column];
		t.move(line, column);
		t.sum(t.color);
		print_tile(t);
	}

	putchar('\n');

	return tiles;
}

void process_tiles(const pnm& snapshot, const pos_t size, const pos_t origin) {

	/////////////////////////////////////////////
	// calibration text is 8 lines and 32 columns
	/////////////////////////////////////////////
	// 1.08 background colors
	// 2.08 foreground colors
	// 3.08 bold foreground colors
	// 4.26 uppercase alphabet
	// 5.26 lowercase alphabet
	// 6.10 numbers

	tile base(snapshot, size, origin);

	tiles bg_tiles(8, base);
	tiles fg_tiles(8, base);
	tiles bold_tiles(8, base);
	tiles uppercase_tiles(26, base);
	tiles lowercase_tiles(26, base);
	tiles number_tiles(10, base);

	assign_tiles(bg_tiles, 1);
	assign_tiles(fg_tiles, 2);
	assign_tiles(bold_tiles, 3);
	assign_tiles(uppercase_tiles, 4);
	assign_tiles(lowercase_tiles, 5);
	assign_tiles(number_tiles, 6);
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

				process_tiles(snapshot, tile_sz, prefix_pos);
			}
		}
	}
}

//////////
// DISPLAY
//////////

void display_code(code_t code) {
	for(size_t bit_n = 0; bit_n < code_sz; bit_n++) {
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
		std::cout << ansi::fg(i) << 'X';

	std::cout << std::endl;

	/////////////////////////
	// bold foreground colors

	std::cout << ansi::bg(ansi::black);

	for(int i = ansi::first_color; i <= ansi::last_color; i++)
		std::cout << ansi::bold << ansi::fg(i) << 'X';

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
	std::cerr << "the PNM file into this program again using the '-c' option\n\n";
	std::cerr << "\t-d   display the calibration pattern to stdout\n";
	std::cerr << "\t-p   process calibration pattern image snapshot from stdin\n";
	std::cerr << "\t-h   show this help\n\n";
	std::cerr << "report bugs to <aempirei@gmail.com>\n\n";
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
