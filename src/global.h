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

//#include "config.h"
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>
#include <ctype.h>    /* isalpha, isdigit, etc. */
#include <signal.h>    /* SIGINT and SIGQUIT */
#include <stdio.h>    /* standard I/O package */
#include <stdlib.h>     /* standard library functions */

#include <string.h>    /* string functions */

#include "constants.h"    /* misc. constants */
#include "invlib.h"    /* inverted index library */
#include "library.h"    /* library function return values */
#include "stddef.h"    /* size_t */

typedef void (*sighandler_t)(int);

#include <stdarg.h>

#include <fcntl.h>

#include <stdbool.h>

typedef    enum    {		/* findinit return code */
    falseERROR,
    falseTSYMBOL,
    REGCMPERROR
} FINDINIT;

typedef    struct {    	/* mouse action */
    int    button;
    int    percent;
    int    x1;
    int    y1;
    int    x2;
    int    y2;
} MOUSE;

struct cmd {        	/* command history struct */
    struct    cmd *prev, *next;	/* list ptrs */
    int    field;			/* input field number */
    char    *text;			/* input field text */
};

enum {
    CH_NONE   = 0x0000,
    CH_RESULT = 0x0001 << 0,
    CH_INPUT  = 0x0001 << 1,
    CH_MODE   = 0x0001 << 2,
    CH_HELP   = 0x0001 << 3, /* do NOT add to CH_ALL */
    CH_ALL    = CH_RESULT | CH_INPUT | CH_MODE
};
enum {
	INPUT_NORMAL,
	INPUT_APPEND,
	INPUT_PIPE,
	INPUT_READ,
	INPUT_CHANGE
};

#ifndef DFLT_INCDIR
# define DFLT_INCDIR "/usr/include"
#endif

/* digraph data for text compression */
extern    char    dichar1[];	/* 16 most frequent first chars */
extern    char    dichar2[];	/* 8 most frequent second chars
        		   using the above as first chars */
extern    char    dicode1[];	/* digraph first character code */
extern    char    dicode2[];	/* digraph second character code */

/* and some macros to help using dicodes: */
/* Check if a given pair of chars is compressable as a dicode: */
#define IS_A_DICODE(inchar1, inchar2)        			   \
  (dicode1[(unsigned char)(inchar1)] && dicode2[(unsigned char)(inchar2)])
/* Combine the pair into a dicode */
#define DICODE_COMPRESS(inchar1, inchar2)        \
  ((0200 - 2) + dicode1[(unsigned char)(inchar1)]    \
   + dicode2[(unsigned char)(inchar2)])

/* main.c global data */
extern    char    *editor, *home, *shell, *lineflag;	/* environment variables */
extern    char    *home;		/* Home directory */
extern     bool    lineflagafterfile;
extern    char    *argv0;		/* command name */
extern    bool    compress;	/* compress the characters in the crossref */
extern    bool    dbtruncated;	/* database symbols truncated to 8 chars */
extern    int    dispcomponents;	/* file path components to display */
#if CCS
extern    bool    displayversion;	/* display the C Compilation System version */
#endif
extern    bool    editallprompt;	/* prompt between editing files */
extern    unsigned int fileargc;    /* file argument count */
extern    char    **fileargv;	/* file argument values */
extern    int    fileversion;	/* cross-reference file version */
extern    bool    incurses;	/* in curses */
extern    bool    invertedindex;	/* the database has an inverted index */
extern    bool    isuptodate;	/* consider the crossref up-to-date */
extern    bool    kernelmode;	/* don't use DFLT_INCDIR - bad for kernels */
extern    bool    linemode;	/* use line oriented user interface */
extern    bool    verbosemode;	/* print extra information on line mode */
extern    bool    recurse_dir;	/* recurse dirs when searching for src files */
extern    char    *namefile;	/* file of file names */
extern    bool    ogs;		/* display OGS book and subsystem names */
extern    char    *prependpath;	/* prepend path to file names */
extern    FILE    *refsfound;	/* references found file */
extern    char    temp1[];	/* temporary file name */
extern    char    temp2[];	/* temporary file name */
extern    long    totalterms;	/* total inverted index terms */
extern    bool    trun_syms;	/* truncate symbols to 8 characters */
extern    char    tempstring[TEMPSTRING_LEN + 1]; /* global dummy string buffer */
extern    char    *tmpdir;	/* temporary directory */

/* command.c global data */
extern    bool    caseless;	/* ignore letter case when searching */
extern    bool    *change;	/* change this line */
extern    bool    changing;	/* changing text */
extern    unsigned int curdispline;
extern    char    newpat[];	/* new pattern */

/* crossref.c global data */
extern    long    dboffset;	/* new database offset */
extern    bool    errorsfound;	/* prompt before clearing error messages */
extern    long    lineoffset;	/* source line database offset */
extern    long    npostings;	/* number of postings */
extern    unsigned long symbols;    /* number of symbols */

/* dir.c global data */
extern    char    currentdir[];	/* current directory */
extern    char    **incdirs;	/* #include directories */
extern    char    **srcdirs;	/* source directories */
extern    char    **srcfiles;	/* source files */
extern    unsigned long nincdirs;    /* number of #include directories */
extern    unsigned long nsrcdirs;    /* number of source directories */
extern    unsigned long nsrcfiles; /* number of source files */
extern    unsigned long msrcfiles; /* maximum number of source files */

