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

#include "path.h"
#include "library.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

/* expand the ~ and $ shell meta characters in a path */
void shellpath(char * out, int limit, char * in) {
	char * lastchar;
	char * s, * v;

	/* skip leading white space */
	while(isspace((unsigned char)*in)) {
		++in;
	}
	lastchar = out + limit - 1;

	/* a tilde (~) by itself represents $HOME; followed by a name it
	   represents the $LOGDIR of that login name */
	if(*in == '~') {
		*out++ = *in++; /* copy the ~ because it may not be expanded */

		/* get the login name */
		s = out;
		while(s < lastchar && *in != '/' && *in != '\0' && !isspace((unsigned char)*in)) {
			*s++ = *in++;
		}
		*s = '\0';

		/* if the login name is null, then use $HOME */
		if(*out == '\0') {
			v = getenv("HOME");
		} else { /* get the home directory of the login name */
			v = logdir(out);
		}
		/* copy the directory name if it isn't too big */
		if(v != NULL && strlen(v) < (lastchar - out)) {
			strcpy(out - 1, v);
			out += strlen(v) - 1;
		} else {
			/* login not found, so ~ must be part of the file name */
			out += strlen(out);
		}
	}
	/* get the rest of the path */
	while(out < lastchar && *in != '\0' && !isspace((unsigned char)*in)) {

		/* look for an environment variable */
		if(*in == '$') {
			*out++ = *in++; /* copy the $ because it may not be expanded */

			/* get the variable name */
			s = out;
			while(s < lastchar && *in != '/' && *in != '\0' &&
				  !isspace((unsigned char)*in)) {
				*s++ = *in++;
			}
			*s = '\0';

			/* get its value, but only it isn't too big */
			if((v = getenv(out)) != NULL && strlen(v) < (lastchar - out)) {
				strcpy(out - 1, v);
				out += strlen(v) - 1;
			} else {
				/* var not found, or too big, so assume $ must be part of the
				 * file name */
				out += strlen(out);
			}
		} else { /* ordinary character */
			*out++ = *in++;
		}
	}
	*out = '\0';
}

const char * basename(const char *path) {
	const char *s;

	if((s = strrchr(path, '/')) != 0) { return (s + 1); }
	return path;
}

/* if requested, prepend a path to a relative file name */
const char * prepend_path(const char * prepand_with, const char * file) {
	static char path[PATHLEN + 1]; // XXX

    if (!prepand_with
    ||  *file == '/') {
        return file;
    }

	snprintf(path, sizeof(path), "%s/%s", prepand_with, file);
	return path;
}

/* get the requested path components */
char * pathcomponents(char *path, int components) {
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
void path_remove_current_directory_references(char * path) {
	char * read = path;
	char * write = path;

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
char * compress_path(const char *pathname_) {
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
char * nextfield(char *p) {
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
char * logdir(char *name) {
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
