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

/*    cscope - interactive C symbol cross-reference
 *
 *    file editing functions
 */

#include <ncurses.h> // XXX
#include "global.h"
#include "exec.h"
#include "path.h"
#include "refsfound.h"

/* edit this displayed reference */

void editref(int i) {
	char file[PATHLEN + 1];	  /* file name */
	char linenum[NUMLEN + 1]; /* line number */

	/* verify that there is a references found file */
	if(refsfound == NULL) { return; }
	/* get the selected line */
	seekrelline(i);

	/* get the file name and line number */
	if(fscanf(refsfound, "%" PATHLEN_STR "s%*s%" NUMLEN_STR "s", file, linenum) == 2) {
		edit(file, linenum);
	}
}

/* edit all references */
void editall(void) {
	char file[PATHLEN + 1];	  /* file name */
	char linenum[NUMLEN + 1]; /* line number */
	int	 c;

	/* verify that there is a references found file */
	if(refsfound == NULL) { return; }
	/* get the first line */
	fseek(refsfound, 0, SEEK_SET);

	/* get each file name and line number */
	while(true) {
        int e;

	    e = fscanf(refsfound, "%" PATHLEN_STR "s%*s%" NUMLEN_STR "s%*[^\n]", file, linenum);
        if (e != 2) { break; }

		e = edit(file, linenum);
        if (e) { break; }
	}
}

/* call the editor */
int edit(const char *filename, const char *const linenum) {
    int r = 0;
	const char *const editor_basename = basename(editor);
	char			  msg[MSGLEN + 1]; /* message */
	char plusnum[NUMLEN + 20]; /* line number option: allow space for wordy line# flag */

	filename = prepend_path(prependpath, filename);
	snprintf(msg, sizeof(msg), "%s +%s %s", basename(editor), linenum, filename);
	postmsg(msg);
	snprintf(plusnum, sizeof(plusnum), lineflag, linenum);

	/* Some pagers will not start paging, unless the input
	 *  file has more lines thant the screen does.
	 *  The way to get them to pause, is to pass in /dev/null too,
	 *  imatating endless blank lines.
	 */
	const char *const shit_pagers[] = {"page", "more", NULL};
	for(const char *const *sp = shit_pagers; *sp != NULL; sp++) {
		if(!strcmp(editor_basename, *sp)) {
			r = execute(editor, editor, plusnum, filename, "/dev/null", NULL);
			goto end;
		}
	}

	if(lineflagafterfile) {
		r = execute(editor, editor, filename, plusnum, NULL);
	} else {
		r = execute(editor, editor, plusnum, filename, NULL);
	}

end:
	clear(); /* redisplay screen */

    return r;
}
