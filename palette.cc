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
	// std::cerr << "example: djpeg screenshot.jpg | calibration -p > conf.d/ubuntu-mono-12.conf\n\n";
	std::cerr << "report bugs to <aempirei@256.bz>\n\n";
}

int main(int argc, char **argv) {

	char line[128];

	if(argc == 2 && strcmp(argv[1], "-h") == 0) {
		help(*argv);
		return -1;
	}

	while(fgets(line, sizeof(line) - 1, stdin) != NULL) {
		printf("get line length of %d: %s", (int)strlen(line), line);
	}

	exit(EXIT_SUCCESS);
}
