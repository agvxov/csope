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
 IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
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

#include "build.h"

#include "global.h" /* FIXME: get rid of this! */

#include "library.h"

#include "scanner.h"
#include "version.inc"
#include "vpath.h"

# include <ncurses.h>

/* Exported variables: */
bool buildonly	   = false; /* only build the database */
bool unconditional = false; /* unconditionally build database */
bool fileschanged;			/* assume some files changed */

/* variable copies of the master strings... */
char  invname_buf[] = INVNAME;
char  invpost_buf[] = INVPOST;
char  reffile_buf[] = REFFILE;
char *invname		= invname_buf; /* inverted index to the database */
char *invpost		= invpost_buf; /* inverted index postings */
char *reffile		= reffile_buf; /* cross-reference file path name */

char *newreffile;				   /* new cross-reference file name */
FILE *newrefs;					   /* new cross-reference */
FILE *postings;					   /* new inverted index postings */
int	  symrefs = -1;				   /* cross-reference file */

INVCONTROL invcontrol;			   /* inverted file control structure */

char temp1[PATHLEN + 1];		/* temporary file name */
char temp2[PATHLEN + 1];		/* temporary file name */


/* Local variables: */
static char *newinvname;	/* new inverted index file name */
static char *newinvpost;	/* new inverted index postings file name */
static long	 traileroffset; /* file trailer offset */


/* Internal prototypes: */
static void	 cannotindex(void);
static int	 compare(const void *s1, const void *s2);
static void	 copydata(void);
static void	 copyinverted(void);
static char *getoldfile(void);
static void	 movefile(char *new, char *old);
static void	 putheader(char *dir);
static void	 fetch_include_from_dbase(char *, size_t);
static void	 putlist(char **names, int count);
static bool	 samelist(FILE *oldrefs, char **names, int count);

/* Error handling routine if inverted index creation fails */
static void cannotindex(void) {
	invertedindex = false;
	errorsfound	  = true;

	fprintf(stderr, PROGRAM_NAME ": cannot create inverted index; ignoring -q option\n");

	unlink(newinvname);
	unlink(newinvpost);
	fprintf(stderr, PROGRAM_NAME ": removed files %s and %s\n", newinvname, newinvpost);
}

/* see if the name list is the same in the cross-reference file */
static bool samelist(FILE *oldrefs, char **names, int count) {
	char oldname[PATHLEN + 1]; /* name in old cross-reference */
	int	 oldcount;
	int	 i;

	/* see if the number of names is the same */
	if(fscanf(oldrefs, "%d", &oldcount) != 1 || oldcount != count) { return false; }
	/* see if the name list is the same */
	for(i = 0; i < count; ++i) {
		if((1 != fscanf(oldrefs, " %[^\n]", oldname))
        || strnotequal(oldname, names[i])) {
			return false;
		}
	}
	return true;
}

/* create the file name(s) used for a new cross-referene */
void setup_build_filenames(const char * const reffile) {
	char path[strlen(reffile)+10]; /* file pathname */
	strcpy(path, reffile);

	char * s = (char*)basename(path);	// this is safe because the returned pointer is inside $path, and we know we can edit it
	*s = '\0';
	strcat(path, "n"); // XXX: prefix signalling "new"
	++s;

	strcpy(s, basename(reffile));
	newreffile = strdup(path);
	strcpy(s, basename(invname));
	newinvname = strdup(path);
	strcpy(s, basename(invpost));
	newinvpost = strdup(path);
}

/* open the database */
void opendatabase(const char * const reffile) {
	if((symrefs = vpopen(reffile, O_BINARY | O_RDONLY)) == -1) {
		cannotopen(reffile);
		myexit(1);
	}
	blocknumber = -1; /* force next seek to read the first block */

	/* open any inverted index */
	if (invertedindex == true
    &&  invopen(&invcontrol, invname, invpost, INVAVAIL) == -1) {
		askforreturn(); /* so user sees message */
		invertedindex = false;
	}
}

/* rebuild the database */
void rebuild(void) {
	close(symrefs);
	if(invertedindex == true) {
		invclose(&invcontrol);
		nsrcoffset = 0;
		npostings  = 0;
	}
	build();
	opendatabase(reffile);

	/* revert to the initial display */
	if(refsfound != NULL) {
		fclose(refsfound);
		refsfound = NULL;
	}
}

