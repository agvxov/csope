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
 *    terminal input functions
 */

#include "global.h"
#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
#include <ncurses.h>
#else
#include <curses.h>
#endif
#include <setjmp.h>    /* jmp_buf */
#include <stdlib.h>
#include <errno.h>
#if HAVE_SYS_TERMIOS_H
#include <sys/termios.h>
#endif

static    jmp_buf    env;		/* setjmp/longjmp buffer */
static    int    prevchar;	/* previous, ungotten character */

/* Internal prototypes: */
static void catchint(int sig);

/* catch the interrupt signal */

static void
catchint(int sig)
{
    signal(SIGINT, catchint);
    longjmp(env, 1);
}

/* unget a character */
void
myungetch(int c)
{
    prevchar = c;
}

/* get a character from the terminal */
int
mygetch(void)
{
    sighandler_t savesig = 0; /* old value of signal */
    int c;

    /* change an interrupt signal to a break key character */
    if (setjmp(env) == 0) {
    savesig = signal(SIGINT, catchint);
    refresh();    /* update the display */
    mousereinit();    /* curses can change the menu number */
    if(prevchar) {
        c = prevchar;
        prevchar = 0;
    } else {
        c = -1;
        while (c == -1) {
        /* get a character from the terminal */
        c = getch();
        if ((c == -1) && (errno != EINTR))
            break;
        }
    }
    } else {    /* longjmp to here from signal handler */
    c = KEY_BREAK;
    }
    signal(SIGINT, savesig);
    return(c);
}


/* get a line from the terminal in non-canonical mode */
int
mygetline(char p[], char s[], unsigned size, int firstchar, BOOL iscaseless)
{
    int    c;
    unsigned int i = 0, j;
    char *sright;    /* substring to the right of the cursor */
    unsigned int ri = 0;        /* position in right-string */

    /* Inserts and deletes are always performed on the left-string,
     * but we'll also have a right-string 'sright' to hold characters
     * which are on the right of the cursor [insertion point].
     *
     * Think of 'sright' as a stack -- we push chars into it when the cursor
     * moves left, and we pop chars off it when the cursor moves right again.
     * At the end of the function, we'll pop off any remaining characters
     * onto the end of 's'
     */
    sright = calloc(size, sizeof(*sright));

    strcpy ( s, p);
    i += strlen(p);
    /* if a character already has been typed */
    if (firstchar != '\0') {
    if(iscaseless == YES) {
        firstchar = tolower(firstchar);
    }
    addch(firstchar);    /* display it */
    s[i++] = firstchar;    /* save it */
    }
    /* until the end of the line is reached */
    while ((c = mygetch()) != '\r' && c != '\n' && c != KEY_ENTER) {
    if (c == KEY_LEFT || c == ctrl('B')) {    /* left */
        if (i > 0) {
        addch('\b');
        /* move this char into the second (rhs) string */
        sright[ri++] = s[--i];
        }
    } else if (c == KEY_RIGHT || c == ctrl('F')) {    /* right */
        if (i < size && ri > 0) {
        /* move this char to the left of the cursor */
        s[i++] = sright[--ri];
        addch(s[i-1]);
        }
    } else if (
#ifdef KEY_HOME
           c == KEY_HOME ||
#endif
           c == ctrl('A') ) {
        while (i > 0) {
        sright[ri++] = s[--i];
        addch('\b');
        addch(s[i]);
        addch('\b');
        }
    } else if (
#ifdef KEY_END
           c == KEY_END ||
#endif
           c == ctrl('E') ) {
        while (ri > 0) {
        s[i++] = sright[--ri];
        addch(s[i-1]);
        }
    } else if (c == erasechar() || c == KEY_BACKSPACE
           || c == DEL || c == ctrl('H') ) {
        /* erase */
        if (i > 0) {
        if (ri == 0)  {
            addstr("\b \b");
        } else {
            addch('\b');
            delch();
        }
        s[i] = '\0';
        --i;
        }
    } else if (c == killchar() || c == KEY_BREAK) {
        /* kill */
        for (j = 0; j < i; ++j) {
        addch('\b');
        }
        for (j = 0; j < i; ++j) {
        addch(' ');
        }
        for (j = 0; j < i; ++j) {
        addch('\b');
        }
        i = 0;
    } else if (isprint(c) || c == '\t') {
        /* printable */
        if(iscaseless == YES) {
        c = tolower(c);
        }
        /* if it will fit on the line */
        if (i < size) {
        s[i++] = c;	/* save it */
        if (ri == 0) {
            addch(c);	/* display it */
        } else {
            insch(c);	/* display it */
            addch(c);	/* advance cursor */
        }
        }
#if UNIXPC
    } else if (unixpcmouse == YES && c == ESC) {    /* mouse */
        getmouseaction(ESC);    /* ignore it */
#endif
    } else if (mouse == YES && c == ctrl('X')) {
        getmouseaction(ctrl('X'));    /* ignore it */
    } else if (c == EOF) {    			/* end-of-file */
        break;
    }

    /* return on an empty line to allow a command to be entered */
    if (firstchar != '\0' && (i+ri) == 0) {
        break;
    }
    }

    /* move any remaining chars on the rhs of the cursor
     * onto the end of our string
     */
    while (ri > 0) {
    s[i++] = sright[--ri];
    }
    free(sright);

    s[i] = '\0';
    return(i);
}

