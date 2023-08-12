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
 *    terminal input functions
 */

#include "global.h"
#include <ncurses.h>
#include <setjmp.h>    /* jmp_buf */
#include <stdlib.h>
#include <errno.h>
#if HAVE_SYS_TERMIOS_H
# include <sys/termios.h>
#endif

#include "keys.h"

bool do_press_any_key = false;

static    jmp_buf    env;        /* setjmp/longjmp buffer */
static    int    prevchar;    /* previous, ungotten character */

/* Internal prototypes: */
static void catchint(int sig);

/* catch the interrupt signal */

static void
catchint(int sig){
	UNUSED(sig);

    signal(SIGINT, catchint);
    longjmp(env, 1);
}

/* unget a character */
void
myungetch(int c)
{
    prevchar = c;
}
/* ask user to enter a character after reading the message */
void
askforchar(void){
    addstr("Type any character to continue: ");
    getch();
}

/* ask user to press the RETURN key after reading the message */
void
askforreturn(void){
    fprintf(stderr, "Press the RETURN key to continue: ");
    getchar();
    /* HBB 20060419: message probably messed up the screen --- redraw */
    if (incurses == true) {
    redrawwin(curscr);
    }
}

/* expand the ~ and $ shell meta characters in a path */
void
shellpath(char *out, int limit, char *in){
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
        case ctrl('N'):    /* go to next input field */
        case KEY_DOWN:
        case KEY_RIGHT:
            field = (field + 1) % FIELDS;
            resetcmd();
			break;
        case ctrl('P'):    /* go to previous input field */
        case KEY_UP:
        case KEY_LEFT:
            field = (field + (FIELDS - 1)) % FIELDS;
            resetcmd();
			break;
        case KEY_HOME:    /* go to first input field */
            field = 0;
            resetcmd();
			break;
        case KEY_LL:    /* go to last input field */
            curdispline = disprefs;
			break;
		default:
			return 0;
    }

	window_change |= CH_MODE;
	return 1;
}

static int
wresult_input(const int c){
    switch (c) {
        case KEY_ENTER: /* open for editing */
        case '\r':
        case '\n':
            editref(curdispline);
            window_change = CH_ALL;
			break;
        case ctrl('N'):
        case KEY_DOWN:
        case KEY_RIGHT:
            if ((curdispline + 1) < disprefs) {
                ++curdispline;
            }
			break;
        case ctrl('P'):
        case KEY_UP:
        case KEY_LEFT:
            if (curdispline) {
                --curdispline;
            }
			break;
        case KEY_HOME:
            curdispline = 0;
			break;
        case KEY_LL:
            field = FIELDS - 1;
            resetcmd();
			break;
        default:
			if(c > mdisprefs){ goto noredisp; }
            const int pos = dispchar2int(c);
            if(pos > -1){ editref(pos); }
			goto noredisp;
    }

	window_change |= CH_RESULT;
	noredisp:
	return 1;
}

