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
#include "vp.h"
#include "version.inc"
#include "scanner.h"

#include <stdlib.h>	   /* atoi */
#include <ncurses.h>
#include <sys/types.h> /* needed by stat.h */
#include <sys/stat.h>  /* stat */
#include <signal.h>
#include <getopt.h>

/* note: these digraph character frequencies were calculated from possible
   printable digraphs in the cross-reference for the C compiler */
char dichar1[] = " teisaprnl(of)=c";	/* 16 most frequent first chars */
char dichar2[] = " tnerpla";			/* 8 most frequent second chars
						   using the above as first chars */
char dicode1[256];						/* digraph first character code */
char dicode2[256];						/* digraph second character code */

char		*argv0;						/* command name */
bool		 compress = true;			/* compress the characters in the crossref */
bool		 dbtruncated;				/* database symbols are truncated to 8 chars */
int			 dispcomponents = 1;		/* file path components to display */
bool		 editallprompt	= true;		/* prompt between editing files */
unsigned int fileargc;					/* file argument count */
char	   **fileargv;					/* file argument values */
int			 fileversion;				/* cross-reference file version */
bool		 incurses = false;			/* in curses */
bool		 invertedindex;				/* the database has an inverted index */
bool		 isuptodate;				/* consider the crossref up-to-date */
bool		 kernelmode;				/* don't use DFLT_INCDIR - bad for kernels */
bool		 linemode	 = false;		/* use line oriented user interface */
bool		 verbosemode = false;		/* print extra information on line mode */
bool		 recurse_dir = false;		/* recurse dirs when searching for src files */
char		*namefile;					/* file of file names */
char		*prependpath;				/* prepend path to file names */
FILE		*refsfound;					/* references found file */
char		 temp1[PATHLEN + 1];		/* temporary file name */
char		 temp2[PATHLEN + 1];		/* temporary file name */
char		 tempdirpv[PATHLEN + 1];	/* private temp directory */
long		 totalterms;				/* total inverted index terms */
bool		 trun_syms;					/* truncate symbols to 8 characters */
char		 tempstring[TEMPSTRING_LEN + 1]; /* use this as a buffer, instead of 'yytext',
											  * which had better be left alone */

static char path[PATHLEN + 1];				 /* file path */

/* Internal prototypes: */
static void		   skiplist(FILE *oldrefs);
static void		   initcompress(void);
static inline void linemode_event_loop(void);
static inline void screenmode_event_loop(void);