/* build the cross-reference */
void build(void) {
	unsigned long i;
	FILE		 *oldrefs;				/* old cross-reference file */
	time_t		  reftime;				/* old crossref modification time */
	char		 *file;					/* current file */
	char		 *oldfile;				/* file in old cross-reference */
	char		  newdir[PATHLEN + 1];	/* directory in new cross-reference */
	char		  olddir[PATHLEN + 1];	/* directory in old cross-reference */
	char		  oldname[PATHLEN + 1]; /* name in old cross-reference */
	unsigned long oldnum;				/* number in old cross-ref */
	struct stat	  file_status;
	unsigned long firstfile;			/* first source file in pass */
	unsigned long lastfile;				/* last source file in pass */
	int			  built	 = 0;			/* built crossref for these files */
	int			  copied = 0;			/* copied crossref for these files */
	unsigned long fileindex;			/* source file name index */
	bool		  interactive = true;	/* output progress messages */

    // XXX: find a safe way to remove this,
    //       building is cheap, $HOME moves rarely
	/* normalize the current directory relative to the home directory so
	   the cross-reference is not rebuilt when the user's login is moved */
	strcpy(newdir, currentdir);
	if(!strcmp(currentdir, home)) {
		strcpy(newdir, "$HOME");
	} else if(!strncmp(currentdir, home, strlen(home))) {
		snprintf(newdir, sizeof(newdir), "$HOME%s", currentdir + strlen(home));
	}
	/* sort the source file names (needed for rebuilding) */
	qsort(srcfiles, nsrcfiles, sizeof(*srcfiles), compare);

	/* if there is an old cross-reference and its current directory matches */
	/* or this is an unconditional build */
	if((oldrefs = vpfopen(reffile, "rb")) != NULL
    && !unconditional
    && fscanf(oldrefs, PROGRAM_NAME " %d %" PATHLEN_STR "s", &fileversion, olddir) == 2
    && (strcmp(olddir, currentdir) == 0 /* remain compatible */ || strcmp(olddir, newdir) == 0)) {
		/* get the cross-reference file's modification time */
		fstat(fileno(oldrefs), &file_status);
		reftime = file_status.st_mtime;
		if(fileversion >= 8) {
			bool oldcompress	  = true;
			bool oldinvertedindex = false;
			bool oldtruncate	  = false;

			/* see if there are options in the database */
			for(int c;;) {
				while((c = getc(oldrefs)) == ' ') { ; }
				if(c != '-') {
					ungetc(c, oldrefs);
					break;
				}
				switch(getc(oldrefs)) {
					case 'c': /* ASCII characters only */
						oldcompress = false;
						break;
					case 'q': /* quick search */
						oldinvertedindex = true;
						fscanf(oldrefs, "%ld", &totalterms);
						break;
					case 'T': /* truncate symbols to 8 characters */
						oldtruncate = true;
						break;
				}
			}
			/* check the old and new option settings */
			if(oldcompress != compress || oldtruncate != trun_syms) {
				posterr(PROGRAM_NAME
					": -c or -T option mismatch between command line and old symbol database\n");
				goto force;
			}
			if(oldinvertedindex != invertedindex) {
				posterr(PROGRAM_NAME
					": -q option mismatch between command line and old symbol database\n");
				if(invertedindex == false) {
					posterr(PROGRAM_NAME ": removed files %s and %s\n", invname, invpost);
					unlink(invname);
					unlink(invpost);
				}
				goto outofdate;
			}
			/* seek to the trailer */
			if(fscanf(oldrefs, "%ld", &traileroffset) != 1 ||
				fseek(oldrefs, traileroffset, SEEK_SET) == -1) {
				posterr(PROGRAM_NAME ": incorrect symbol database file format\n");
				goto force;
			}
		}
		/* if assuming that some files have changed */
		if(fileschanged == true) { goto outofdate; }
		/* see if the directory lists are the same */
		if(samelist(oldrefs, srcdirs, nsrcdirs) == false ||
			samelist(oldrefs, incdirs, nincdirs) == false
			/* get the old number of files */
			|| fscanf(oldrefs, "%lu", &oldnum) != 1
			/* skip the string space size */
			|| (fileversion >= 9 && fscanf(oldrefs, "%*s") != 0)) {
			goto outofdate;
		}
		/* see if the list of source files is the same and
		   none have been changed up to the included files */
		for(i = 0; i < nsrcfiles; ++i) {
			if((1 != fscanf(oldrefs, " %[^\n]", oldname)) ||
				strnotequal(oldname, srcfiles[i]) ||
				(lstat(srcfiles[i], &file_status) != 0) ||
				(file_status.st_mtime > reftime)) {
				goto outofdate;
			}
		}
		/* the old cross-reference is up-to-date */
		/* so get the list of included files */
		while(i++ < oldnum && fgets(oldname, sizeof(oldname), oldrefs)) {
			addsrcfile(oldname);
		}
		fclose(oldrefs);
		return;

	outofdate:
		/* if the database format has changed, rebuild it all */
		if(fileversion != FILEVERSION) {
			fprintf(stderr,
				PROGRAM_NAME ": converting to new symbol database file format\n");
			goto force;
		}
		/* reopen the old cross-reference file for fast scanning */
		if((symrefs = vpopen(reffile, O_BINARY | O_RDONLY)) == -1) {
			postfatal(PROGRAM_NAME ": cannot open file %s\n", reffile);
			/* NOTREACHED */
		}
		/* get the first file name in the old cross-reference */
		blocknumber = -1;
		read_crossreference_block();	/* read the first cross-ref block */
		scanpast('\t'); /* skip the header */
		oldfile = getoldfile();
	} else {			/* force cross-referencing of all the source files */
	force:
		reftime = 0;
		oldfile = NULL;
	}
	/* open the new cross-reference file */
	if((newrefs = myfopen(newreffile, "wb")) == NULL) {
		postfatal(PROGRAM_NAME ": cannot open file %s\n", reffile);
		/* NOTREACHED */
	}
	if(invertedindex == true && (postings = myfopen(temp1, "wb")) == NULL) {
		cannotwrite(temp1);
		cannotindex();
	}
	putheader(newdir);
	fileversion = FILEVERSION;
	if(buildonly == true && verbosemode != true && !isatty(0)) {
		interactive = false;
	} else {
		searchcount = 0;
	}
	/* output the leading tab expected by crossref() */
	dbputc('\t');

	/* make passes through the source file list until the last level of
	   included files is processed */
	firstfile = 0;
	lastfile  = nsrcfiles;
	if(invertedindex == true) {
		srcoffset = malloc((nsrcfiles + 1u) * sizeof(*srcoffset));
	}
	for(;;) {
		progress("Building symbol database", (long)built, (long)lastfile);
		if(linemode == false) refresh();

		/* get the next source file name */
		for(fileindex = firstfile; fileindex < lastfile; ++fileindex) {

			/* display the progress about every three seconds */
			if(interactive == true && fileindex % 10 == 0) {
				progress("Building symbol database", fileindex, lastfile);
			}
			/* if the old file has been deleted get the next one */
			file = srcfiles[fileindex];
			while(oldfile != NULL && strcmp(file, oldfile) > 0) {
				oldfile = getoldfile();
			}
			/* if there isn't an old database or this is a new file */
			if(oldfile == NULL || strcmp(file, oldfile) < 0) {
				crossref(file);
				++built;
			} else if(lstat(file, &file_status) == 0 && file_status.st_mtime > reftime) {
				/* if this file was modified */
				crossref(file);
				++built;

				/* skip its old crossref so modifying the last source
				 * file does not cause all included files to be built.
				 * Unfortunately a new file that is alphabetically
				 * last will cause all included files to be build, but
				 * this is less likely */
				oldfile = getoldfile();
			} else {
				/* copy its cross-reference */
				putfilename(file);
				if(invertedindex == true) {
					copyinverted();
				} else {
					copydata();
				}
				++copied;
				oldfile = getoldfile();
			}
		}
		/* see if any included files were found */
		if(lastfile == nsrcfiles) { break; }
		firstfile = lastfile;
		lastfile  = nsrcfiles;
		if(invertedindex == true) {
			srcoffset = realloc(srcoffset, (nsrcfiles + 1) * sizeof(*srcoffset));
		}
		/* sort the included file names */
		qsort(srcfiles + firstfile, lastfile - firstfile, sizeof(*srcfiles), compare);
	}
	/* add a null file name to the trailing tab */
	putfilename("");
	dbputc('\n');

	/* get the file trailer offset */
	traileroffset = dboffset;

	/* output the source and include directory and file lists */
	putlist(srcdirs, nsrcdirs);
	putlist(incdirs, nincdirs);
	putlist(srcfiles, nsrcfiles);
	if(fflush(newrefs) == EOF) {
		/* rewind doesn't check for write failure */
		cannotwrite(newreffile);
		/* NOTREACHED */
	}

	/* create the inverted index if requested */
	if(invertedindex == true) {
		char sortcommand[PATHLEN + 1];

		if(fflush(postings) == EOF) {
			cannotwrite(temp1);
			/* NOTREACHED */
		}
		fstat(fileno(postings), &file_status);
		fclose(postings);
		snprintf(sortcommand,
			sizeof(sortcommand),
			"env LC_ALL=C sort -T %s %s",
			tmpdir,
			temp1);
		if((postings = mypopen(sortcommand, "r")) == NULL) {
			fprintf(stderr, PROGRAM_NAME ": cannot open pipe to sort command\n");
			cannotindex();
		} else {
			if((totalterms = invmake(newinvname, newinvpost, postings)) > 0) {
				movefile(newinvname, invname);
				movefile(newinvpost, invpost);
			} else {
				cannotindex();
			}
			mypclose(postings);
		}
		unlink(temp1);
		free(srcoffset);
	}
	/* rewrite the header with the trailer offset and final option list */
	rewind(newrefs);
	putheader(newdir);
	fclose(newrefs);

	/* close the old database file */
	if(symrefs >= 0) { close(symrefs); }
	if(oldrefs != NULL) { fclose(oldrefs); }
	/* replace it with the new database file */
	movefile(newreffile, reffile);
}

