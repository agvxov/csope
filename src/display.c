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
 *    display functions
 */

#include "global.h"
#include "build.h"

#ifdef CCS
#include "sgs.h"    /* ESG_PKG and ESG_REL */
#else
#include "version.h"    /* FILEVERSION and FIXVERSION */
#endif

#include <ncurses.h>
#include <setjmp.h>    /* jmp_buf */
#include <stdarg.h>    /* va_list stuff */
#include <time.h>
#include <errno.h>
#include <stdarg.h>
#include <assert.h>

static    const char    appendprompt[] = "Append to file: ";
static    const char    pipeprompt[] = "Pipe to shell command: ";
static    const char    readprompt[] = "Read from file: ";
static    const char    toprompt[] = "To: ";
static    const char    inputprompt[] = "$ ";

int subsystemlen    = sizeof("Subsystem")-1;    /* OGS subsystem name display field length */
int booklen            = sizeof("Book")-1;         /* OGS book name display field length */
int filelen            = sizeof("File")-1;         /* file name display field length */
int fcnlen            = sizeof("Function")-1;     /* function name display field length */
int numlen            = 0;                        /* line number display field length */

int    *displine;        /* screen line of displayed reference */
unsigned int disprefs;        /* displayed references */
int    field;            /* input field */
unsigned int mdisprefs;        /* maximum displayed references */
unsigned int nextline;        /* next line to be shown */
FILE    *nonglobalrefs;        /* non-global references file */
unsigned int topline = 1;        /* top line of page */
static int    bottomline;        /* bottom line of page */
long    searchcount;        /* count of files searched */
unsigned int totallines;    /* total reference lines */
unsigned fldcolumn;        /* input field column */
unsigned int curdispline = 0;
int current_page = 0;

WINDOW* winput;
WINDOW* wmode;
WINDOW* wresult;
WINDOW* whelp;
WINDOW** current_window;
static WINDOW** last_window;

static int result_window_height;
static int second_col_width;
static int first_col_width;
static int input_window_height;
static int mode_window_height;

#define WRESULT_TABLE_BODY_START 4

int window_change;

/* NOTE: It's declared like this because we dont need a terminating '\00'. */
static const char dispchars[] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};
int dispchar2int(const char c){
	const char fst = dispchars[0];
	const char lst = dispchars[sizeof(dispchars)-1];
	int r = c - fst;
	if(r < 0 || lst < r){ return -1; }
	return r;
}

static    sigjmp_buf    env;        /* setjmp/longjmp buffer */
static    char    lastmsg[MSGLEN + 1];    /* last message displayed */
static    const char    helpstring[] = "Press the ? key for help";
static    const char    selprompt[] =
    "Select lines to change (press the ? key for help): ";

typedef char * (*FP)(char *);    /* pointer to function returning a character pointer */

static    struct    {        /* text of input fields */
    char    *text1;
    char    *text2;
    FP    findfcn;
} fields[FIELDS + 1] = {    /* samuel has a search that is not part of the cscope display */
    {"Find this", "C symbol",            findsymbol},
    {"Find this", "global definition",        finddef},
    {"Find", "functions called by this function",    findcalledby},
    {"Find", "functions calling this function",    findcalling},
    {"Find this", "text string",            findstring},
    {"Change this", "text string",            findstring},
    {"Find this", "egrep pattern",            findregexp},
    {"Find this", "file",                findfile},
    {"Find", "files #including this file",        findinclude},
    {"Find", "assignments to this symbol",         findassign},
    {"Find all", "function definitions",        findallfcns},    /* samuel only */
};

/* Internal prototypes: */
static    void    jumpback(int sig);

/* initialize display parameters */
void
dispinit(void)
{
    /* initialize the curses display package */
    initscr();    /* initialize the screen */
    entercurses();

    /* Calculate section sizes */
    result_window_height = LINES - 2;
    input_window_height = 1;
    mode_window_height = LINES - input_window_height - 2 - 1;
    first_col_width = 48; // (((COLS - 2)%2 == 0) ? ((COLS-2)/2) : (((COLS-2)/2)+1));
    second_col_width = COLS - 2 - 1 - first_col_width; //((COLS - 2) / 2) - 1;
    mdisprefs = result_window_height - WRESULT_TABLE_BODY_START - 1 - 1;

    if (mdisprefs <= 0) {
        postfatal("%s: screen too small\n", argv0);
        /* NOTREACHED */
    }
    if(mdisprefs > sizeof(dispchars)){
        mdisprefs = sizeof(dispchars);
    }

    /* allocate the displayed line array */
    displine = malloc(mdisprefs * sizeof(*displine));

    /* readline */
    rlinit();

    /* initialize windows */
    winput = newwin(input_window_height, first_col_width, 1, 1);
    wmode = newwin(mode_window_height, first_col_width, input_window_height+1 + 1, 1);
    wresult = newwin(result_window_height, second_col_width, 1, first_col_width + 1 + 1);
    whelp = newwin(LINES-2, COLS-2, 1, 1);
    refresh();

    current_window = &winput;
}

