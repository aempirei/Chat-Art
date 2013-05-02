/* calibrate
 * Calibrate your terminal for use with Chat-Art tools.
 * Copyright(c) 2013 by Christopher Abad | 20 GOTO 10
 *
 * email: aempirei@gmail.com aempirei@256.bz
 * http://www.256.bz/ http://www.twentygoto10.com/
 * git: git@github.com:aempirei/Chat-Art.git
 * aim: ambientempire
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>
#include <ctype.h>
#include <signal.h>

const size_t height = 5;
const char clrscr[] = "\x1b[2J";
const char cursor_on[] = "\x1b[?25h";
const char cursor_off[] = "\x1b[?25l";
const char cusor_home[] = "\x1b[H";
const char cursor_save[] = "\x1b[s";
const char cursor_restore[] = "\x1b[u";
const char ansi_clear[] = "\x1b[0m";

void moveyx(char *s, size_t sz, size_t y, size_t x) {
	/* origin is (1,1) */
	snprintf(s, sz, "\x1b[%d;%dH", (int)y, (int)x);
}

void setfg(char *s, size_t sz, size_t color) {
	snprintf(s, sz, "\x1b[38;5;%dm", (int)color);
}
void setbg(char *s, size_t sz, size_t color) {
	snprintf(s, sz, "\x1b[48;5;%dm", (int)color);
}

void fsetfg(size_t color) {
	char s[32];
	setfg(s, sizeof(s), color);
	fputs(s, stdout);
}
void fsetbg(size_t color) {
	char s[32];
	setbg(s, sizeof(s), color);
	fputs(s, stdout);
}
void fmoveyx(size_t y, size_t x) {
	char s[32];
	moveyx(s, sizeof(s), y, x);
	fputs(s, stdout);
}

void sighandler(int signo) {
	if(signo != SIGALRM) {
		signal(signo, SIG_IGN);
		fputs(ansi_clear, stdout);
		fputs(cursor_on, stdout);
		fmoveyx(height + 2, 1);
		exit(EXIT_SUCCESS);
	}
}

void calibrate() {

	const char *locale = "";

	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);

	srandom(getpid());

	setvbuf(stdout, NULL, _IONBF, 0);
	
	if (setlocale(LC_CTYPE, locale) == NULL) {
		fprintf(stderr, "failed to set locale LC_CTYPE=\"%s\"\n", locale);
		exit(EXIT_FAILURE);
	}

	fputs(cursor_off, stdout);
	fputs(clrscr, stdout);

	fmoveyx(1, 1);

	/*
	   for(int i = 0; i <= 100; i++) {
	   int p = (int)floor(100.0 * random() / (RAND_MAX + 1.0));
	   fputc('O', stdout);
	   }
	   fputc('\n', stdout);
	 */

	for(;;) {
		fputs(cursor_save, stdout);
		for(unsigned int j = 1; j <= height; j++)  {
			for(int i = 0; i <= 100; i++) {
				int p = (int)floor(100.0 * random() / (RAND_MAX + 1.0));
				if(p < i) {
					fsetfg(0);
					fsetbg(i%15);
				} else {
					fsetfg(i%15);
					fsetbg(0);
				}

				fputc('O',stdout);
			}
			fputc('\n', stdout);
		}
		fputs(cursor_restore, stdout);
	}
}

int main() {
	calibrate();
	exit(EXIT_SUCCESS);
}
