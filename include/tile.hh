#ifndef TILE_HH
#define TILE_HH

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

#include <string>

#include <rgb.hh>
#include <geometry.hh>
#include <pnm.hh>

struct tile {

 	constexpr static int mean_shift = 2;
	constexpr static unsigned int mean_mask = ~((1U << mean_shift) - 1);

	const pnm *source;

	geometry geo;

	pos_t pos;

	rgb_sum_t rgb_mean;
	rgb_sum_t rgb_sum;
	rgb_sum_t rgb_stdev;

	tile(const pnm *source, const pos_t size, const pos_t pos);
	tile(const pnm *source, size_t line, size_t column, const pos_t size, const pos_t origin);
	tile(const pnm *source);

	const pos_t& move(ssize_t lines, ssize_t columns);

	const rgb_sum_t& stdev(rgb_sum_t& v) const;
	const rgb_sum_t& mean(rgb_sum_t& v) const;
	const rgb_sum_t& sum(rgb_sum_t& v) const;
	const rgb_sum_t& maxima(rgb_sum_t& v, const tile& mask) const;

	void set_color_lum();

	rgb_t *pixel(size_t y, size_t x) const;

	size_t n() const;

	std::string to_string() const;

	tile& set_status(geometry::mode_type my_mode, int my_code, int my_base);
	
	static const pos_t& position(pos_t& xy, size_t line, size_t column, const pos_t size, const pos_t origin);
};

tile::tile(const pnm *source) : source(source) {
}

tile::tile(const pnm *source, const pos_t size, const pos_t pos) : source(source) {
	memcpy(geo.size, size, sizeof(pos_t));
	memcpy(this->pos, pos, sizeof(pos_t));
}

tile::tile(const pnm *source, size_t line, size_t column, const pos_t size, const pos_t origin) : source(source) {
	memcpy(geo.size, size, sizeof(pos_t));
	tile::position(this->pos, line, column, size, origin);
}
tile& tile::set_status(geometry::mode_type my_mode, int my_code, int my_base) {
	geo.mode = my_mode;
	geo.code = my_code;
	geo.base = my_base;
	return *this;
}
const pos_t& tile::move(ssize_t lines, ssize_t columns) {

	pos[1] += lines * geo.size[1];
	pos[0] += columns * geo.size[0];

	sum(rgb_sum);
	mean(rgb_mean);
	stdev(rgb_stdev);

	set_color_lum();

	return pos;
}

const rgb_sum_t& tile::mean(rgb_sum_t& v) const {

	for(int i = 0; i < 3; i++)
		v[i] = (rgb_sum[i] / n()) & mean_mask;

	return v;
}

const rgb_sum_t& tile::stdev(rgb_sum_t& v) const {

	v[0] = v[1] = v[2] = 0;

	for(size_t y = 0; y < geo.size[1]; y++) {
		for(size_t x = 0; x < geo.size[0]; x++) {

			const rgb_t& w = *pixel(y,x);

			for(int i = 0; i < 3; i++) {
				int32_t xx = w[i] - rgb_mean[i];
				v[i] += xx * xx;
			}
		}
	}

	for(int i = 0; i < 3; i++)
		v[i] = (int32_t)rint(sqrt((double)v[i] / n()));

	return v;
}

const rgb_sum_t& tile::sum(rgb_sum_t& v) const {

	v[0] = v[1] = v[2] = 0;

	for(size_t y = 0; y < geo.size[1]; y++) {
		for(size_t x = 0; x < geo.size[0]; x++) {

			const rgb_t& w = *pixel(y,x);

			for(int i = 0; i < 3; i++)
				v[i] += w[i];
		}
	}

	return v;
}

void tile::set_color_lum() {
	geo.ratio = 0;
	for(int i = 0; i < 3; i++) {
		uint8_t level = (uint8_t)(rgb_sum[i] / n()) >> geometry::ratio_shift;
		geo.color[i] = (uint8_t)(rgb_sum[i] * 255 / (0x3f * n())) & mean_mask;
		if(level > geo.ratio)
			geo.ratio = level;
	}
}

rgb_t *tile::pixel(size_t y, size_t x) const {
	return source->pixel(pos[1] + y, pos[0] + x);
}

size_t tile::n() const {
	return geo.size[0] * geo.size[1];
}

std::string tile::to_string() const {

	char buf[256];
	char basebuf[16];

	switch(geo.mode) {
		case geometry::BGCOLOR: 
			snprintf(basebuf, sizeof(basebuf), "%d,%d", geo.base, 40 + geo.code );
			break;
		case geometry::FGCOLOR: 
			snprintf(basebuf, sizeof(basebuf), "%d,%d", geo.base, 30 + geo.code );
			break;
		case geometry::SYMBOL:
			snprintf(basebuf, sizeof(basebuf), "(%c)" , geo.base + geo.code);
			break;
		case geometry::SOLID:
		default:
			*basebuf = '\0';
	}

	snprintf(buf, sizeof(buf), 
			"mode %02x "
			"code %02x "
			"base %02x "
			"%4s "
			"color #%02x%02x%02x "
			"~ #%02x%02x%02x "
			"ratio %2d:%-2d %3d%% "
			"n %3d "
			"x %3d "
			"y %3d "
			"rgb_mean %3d %3d %3d "
			"rgb_stdev %3d %3d %3d "
			"size %dx%d",
			geo.mode,
			geo.code,
			geo.base,
			basebuf,
			(uint8_t)rgb_mean[0], (uint8_t)rgb_mean[1], (uint8_t)rgb_mean[2],
			geo.color[0],geo.color[1],geo.color[2],
			geo.ratio, geometry::ratio_max-geo.ratio, 100*geo.ratio/geometry::ratio_max,
			(int)n(),
			(int)pos[0], (int)pos[1],
			(int)rgb_mean[0],
			(int)rgb_mean[1],
			(int)rgb_mean[2],
			(int)rgb_stdev[0],
			(int)rgb_stdev[1],
			(int)rgb_stdev[2],
			(int)geo.size[1], (int)geo.size[0]
	);

	return std::string(buf);
}

const pos_t& tile::position(pos_t& xy, size_t line, size_t column, const pos_t size, const pos_t origin) {
	xy[0] = column * size[0] + origin[0];
	xy[1] = line * size[1] + origin[1];
	return xy;
}

#endif
