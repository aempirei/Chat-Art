#ifndef GEOMETRY_HH
#define GEOMETRY_HH

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <string>

#include "ansi.hh"

typedef uint8_t rgb_t[3];
typedef size_t pos_t[2];

struct geometry {

	constexpr static int ratio_shift = 2;
	constexpr static int ratio_max = ((1 << (8-ratio_shift)) - 1);

	enum mode_type { SOLID = 0, BGCOLOR = 1, FGCOLOR = 2, SYMBOL = 3 };

	pos_t size;

	rgb_t color;

	int ratio;

	mode_type mode;

	int code;
	int base;

	std::string to_string() const;

	void from_string(std::string& str);
};

std::string geometry::to_string() const {

	char buf[128];
	char modebuf[128];

	switch(mode) {

		case geometry::BGCOLOR: 
		case geometry::FGCOLOR: 

			snprintf(modebuf, sizeof(modebuf), " color %3d %3d %3d   # %s%s", color[0], color[1], color[2],
					base ? "bold " : "", ansi::color_name[code]
					);
			break;

		case geometry::SYMBOL:

			snprintf(modebuf, sizeof(modebuf), " ratio %2d %2d size %d %d   # (%c)",
					ratio, ratio_max - ratio,
					(int)size[1], (int)size[0], base + code
				);
			break;

		case geometry::SOLID:
		default:
			*modebuf = '\0';
	}



	snprintf(buf, sizeof(buf), "mcb %2d %2d %2d%s", mode, code, base, modebuf);

	return std::string(buf);

}
#endif
