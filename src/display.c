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

/*	cscope - interactive C symbol cross-reference
 *
 *	display functions
 */

#include "global.h"
#include "build.h"

#ifdef CCS
#include "sgs.h"	/* ESG_PKG and ESG_REL */
#else
#include "version.h"	/* FILEVERSION and FIXVERSION */
#endif

#include <ncurses.h>
#include <setjmp.h>	/* jmp_buf */
#include <stdarg.h>	/* va_list stuff */
#include <time.h>
#include <errno.h>
#include <stdarg.h>

int	booklen;		/* OGS book name display field length */
int	*displine;		/* screen line of displayed reference */
unsigned int disprefs;		/* displayed references */
int	field;			/* input field */
int	filelen;		/* file name display field length */
int	fcnlen;			/* function name display field length */
unsigned int mdisprefs;		/* maximum displayed references */
unsigned int nextline;		/* next line to be shown */
FILE	*nonglobalrefs;		/* non-global references file */
int	numlen;			/* line number display field length */
unsigned int topline = 1;		/* top line of page */
int	bottomline;		/* bottom line of page */
long	searchcount;		/* count of files searched */
int	subsystemlen;		/* OGS subsystem name display field length */
unsigned int totallines;	/* total reference lines */
unsigned fldcolumn;		/* input field column */
WINDOW* body;
WINDOW* input_fields;

static enum {
	CH_BODY = 0x0001,
	CH_INPUT_FIELDS = CH_BODY << 1,
	CH_COMMAND_FIELD = CH_BODY << 2,
	CH_ALL = CH_BODY | CH_INPUT_FIELDS | CH_COMMAND_FIELD
};