/* string comparison function for qsort */
static
int compare(const void *arg_s1, const void *arg_s2) {
	const char **s1 = (const char **)arg_s1;
	const char **s2 = (const char **)arg_s2;

	return (strcmp(*s1, *s2));
}

/* seek to the trailer, in a given file */
void seek_to_trailer(FILE *f) {
	if(fscanf(f, "%ld", &traileroffset) != 1) {
		postfatal(PROGRAM_NAME ": cannot read trailer offset from file %s\n", reffile);
		/* NOTREACHED */
	}
	if(fseek(f, traileroffset, SEEK_SET) == -1) {
		postfatal(PROGRAM_NAME ": cannot seek to trailer in file %s\n", reffile);
		/* NOTREACHED */
	}
}

/* get the next file name in the old cross-reference */
static
char *getoldfile(void) {
	static char file[PATHLEN + 1]; /* file name in old crossref */

	if(blockp != NULL) {
		do {
			if(*blockp == NEWFILE) {
				skiprefchar();
				fetch_string_from_dbase(file, sizeof(file));
				if(file[0] != '\0') { /* if not end-of-crossref */
					return file;
				}
				return NULL;
			}
		} while(scanpast('\t') != NULL);
	}

	return NULL;
}

/* Free all storage allocated for filenames: */
void free_newbuildfiles(void) {
	free(newinvname);
	free(newinvpost);
	free(newreffile);
}

