// palette
// Diplay color palette based on a given calibration file

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
#include <rgb.hh>

void help(const char *prog) {
	std::cerr << "\nusage: " << prog << " < filename.conf\n\n";
	std::cerr << "report bugs to <aempirei@256.bz>\n\n";
}

int main(int argc, char **argv) {

	char line[128];

	std::list<geometry> geometry_mode[4];
	std::list<geometry>& bgcolor = geometry_mode[geometry::BGCOLOR];
	std::list<geometry>& fgcolor = geometry_mode[geometry::FGCOLOR];
	std::list<geometry>& symbol = geometry_mode[geometry::SYMBOL];

	if(argc == 2 && strcmp(argv[1], "-h") == 0) {
		help(*argv);
		return -1;
	}

	while(fgets(line, sizeof(line) - 1, stdin) != NULL) {

		geometry g;

		int rgb[3];
		int n;
		int mode;

		char *p = strpbrk(line, "\r\n#");

		if(p != NULL)
			*p = '\0';

		if(sscanf(line, " mcb %d %d %d color %d %d %d ", &mode, &g.code, &g.base, rgb+0, rgb+1, rgb+2) == 6) {

			g.mode = (geometry::mode_type)mode;

			for(int i = 0; i < 3; i++)
				g.color[i] = (uint8_t)rgb[i];

		} else if(sscanf(line, " mcb %d %d %d ratio %d %d size %d %d ",
					&mode, &g.code, &g.base, &g.ratio, &n, g.size + 1, g.size + 0) == 7) {

			g.mode = (geometry::mode_type)mode;

		} else if(strspn(line, "\t ") == strlen(line)) {

			// blank line, just continue
			continue;

		} else {

			std::cerr << "bad configuration line: " << line << std::endl;
			return -1;
		}

		if(g.mode >= geometry::MODE_MIN && g.mode <= geometry::MODE_MAX)
			geometry_mode[g.mode].push_back(g);
	}

	std::cout << "background colors: " << bgcolor.size() << std::endl;
	std::cout << "foreground colors: " << fgcolor.size() << std::endl;
	std::cout << "character symbols: " << symbol.size() << std::endl;

	for(int gm = geometry::MODE_MIN; gm <= geometry::MODE_MAX; gm++) {
		std::cout << "[ mode " << gm << " ]" << std::endl;
		for(auto g : geometry_mode[gm]) {
			std::cout << g.to_string() << std::endl;
		}
	}
}