/* ask user to enter a character after reading the message */

void
askforchar(void)
{
    addstr("Type any character to continue: ");
    mygetch();
}

/* ask user to press the RETURN key after reading the message */

void
askforreturn(void)
{
    fprintf(stderr, "Press the RETURN key to continue: ");
    getchar();
    /* HBB 20060419: message probably messed up the screen --- redraw */
    if (incurses == YES) {
    redrawwin(curscr);
    }
}

/* expand the ~ and $ shell meta characters in a path */

void
shellpath(char *out, int limit, char *in)
{
    char    *lastchar;
    char    *s, *v;

    /* skip leading white space */
    while (isspace((unsigned char)*in)) {
    ++in;
    }
    lastchar = out + limit - 1;

    /* a tilde (~) by itself represents $HOME; followed by a name it
       represents the $LOGDIR of that login name */
    if (*in == '~') {
    *out++ = *in++;    /* copy the ~ because it may not be expanded */

    /* get the login name */
    s = out;
    while (s < lastchar && *in != '/' && *in != '\0' && !isspace((unsigned char)*in)) {
        *s++ = *in++;
    }
    *s = '\0';

    /* if the login name is null, then use $HOME */
    if (*out == '\0') {
        v = getenv("HOME");
    } else {    /* get the home directory of the login name */
        v = logdir(out);
    }
    /* copy the directory name if it isn't too big */
    if (v != NULL && strlen(v) < (lastchar - out)) {
        strcpy(out - 1, v);
        out += strlen(v) - 1;
    } else {
        /* login not found, so ~ must be part of the file name */
        out += strlen(out);
    }
    }
    /* get the rest of the path */
    while (out < lastchar && *in != '\0' && !isspace((unsigned char)*in)) {

    /* look for an environment variable */
    if (*in == '$') {
        *out++ = *in++;    /* copy the $ because it may not be expanded */

        /* get the variable name */
        s = out;
        while (s < lastchar && *in != '/' && *in != '\0' &&
           !isspace((unsigned char)*in)) {
        *s++ = *in++;
        }
        *s = '\0';

        /* get its value, but only it isn't too big */
        if ((v = getenv(out)) != NULL && strlen(v) < (lastchar - out)) {
        strcpy(out - 1, v);
        out += strlen(v) - 1;
        } else {
        /* var not found, or too big, so assume $ must be part of the
         * file name */
        out += strlen(out);
        }
    }
    else {    /* ordinary character */
        *out++ = *in++;
    }
    }
    *out = '\0';
}

static int
wmode_input(const int c){
    switch (c) {
        case KEY_ENTER:
        case '\r':
        case '\n':
        case ctrl('N'):	/* go to next input field */
        case KEY_DOWN:
        case KEY_RIGHT:
        	field = (field + 1) % FIELDS;
        	resetcmd();
        	return(NO);
        case ctrl('P'):	/* go to previous input field */
        case KEY_UP:
        case KEY_LEFT:
        	field = (field + (FIELDS - 1)) % FIELDS;
        	resetcmd();
        	return(NO);
        case KEY_HOME:    /* go to first input field */
        	field = 0;
        	resetcmd();
        	return(NO);
        case KEY_LL:    /* go to last input field */
        	curdispline = disprefs;
        	return(YES);
    }
}

static int
wresult_input(const int c){
    switch (c) {
        case KEY_ENTER: /* open for editing */
        case '\r':
        case '\n':
        	editref(curdispline);
        	window_change = CH_ALL;
            return(YES);
        case ctrl('N'):
        case KEY_DOWN:
        case KEY_RIGHT:
        	if ((curdispline + 1) < disprefs) {
        		++curdispline;
        	}
        	return(NO);
        case ctrl('P'):
        case KEY_UP:
        case KEY_LEFT:
        	if (curdispline) {
        		--curdispline;
        	}
        	return(NO);
        case KEY_HOME:
            curdispline = 0;
        	return(NO);
        case KEY_LL:
        	field = FIELDS - 1;
        	resetcmd();
        	return(NO);
        default:
        	char *e;
        	if ((e = strchr(dispchars, c)))
        	editref(e - dispchars);
    }
}