/* output the cscope version, current directory, database format options, and
   the database trailer offset */
static
void putheader(char *dir) {
	dboffset = fprintf(newrefs, PROGRAM_NAME " %d %s", FILEVERSION, dir);
	if(compress == false) { dboffset += fprintf(newrefs, " -c"); }
	if(invertedindex == true) {
		dboffset += fprintf(newrefs, " -q %.10ld", totalterms);
	} else {
		/* leave space so if the header is overwritten without -q
		 * because writing the inverted index failed, the header
		 * is the same length */
		dboffset += fprintf(newrefs, "              ");
	}
	if(trun_syms == true) { dboffset += fprintf(newrefs, " -T"); }

	dboffset += fprintf(newrefs, " %.10ld\n", traileroffset);
}

/* put the name list into the cross-reference file */
static
void putlist(char **names, int count) {
	int size = 0;

	fprintf(newrefs, "%d\n", count);
	if(names == srcfiles) {
		/* calculate the string space needed */
		for(int i = 0; i < count; ++i) {
			size += strlen(names[i]) + 1;
		}
		fprintf(newrefs, "%d\n", size);
	}
	for(int i = 0; i < count; i++) {
		if(fputs(names[i], newrefs) == EOF
        || putc('\n', newrefs) == EOF) {
			cannotwrite(newreffile);
			/* NOTREACHED */
		}
	}
}

/* copy this file's symbol data */
static
void copydata(void) {
	char *cp;

	setmark('\t');
	cp = blockp;
	for(;;) {
		/* copy up to the next \t */
		do { /* innermost loop optimized to only one test */
			while(*cp != '\t') {
				dbputc(*cp++);
			}
		} while(*++cp == '\0' && (cp = read_crossreference_block()) != NULL);
		dbputc('\t'); /* copy the tab */

		/* get the next character */
		/* HBB 2010-08-21: potential problem if above loop was left
		 * with cp==NULL */
		if(cp && (*(cp + 1) == '\0')) { cp = read_crossreference_block(); }
		/* exit if at the end of this file's data */
		if(cp == NULL || *cp == NEWFILE) { break; }
		/* look for an #included file */
		if(*cp == INCLUDE) {
			char symbol[PATLEN + 1];
			blockp = cp;
			fetch_include_from_dbase(symbol, sizeof(symbol));
			writestring(symbol);
			setmark('\t');
			cp = blockp;
		}
	}
	blockp = cp;
}

