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
 *    global type, data, and function definitions
 */

#ifndef CSCOPE_GLOBAL_H
#define CSCOPE_GLOBAL_H

#define UNUSED(x) (void)(x)

#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <assert.h>
#include <ctype.h>	/* isalpha, isdigit, etc. */
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#include <string.h>	   /* string functions */

#include "constants.h" /* misc. constants */
#include "invlib.h"	   /* inverted index library */

extern FILE * _myfopen(const char * path, const char * mode);
extern FILE * _mypopen(char * cmd, char * mode);
extern int    _myopen(const char * path, int flag, int mode);
extern int    _mypclose(FILE * ptr);

#define open(...)   _myopen(__VA_ARGS__)
#define fopen(...)  _myfopen(__VA_ARGS__)
#define popen(...)  _mypopen(__VA_ARGS__)
#define pclose(...) _mypclose(__VA_ARGS__)

#define EMPTY()
#define QUOTE(id) id EMPTY()

typedef void (*sighandler_t)(int); // XXX: this got copied to multiple places; investigate

enum {
	INPUT_NORMAL,
	INPUT_APPEND,
	INPUT_PIPE,
	INPUT_READ,
	INPUT_CHANGE_TO,
	INPUT_CHANGE
};
extern int input_mode;

extern const char * incdir;

/* digraph data for text compression */
extern char dichar1[]; /* 16 most frequent first chars */
extern char dichar2[]; /* 8 most frequent second chars using the above as first chars */
extern char dicode1[]; /* digraph first character code */
extern char dicode2[]; /* digraph second character code */

/* and some macros to help using dicodes: */
/* Check if a given pair of chars is compressable as a dicode: */
#define IS_A_DICODE(inchar1, inchar2)                                                    \
	(dicode1[(unsigned char)(inchar1)] && dicode2[(unsigned char)(inchar2)])
/* Combine the pair into a dicode */
#define DICODE_COMPRESS(inchar1, inchar2)                                                \
	((0200 - 2) + dicode1[(unsigned char)(inchar1)] + dicode2[(unsigned char)(inchar2)])

/* main.c global data */
extern char *editor, /* EDITOR env var */
	*home,           /* Home directory */
	*shell,          /* Users shell */
	*lineflag;       /* Flag used to open EDITOR at a specified line */

extern bool			lineflagafterfile;
extern bool			compress;		/* compress the characters in the crossref */
extern int			dispcomponents; /* file path components to display */
extern bool			invertedindex;	/* the database has an inverted index */
extern bool			preserve_database;		/* consider the crossref up-to-date */
extern bool			kernelmode;		/* don't use DEFAULT_INCLUDE_DIRECTORY - bad for kernels */
extern bool			linemode;		/* use line oriented user interface */
extern bool			verbosemode;	/* print extra information on line mode */
extern bool			recurse_dir;	/* recurse dirs when searching for src files */
extern char		   *namefile;		/* file of file names */
extern char		   *prependpath;	/* prepend path to file names */
extern FILE		   *refsfound;		/* references found file */
extern long			totalterms;		/* total inverted index terms */
extern char			tempstring[TEMPSTRING_LEN + 1]; /* global dummy string buffer */

extern char	 temp1[];								/* temporary file name */
extern char	 temp2[];								/* temporary file name */

/* command.c global data */
extern bool			caseless; /* ignore letter case when searching */
extern bool		   *change;	  /* change this line */
extern unsigned int curdispline;
extern char			newpat[]; /* new pattern */

/* crossref.c global data */
extern long			 dboffset;	  /* new database offset */
extern bool			 errorsfound; /* prompt before clearing error messages */
extern long			 lineoffset;  /* source line database offset */
extern long			 npostings;	  /* number of postings */
extern unsigned long symbols;	  /* number of symbols */

/* dir.c global data */
extern char	  currentdir[]; /* current directory */
extern char **incdirs;		/* #include directories */
extern char **srcdirs;		/* source directories */
extern char **srcfiles;		/* source files */
extern size_t nincdirs;		/* number of #include directories */
extern size_t nsrcdirs;		/* number of source directories */
extern size_t nsrcfiles;	/* number of source files */
extern size_t msrcfiles;	/* maximum number of source files */

/* display.c global data */
extern int			filelen;	  /* file name display field length */
extern int			fcnlen;		  /* function name display field length */
extern int			numlen;		  /* line number display field length */
extern unsigned int disprefs;	  /* displayed references */
extern int			field;		  /* input field */
extern unsigned int mdisprefs;	  /* maximum displayed references */
extern unsigned int nextline;	  /* next line to be shown */
extern long			searchcount;  /* count of files searched */
extern unsigned int totallines;	  /* total reference lines */

/* find.c global data */
extern char	 block[];	  /* cross-reference file block */
extern char	 blockmark;	  /* mark character to be searched for */
extern long	 blocknumber; /* block number */
extern char *blockp;	  /* pointer to current character in block */
extern int	 blocklen;	  /* length of disk block read */

/* lookup.c global data */
extern struct keystruct {
	const char * text;
	const char delim;
	struct keystruct *next;
} keyword[];

/* cscope functions called from more than one function or between files */

char *inviewpath(const char *file);
char *lookup(char *ident, bool do_compressed);
char *read_crossreference_block(void);
char *scanpast(char c);

char **parse_options(const int argc, const char * const * const argv);
void readenv(bool preserve_database);

extern bool	 remove_symfile_onexit;
extern bool	 onesearch;		   /* one search only in line mode */
extern char *reflines;		   /* symbol reference lines file */
extern bool	 do_press_any_key; /* wait for any key to continue */
extern int	 current_page;
#define topref (current_page * mdisprefs)
int		   handle_input(const int c);
int		   dispchar2int(const char c);
int		   changestring(const char *from, const char *to, const bool *const change, const int change_len);

void init_temp_files(void);
void deinit_temp_files(void);

void addsrcfile(char *path);
void askforchar(void);
void askforreturn(void);
void cannotwrite(const char *const file);
void cannotopen(const char *const file);
void countrefs(void);
void crossref(char *srcfile);
int  edit(const char * filename, const char *const linenum);
void editall(void);
void editref(int);
void force_window(void);
void findcleanup(void);
void freesrclist(void);
void freeinclist(void);
void freecrossref(void);
void freefilelist(void);
void incfile(char *file, char *type);
void includedir(const char *dirname);
void initsymtab(void);
void makefilelist(const char * const * const fileargv);
void myexit(int sig);
void myperror(char *text);
void progress(char *what, long current, long max);
void putfilename(char *srcfile);
void postmsg(char *msg);
void postmsg2(char *msg);
void posterr(char *msg, ...);
void postfatal(const char *msg, ...);
void putposting(char *term, int type);
void fetch_string_from_dbase(char *, size_t);
void sourcedir(const char * dirlist);
void myungetch(int c);
void parse_warning(char *text);
void writestring(char *s);

bool infilelist(const char * file);
bool readrefs(char *filename);
bool search(const char *query);
bool writerefsfound(void);

int	findinit(const char *pattern_);
int	 hash(const char * ss);

char	   *egrepinit(const char *egreppat);
void		egrepcaseless(int i);

#endif /* CSCOPE_GLOBAL_H */
