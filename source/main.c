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
 *    main functions
 */

#include "global.h"

#include "build.h"
#include "vpath.h"
#include "version.inc"
#include "scanner.h"
#include "display.h"
#include "readline.h"

#include <stdlib.h>	   /* atoi */
#include <ncurses.h>
#include <signal.h>

/* note: these digraph character frequencies were calculated from possible
   printable digraphs in the cross-reference for the C compiler */
char dichar1[] = " teisaprnl(of)=c";	/* 16 most frequent first chars */
char dichar2[] = " tnerpla";			/* 8 most frequent second chars
						   using the above as first chars */
char dicode1[256];						/* digraph first character code */
char dicode2[256];						/* digraph second character code */

bool  compress = true;			/* compress the characters in the crossref */
int	  dispcomponents = 1;		/* file path components to display */
bool  incurses = false;			/* in curses */
char *prependpath;				/* prepend path to file names */
FILE *refsfound;				/* references found file */

static char path[PATHLEN + 1];	/* file path */

/* Internal prototypes: */
static inline void linemode_event_loop(void);
static inline void screenmode_event_loop(void);


static inline
void siginit(void) {
	/* if running in the foreground */
	if (signal(SIGINT, SIG_IGN) != SIG_IGN) {
		/* cleanup on the interrupt and quit signals */
		signal(SIGINT, myexit);
		signal(SIGQUIT, myexit);
	}

	signal(SIGHUP, myexit);
	signal(SIGTERM, myexit);

	/* ignore PIPE signal, so myexit() will have a chance to clean up in
	 * linemode, while in curses mode the "|" command can cause a pipe signal
	 * too
	 */
	signal(SIGPIPE, SIG_IGN);

	//if (linemode == false) { signal(SIGWINCH, redisplay); }
}

void cannotopen(const char *const file) {
	posterr("Cannot open file %s", file);
}

void cannotwrite(const char * const file) {
	unlink(file);
    // XXX this seems like a terrible practice
    postfatal("Removed file %s because write failed", file);
}

/* cleanup and exit */
void myexit(int sig) {
	if (refsfound != NULL) { fclose(refsfound); }

    deinit_temp_files();

	if (incurses == true) { exitcurses(); }

	if (sig == SIGQUIT) { abort(); }

	freefilelist();
	freeinclist();
	freesrclist();
	freecrossref();
	free_newbuildfiles();

	if (remove_symfile_onexit == true) {
		unlink(reffile);
		unlink(invname);
		unlink(invpost);
	}

	exit(sig);
}


/* Remove a singular newline from the end of a string (if any). */
static inline
void remove_trailing_newline(char *buf, size_t len) {
	if ((len > 0)
    &&  (buf[len - 1] == '\n')) {
		buf[len - 1] = '\0';
	}
}

static inline
void linemode_event_loop(void) {
	if (*input_line != '\0') { /* do any optional search */
		if (search(input_line) == true) {
			if (verbosemode == true) {
				printf(PROGRAM_NAME ": %d lines\n", totallines);
			}

			int c = getc(refsfound);
			while(c != EOF) {
				putchar(c);
				c = getc(refsfound);
            }
		}
	}

	if (onesearch == true) {
		myexit(0);
	}

	for (char *s;;) {
		char buf[PATLEN + 2];

		printf(">> ");
		fflush(stdout);
		if (fgets(buf, sizeof(buf), stdin) == NULL) { myexit(0); }
		remove_trailing_newline(buf, strlen(buf));

		switch (*buf) {
			case ASCII_DIGIT: {
				field = *buf - '0';
				strcpy(input_line, buf + 1);
				if (search(input_line) == false) {
					printf("Unable to search database\n");
				} else {
					printf("cscope: %d lines\n", totallines);
                    int c;
					while((c = getc(refsfound)) != EOF) {
						putchar(c);
					}
				}
			} break;
			case 'c': /* toggle caseless mode */
			case ctrl('C'): {
                  caseless = !caseless;
                  egrepcaseless(caseless);
            } break;
			case 'r': /* rebuild database */
			case 'R':
			case ctrl('R'): {
				rebuild();
				putchar('\n');
			} break;
			case 'C': { /* clear file names */
				freefilelist();
				putchar('\n');
			} break;
			case 'F': { /* add a file name */
				strcpy(path, buf + 1);
				if (infilelist(path) == false && (s = inviewpath(path)) != NULL) {
					addsrcfile(s);
				}
				putchar('\n');
			} break;
			case 'q': /* quit */
			case ctrl('D'):
			case ctrl('Z'): {
				myexit(0);
            } break;
			default: {
				fprintf(stderr, PROGRAM_NAME ": unknown command '%s'\n", buf);
			} break;
		}
	}
}

static inline
void screenmode_event_loop(void) {
	/* pause before clearing the screen if there have been error messages */
	if (errorsfound == true) {
		errorsfound = false;
		askforreturn();
	}
	/* do any optional search */
	if (*input_line != '\0') {
		// command(ctrl('Y'));    /* search */	// XXX: fix
	} else if (reflines != NULL) {
		/* read any symbol reference lines file */
		readrefs(reflines);
	}

	for (;;) {
		display();
		handle_input(wgetch(stdscr));	 // NOTE: getch() does not return KEY_* codes
	}
}

int main(const int argc, const char * const * const argv) {

	yyin  = stdin;
	yyout = stdout;

	const char * const * fileargv = (const char*const*)parse_options(argc, argv);

    /* NOTE: the envirnment under no condition can overwrite cli set variables
     */
	readenv(preserve_database);

    /* XXX */
    init_temp_files();

	/* if the database path is relative and it can't be created */
	if (reffile[0] != '/'
    &&  access(".", WRITE) != 0) {
		/* put it in the home directory if the database may not be
		 * up-to-date or doesn't exist in the relative directory,
		 * so a database in the current directory will be
		 * used instead of failing to open a non-existant database in
		 * the home directory
		 */
		snprintf(path, sizeof(path), "%s/%s", home, reffile);
		if (preserve_database == false
        ||  access(path, READ) == 0) {
			reffile = strdup(path);
			snprintf(path, sizeof(path), "%s/%s", home, invname);
			invname = strdup(path);
			snprintf(path, sizeof(path), "%s/%s", home, invpost);
			invpost = strdup(path);
		}
	}

	siginit();

	if (linemode == false) {
		dispinit();	 /* initialize display parameters */
		postmsg(""); /* clear any build progress message */
		display();	 /* display the version number and input fields */
	}


	/* if the cross-reference is to be considered up-to-date */
	if (preserve_database == true) {
        read_old_reffile(reffile);
	} else {
		/* make the source file list */
		srcfiles = malloc(msrcfiles * sizeof(*srcfiles));
		makefilelist(fileargv);
		if (nsrcfiles == 0) {
			postfatal(PROGRAM_NAME ": no source files found\n");
		}

		/* add /usr/include to the #include directory list,
		   but not in kernelmode... kernels tend not to use it. */
		if (!kernelmode) {
            includedir(incdir);
		}

		/* initialize the C keyword table */
		initsymtab();

		/* Tell build.c about the filenames to create: */
		setup_build_filenames(reffile);

		/* build the cross-reference */
		initcompress();
		if (linemode == false
        || verbosemode == true) { /* display if verbose as well */
			postmsg("Building cross-reference...");
		}
		build();
		if (linemode == false) { postmsg(""); /* clear any build progress message */ }
		if (buildonly == true) {
			myexit(0);
		}
	}

	opendatabase(reffile);

	if (linemode == true) {
        linemode_event_loop();
    } else {
        screenmode_event_loop();
    }

	return 0;
}