/* enter curses mode */
void
entercurses(void)
{
    incurses = true;
	window_change = CH_ALL;

    nonl();             /* don't translate an output \n to \n\r */
    cbreak();  	     	/* single character input */
    noecho();        	/* don't echo input characters */
    curs_set(0);
    clear();        	/* clear the screen */
    mouseinit();        /* initialize any mouse interface */
    drawscrollbar(topline, nextline);
    keypad(stdscr, TRUE);    /* enable the keypad */
    //fixkeypad();    /* fix for getch() intermittently returning garbage */
    standend();    /* turn off reverse video */
}

/* exit curses mode */
void
exitcurses(void)
{
    /* clear the bottom line */
    move(LINES - 1, 0);
    clrtoeol();
    refresh();

    /* exit curses and restore the terminal modes */
    endwin();
    incurses = false;

    /* restore the mouse */
    mousecleanup();
    fflush(stdout);
}

static inline void display_help(){
	werase(whelp);
	wmove(whelp, 0, 0);
	waddstr(whelp, help());
	wrefresh(whelp);
	do_press_any_key = true;
	window_change = CH_ALL;
}

static inline void display_frame(){
    box(stdscr, 0, 0);
    /* Vertical line */
    mvaddch(0, first_col_width + 1, ACS_TTEE);
    for(int i = 0; i < LINES-2; i++){
        mvaddch(i+1, first_col_width + 1, ACS_VLINE);
    }
    mvaddch(LINES-1, first_col_width + 1, ACS_BTEE);
    /* Horizontal line */
    wmove(stdscr, input_window_height + 1, 0);
    addch(ACS_LTEE);
    for(int i = 0; i < first_col_width; i++){
        addch(ACS_HLINE);
    }
    addch(ACS_RTEE);
    /* Title*/
    const int LEFT_PADDING = 5;
    wmove(stdscr, 0, LEFT_PADDING);
#if CCS
    if (displayversion == true) {
        wprintw(stdscr, "cscope %s", ESG_REL);
    }
    else {
        waddstr(stdscr, "cscope");
    }
#else
    wprintw(stdscr, "Cscope version %d%s", FILEVERSION, FIXVERSION);
#endif
    wmove(stdscr, 0, COLS - (int)sizeof(helpstring) - 3);
    waddstr(stdscr, helpstring);
}

static inline void display_mode(){
    for(int i = 0; i < FIELDS; ++i){
		if(i == field){
			wattron(wmode, A_REVERSE);
			mvwprintw(wmode, i, 0, "%s %s", fields[i].text1, fields[i].text2);
			wattroff(wmode, A_REVERSE);
		}else{
			mvwprintw(wmode, i, 0, "%s %s", fields[i].text1, fields[i].text2);
		}
    }
}