static inline
void siginit(void) {
	/* if running in the foreground */
	if(signal(SIGINT, SIG_IGN) != SIG_IGN) {
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

	//if(linemode == false) { signal(SIGWINCH, redisplay); }
}

void cannotopen(const char *const file) {
	posterr("Cannot open file %s", file);
}

/* FIXME MTE - should use postfatal here */
void cannotwrite(const char *const file) {
	char msg[MSGLEN + 1];

	snprintf(msg, sizeof(msg), "Removed file %s because write failed", file);

	myperror(msg); /* display the reason */

	unlink(file);
	myexit(1); /* calls exit(2), which closes files */
}

/* set up the digraph character tables for text compression */
static
void initcompress(void) {
	if(compress == true) {
		for(int i = 0; i < 16; i++) {
			dicode1[(unsigned char)(dichar1[i])] = i * 8 + 1;
		}
		for(int i = 0; i < 8; i++) {
			dicode2[(unsigned char)(dichar2[i])] = i + 1;
		}
	}
}

/* skip the list in the cross-reference file */
static
void skiplist(FILE *oldrefs) {
	int i;

	if(fscanf(oldrefs, "%d", &i) != 1) {
		postfatal(PROGRAM_NAME ": cannot read list size from file %s\n", reffile);
		/* NOTREACHED */
	}
	while(--i >= 0) {
		if(fscanf(oldrefs, "%*s") != 0) {
			postfatal(PROGRAM_NAME ": cannot read list name from file %s\n", reffile);
			/* NOTREACHED */
		}
	}
}

/* cleanup and exit */
void myexit(int sig) {
	/* Close file before unlinking it. DOS absolutely needs it */
	if(refsfound != NULL) { fclose(refsfound); }

	/* remove any temporary files */
	if(temp1[0] != '\0') {
		unlink(temp1);
		unlink(temp2);
		rmdir(tempdirpv);
	}
	/* restore the terminal to its original mode */
	if(incurses == true) { exitcurses(); }
	/* dump core for debugging on the quit signal */
	if(sig == SIGQUIT) { abort(); }
	/* HBB 20000421: be nice: free allocated data */
	freefilelist();
	freeinclist();
	freesrclist();
	freecrossref();
	free_newbuildfiles();

	if(remove_symfile_onexit == true) {
		unlink(reffile);
		unlink(invname);
		unlink(invpost);
	}

	exit(sig);
}

static inline
void linemode_event_loop(void) {
	int c;

	if(*input_line != '\0') { /* do any optional search */
		if(search(input_line) == true) {
			/* print the total number of lines in
			 * verbose mode */
			if(verbosemode == true) printf(PROGRAM_NAME ": %d lines\n", totallines);

			while((c = getc(refsfound)) != EOF) {
				putchar(c);
            }
		}
	}

	if(onesearch == true) { myexit(0); }

	for(char *s;;) {
		char buf[PATLEN + 2];

		printf(">> ");
		fflush(stdout);
		if(fgets(buf, sizeof(buf), stdin) == NULL) { myexit(0); }
		/* remove any trailing newline character */
		if(*(s = buf + strlen(buf) - 1) == '\n') { *s = '\0'; }
		switch(*buf) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': /* samuel only */
				field = *buf - '0';
				strcpy(input_line, buf + 1);
				if(search(input_line) == false) {
					printf("Unable to search database\n");
				} else {
					printf("cscope: %d lines\n", totallines);
					while((c = getc(refsfound)) != EOF) {
						putchar(c);
					}
				}
				break;

			case 'c': /* toggle caseless mode */
			case ctrl('C'):
				if(caseless == false) {
					caseless = true;
				} else {
					caseless = false;
				}
				egrepcaseless(caseless);
				break;

			case 'r': /* rebuild database cscope style */
			case ctrl('R'):
				freefilelist();
				makefilelist();
				/* FALLTHROUGH */

			case 'R': /* rebuild database samuel style */
				rebuild();
				putchar('\n');
				break;

			case 'C': /* clear file names */
				freefilelist();
				putchar('\n');
				break;

			case 'F': /* add a file name */
				strcpy(path, buf + 1);
				if(infilelist(path) == false && (s = inviewpath(path)) != NULL) {
					addsrcfile(s);
				}
				putchar('\n');
				break;

			case 'q': /* quit */
			case ctrl('D'):
			case ctrl('Z'):
				myexit(0);
				/* NOTREACHED */
				break;
			default:
				fprintf(stderr, PROGRAM_NAME ": unknown command '%s'\n", buf);
				break;
		}
	}
}

static inline
void screenmode_event_loop(void) {
	for(;;) {
		display();
		handle_input(wgetch(stdscr));	 // NOTE: getch() does not return KEY_* codes
	}
}

