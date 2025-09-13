/*===========================================================================
 Copyright (c) 1998-2000, The Santa Cruz Operation
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 *Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 *Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 *Neither name of The Santa Cruz Operation nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
 IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT falseT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT falseT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 DAMAGE.
 =========================================================================*/

/*    cscope - interactive C symbol or text cross-reference
 *
 *    command functions
 */

#include "path.h"
#include "build.h" /* for rebuild() */


#include <stdlib.h>
#include <ncurses.h>
#include <ctype.h>

/* These are strictly used to test how keys are suppose to behave.
 * Think of it as modes: in _input mode_ the up arrow interacts
 *  with the history, while in _mode mode_ it selects what operation
 *  to perform with the user input.
 * In the original version this was handled by
 *  `int selecting // whether the (upper) symbol list is being browsed`.
 */
extern const void *const		winput;
extern const void *const		wmode;
extern const void *const		wresult;
extern const void *const *const current_window;

bool  caseless;			  /* ignore letter case when searching */
bool *change;			  /* change this line */
char  newpat[PATLEN + 1]; /* new pattern */

/* read references from a file */
bool readrefs(char *filename) {
	FILE *file;
	int	  c;

	if((file = fopen(filename, "r")) == NULL) {
		cannotopen(filename);
		return false;
	}
	if((c = getc(file)) == EOF) { /* if file is empty */
		fclose(file);
		return false;
	}
	totallines = 0;
	disprefs   = 0;
	nextline   = 1;
	if(writerefsfound() == true) {
		putc(c, refsfound);
		while((c = getc(file)) != EOF) {
			putc(c, refsfound);
		}
		fclose(file);
		fclose(refsfound);
		if((refsfound = fopen(temp1, "rb")) == NULL) {
			cannotopen(temp1);
			return (false);
		}
		countrefs();
	} else {
		fclose(file);
	}
	return (true);
}

/* count the references found */
void countrefs(void) {
	char  file[PATHLEN + 1];	/* file name */
	char  function[PATLEN + 1]; /* function name */
	char  linenum[NUMLEN + 1];	/* line number */
	int	  i;

	/* count the references found and find the length of the file,
	   function, and line number display fields */

	/* HBB NOTE 2012-04-07: it may look like we shouldn't assing tempstring here,
	 * since it's not used.  But it has to be assigned just so the return value
	 * of fscanf will actually reach 4. */
	while(EOF != (i = fscanf(refsfound,
					  "%" PATHLEN_STR "s%" PATLEN_STR "s%" NUMLEN_STR
					  "s %" TEMPSTRING_LEN_STR "[^\n]",
					  file,
					  function,
					  linenum,
					  tempstring))) {
		if((i != 4) || !isgraph((unsigned char)*file) ||
			!isgraph((unsigned char)*function) || !isdigit((unsigned char)*linenum)) {
			postmsg("File does not have expected format");
			totallines = 0;
			disprefs   = 0;
			return;
		}
		if((i = strlen(pathcomponents(file, dispcomponents))) > filelen) { filelen = i; }
		if((i = strlen(function)) > fcnlen) { fcnlen = i; }
		if((i = strlen(linenum)) > numlen) { numlen = i; }
		++totallines;
	}
	rewind(refsfound);

	/* restrict the width of displayed columns */
	/* HBB FIXME 20060419: magic number alert! */
	i = (COLS - 5) / 3;
	if(filelen > i && i > 4) { filelen = i; }
	if(fcnlen > i && i > 8) { fcnlen = i; }
}