static int
global_input(const int c){
    switch(c){
        case ' ':	/* display next page */
        case '+':
        case ctrl('V'):
        case KEY_NPAGE:
        	if (totallines == 0) { return(NO); } /* don't redisplay if there are no lines */
        	/* XXX: figure out whether this comment is useful or not */
        	/* NOTE: seekline() is not used to move to the next
        	 * page because display() leaves the file pointer at
        	 * the next page to optimize paging forward
        	 */
        	curdispline = 0;
        	break;
        case ctrl('H'):	/* display previous page */
        case '-':
        case KEY_PPAGE:
        	if (totallines == 0) { return(NO); } /* don't redisplay if there are no lines */
        	curdispline = 0;
        	/* if there are only two pages, just go to the other one */
        	if (totallines <= 2 * mdisprefs) {
        		break;
        	}
        	/* if on first page but not at beginning, go to beginning */
        	nextline -= mdisprefs;	/* already at next page */
        	if (nextline > 1 && nextline <= mdisprefs) {
        		nextline = 1;
        	} else {
        		nextline -= mdisprefs;
        		if (nextline < 1) {
        			nextline = totallines - mdisprefs + 1;
        			if (nextline < 1) {
        				nextline = 1;
        			}
        		}
        	}
        	seekline(nextline);
        	break;
        case '>':	/* write or append the lines to a file */
        	break;					// XXX
            //char filename[PATHLEN + 1];
        	//char* s;
        	//char ch;
        	//FILE* file;
        	//if (totallines == 0) {
        	//	postmsg("There are no lines to write to a file");
        	//	return(NO);
        	//}
        	//move(PRLINE, 0);
        	////addstr("Write to file: ");		// XXX
        	//s = "w";
        	//if ((ch = mygetch()) == '>') {
        	//move(PRLINE, 0);
        	////addstr(appendprompt);	// XXX fix
        	////ch = '\0';
        	////s = "a";
        	////}
        	////if (ch != '\r' && mygetline("", newpat, COLS - sizeof(appendprompt), c, NO) > 0) {
        	////	shellpath(filename, sizeof(filename), newpat);
        	////	if ((file = myfopen(filename, s)) == NULL) {
        	////		cannotopen(filename);
        	////	} else {
        	////		seekline(1);
        	////		while ((ch = getc(refsfound)) != EOF) {
        	////		putc(ch, file);
        	////		}
        	////		seekline(topline);
        	////		fclose(file);
        	////	}
        	////}
        	////clearprompt();
        	return(NO);	/* return to the previous field */
        case '<':	/* read lines from a file */
        	break;					// XXX
        	move(PRLINE, 0);
        	//addstr(readprompt); // XXX fix
        	//if (mygetline("", newpat, COLS - sizeof(readprompt), '\0', NO) > 0) {
        	//	clearprompt();
        	//	shellpath(filename, sizeof(filename), newpat);
        	//	if (readrefs(filename) == NO) {
        	//		postmsg2("Ignoring an empty file");
        	//		return(NO);
        	//	}
        	//	window_change |= CH_INPUT;
        	//	return(YES);
        	//}
        	//clearprompt();
        	return(NO);
        case '|':	/* pipe the lines to a shell command */
        case '^':
        	break;		// XXX fix
        	if (totallines == 0) {
        		postmsg("There are no lines to pipe to a shell command");
        		return(NO);
        	}
        	/* get the shell command */
        	move(PRLINE, 0);
        	//addstr(pipeprompt);
        	//if (mygetline("", newpat, COLS - sizeof(pipeprompt), '\0', NO) == 0) {
        	//	clearprompt();
        	//	return(NO);
        	//}
        	///* if the ^ command, redirect output to a temp file */
        	//if (commandc == '^') {
        	//	strcat(strcat(newpat, " >"), temp2);
        	//	/* HBB 20020708: somebody might have even
        	//	 * their non-interactive default shells
        	//	 * complain about clobbering
        	//	 * redirections... --> delete before
        	//	 * overwriting */
        	//	remove(temp2);
        	//}
        	//exitcurses();
        	//if ((file = mypopen(newpat, "w")) == NULL) {
        	//	fprintf(stderr, "cscope: cannot open pipe to shell command: %s\n", newpat);
        	//} else {
        	//	seekline(1);
        	//	while ((c = getc(refsfound)) != EOF) {
        	//	putc(c, file);
        	//	}
        	//	seekline(topline);
        	//	mypclose(file);
        	//}
        	//if (commandc == '^') {
        	//	if (readrefs(temp2) == NO) {
        	//	postmsg("Ignoring empty output of ^ command");
        	//	}
        	//}
        	//askforreturn();
        	//entercurses();
        	break;
        case '!':	/* shell escape */
        	execute(shell, shell, NULL);
        	seekline(topline);
        	break;
        case KEY_RESIZE:
        	/* XXX: fill in*/
        	break;
        case ctrl('L'):	/* redraw screen */
        case KEY_CLEAR:
        	window_change = CH_ALL;
        	return(NO);
        case '?':	/* help */
        	clear();
        	help();
        	clear();
        	seekline(topline);
        	break;
        case ctrl('E'):	/* edit all lines */
        	editall();
        	break;
    }

    return 0;
}

extern const void const* winput;
extern const void const* wmode;
extern const void const* wresult;
extern const void const* const* current_window;

int
handle_input(const char c){
    /* --- global --- */
    const int r = global_input(c);
    if(r){ return r; }
    /* --- mode specific --- */
    if(*current_window == winput){
        return interpret(c);
    }else if(*current_window == wmode){
        return wmode_input(c);
    }else if(*current_window == wresult){
        return wresult_input(c);
    }

    return 0;
}
