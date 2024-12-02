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

/* get a file's base name from its path name */

#include "global.h"

#include <unistd.h>
#include <string.h>

const char *basename(const char *path) {
	const char *s;

	if((s = strrchr(path, '/')) != 0) { return (s + 1); }
	return path;
}

/* get the requested path components */
char *pathcomponents(char *path, int components) {
	char * s = path + strlen(path) - 1;
	for(int i = 0; i < components; i++) {
		while(s > path && *--s != '/') {
			;
		}
	}
	if(s > path && *s == '/') { ++s; }
	return (s);
}

/* Remove multiple slashes from a path string. */
static inline
void path_remove_multiple_slashes(char *path) {
	char *read = path;
	char *write = path;

	while (*read) {
		*write++ = *read++;
		if (*(write - 1) == '/') {
			while (*read == '/') {
				read++;
			}
		}
	}
	*write = '\0';
}


static inline
void path_remove_current_directory_references(char *path) {
	char *read = path;
	char *write = path;

	while (*read) {
		if (read[0] == '.' && read[1] == '/' &&
			(read == path || *(read - 1) == '/')) {
			read += 2; // Skip "./"
		} else {
			*write++ = *read++;
		}
	}
	*write = '\0';
}


static inline
void path_remove_parent_directory_references(char *path) {
	char *read = path;
	char *write = path;

    while (*read) {
		if (read[0] == '/' && read[1] == '.' && read[2] == '.' &&
			(read[3] == '/' || read[3] == '\0')) {
			read += (read[3] == '/') ? 4 : 3; // Skip "/.."
			if (write > path) {
				write--; // Go back to previous slash
				while (write > path && *(write - 1) != '/') {
					write--;
				}
			}
		} else {
			*write++ = *read++;
		}
	}
	*write = '\0';
}


/*
 *    compress_path(pathname)
 *
 *    This compresses pathnames.  All strings of multiple slashes are
 *    changed to a single slash.  All occurrences of "./" are removed.
 *    Whenever possible, strings of "/.." are removed together with
 *    the directory names that they follow.
 *
 *    WARNING: since pathname is altered by this function, it should
 *         be located in a temporary buffer. This avoids the problem
 *         of accidently changing strings obtained from makefiles
 *         and stored in global structures.
 */
char *compress_path(const char *pathname_) {
	if (pathname_ == NULL) {
		return NULL;
	}

	char *pathname = strdup(pathname_);

	/*
	 *	do not change the path if it has no "/"
	 */
	if(strchr(pathname, '/') == NULL) {
		return (pathname);
	}

	/* Step 1: Remove multiple slashes */
	path_remove_multiple_slashes(pathname);

	/* Step 2: Remove curr dir "./" references */
	path_remove_current_directory_references(pathname);

	/* Step 3: Remove parent dir "/.." references */
	path_remove_parent_directory_references(pathname);

	/* Handle trailing slashes */
	size_t len = strlen(pathname);
	if (len > 1 && pathname[len - 1] == '/') {
		pathname[len - 1] = '\0';
	}

	/* Step 5: If the path is empty, return "." */
	if (*pathname == '\0') {
		strcpy(pathname, ".");
	}

	return pathname;
}

static
char *nextfield(char *p) {
	while(*p && *p != ':') { ++p; }
	if(*p) { *p++ = 0; }
	return p;
}

/*
 *    logdir()
 *
 *    This routine does not use the getpwent(3) library routine
 *    because the latter uses the stdio package.  The allocation of
 *    storage in this package destroys the integrity of the shell's
 *    storage allocation.
 */
char *logdir(char *name) {
	#define BUFFER_SIZE 160
	static char line[BUFFER_SIZE];
	char *p;
	int	  i, j;
	int	  pwf;

	/* attempt to open the password file */
	if((pwf = myopen("/etc/passwd", 0, 0)) == -1) return (0);

	/* find the matching password entry */
	do {
		/* get the next line in the password file */
		i = read(pwf, line, BUFFER_SIZE-1);
		for(j = 0; j < i; j++) {
			if(line[j] == '\n') { break; }
        }
		/* return a null pointer if the whole file has been read */
		if(j >= i) return (0);
		line[++j] = 0;						/* terminate the line */
		(void)lseek(pwf, (long)(j - i), 1); /* point at the next line */
		p = nextfield(line);				/* get the logname */
	} while(*name != *line ||				/* fast pretest */
			strcmp(name, line) != 0);
	(void)close(pwf);

	/* skip the intervening fields */
	p = nextfield(p);
	p = nextfield(p);
	p = nextfield(p);
	p = nextfield(p);

	/* return the login directory */
	(void)nextfield(p);
	return p;
}