/* display.c global data */
extern    int subsystemlen;    		/* OGS subsystem name display field length */
extern     int booklen;    			/* OGS book name display field length */
extern    int filelen;    			/* file name display field length */
extern    int fcnlen;    				/* function name display field length */
extern    int numlen;    				/* line number display field length */
extern    int    *displine;				/* screen line of displayed reference */
extern    unsigned int disprefs;    	/* displayed references */
extern    int    field;					/* input field */
extern    unsigned int mdisprefs;    	/* maximum displayed references */
extern    unsigned int nextline;    	/* next line to be shown */
extern    long    searchcount;		/* count of files searched */
extern    unsigned int totallines;    /* total reference lines */
extern    int window_change;		/* bitmask type to mark which windows have to be rerendered by display() */

/* find.c global data */
extern    char    block[];	/* cross-reference file block */
extern    char    blockmark;	/* mark character to be searched for */
extern    long    blocknumber;	/* block number */
extern    char    *blockp;	/* pointer to current character in block */
extern    int    blocklen;	/* length of disk block read */

/* lookup.c global data */
extern    struct    keystruct {
    char    *text;
    char    delim;
    struct    keystruct *next;
} keyword[];

/* mouse.c global data */
extern    bool    mouse;		/* mouse interface */

/* readline.c global data */
extern char* rl_line_buffer;
extern char input_line[PATLEN + 1];
extern int rl_point;

#if UNIXPC
extern    bool    unixpcmouse;		/* UNIX PC mouse interface */
#endif

/* cscope functions called from more than one function or between files */

char    *filepath(char *file);
char    *findcalledby(char *pattern);
char    *findcalling(char *pattern);
char    *findallfcns(char *dummy);
char    *finddef(char *pattern);
char    *findfile(char *dummy);
char    *findinclude(char *pattern);
char    *findsymbol(char *pattern);
char    *findassign(char *pattern);
char    *findregexp(char *egreppat);
char    *findstring(char *pattern);
char    *inviewpath(char *file);
char    *lookup(char *ident);
char    *pathcomponents(char *path, int components);
char    *read_block(void);
char    *scanpast(char c);

char ** parse_options(int *argc, char **argv);
void    error_usage(void);
void    longusage(void);
void    usage(void);
extern bool    remove_symfile_onexit;
extern bool    onesearch;    	 /* one search only in line mode */
extern char    *reflines;    	 /* symbol reference lines file */
extern bool    do_press_any_key; /* wait for any key to continue */
extern int     current_page;
void	verswp_field(void);
void	horswp_field(void);
bool	interpret(int c);    // XXX: probably rename
int		handle_input(const int c);
int		dispchar2int(const char c);
int process_mouse();
extern int input_mode;
int changestring(bool *change);

long	seekpage(size_t i);
long	seekrelline(unsigned i);
void	PCS_reset(void);

void    rlinit(void);

void    addcmd(int f, char *s);
void    addsrcfile(char *path);
void    askforchar(void);
void    askforreturn(void);
void    cannotwrite(char *file);
void    cannotopen(char *file);
void    clearmsg(void);
void    clearmsg2(void);
void    countrefs(void);
void    crossref(char *srcfile);
void    dispinit(void);
void    display(void);
void    drawscrollbar(int top, int bot);
void    edit(char *file, char *linenum);
void    editall(void);
void    editref(int);
void    entercurses(void);
void    exitcurses(void);
void    findcleanup(void);
void    freesrclist(void);
void    freeinclist(void);
void    freecrossref(void);
void    freefilelist(void);
const char* help(void);
void    incfile(char *file, char *type);
void    includedir(char *_dirname);
void    initsymtab(void);
void    makefilelist(void);
void    mousecleanup(void);
void    mousemenu(void);
void    mouseinit(void);
void    mousereinit(void);
void    myexit(int sig);
void    myperror(char *text);
void    ogsnames(char *file, char **subsystem, char **book);
void    progress(char *what, long current, long max);
void    putfilename(char *srcfile);
void    postmsg(char *msg);
void    postmsg2(char *msg);
void    posterr(char *msg,...);
void    postfatal(const char *msg,...);
void    putposting(char *term, int type);
void    fetch_string_from_dbase(char *, size_t);
void    resetcmd(void);
void    shellpath(char *out, int limit, char *in);
void    sourcedir(char *dirlist);
void    myungetch(int c);
void    warning(char *text);
void    writestring(char *s);

bool    infilelist(char *file);
bool    readrefs(char *filename);
bool    search(void);
bool    writerefsfound(void);

FINDINIT findinit(char *pattern);
MOUSE    *getmouseaction(char leading_char);
struct    cmd *currentcmd(void);
struct    cmd *prevcmd(void);
struct    cmd *nextcmd(void);

int    egrep(char *file, FILE *output, char *format);
int    mygetline(char p[], char s[], unsigned size, int firstchar, bool iscaseless);
int    hash(char *ss);
int    execute(char *a, ...);
long    dbseek(long offset);


#endif /* CSCOPE_GLOBAL_H */
