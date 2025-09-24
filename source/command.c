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
  #define DISCARD_BUFFER_SIZE 1024
    static char discard_buffer[DISCARD_BUFFER_SIZE + 1];
	char  file[PATHLEN + 1];	/* file name */
	char  function[PATLEN + 1]; /* function name */
	char  linenum[NUMLEN + 1];	/* line number */
	int	  i;

	/* count the references found and find the length of the file,
	   function, and line number display fields */

	/* NOTE: it may look like we shouldn't assing discard_buffer here,
	 * since it's not used.  But it has to be assigned just so the return value
	 * of fscanf will actually reach 4. */
	while(EOF != (i = fscanf(refsfound,
					  "%" PATHLEN_STR "s%" PATLEN_STR "s%" NUMLEN_STR
					  "s %" STRINGIZE(DISCARD_BUFFER_SIZE) "[^\n]",
					  file,
					  function,
					  linenum,
					  discard_buffer))) {
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
  #undef DISCARD_BUFFER_SIZE
}
