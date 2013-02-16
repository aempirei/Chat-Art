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
 * prepare a jpeg for conversion using netpbm:
 *
 * djpeg image.pgm | pnmscale -ysize 120 | ppmnorm | ppmquant -fs 2 | ppmtopgm | ./pgmtobraile
 *
 */
 

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

void braileencode(FILE *fpout, uint8_t *data, size_t width, size_t height) {
	for(size_t y = 0; y < height; y += 4) {
		for(size_t x = 0; x < width; x += 2) {
			wint_t wch = 0x2800;
			if(BIT0(data,x,y,width) > 0x80) wch |= 0x01;
			if(BIT1(data,x,y,width) > 0x80) wch |= 0x02;
			if(BIT2(data,x,y,width) > 0x80) wch |= 0x04;
			if(BIT3(data,x,y,width) > 0x80) wch |= 0x08;
			if(BIT4(data,x,y,width) > 0x80) wch |= 0x10;
			if(BIT5(data,x,y,width) > 0x80) wch |= 0x20;
			if(BIT6(data,x,y,width) > 0x80) wch |= 0x40;
			if(BIT7(data,x,y,width) > 0x80) wch |= 0x80;
			fputwc(wch, fpout);
		}
		fputwc(L'\n', fpout);
	}
}

void pgmtobraile(FILE * fpin, FILE * fpout) {

	const int expected_maxval = 255;
	const char *locale = "";
	uint8_t *data;
	int width;
	int height;
	int maxval;
	int ch;
	
	if (setlocale(LC_CTYPE, locale) == NULL) {
		fprintf(stderr, "failed to set locale LC_CTYPE=\"%s\"\n", locale);
		exit(EXIT_FAILURE);
	}

	if(fscanf(fpin, "P5 %d %d %d", &width, &height, &maxval) != 3) {
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

	data = malloc(width * height);

	if(fread(data, width, height, fpin) != (size_t)height) {
		perror("fread()");
		exit(EXIT_FAILURE);
	}

	if(fgetc(fpin) != EOF || !feof(fpin)) {
		fprintf(stderr, "unexpected trailing data after pgm\n");
	}

	braileencode(fpout, data, width, height);

	free(data);
}

int main() {
	pgmtobraile(stdin, stdout);
	exit(EXIT_SUCCESS);
}
