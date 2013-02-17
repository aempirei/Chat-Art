/* pgmtobraile
 * Convert a pgm image to a unicode braile image
 * Copyright(c) 2013 by Christopher Abad | 20 GOTO 10
 *
 * email: aempirei@gmail.com aempirei@256.bz
 * http://www.256.bz/ http://www.twentygoto10.com/
 * git: git@github.com:aempirei/Chat-Art.git
 * aim: ambientempire
 */

/*
 * also it includes its own pgm dithering now
 * an example preparation of a jpeg for conversion using netpbm:
 *
 * djpeg rms.jpg | pnmscale -ysize 120 | ppmnorm | ppmtopgm | ./pgmtobraile
 *
 */

/* -- FIXME: after the image is approximated via dither, then the error is calculated against the ORIGINAL image */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>
#include <ctype.h>

#define BIT(p,x,y,w) (p)[(x) + (y) * (w)]
#define BIT0(p,x,y,w) BIT((p),(x)+0,(y)+0,(w))
#define BIT1(p,x,y,w) BIT((p),(x)+0,(y)+1,(w))
#define BIT2(p,x,y,w) BIT((p),(x)+0,(y)+2,(w))
#define BIT3(p,x,y,w) BIT((p),(x)+1,(y)+0,(w))
#define BIT4(p,x,y,w) BIT((p),(x)+1,(y)+1,(w))
#define BIT5(p,x,y,w) BIT((p),(x)+1,(y)+2,(w))
#define BIT6(p,x,y,w) BIT((p),(x)+0,(y)+3,(w))
#define BIT7(p,x,y,w) BIT((p),(x)+1,(y)+3,(w))

const int upper = 0xff;
const int lower = 0x01;
int mid = 0x80;

int getmean(int *data, size_t sz) {
	int mean = 0;
	for(size_t z = 0; z < sz; z++)
	mean += data[sz];
	return mean / sz;
}

void dither(int *data, int *error, size_t width, size_t height) {

	int *front;

	size_t sz = width * height;

	front = malloc(sizeof(int) * sz);
	error = malloc(sizeof(int) * sz);

	memset(error, 0, sizeof(int) * sz);

	for(size_t z = 0; z < sz; z++)
		front[z] = ((data[z] >= mid) ? upper : lower) - data[z];

	for(size_t y = 0; y < height; y++) {
		for(size_t x = 0; x < width; x++) {

			int used = (x<width-1)+(y<height-1)+1;
			int e = BIT(front,x,y,width) / used;

			if(x < width - 1) 
				BIT(error,x+1,y,width) += e;
			if(y < height - 1) 
				BIT(error,x,y+1,width) += e;

			//	BIT(error,x,y,width) -= e * used;

		}
	}

	int max = lower;
	int min = upper;

	for(size_t z = 0; z < sz; z++) {
		data[z] += error[z];
		if(data[z] > max)
			max = data[z];
		if(data[z] < min)
			min = data[z];
	}

	for(size_t z = 0; z < sz; z++) {
		data[z] = ((upper - lower) * (data[z] - min) / (max - min)) + lower;
	}

	free(front);
}

void braileencode(FILE *fpout, int *data, size_t width, size_t height) {
	for(size_t y = 0; y < height; y += 4) {
		for(size_t x = 0; x < width; x += 2) {
			wint_t wch = 0x2800;
			if(BIT0(data,x,y,width)>mid) wch |= 0x01;
			if(BIT1(data,x,y,width)>mid) wch |= 0x02;
			if(BIT2(data,x,y,width)>mid) wch |= 0x04;
			if(BIT3(data,x,y,width)>mid) wch |= 0x08;
			if(BIT4(data,x,y,width)>mid) wch |= 0x10;
			if(BIT5(data,x,y,width)>mid) wch |= 0x20;
			if(BIT6(data,x,y,width)>mid) wch |= 0x40;
			if(BIT7(data,x,y,width)>mid) wch |= 0x80;
			fputwc(wch, fpout);
		}
		fputwc(L'\n', fpout);
	}
}

void pgmtobraile(FILE * fpin, FILE * fpout) {

	const size_t expected_maxval = 255;
	const char *locale = "";
	uint8_t *bytedata;
	int *img;
	int *error;
	size_t width;
	size_t height;
	size_t maxval;
	int ch;
	
	if (setlocale(LC_CTYPE, locale) == NULL) {
		fprintf(stderr, "failed to set locale LC_CTYPE=\"%s\"\n", locale);
		exit(EXIT_FAILURE);
	}

	if(fscanf(fpin, "P5 %u %u %u", &width, &height, &maxval) != 3) {
		fprintf(stderr, "failed to parse pgm header\n");
		exit(EXIT_FAILURE);
	}

	ch = fgetc(fpin);
	if(ch == EOF) {
		perror("fgetc()");
		exit(EXIT_FAILURE);
	}

	if(!isspace(ch)) {
		fprintf(stderr, "unexpected character found after maxval, was expecting whitespace\n");
		exit(EXIT_FAILURE);
	}

	if(maxval != expected_maxval) {
		fprintf(stderr, "unexpected maxval of %d, was expecting %d\n", maxval, expected_maxval);
	}

	bytedata = malloc(width * height);

	if(fread(bytedata, width, height, fpin) != (size_t)height) {
		perror("fread()");
		exit(EXIT_FAILURE);
	}

	if(fgetc(fpin) != EOF || !feof(fpin)) {
		fprintf(stderr, "unexpected trailing data after pgm\n");
	}

	img = malloc(width * height * sizeof(int));
	error = malloc(width * height * sizeof(int));

	for(size_t z = 0; z < width * height; z++) {
		img[z] = bytedata[z];
		error[z] = 0;
	}

	for(int i = 0; i < 5; i++)
	dither(img, error, width, height);

	braileencode(fpout, img, width, height);

	free(bytedata);
	free(error);
	free(img);
}

int main(int argc, char **argv) {
	if(argc == 2) mid = atoi(argv[1]);
	pgmtobraile(stdin, stdout);
	exit(EXIT_SUCCESS);
}