static inline void display_command_field(){
    mvwaddstr(winput, 0, 0, inputprompt);
    waddstr(winput, rl_line_buffer);
}
static inline void display_results(){
    int     i;
    char    *s;
    int     screenline;             /* screen line number */
    int     srctxtw;                /* source line display width */
	/* column headings */
    char    *subsystem;             /* OGS subsystem name */
    char    *book;                  /* OGS book name */
    char    file[PATHLEN + 1];      /* file name */
    char    function[PATLEN + 1];   /* function name */
    char    linenum[NUMLEN + 1];    /* line number */

    if (totallines == 0) {
        /* if no references were found */
        /* redisplay the last message */
        waddstr(wresult, lastmsg);
        return;
        /* NOTREACHED */
    }


    /* --- Display the pattern --- */
    if (changing == true) {
        wprintw(wresult, "Change \"%s\" to \"%s\"", input_line, newpat);
    } else {
        wprintw(wresult, "%c%s: %s", toupper((unsigned char)fields[field].text2[0]),
           fields[field].text2 + 1, input_line);
    }
    /* --- Display the column headings --- */
    wmove(wresult, 2, 2);
    if (ogs == true && field != FILENAME) {
        wprintw(wresult, "%-*s ", subsystemlen, "Subsystem");
        wprintw(wresult, "%-*s ", booklen, "Book");
    }
    if (dispcomponents > 0)
        wprintw(wresult, "%-*s ", filelen, "File");

    if (field == SYMBOL || field == CALLEDBY || field == CALLING) {
        wprintw(wresult, "%-*s ", fcnlen, "Function");
    }
    if (field != FILENAME) {
        waddstr(wresult, "Line");
    }

	/* --- Display table entries --- */
    wmove(wresult, WRESULT_TABLE_BODY_START, 0);

    /* calculate the source text column */
    /* NOTE: the +1s are column gaps */
    srctxtw = second_col_width;
    srctxtw -= 1+1; // dispchars
    if (ogs == true) {
        srctxtw -= subsystemlen+1 + booklen+1;
    }
    if (dispcomponents > 0) {
        srctxtw -= filelen+1;
    }
    if (field == SYMBOL || field == CALLEDBY || field == CALLING) {
        srctxtw -= fcnlen+1;
    }
    srctxtw -= numlen+1;

	/* decide where to list from */
	/* XXX: this error handling migth be redundant*/
	int seekerr;
	do{
		seekerr = seekpage(current_page);
	}while(seekerr == -1 && current_page--);

    /* until the max references have been displayed or
       there is no more room */
    topline = nextline;
    for (disprefs = 0, screenline = WRESULT_TABLE_BODY_START;
         disprefs < mdisprefs && screenline <= result_window_height;
         ++disprefs, ++screenline)
    {
        /* read the reference line */
        if (
                fscanf(refsfound, "%" PATHLEN_STR "s%" PATHLEN_STR "s%" NUMLEN_STR "s %" TEMPSTRING_LEN_STR "[^\n]",
                        file,
                        function,
                        linenum,
                        tempstring
                    )
                <
                    4
            ){ break; }
        ++nextline;
        displine[disprefs] = screenline;

        wprintw(wresult, "%c", dispchars[disprefs]);

        /* display any change mark */
        if (changing == true && change[topline + disprefs - 1] == true) {
        	waddch(wresult, '>');
        } else {
            waddch(wresult, ' ');
        }

        /* display the file name */
        if (field == FILENAME) {
        wprintw(wresult, "%-*s ", filelen, file);
        } else {
        /* if OGS, display the subsystem and book names */
        if (ogs == true) {
            ogsnames(file, &subsystem, &book);
            wprintw(wresult, "%-*.*s ", subsystemlen, subsystemlen, subsystem);
            wprintw(wresult, "%-*.*s ", booklen, booklen, book);
        }
        /* display the requested path components */
        if (dispcomponents > 0) {
            wprintw(wresult, "%-*.*s ", filelen, filelen,
               pathcomponents(file, dispcomponents));
        }
        } /* else(field == FILENAME) */

        /* display the function name */
        if (field == SYMBOL || field == CALLEDBY || field == CALLING) {
        wprintw(wresult, "%-*.*s ", fcnlen, fcnlen, function);
        }
        if (field == FILENAME) {
        waddch(wresult, '\n');    /* go to next line */
        continue;
        }

        /* display the line number */
        wprintw(wresult, "%*s ", numlen, linenum);
        /* there may be tabs in egrep output */
        while ((s = strchr(tempstring, '\t')) != NULL) {
        *s = ' ';
        }

        /* display the source line */
        s = tempstring;
        for (;;) {
        /* if the source line does not fit */
        if ((i = strlen(s)) > srctxtw) {

            /* find the nearest blank */
            for (i = srctxtw; s[i] != ' ' && i > 0; --i) {
                ;
            }

            if (i == 0) {
                i = srctxtw;    /* no blank */
            }
        }
        /* print up to this point */
        wprintw(wresult, "%.*s", i, s);
        s += i;

        /* if line didn't wrap around */
        if (i < srctxtw) {
            waddch(wresult, '\n');    /* go to next line */
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
        if (++screenline > result_window_height) {

            /* if this is the first displayed line,
               display what will fit on the screen */
            if (topline == nextline-1) {
            disprefs++;
            /* break out of two loops */
            goto endrefs;
            }

            /* erase the reference */
            while (--screenline >= displine[disprefs]) {
                wmove(wresult, screenline, 0);
                wclrtoeol(wresult);
            }
            ++screenline;

            /* go back to the beginning of this reference */
            --nextline;
			fseek(refsfound, 0, SEEK_SET);
            goto endrefs;
        }
        /* indent the continued source line */
        wmove(wresult, screenline, second_col_width - srctxtw);
        } /* for(ever) */
    } /* for(reference output lines) */

endrefs:
    /* position the screen cursor for the message */
    i = result_window_height - 1;
    if (screenline < i) {
        waddch(wresult, '\n');
    }
    else {
        wmove(wresult, i, 0);
    }
    /* check for more references */
    i = totallines - nextline + 1;
    bottomline = nextline;
    if (i > 0) {
        wprintw(wresult, "* Lines %d-%d of %d, %d more - press the space bar to display more *", topline, bottomline, totallines, i);
    }
    /* if this is the last page of references */
    else if (topline > 1 && nextline > totallines) {
        waddstr(wresult, "* Press the space bar to display the first lines again *");
    }
}

void display_cursor(void){
    chtype i;
    int yoffset = 0, xoffset = 0;

    if(current_window == &winput){
        xoffset = sizeof(inputprompt)-1 + rl_point;
    }else if(current_window == &wmode){
        yoffset = field;
    }else if(current_window == &wresult){
        yoffset = displine[curdispline];
    }else{
        assert(("No window selected.", true));
    }

    wmove(*current_window, yoffset, xoffset);

    i = winch(*current_window);
    i |= A_REVERSE;
    waddch(*current_window, i);
}

void
display(void)
{
    //drawscrollbar(topline, nextline);    /* display the scrollbar */

    if(window_change){
		if(window_change == CH_HELP){
			werase(whelp);
			display_help();
			/* Do not display over the help msg and */
			/*  rely on display_help() setting CH_ALL */
			return;
		}
		/**/
        if(window_change == CH_ALL){
            display_frame();
        }
        if(window_change & CH_INPUT){
            werase(winput);
            display_command_field();
        }
        if(window_change & CH_RESULT){
            werase(wresult);
            display_results();
        }
        if(window_change & CH_MODE){
            werase(wmode);
            display_mode();
        }

        display_cursor();

        refresh();
        wrefresh(winput);
        wrefresh(wmode);
        wrefresh(wresult);
    }

    window_change = CH_NONE;
}

void
horswp_field(void){
    if(current_window != &wresult){
        if(current_window == &winput){
            window_change |= CH_INPUT;
        }else{
            window_change |= CH_MODE;
        }
        last_window = current_window;
        current_window = &wresult;
    }else{
        current_window = last_window;
    }
    window_change |= CH_RESULT;
}

void
verswp_field(void){
    if(current_window == &wresult){ return; }
    current_window = (current_window == &winput) ? &wmode : &winput;
    window_change |= CH_INPUT | CH_MODE;
}

/*ARGSUSED*/
static void
jumpback(int sig)
{
    signal(sig, jumpback);
    siglongjmp(env, 1);
}

bool
search(void)
{
    char    *findresult = NULL;	/* find function output */
    bool    funcexist = true;		/* find "function" error */
    FINDINIT rc = falseERROR;    	/* findinit return code */
    sighandler_t savesig;    	/* old value of signal */
    FP    f;			/* searching function */
    int    c;

    /* open the references found file for writing */
    if (writerefsfound() == false) {
        return(false);
    }
    /* find the pattern - stop on an interrupt */
    if (linemode == false) {
        postmsg("Searching");
    }
    searchcount = 0;
    savesig = signal(SIGINT, jumpback);
    if (sigsetjmp(env, 1) == 0) {
        f = fields[field].findfcn;
        if (f == findregexp || f == findstring) {
            findresult = (*f)(input_line);
        } else {
        	if ((nonglobalrefs = myfopen(temp2, "wb")) == NULL) {
        		cannotopen(temp2);
        		return(false);
        	}
        	if ((rc = findinit(input_line)) == falseERROR) {
        		(void) dbseek(0L); /* read the first block */
        		findresult = (*f)(input_line);
        		if (f == findcalledby)
        			funcexist = (*findresult == 'y');
        		findcleanup();

        		/* append the non-global references */
        		(void) fclose(nonglobalrefs);
        		if ((nonglobalrefs = myfopen(temp2, "rb"))
        		     == NULL) {
        			cannotopen(temp2);
        			return(false);
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
        return(false);
    }
    nextline = 1;
    totallines = 0;
    disprefs = 0;

    /* see if it is empty */
    if ((c = getc(refsfound)) == EOF) {
        if (findresult != NULL) {
        	(void) snprintf(lastmsg, sizeof(lastmsg), "Egrep %s in this pattern: %s",
        		       findresult, input_line);
        } else if (rc == falseTSYMBOL) {
        	(void) snprintf(lastmsg, sizeof(lastmsg), "This is not a C symbol: %s",
        		       input_line);
        } else if (rc == REGCMPERROR) {
            (void) snprintf(lastmsg, sizeof(lastmsg), "Error in this regcomp(3) regular expression: %s",
                       input_line);

        } else if (funcexist == false) {
        	(void) snprintf(lastmsg, sizeof(lastmsg), "Function definition does not exist: %s",
        		       input_line);
        } else {
            (void) snprintf(lastmsg, sizeof(lastmsg), "Could not find the %s: %s",
                       fields[field].text2, input_line);
        }
        return(false);
    }
    /* put back the character read */
    (void) ungetc(c, refsfound);

    countrefs();

    window_change |= CH_RESULT;

    return(true);
}

/* display search progress with default custom format */
void
progress(char *what, long current, long max)
{
    static    long    start;
    long    now;
    char    msg[MSGLEN + 1];
    int    i;

    /* save the start time */
    if (searchcount == 0) {
        start = time(NULL);
    }
    if ((now = time(NULL)) - start >= 1)
    {
        if (linemode == false)
        {
            wmove(wresult, MSGLINE, 0);
            wclrtoeol(wresult);
            waddstr(wresult, what);
            snprintf(msg, sizeof(msg), "%ld", current);
            wmove(wresult, MSGLINE, (COLS / 2) - (strlen(msg) / 2));
            waddstr(wresult, msg);
            snprintf(msg, sizeof(msg), "%ld", max);
            wmove(wresult, MSGLINE, COLS - strlen(msg));
            waddstr(wresult, msg);
            refresh();
        }
        else if (verbosemode == true)
        {
            snprintf(msg, sizeof(msg), "> %s %ld of %ld", what, current, max);
        }

        start = now;
        if ((linemode == false) && (incurses == true))
        {
            wmove(wresult, MSGLINE, 0);
            i = (float)COLS * (float)current / (float)max;

            standout();
            for (; i > 0; i--)
                waddch(wresult, inch());
            standend();
            refresh();
        }
        else
        	if (linemode == false || verbosemode == true)
        		postmsg(msg);
    }
    ++searchcount;
}

/* print error message on system call failure */
void
myperror(char *text)
{
    char    msg[MSGLEN + 1];    /* message */
    char    *s;

    s = strerror(errno);

    (void) snprintf(msg, sizeof(msg), "%s: %s", text, s);
    postmsg(msg);
}

/* postmsg clears the message line and prints the message */

void
postmsg(char *msg)
{
    if (linemode == true || incurses == false) {
        (void) printf("%s\n", msg);
        fflush(stdout);
    }
    else {
        clearmsg();
        waddstr(wresult, msg);
        wrefresh(wresult);
    }
    (void) strncpy(lastmsg, msg, sizeof(lastmsg) - 1);
}

/* clearmsg clears the first message line */

void
clearmsg(void)
{
    wmove(wresult, MSGLINE, 0);
    wclrtoeol(wresult);
}

/* clearmsg2 clears the second message line */
void
clearmsg2(void)
{
    if (linemode == false) {
        wmove(wresult, MSGLINE + 1, 0);
        wclrtoeol(wresult);
    }
}

/* postmsg2 clears the second message line and prints the message */
void
postmsg2(char *msg)
{
    if (linemode == true) {
        (void) printf("%s\n", msg);
    }
    else {
        clearmsg2();
        waddstr(wresult, msg);
        wrefresh(wresult);
    }
}

/* display an error mesg - stdout or on second msg line */
void
posterr(char *msg, ...)
{
    va_list ap;
    char errbuf[MSGLEN];

    va_start(ap, msg);
    if (linemode == true || incurses == false)
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
    if (incurses == true) {
        exitcurses();
    }

    /* display fatal error messages */
    fprintf(stderr,"%s",errbuf);

    /* shut down */
    myexit(1);
}

/* get the OGS subsystem and book names */
void
ogsnames(char *file, char **subsystem, char **book)
{
    static    char    buf[PATHLEN + 1];
    char    *s, *slash;

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
    int    i;
    char    *s;

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
bool
writerefsfound(void)
{
    if (refsfound == NULL) {
        if ((refsfound = myfopen(temp1, "wb")) == NULL) {
        	cannotopen(temp1);
        	return(false);
        }
    } else {
        (void) fclose(refsfound);
        if ( (refsfound = myfopen(temp1, "wb")) == NULL) {
        	postmsg("Cannot reopen temporary file");
        	return(false);
        }
    }
    return(true);
}