/* copy this file's symbol data and output the inverted index postings */
static
void copyinverted(void) {
	char *cp;
	char  c;
	int	  type; /* reference type (mark character) */
	char  symbol[PATLEN + 1];

	/* note: this code was expanded in-line for speed */
	/* while (scanpast('\n') != NULL) { */
	/* other macros were replaced by code using cp instead of blockp */
	cp = blockp;
	for(;;) {
		setmark('\n');
		do { /* innermost loop optimized to only one test */
			while(*cp != '\n') {
				dbputc(*cp++);
			}
		} while(*++cp == '\0' && (cp = read_crossreference_block()) != NULL);
		dbputc('\n'); /* copy the newline */

		/* get the next character */
		/* HBB 2010-08-21: potential problem if above loop was left
		 * with cp==NULL */
		if(cp && (*(cp + 1) == '\0')) { cp = read_crossreference_block(); }
		/* exit if at the end of this file's data */
		if(cp == NULL) { break; }
		switch(*cp) {
			case '\n':
				lineoffset = dboffset + 1;
				continue;
			case '\t':
				dbputc('\t');
				blockp = cp;
				type   = getrefchar();
				switch(type) {
					case NEWFILE: /* file name */
						return;
					case INCLUDE: /* #included file */
						fetch_include_from_dbase(symbol, sizeof(symbol));
						goto output;
				}
				dbputc(type);
				skiprefchar();
				fetch_string_from_dbase(symbol, sizeof(symbol));
				goto output;
		}
		c = *cp;
		if(c & 0200) { /* digraph char? */
			c = dichar1[(c & 0177) / 8];
		}
		/* if this is a symbol */
		if(isalpha((unsigned char)c) || c == '_') {
			blockp = cp;
			fetch_string_from_dbase(symbol, sizeof(symbol));
			type = ' ';
		output:
			putposting(symbol, type);
			writestring(symbol);
			if(blockp == NULL) { return; }
			cp = blockp;
		}
	}
	blockp = cp;
}

/* replace the old file with the new file */
static
void movefile(char *new, char *old) {
	unlink(old);
	if(rename(new, old) == -1) {
		myperror(PROGRAM_NAME);
		postfatal(PROGRAM_NAME ": cannot rename file %s to file %s\n", new, old);
		/* NOTREACHED */
	}
}

/* process the #included file in the old database */
static
void fetch_include_from_dbase(char *s, size_t length) {
	dbputc(INCLUDE);
	skiprefchar();
	fetch_string_from_dbase(s, length);
	incfile(s + 1, s);
}

// -----------

static char tempdirpv[PATHLEN + 1];	/* private temp directory */

void init_temp_files(void) {
	/* make sure that tmpdir exists */
	struct stat	stat_buf;
	if(lstat(tmpdir, &stat_buf)) {
        postfatal(
            PROGRAM_NAME ": Temporary directory %s does not exist or cannot be accessed\n"
            PROGRAM_NAME ": Please create the directory or set the environment variable\n"
            PROGRAM_NAME ": TMPDIR to a valid directory\n",
			tmpdir
        );
	}

	/* create the temporary file names */
	mode_t orig_umask = umask(S_IRWXG | S_IRWXO);
	pid_t pid		  = getpid();
	snprintf(tempdirpv, sizeof(tempdirpv), "%s/" PROGRAM_NAME ".%d", tmpdir, pid);
	if(mkdir(tempdirpv, S_IRWXU)) {
        postfatal(
            PROGRAM_NAME ": Could not create private temp dir %s\n",
			tempdirpv
        );
	}
	umask(orig_umask);

	snprintf(temp1, sizeof(temp1), "%s/" PROGRAM_NAME ".1", tempdirpv);
	snprintf(temp2, sizeof(temp2), "%s/" PROGRAM_NAME ".2", tempdirpv);
}

void deinit_temp_files(void) {
	/* remove any temporary files */
	if(temp1[0] != '\0') {
		unlink(temp1);
		unlink(temp2);
		rmdir(tempdirpv);
	}
}
