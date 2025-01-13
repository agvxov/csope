/* Possibly rename */
#include "global.h"

#include <stdio.h>
#include <assert.h>
#include "refsfound.h"

struct FILE;

/* Page cursor stack */
static FILE **hto_page = &refsfound;
#define PCS_MAXPAGE 16
static size_t PCS_pos[PCS_MAXPAGE] = {0};
static size_t PCS_top			   = 0;

long seekpage(const size_t i) {
	if(i > PCS_MAXPAGE - 1) { return -1; }

	if(i < PCS_top) {
		fseek(*hto_page, PCS_pos[i], SEEK_SET);
		return PCS_pos[i];
	}

	fseek(*hto_page, PCS_pos[PCS_top], SEEK_SET);

	size_t lc = 0;
	while(PCS_top < i) {
		const char c = getc(*hto_page);
		if(c == '\n') { ++lc; }
		if(c == EOF) { return -1; }
		if(lc == mdisprefs) { PCS_pos[++PCS_top] = ftell(*hto_page); }
	}
	return PCS_pos[PCS_top];
}

long seekrelline(unsigned i) {
	seekpage(current_page);
	size_t lc = 0;
	while(lc < i) {
		const char c = getc(*hto_page);
		assert("seekrelline() tried to read past the reference file" && !(c == EOF));
		if(c == '\n') { ++lc; }
	}
	return ftell(*hto_page);
}

void PCS_reset(void) {
	PCS_top = 0;
}
