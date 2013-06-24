#ifndef PNM_HH
#define PNM_HH

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cctype>

#include <rgb.hh>

struct pnm {
	size_t width = 0;
	size_t height = 0;
	rgb_t *data = NULL;

	pnm(FILE *fp);
	~pnm();

	bool free();
	bool isloaded() const;
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
				free();
			}
		}
	}
}

bool pnm::isloaded() const {
	return data != NULL && width > 0 && height > 0;
}

bool pnm::free() {
	if(data != NULL) {
		delete data;
		data = NULL;
		return true;
	}
	return false;
}

pnm::~pnm() {
	free();
}

rgb_t *pnm::pixel(size_t y, size_t x) const {
	return &data[y * width + x];
}

#endif
