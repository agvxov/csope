/*    cscope - interactive C symbol cross-reference
 *
 *    file editing functions
 */

#include "global.h"
#include "edit.h"
#include "display.h"
#include "exec.h"
#include "path.h"
#include "refsfound.h"

/* edit this displayed reference */
void editref(int i) {
	char filename[PATHLEN + 1];
	char linenum[NUMLEN + 1];

	if(refsfound == NULL) { return; }

	seekrelline(i);

	if(fscanf(refsfound, "%" PATHLEN_STR "s%*s%" NUMLEN_STR "s", filename, linenum) == 2) {
		edit(filename, linenum);
	}
}

void editall(void) {
	char filename[PATHLEN + 1];
	char linenum[NUMLEN + 1];

	if(refsfound == NULL) { return; }

	fseek(refsfound, 0, SEEK_SET);

	while(true) {
        int e;

	    e = fscanf(refsfound, "%" PATHLEN_STR "s%*s%" NUMLEN_STR "s%*[^\n]", filename, linenum);
        if (e != 2) { break; }

		e = edit(filename, linenum);
        if (e) { break; }
	}
}

/* call the editor */
int edit(const char *filename, const char *const linenum) {
    int r = 0;
	const char *const editor_basename = basename(editor);
	char			  msg[MSGLEN + 1];
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
			return r;
		}
	}

	if(lineflagafterfile) {
		r = execute(editor, editor, filename, plusnum, NULL);
	} else {
		r = execute(editor, editor, plusnum, filename, NULL);
	}

    return r;
}