const char	dispchars[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

static	int	fldline;		/* input field line */
static	sigjmp_buf	env;		/* setjmp/longjmp buffer */
static	int	lastdispline;		/* last displayed reference line */
static	char	lastmsg[MSGLEN + 1];	/* last message displayed */
static	const char	helpstring[] = "Press the ? key for help";
static	const char	selprompt[] = 
	"Select lines to change (press the ? key for help): ";

typedef char * (*FP)(char *);	/* pointer to function returning a character pointer */

/* HBB 2000/05/05: I removed the casts to function pointer type. It is
 * fundamentally unsafe to call a function through a pointer of a
 * different type ('undefined behaviour' in the words of the ANSI/ISO
 * C standard).  Instead, I made all the find...() functions adhere to
 * the same function type, by changing argument passing a bit. */
static	struct	{		/* text of input fields */
	char	*text1;
	char	*text2;
	FP	findfcn;
} fields[FIELDS + 1] = {	/* samuel has a search that is not part of the cscope display */
	{"Find this", "C symbol",			findsymbol},
	{"Find this", "global definition",		finddef},
	{"Find", "functions called by this function",	findcalledby},
	{"Find", "functions calling this function",	findcalling},
	{"Find this", "text string",			findstring},
	{"Change this", "text string",			findstring},
	{"Find this", "egrep pattern",			findregexp},
	{"Find this", "file",				findfile},
	{"Find", "files #including this file",		findinclude},
	{"Find", "assignments to this symbol", 		findassign},
	{"Find all", "function definitions",		findallfcns},	/* samuel only */
};

/* Internal prototypes: */
static	void	jumpback(int sig);

/* initialize display parameters */

void
dispinit(void)
{
	/* initialize the curses display package */
	initscr();	/* initialize the screen */
	entercurses();
	keypad(stdscr, TRUE);	/* enable the keypad */
	//fixkeypad();	/* fix for getch() intermittently returning garbage */
	standend();	/* turn off reverse video */

	/* calculate the maximum displayed reference lines */
	lastdispline = FLDLINE - 3;
	mdisprefs = lastdispline - REFLINE + 1;


	if (mdisprefs <= 0) {
		postfatal("%s: screen too small\n", argv0);
		/* NOTREACHED */
	}

	if (mouse == NO && mdisprefs > strlen(dispchars))
		mdisprefs = strlen(dispchars);

	/* allocate the displayed line array */
	displine = malloc(mdisprefs * sizeof(*displine));

	/* initialize windows */
	body = newwin(LINES-2-FIELDS, COLS-2, 1, 1);
	input_fields = newwin(FIELDS, COLS-2, FLDLINE, 1);
    refresh();
}

static inline void display_frame(){

	box(stdscr, 0, 0);
	/* Title*/
	const int LEFT_PADDING = 5;
	wmove(stdscr, 0, LEFT_PADDING);
#if CCS
	if (displayversion == YES) {
	    wprintw(stdscr, "cscope %s", ESG_REL);
	}
	else {
	    waddstr(stdscr, "cscope");
	}
#else
	wprintw(stdscr, "Cscope version %d%s", FILEVERSION, FIXVERSION);
#endif
	wmove(stdscr, 0, COLS - (int) sizeof(helpstring));
	waddstr(stdscr, helpstring);
	wmove(input_fields, 0, 0);
	for(int i = 0; i < COLS-2; i++){
		waddch(input_fields, ACS_HLINE);
	}
}

static inline void display_input_fields(){
    /* display the input fields */
    wmove(input_fields, 1, 0);
    for(int i = 0; i < FIELDS; ++i){
		wprintw(input_fields, "%s %s:\n", fields[i].text1, fields[i].text2);
    }
}

static inline void display_command_field(){

}

void
display(void)
{
    char    *subsystem;             /* OGS subsystem name */
    char    *book;                  /* OGS book name */
    char    file[PATHLEN + 1];      /* file name */
    char    function[PATLEN + 1];   /* function name */
    char    linenum[NUMLEN + 1];    /* line number */
    int     screenline;             /* screen line number */
    int     width;                  /* source line display width */
    int     i;
    char    *s;

    erase();
	display_frame();

    if (totallines == 0) {
	/* if no references were found */
	/* redisplay the last message */
	waddstr(body, lastmsg);
    } else {
	/* display the pattern */
	if (changing == YES) {
	    wprintw(body, "Change \"%s\" to \"%s\"", Pattern, newpat);
	} else {
	    wprintw(body, "%c%s: %s", toupper((unsigned char)fields[field].text2[0]),
		   fields[field].text2 + 1, Pattern);
	}
	/* display the column headings */
	wmove(body, 2, 2);
	if (ogs == YES && field != FILENAME) {
	    wprintw(body, "%-*s ", subsystemlen, "Subsystem");
	    wprintw(body, "%-*s ", booklen, "Book");
	}
	if (dispcomponents > 0)
	    wprintw(body, "%-*s ", filelen, "File");

	if (field == SYMBOL || field == CALLEDBY || field == CALLING) {
	    wprintw(body, "%-*s ", fcnlen, "Function");
	}
	if (field != FILENAME) {
	    waddstr(body, "Line");
	}
	waddch(body, '\n');

	/* if at end of file go back to beginning */
	if (nextline > totallines) {
	    seekline(1);
	}
	/* calculate the source text column */

	width = COLS - numlen - 3;

	if (ogs == YES) {
	    width -= subsystemlen + booklen + 2;
	}
	if (dispcomponents > 0) {
	    width -= filelen + 1;
	}
	if (field == SYMBOL || field == CALLEDBY || field == CALLING) {
	    width -= fcnlen + 1;
	}

	/* until the max references have been displayed or 
	   there is no more room */
	topline = nextline;
	for (disprefs = 0, screenline = REFLINE;
	     disprefs < mdisprefs && screenline <= lastdispline;
	     ++disprefs, ++screenline) {
	    /* read the reference line */
	    if (fscanf(refsfound, "%" PATHLEN_STR "s%" PATHLEN_STR "s%" NUMLEN_STR "s %" TEMPSTRING_LEN_STR "[^\n]", file, function, 
		       linenum, tempstring) < 4) {
		break;
	    }
	    ++nextline;
	    displine[disprefs] = screenline;
			
	    /* if no mouse, display the selection number */
	    if (mouse == YES) {
		waddch(body, ' ');
	    } else {
		wprintw(body, "%c", dispchars[disprefs]);
	    }

	    /* display any change mark */
	    if (changing == YES && 
		change[topline + disprefs - 1] == YES) {
		waddch(body, '>');
	    } else {
		waddch(body, ' ');
	    }

	    /* display the file name */
	    if (field == FILENAME) {
		wprintw(body, "%-*s ", filelen, file);
	    } else {
		/* if OGS, display the subsystem and book names */
		if (ogs == YES) {
		    ogsnames(file, &subsystem, &book);
		    wprintw(body, "%-*.*s ", subsystemlen, subsystemlen, subsystem);
		    wprintw(body, "%-*.*s ", booklen, booklen, book);
		}
		/* display the requested path components */
		if (dispcomponents > 0) {
		    wprintw(body, "%-*.*s ", filelen, filelen,
			   pathcomponents(file, dispcomponents));
		}
	    } /* else(field == FILENAME) */

	    /* display the function name */
	    if (field == SYMBOL || field == CALLEDBY || field == CALLING) {
		wprintw(body, "%-*.*s ", fcnlen, fcnlen, function);
	    }
	    if (field == FILENAME) {
		waddch(body, '\n');	/* go to next line */
		continue;
	    }

	    /* display the line number */
	    wprintw(body, "%*s ", numlen, linenum);
	    /* there may be tabs in egrep output */
	    while ((s = strchr(tempstring, '\t')) != NULL) {
		*s = ' ';
	    }

	    /* display the source line */
	    s = tempstring;
	    for (;;) {
		/* see if the source line will fit */
		if ((i = strlen(s)) > width) {
					
		    /* find the nearest blank */
		    for (i = width; s[i] != ' ' && i > 0; --i) {
			;
		    }
		    if (i == 0) {
			i = width;	/* no blank */
		    }
		}
		/* print up to this point */
		wprintw(body, "%.*s", i, s);
		s += i;
				
		/* if line didn't wrap around */
		if (i < width) {
		    waddch(body, '\n');	/* go to next line */
		}
		/* skip blanks */
		while (*s == ' ') {
		    ++s;
		}
		/* see if there is more text */
		if (*s == '\0') {
		    break;
		}
		/* if the source line is too long */
		if (++screenline > lastdispline) {

		    /* if this is the first displayed line,
		       display what will fit on the screen */
		    if (topline == nextline -1) {
			disprefs++;
			/* break out of two loops */
			goto endrefs;
		    }
					
		    /* erase the reference */
		    while (--screenline >= displine[disprefs]) {
			wmove(body, screenline, 0);
			clrtoeol();
		    }
		    ++screenline;
					 
		    /* go back to the beginning of this reference */
		    --nextline;
		    seekline(nextline);
		    goto endrefs;
		}
		/* indent the continued source line */
		wmove(body, screenline, COLS - width);
	    } /* for(ever) */
	} /* for(reference output lines) */
    endrefs:
	/* position the cursor for the message */
	i = FLDLINE - 1;
	if (screenline < i) {
	    waddch(body, '\n');
	}
	else {
	    wmove(body, i, 0);
	}
	/* check for more references */
	i = totallines - nextline + 1;
	bottomline = nextline;
	if (i > 0) {
	    wprintw(body, "* Lines %d-%d of %d, %d more - press the space bar to display more *", topline, bottomline, totallines, i);
	}
	/* if this is the last page of references */
	else if (topline > 1 && nextline > totallines) {
	    waddstr(body, "* Press the space bar to display the first lines again *");
	}
    }
    drawscrollbar(topline, nextline);	/* display the scrollbar */

	atfield();

	display_input_fields();
	display_prompt();

    refresh();
    wrefresh(body);
    wrefresh(input_fields);
}

/* set the cursor position for the field */
//void
//setfield(void)
//{
//	fldline = FLDLINE + field;
//	fldcolumn = strlen(fields[field].text1) + strlen(fields[field].text2) + 3;
//}

/* move to the current input field */
//
//void
//atfield(void)
//{
//	wmove(input_fields, fldline, fldcolumn);
//}

/* move to the changing lines prompt */

//void
//atchange(void)
//{
//	wmove(body, PRLINE, (int) sizeof(selprompt) - 1);
//}

/* search for the symbol or text pattern */

/*ARGSUSED*/
static void
jumpback(int sig)
{
	signal(sig, jumpback);
	siglongjmp(env, 1);
}

BOOL
search(void)
{
	char	*findresult = NULL;	/* find function output */
	BOOL	funcexist = YES;		/* find "function" error */
	FINDINIT rc = NOERROR;		/* findinit return code */
	sighandler_t savesig;		/* old value of signal */
	FP	f;			/* searching function */
	int	c;
	
	/* open the references found file for writing */
	if (writerefsfound() == NO) {
		return(NO);
	}
	/* find the pattern - stop on an interrupt */
	if (linemode == NO) {
		postmsg("Searching");
	}
	searchcount = 0;
	savesig = signal(SIGINT, jumpback);
	if (sigsetjmp(env, 1) == 0) {
		f = fields[field].findfcn;
		if (f == findregexp || f == findstring) {
			findresult = (*f)(Pattern);
		} else {
			if ((nonglobalrefs = myfopen(temp2, "wb")) == NULL) {
				cannotopen(temp2);
				return(NO);
			}
			if ((rc = findinit(Pattern)) == NOERROR) {
				(void) dbseek(0L); /* read the first block */
				findresult = (*f)(Pattern);
				if (f == findcalledby) 
					funcexist = (*findresult == 'y');
				findcleanup();

				/* append the non-global references */
				(void) fclose(nonglobalrefs);
				if ((nonglobalrefs = myfopen(temp2, "rb"))
				     == NULL) {
					cannotopen(temp2);
					return(NO);
				}
				while ((c = getc(nonglobalrefs)) != EOF) {
					(void) putc(c, refsfound);
				}
			}
			(void) fclose(nonglobalrefs);
		}
	}
	signal(SIGINT, savesig);

	/* rewind the cross-reference file */
	(void) lseek(symrefs, (long) 0, 0);
	
	/* reopen the references found file for reading */
	(void) fclose(refsfound);
	if ((refsfound = myfopen(temp1, "rb")) == NULL) {
		cannotopen(temp1);
		return(NO);
	}
	nextline = 1;
	totallines = 0;
	disprefs = 0;
	
	/* see if it is empty */
	if ((c = getc(refsfound)) == EOF) {
		if (findresult != NULL) {
			(void) snprintf(lastmsg, sizeof(lastmsg), "Egrep %s in this pattern: %s", 
				       findresult, Pattern);
		} else if (rc == NOTSYMBOL) {
			(void) snprintf(lastmsg, sizeof(lastmsg), "This is not a C symbol: %s", 
				       Pattern);
		} else if (rc == REGCMPERROR) {
			(void) snprintf(lastmsg, sizeof(lastmsg), "Error in this regcomp(3) regular expression: %s", 
				       Pattern);
			
		} else if (funcexist == NO) {
			(void) snprintf(lastmsg, sizeof(lastmsg), "Function definition does not exist: %s", 
				       Pattern);
		} else {
			(void) snprintf(lastmsg, sizeof(lastmsg), "Could not find the %s: %s", 
				       fields[field].text2, Pattern);
		}
		return(NO);
	}
	/* put back the character read */
	(void) ungetc(c, refsfound);

	/* HBB 20041027: this used to hold a copy of the code of 
	 * countrefs(), but with the crucial display width adjustments
	 * missing.  Just call the real thing instead! */
	countrefs();
	return(YES);
}

/* display search progress with default custom format */

void
progress(char *what, long current, long max)
{
	static	long	start;
	long	now;
	char	msg[MSGLEN + 1];
	int	i;

	/* save the start time */
	if (searchcount == 0) {
		start = time(NULL);
	}
	if ((now = time(NULL)) - start >= 1)
	{
		if (linemode == NO)
		{
			wmove(body, MSGLINE, 0);
			clrtoeol();
			waddstr(body, what);
			snprintf(msg, sizeof(msg), "%ld", current);
			wmove(body, MSGLINE, (COLS / 2) - (strlen(msg) / 2));
			waddstr(body, msg);
			snprintf(msg, sizeof(msg), "%ld", max);
			wmove(body, MSGLINE, COLS - strlen(msg));
			waddstr(body, msg);
			refresh();
		}
		else if (verbosemode == YES)
		{
			snprintf(msg, sizeof(msg), "> %s %ld of %ld", what, current, max);
		}

		start = now;
		if ((linemode == NO) && (incurses == YES))
		{
			wmove(body, MSGLINE, 0);
			i = (float)COLS * (float)current / (float)max;

			standout();
			for (; i > 0; i--)
				waddch(body, inch());
			standend();
			refresh();
		}
		else
			if (linemode == NO || verbosemode == YES)
				postmsg(msg);
	}
	++searchcount;
}

/* print error message on system call failure */

void
myperror(char *text) 
{
	char	msg[MSGLEN + 1];	/* message */
	char	*s;

    s = strerror(errno);

	(void) snprintf(msg, sizeof(msg), "%s: %s", text, s);
	postmsg(msg);
}

/* postmsg clears the message line and prints the message */

/* VARARGS */
void
postmsg(char *msg) 
{
	if (linemode == YES || incurses == NO) {
		(void) printf("%s\n", msg);
		fflush(stdout);
	}
	else {
		clearmsg();
		waddstr(body, msg);
		refresh();
	}
	(void) strncpy(lastmsg, msg, sizeof(lastmsg) - 1);
}

/* clearmsg clears the first message line */

void
clearmsg(void)
{
	if (linemode == NO) {
		wmove(body, MSGLINE, 0);
		clrtoeol();
	}
}

/* clearmsg2 clears the second message line */

void
clearmsg2(void)
{
	if (linemode == NO) {
		wmove(body, MSGLINE + 1, 0);
		clrtoeol();
	}
}

/* postmsg2 clears the second message line and prints the message */

void
postmsg2(char *msg) 
{
	if (linemode == YES) {
		(void) printf("%s\n", msg);
	}
	else {
		clearmsg2();
		waddstr(body, msg);
		refresh();
	}
}

/* display an error mesg - stdout or on second msg line */
void
posterr(char *msg, ...) 
{
    va_list ap;
    char errbuf[MSGLEN];
    
    va_start(ap, msg);
    if (linemode == YES || incurses == NO)
    {
        (void) vfprintf(stderr, msg, ap); 
	(void) fputc('\n', stderr);
    } else {
        vsnprintf(errbuf, sizeof(errbuf), msg, ap);
        postmsg2(errbuf); 
    }
    va_end(ap);
}

/* display a fatal error mesg -- stderr *after* shutting down curses */
void
postfatal(const char *msg, ...)
{
	va_list ap;
	char errbuf[MSGLEN];

	va_start(ap, msg);
	vsnprintf(errbuf, sizeof(errbuf), msg, ap);
	/* restore the terminal to its original mode */
	if (incurses == YES) {
		exitcurses();
	}

	/* display fatal error messages */
	fprintf(stderr,"%s",errbuf);

	/* shut down */
	myexit(1);
}

/* position references found file at specified line */

void
seekline(unsigned int line) 
{
	int	c;

	/* verify that there is a references found file */
	if (refsfound == NULL) {
		return;
	}
	/* go to the beginning of the file */
	rewind(refsfound);
	
	/* find the requested line */
	nextline = 1;
	while (nextline < line && (c = getc(refsfound)) != EOF) {
		if (c == '\n') {
			nextline++;
		}
	}
}

/* get the OGS subsystem and book names */

void
ogsnames(char *file, char **subsystem, char **book)
{
	static	char	buf[PATHLEN + 1];
	char	*s, *slash;

	*subsystem = *book = "";
	(void) strcpy(buf,file);
	s = buf;
	if (*s == '/') {
		++s;
	}
	while ((slash = strchr(s, '/')) != NULL) {
		*slash = '\0';
		if ((int)strlen(s) >= 3 && strncmp(slash - 3, ".ss", 3) == 0) {
			*subsystem = s;
			s = slash + 1;
			if ((slash = strchr(s, '/')) != NULL) {
				*book = s;
				*slash = '\0';
			}
			break;
		}
		s = slash + 1;
	}
}

/* get the requested path components */

char *
pathcomponents(char *path, int components)
{
	int	i;
	char	*s;
	
	s = path + strlen(path) - 1;
	for (i = 0; i < components; ++i) {
		while (s > path && *--s != '/') {
			;
		}
	}
	if (s > path && *s == '/') {
		++s;
	}
	return(s);
}

/* open the references found file for writing */

BOOL
writerefsfound(void)
{
	if (refsfound == NULL) {
		if ((refsfound = myfopen(temp1, "wb")) == NULL) {
			cannotopen(temp1);
			return(NO);
		}
	} else {
		(void) fclose(refsfound);
		if ( (refsfound = myfopen(temp1, "wb")) == NULL) {
			postmsg("Cannot reopen temporary file");
			return(NO);
		}
	}
	return(YES);
}