static int
global_input(const int c){
    switch(c){
        case '\t':
            horswp_field();
            break;
        case '%':
            verswp_field();
            break;
		case ctrl('K'):
            field = (field + (FIELDS - 1)) % FIELDS;
            resetcmd();
			window_change |= CH_MODE;
            break;
		case ctrl('J'):
            field = (field + 1) % FIELDS;
            resetcmd();
			window_change |= CH_MODE;
            break;
        case ctrl('H'):    /* display previous page */
        case '-':
        case KEY_PPAGE:
            if (totallines == 0) { return 0; } /* don't redisplay if there are no lines */
            curdispline = 0;
			if(current_page > 0){
				--current_page;
				window_change |= CH_RESULT;
			}
            break;
        case '+':
        case ctrl('L'):
        case KEY_NPAGE:
            if (totallines == 0) { return 0; } /* don't redisplay if there are no lines */
            curdispline = 0;
			++current_page;
			window_change |= CH_RESULT;
            break;
        case '>':    /* write or append the lines to a file */
            break;                    // XXX
            //char filename[PATHLEN + 1];    
            //char* s;
            //char ch;
            //FILE* file;
            //if (totallines == 0) {
            //    postmsg("There are no lines to write to a file");
            //    return(NO);
            //}
            //move(PRLINE, 0);
            ////addstr("Write to file: ");        // XXX
            //s = "w";
            //if ((ch = getch()) == '>') {
            //move(PRLINE, 0);
            ////addstr(appendprompt);    // XXX fix
            ////ch = '\0';
            ////s = "a";
            ////}
            ////if (ch != '\r' && mygetline("", newpat, COLS - sizeof(appendprompt), c, NO) > 0) {
            ////    shellpath(filename, sizeof(filename), newpat);
            ////    if ((file = myfopen(filename, s)) == NULL) {
            ////        cannotopen(filename);
            ////    } else {
            ////        seekline(1);
            ////        while ((ch = getc(refsfound)) != EOF) {
            ////        putc(ch, file);
            ////        }
            ////        seekline(topline);
            ////        fclose(file);
            ////    }
            ////}
            ////clearprompt();
            break;
        case '<':    /* read lines from a file */
            break;                    // XXX
            //move(PRLINE, 0);
            //addstr(readprompt); // XXX fix
            //if (mygetline("", newpat, COLS - sizeof(readprompt), '\0', NO) > 0) {
            //    clearprompt();
            //    shellpath(filename, sizeof(filename), newpat);
            //    if (readrefs(filename) == NO) {
            //        postmsg2("Ignoring an empty file");
            //        return(NO);
            //    }
            //    window_change |= CH_INPUT;
            //    return(YES);
            //}
            //clearprompt();
            return 0;
        case '|':    /* pipe the lines to a shell command */
        case '^':
            break;        // XXX fix
            if (totallines == 0) {
                postmsg("There are no lines to pipe to a shell command");
                return 0;
            }
            /* get the shell command */
            //move(PRLINE, 0);
            //addstr(pipeprompt);
            //if (mygetline("", newpat, COLS - sizeof(pipeprompt), '\0', NO) == 0) {
            //    clearprompt();
            //    return(NO);
            //}
            ///* if the ^ command, redirect output to a temp file */
            //if (commandc == '^') {
            //    strcat(strcat(newpat, " >"), temp2);
            //    /* HBB 20020708: somebody might have even
            //     * their non-interactive default shells
            //     * complain about clobbering
            //     * redirections... --> delete before
            //     * overwriting */
            //    remove(temp2);
            //}
            //exitcurses();
            //if ((file = mypopen(newpat, "w")) == NULL) {
            //    fprintf(stderr, "cscope: cannot open pipe to shell command: %s\n", newpat);
            //} else {
            //    seekline(1);
            //    while ((c = getc(refsfound)) != EOF) {
            //    putc(c, file);
            //    }
            //    seekline(topline);
            //    mypclose(file);
            //}
            //if (commandc == '^') {
            //    if (readrefs(temp2) == NO) {
            //    postmsg("Ignoring empty output of ^ command");
            //    }
            //}
            //askforreturn();
            //entercurses();
            break;
        case '!':    /* shell escape */
            execute(shell, shell, NULL);
			current_page = 0;
            break;
        case KEY_RESIZE:
            /* XXX: fill in*/
            break;
        case ctrl('U'):    /* redraw screen */
        case KEY_CLEAR:
            window_change = CH_ALL;
            break;
        case '?':    /* help */
			window_change = CH_HELP;
            break;
        case ctrl('E'):    /* edit all lines */
            editall();
            break;
        default:
            return 0;
    }

    return 1;
}

extern const void *const winput;
extern const void *const wmode;
extern const void *const wresult;
extern const void *const *const current_window;

int
change_input(const int c){
    MOUSE *p;                       /* mouse data */

	switch(c){
	case '*':	/* invert page */
	    for(int i = 0; topref + i < nextline; i++){
			change[topref + i] = !change[topref + i];
	    }
		window_change |= CH_RESULT;
		break;
	case ctrl('A'):	/* invert all lines */
	    for(unsigned i = 0; i < totallines; i++) {
			change[i] = !change[i];
	    }
		window_change |= CH_RESULT;
	    break;
	case ctrl('X'):	/* mouse selection */
	    if ((p = getmouseaction(DUMMYCHAR)) == NULL) {
			break;	/* unknown control sequence */
	    }
	    /* if the button number is a scrollbar tag */
	    if (p->button == '0') {
			//scrollbar(p);
			break;
	    }
	    /* find the selected line */
	    /* NOTE: the selection is forced into range */
		{
			int i;
			for(i = disprefs - 1; i > 0; --i) {
				if (p->y1 >= displine[i]) {
					break;
				}
			}
			change[i] = !change[i];
		}
	    break;
	case ctrl('D'):
		changestring(input_line, newpat, change, totallines);
		break;
	default:
		{
			/* if a line was selected */
			const int cc = dispchar2int(c);
			if(cc != -1){
				change[cc] = !change[cc];
				window_change |= CH_RESULT;
			}
		}
	}

	return 0;
}

