// pnmtoansi
// Diplay color pnmtoansi based on a given calibration file

// Copyright(c) 2013 by Christopher Abad | 20 GOTO 10

// email: aempirei@gmail.com aempirei@256.bz
// http://www.256.bz/ http://www.twentygoto10.com/
// git: git@github.com:aempirei/Chat-Art.git
// aim: ambientempire

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <climits>

#include <math.h>
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
#include <pnm.hh>
#include <rgb.hh>
#include <tile.hh>

typedef std::list<geometry> geometry_list;

bool geometry_from_string(geometry& g, std::string s) {

	int rgb[3];
	int n;
	int mode;
	int skip;

	auto pos = s.find_first_of("#\r\n");

	if(pos != std::string::npos)
		s.resize(pos);

	if(sscanf(s.c_str(), " mcb %d %d %d color %d %d %d %n",
				&mode, &g.code, &g.base,
				rgb+0, rgb+1, rgb+2,
				&skip)
			== 6)
	{

		// color modes

		g.mode = (geometry::mode_type)mode;

		for(int i = 0; i < 3; i++)
			g.color[i] = (uint8_t)rgb[i];

	} else if(sscanf(s.c_str(), " mcb %d %d %d ratio %d %d size %d %d %n",
				&mode, &g.code, &g.base,
				&g.ratio, &n,
				g.size + 1, g.size + 0,
				&skip)
			== 7)
	{

		// symbol modes

		g.mode = (geometry::mode_type)mode;

	} else if(strspn(s.c_str(), " \t") == strlen(s.c_str())) {

		// blank line

		g.mode = geometry::NOP;
		return false;

	} else {

		// invalid line

		g.mode = geometry::ERR;
		return false;
	}

	// unexpected trailing characters

	if(s[skip] != '\0') {
		g.mode = geometry::ERR;
		return false;
	  }

	// all good

	return true;
}

long ms(const std::list<long>& xs) {
	long long acc = 0;
	for(auto x : xs)
		acc += x * x;
	return acc;//(long)rint(sqrt(acc));
}

std::string ansi_match(int R, int G, int B, const geometry_list& bg, const geometry_list& fg, const geometry_list& sym) {
	char best[128];
	int min = INT_MAX;
	for(auto b : bg) {
		for(auto f : fg) {
			for(auto s : sym) {

				rgb_sum_t y;

				for(int i = 0; i < 3; i++) { 
					y[i] = b.color[i] * (s.n() - s.ratio) + f.color[i] * s.ratio;
					y[i] *= 255;
					y[i] /= 80 * s.n();
				}

				int dr = labs(R - y[0]);
				int dg = labs(G - y[1]);
				int db = labs(B - y[2]);

				long test = ms({dr,dg,db});


				if(test < min) {
					min = test;
					snprintf(best, sizeof(best),
							"%s%c",
							ansi::esc({f.base, f.code + 30, b.code + 40}, 'm').c_str(),
							s.base + s.code);
				}
			}
		}
	}
	return std::string(best);
}

void help(const char *prog) {
	std::cerr << "\nusage: " << prog << " image.pnm < filename.conf\n\n";
	std::cerr << "report bugs to <aempirei@256.bz>\n\n";
}

int main(int argc, char **argv) {

	char line[128];
	int line_num = 0;

	std::list<geometry> geometry_mode[4];

	std::list<geometry>& bgcolor = geometry_mode[geometry::BGCOLOR];
	std::list<geometry>& fgcolor = geometry_mode[geometry::FGCOLOR];
	std::list<geometry>& symbol = geometry_mode[geometry::SYMBOL];

	if(argc == 2 && strcmp(argv[1], "-h") == 0) {
		help(*argv);
		return -1;
	}

	if(argc != 2) {
		help(*argv);
		return -1;
	}

	while(fgets(line, sizeof(line) - 1, stdin) != NULL) {

		geometry g;

		line_num++;

		if(geometry_from_string(g, line)) {

			// good line

			if(g.mode >= geometry::MODE_MIN && g.mode <= geometry::MODE_MAX)
				geometry_mode[g.mode].push_back(g);

		} else if(g.mode == -1) {

			// bad line

			std::cerr << ansi::bold << "invalid calibration data on input line ";
			std::cerr << line_num << ansi::clear << std::endl;
			std::cerr << line;
			return -1;
		}
	}

	FILE *fp = fopen(argv[1],"r");

	if(fp == NULL) {
		perror("fopen()");
		return -1;
	}

	pnm f(fp);
	typedef std::vector<tile> tiles;

	const pos_t t0 = {0,0};
	pos_t t_sz;
	memcpy(t_sz, symbol.front().size, sizeof(t_sz));

	t_sz[0] >>= 1;
	t_sz[1] >>= 1;

	if(!f.isloaded()) {
		std::cerr << "failed to load pnm image from " << argv[1] << std::endl;
	}

	std::cout << "background colors: " << bgcolor.size() << std::endl;
	std::cout << "foreground colors: " << fgcolor.size() << std::endl;
	std::cout << "character symbols: " << symbol.size() << std::endl;
	std::cout << "max. pnmtoansi  " << t_sz[0] << ' ' << t_sz[1] << " size: " << (bgcolor.size() * fgcolor.size() * symbol.size()) << std::endl;

	int w = f.width / t_sz[0];
	int h = f.height / t_sz[1];

	for(int y = 0; y < h; y++) {
		for(int x = 0; x < w; x++) {
			tile base(&f,t_sz,t0);
			base.move(y,x);
			std::string s( ansi_match( base.rgb_mean[0],base.rgb_mean[1],base.rgb_mean[2], bgcolor,fgcolor,symbol ));
			fputs(s.c_str(), stdout);
		}
		putchar('\n');
	}
}