int main(int argc, char **argv) {
	FILE		*names;	  /* name file pointer */
	int			 oldnum;  /* number in old cross-ref */
	FILE		*oldrefs; /* old cross-reference file */
	char		*s;
	unsigned int i;
	pid_t		 pid;
	struct stat	 stat_buf;
	mode_t		 orig_umask;

	yyin  = stdin;
	yyout = stdout;
	/* save the command name for messages */
	argv0 = argv[0];

	/* set the options */
	argv = parse_options(&argc, argv);

	/* read the environment */
	readenv();

	/* make sure that tmpdir exists */
	if(lstat(tmpdir, &stat_buf)) {
		fprintf(stderr,
			PROGRAM_NAME
			": Temporary directory %s does not exist or cannot be accessed\n",
			tmpdir);
		fprintf(stderr,
			PROGRAM_NAME
			": Please create the directory or set the environment variable\n" PROGRAM_NAME
			": TMPDIR to a valid directory\n");
		myexit(1);
	}

	/* create the temporary file names */
	orig_umask = umask(S_IRWXG | S_IRWXO);
	pid		   = getpid();
	snprintf(tempdirpv, sizeof(tempdirpv), "%s/" PROGRAM_NAME ".%d", tmpdir, pid);
	if(mkdir(tempdirpv, S_IRWXU)) {
		fprintf(stderr,
			PROGRAM_NAME ": Could not create private temp dir %s\n",
			tempdirpv);
		myexit(1);
	}
	umask(orig_umask);

	snprintf(temp1, sizeof(temp1), "%s/" PROGRAM_NAME ".1", tempdirpv);
	snprintf(temp2, sizeof(temp2), "%s/" PROGRAM_NAME ".2", tempdirpv);

	/* if the database path is relative and it can't be created */
	if(reffile[0] != '/'
    && access(".", WRITE) != 0) {
		/* put it in the home directory if the database may not be
		 * up-to-date or doesn't exist in the relative directory,
		 * so a database in the current directory will be
		 * used instead of failing to open a non-existant database in
		 * the home directory
		 */
		snprintf(path, sizeof(path), "%s/%s", home, reffile);
		if(isuptodate == false || access(path, READ) == 0) {
			reffile = strdup(path);
			snprintf(path, sizeof(path), "%s/%s", home, invname);
			invname = strdup(path);
			snprintf(path, sizeof(path), "%s/%s", home, invpost);
			invpost = strdup(path);
		}
	}

	siginit();

	if(linemode == false) {
		dispinit();	 /* initialize display parameters */
		postmsg(""); /* clear any build progress message */
		display();	 /* display the version number and input fields */
	}


	/* if the cross-reference is to be considered up-to-date */
	if(isuptodate == true) {
		if((oldrefs = vpfopen(reffile, "rb")) == NULL) {
			postfatal(PROGRAM_NAME ": cannot open file %s\n", reffile);
			/* NOTREACHED */
		}
		/* get the crossref file version but skip the current directory */
		if(fscanf(oldrefs, PROGRAM_NAME " %d %*s", &fileversion) != 1) {
			postfatal(PROGRAM_NAME ": cannot read file version from file %s\n", reffile);
			/* NOTREACHED */
		}
		if(fileversion >= 8) {

			/* override these command line options */
			compress	  = true;
			invertedindex = false;

			/* see if there are options in the database */
			for(int c;;) {
				getc(oldrefs); /* skip the blank */
				if((c = getc(oldrefs)) != '-') {
					ungetc(c, oldrefs);
					break;
				}
				switch(getc(oldrefs)) {
					case 'c': /* ASCII characters only */
						compress = false;
						break;
					case 'q': /* quick search */
						invertedindex = true;
						fscanf(oldrefs, "%ld", &totalterms);
						break;
					case 'T': /* truncate symbols to 8 characters */
						dbtruncated = true;
						trun_syms	= true;
						break;
				}
			}
			initcompress();
			seek_to_trailer(oldrefs);
		}
		/* skip the source and include directory lists */
		skiplist(oldrefs);
		skiplist(oldrefs);

		/* get the number of source files */
		if(fscanf(oldrefs, "%lu", &nsrcfiles) != 1) {
			postfatal(PROGRAM_NAME ": cannot read source file size from file %s\n",
				reffile);
			/* NOTREACHED */
		}
		/* get the source file list */
		srcfiles = malloc(nsrcfiles * sizeof(*srcfiles));
		if(fileversion >= 9) {

			/* allocate the string space */
			if(fscanf(oldrefs, "%d", &oldnum) != 1) {
				postfatal(PROGRAM_NAME ": cannot read string space size from file %s\n",
					reffile);
				/* NOTREACHED */
			}
			s = malloc(oldnum);
			getc(oldrefs); /* skip the newline */

			/* read the strings */
			if(fread(s, oldnum, 1, oldrefs) != 1) {
				postfatal(PROGRAM_NAME ": cannot read source file names from file %s\n",
					reffile);
				/* NOTREACHED */
			}
			/* change newlines to nulls */
			for(i = 0; i < nsrcfiles; ++i) {
				srcfiles[i] = s;
				for(++s; *s != '\n'; ++s) {
					;
				}
				*s = '\0';
				++s;
			}
			/* if there is a file of source file names */
			if((namefile != NULL && (names = vpfopen(namefile, "r")) != NULL) ||
				(names = vpfopen(NAMEFILE, "r")) != NULL) {

				/* read any -p option from it */
				while(fgets(path, sizeof(path), names) != NULL && *path == '-') {
					i = path[1];
					s = path + 2;	 /* for "-Ipath" */
					if(*s == '\0') { /* if "-I path" */
						fgets(path, sizeof(path), names);
						s = path;
					}
					switch(i) {
						case 'p': /* file path components to display */
							if(*s < '0' || *s > '9') {
								posterr(PROGRAM_NAME
									": -p option in file %s: missing or invalid numeric value\n",
									namefile);
							}
							dispcomponents = atoi(s);
					}
				}
				fclose(names);
			}
		} else {
			for(i = 0; i < nsrcfiles; ++i) {
				if(!fgets(path, sizeof(path), oldrefs)) {
					postfatal(PROGRAM_NAME
						": cannot read source file name from file %s\n",
						reffile);
					/* NOTREACHED */
				}
				srcfiles[i] = strdup(path);
			}
		}
		fclose(oldrefs);
	} else {
		/* save the file arguments */
		fileargc = argc;
		fileargv = argv;

		/* get source directories from the environment */
		if((s = getenv("SOURCEDIRS")) != NULL) { sourcedir(s); }
		/* make the source file list */
		srcfiles = malloc(msrcfiles * sizeof(*srcfiles));
		makefilelist();
		if(nsrcfiles == 0) {
			postfatal(PROGRAM_NAME ": no source files found\n");
			/* NOTREACHED */
		}
		/* get include directories from the environment */
		if((s = getenv("INCLUDEDIRS")) != NULL) { includedir(s); }
		/* add /usr/include to the #include directory list,
		   but not in kernelmode... kernels tend not to use it. */
		if(kernelmode == false) {
			if(NULL != (s = getenv("INCDIR"))) {
				includedir(s);
			} else {
				includedir(DFLT_INCDIR);
			}
		}

		/* initialize the C keyword table */
		initsymtab();

		/* Tell build.c about the filenames to create: */
		setup_build_filenames(reffile);

		/* build the cross-reference */
		initcompress();
		if(linemode == false || verbosemode == true) { /* display if verbose as well */
			postmsg("Building cross-reference...");
		}
		build();
		if(linemode == false) { postmsg(""); /* clear any build progress message */ }
		if(buildonly == true) {
			myexit(0);
			/* NOTREACHED */
		}
	}
	opendatabase();

	/* if using the line oriented user interface so cscope can be a
	   subprocess to emacs or samuel */
	if(linemode == true) { linemode_event_loop(); }
	/* pause before clearing the screen if there have been error messages */
	if(errorsfound == true) {
		errorsfound = false;
		askforreturn();
	}
	/* do any optional search */
	if(*input_line != '\0') {
		// command(ctrl('Y'));    /* search */	// XXX: fix
	} else if(reflines != NULL) {
		/* read any symbol reference lines file */
		readrefs(reflines);
	}

	screenmode_event_loop();
	/* cleanup and exit */
	myexit(0);
	/* NOTREACHED */
	return 0; /* avoid warning... */
}