int
changestring(const char* from,
				const char* to,
				const bool *const change,
				const int change_len
){
    char    newfile[PATHLEN + 1];   /* new file name */
    char    oldfile[PATHLEN + 1];   /* old file name */
    char    linenum[NUMLEN + 1];    /* file line number */
    char    msg[MSGLEN + 1];        /* message */
    FILE    *script;                /* shell script file */

	/* Return early */
    bool    anymarked = false;         /* any line marked */
	for(int i = 0; i < change_len; i++){
		if(change[i]){
			anymarked = true;
			break;
		}
	}
	if(!anymarked){ return false; }

    /* open the temporary file */
    if((script = myfopen(temp2, "w")) == NULL) {
		cannotopen(temp2);
		return(false);
    }

    /* for each line containing the old text */
    fprintf(script, "ed - <<\\!\n");
    *oldfile = '\0';
	fseek(refsfound, 0, SEEK_SET);
    for(int i = 0; 
	 	fscanf(refsfound, "%" PATHLEN_STR "s%*s%" NUMLEN_STR "s%*[^\n]", newfile, linenum) == 2;
	 	++i)
	{
		/* see if the line is to be changed */
		if (!change[i]) { break; }
		
	    /* if this is a new file */
	    if (strcmp(newfile, oldfile) != 0) {
				
		/* make sure it can be changed */
		if (access(newfile, WRITE) != 0) {
		    snprintf(msg, sizeof(msg), "Cannot write to file %s", newfile);
		    postmsg(msg);
		    goto end;
		}
		/* if there was an old file */
		if (*oldfile != '\0') {
		    fprintf(script, "w\n");	/* save it */
		}
		/* edit the new file */
		strcpy(oldfile, newfile);
		fprintf(script, "e %s\n", oldfile);
	    }
	    /* output substitute command */
	    fprintf(script, "%ss/", linenum);	/* change */
	    for(const char *s = from; *s != '\0'; ++s) {
			/* old text */
			if (strchr("/\\[.^*", *s) != NULL) {
				putc('\\', script);
			}
			if (caseless == true && isalpha((unsigned char)*s)) {
				putc('[', script);
				if(islower((unsigned char)*s)) {
				putc(toupper((unsigned char)*s), script);
				putc(*s, script);
				} else {
				putc(*s, script);
				putc(tolower((unsigned char)*s), script);
				}
				putc(']', script);
			} else {
				putc(*s, script);
			}
	    }
	    putc('/', script);			/* to */
	    for(const char *s = to; *s != '\0'; ++s) {	/* new text */
			if (strchr("/\\&", *s) != NULL) {
				putc('\\', script);
			}
			putc(*s, script);
	    }
	    fprintf(script, "/gp\n");	/* and print */
    }
    fprintf(script, "w\nq\n!\n");	/* write and quit */
    fflush(script);

	/* edit the files */
	fprintf(stderr, "Changed lines:\n\r");
	execute("sh", "sh", temp2, NULL);
	askforchar();
end:
    fclose(script);
    return true;
}

int
handle_input(const int c){
	/* - was wating for any input - */
	if(do_press_any_key){
		do_press_any_key = false;
		return 0;
	}
    /* --- global --- */
    const int r = global_input(c);
    if(r){ return 0; }
    /* --- mode specific --- */
	switch(input_mode){
		case INPUT_NORMAL:
			if(*current_window == winput){
				return interpret(c);
			}else if(*current_window == wmode){
				return wmode_input(c);
			}else if(*current_window == wresult){
				return wresult_input(c);
			}
			assert("'current_window' dangling.");
			break; /* NOTREACHED */
		case INPUT_CHANGE_TO:
			return interpret(c);
		case INPUT_CHANGE:
			return change_input(c);
	}

    return 0;
}
