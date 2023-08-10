#include <stdio.h>
#include <readline/readline.h>
#include "global.h"
#include "build.h"
#include <ncurses.h>

static int input_available = 0;
static char input_char;
char input_line[PATLEN + 1];

bool interpret(int c){
    input_char = c;
    input_available = 1;
    rl_callback_read_char();

    return 0;
}

static int getc_function(FILE* ignore){
	UNUSED(ignore);

    input_available = 0;
    return (int)input_char;
}

static int input_available_hook(){
    return input_available;
}

static void redisplay_function(){
    window_change |= CH_INPUT;
}

static void callback_handler(char* line){
	if(!line){ return; }
    strncpy(input_line, line, PATLEN);

	switch(input_mode){
		case INPUT_NORMAL:
			search();
			break;
		case INPUT_CHANGE:
			changestring(change);
			input_mode = INPUT_NORMAL;
			break;
	}

	switch(field){
		case CHANGE:
			input_mode = INPUT_CHANGE;
			break;
		case DEFINITION:
		case FILENAME:
			if(totallines == 1){ editref(0); }
			break;
	}

	curdispline = 0;
	PCS_reset();
	current_page = 0;
}

static int ctrl_z(){
    kill(0, SIGTSTP);
	return 0;
}

static int toggle_caseless(){
    if (caseless == false) {
        caseless = true;
        postmsg2("Caseless mode is now ON");
    } else {
        caseless = false;
        postmsg2("Caseless mode is now OFF");
    }
    egrepcaseless(caseless);    /* turn on/off -i flag */
	return 0;
}

static int rebuild_reference(){
    if (isuptodate == true) {
        postmsg("The -d option prevents rebuilding the symbol database");
        return(false);
    }
    exitcurses();
    freefilelist();        /* remake the source file list */
    makefilelist();
    rebuild();
    if (errorsfound == true) {
        errorsfound = false;
        askforreturn();
    }
    entercurses();
    postmsg("");        /* clear any previous message */
    totallines = 0;
    disprefs = 0;
    return(true);
}

void rlinit(){
    rl_catch_signals = 0;
    rl_catch_sigwinch = 0;
    rl_prep_term_function = NULL;
    rl_deprep_term_function = NULL;
    rl_change_environment = 0;

    rl_getc_function = getc_function;
    rl_input_available_hook = input_available_hook;
    rl_redisplay_function = redisplay_function;
    rl_callback_handler_install("", callback_handler);

    rl_bind_key(7, rl_rubout);	// XXX: 7 is backspace for some reason (on my system anyways?)
    rl_bind_key(KEY_BACKSPACE, rl_rubout);

    rl_bind_key(EOF, exit);
    rl_bind_key(ctrl('Z'), ctrl_z);
    rl_bind_key(ctrl('Z'), toggle_caseless);
    rl_bind_key(ctrl('R'), rebuild_reference);
    rl_bind_key(ESC, process_mouse);        /* possible unixpc mouse selection */
    rl_bind_key(ctrl('X'), process_mouse);    /* mouse selection */

    //rl_bind_key(ctrl('\\'), /**/);        /* bypass bindings */
}
