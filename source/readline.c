#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "global.h"
#include "build.h"
#include <ncurses.h>

static int	input_available = 0;
static int  input_char;
char		input_line[PATLEN + 1];

/* used for saving a line not [Enter]-ed yet,
 *  so its not lost if the user scrolls the history
 */
static struct PARTIAL_LINE {
	bool is_active;
	char* line;
	int pos;
} partial_line = {
	.line = NULL,
	.is_active = true
};


static inline void previous_history_proxy(void);
static inline void next_history_proxy(void);
											 		

bool interpret(int c) {
	/* A switch is faster then binding Readline to a billion functions
	 *  and since KEY_* values can't be bound anyways (values too large
	 *  (because while the first argument of rl_bind_key() is an int,
	 *  only unsigned chars are valid)), handling everything here
	 *  creates consistency too.
	 */
	switch(c){
		case KEY_BACKSPACE:
			rl_rubout(1, 0);
			break;
		case KEY_UP:
			previous_history_proxy();
			break;
		case KEY_DOWN:
			next_history_proxy();
			break;
		case KEY_LEFT:
			if(rl_point) {
				--rl_point;
			}
			break;
		case KEY_RIGHT:
			if(rl_point < rl_end) {
				++rl_point;
			}
			break;
		case ESC:
		case ctrl('X'):
			process_mouse();
			break;
		default:
			input_char		= c;
			input_available = 1;
			rl_callback_read_char();
			return 0;
	}
	//XXX:
	// rl_bind_key(ctrl('\\'), /**/);        /* bypass bindings */

	window_change |= CH_INPUT;
	return 0;
}

static int getc_function(FILE *ignore) {
	UNUSED(ignore);

	input_available = 0;
	return input_char;
}

static int input_available_hook() {
	return input_available;
}

static void redisplay_function() {
	window_change |= CH_INPUT;
}

static void callback_handler(char *line) {
	if(!line) {
		input_mode = INPUT_NORMAL;
		return;
	}

	add_history(line);

	switch(input_mode) {
		case INPUT_NORMAL: {
			strncpy(input_line, line, PATLEN);
			search(input_line);
			horswp_window();
			curdispline = 0;
			current_page = 0;
			PCS_reset();
		} break;
		case INPUT_CHANGE_TO: {
			strncpy(newpat, line, PATLEN);
			change	   = (bool *)calloc(totallines, sizeof(*change));
			input_mode = INPUT_CHANGE;
			force_window();
		} return;
		case INPUT_APPEND: {
			char filename[PATHLEN + 1];
			FILE* file;
			char ch;
			shellpath(filename, sizeof(filename), line);
			file = fopen(filename, "a+");
			if (file) {
				seekpage(0);
				while ((ch = getc(refsfound)) != EOF) {
					putc(ch, file);
				}
				fclose(file);
			} else {
				postmsg2("Failed to open file.");
			}
			input_mode = INPUT_NORMAL;
		} return;
		case INPUT_READ: {
			char filename[PATHLEN + 1];
			shellpath(filename, sizeof(filename), line);
			if (!readrefs(filename)) {
				postmsg2("Ignoring an empty file");
			}
			window_change |= CH_INPUT | CH_RESULT;
			input_mode = INPUT_NORMAL;
		} return;
	}

	switch(field) {
		case CHANGE:
			if(totallines == 0){ return; }
			input_mode = INPUT_CHANGE_TO;
			force_window();
			break;
		case DEFINITION:
		case FILENAME:
			if(totallines == 1) { editref(0); }
			break;
	}
}

static inline void previous_history_proxy(){
	HIST_ENTRY* i = previous_history();
    if(!i) { return; }

	if(partial_line.is_active){
		free(partial_line.line);
		partial_line = (struct PARTIAL_LINE){
			.line = strdup(rl_line_buffer),
			.pos = rl_point,
			.is_active = false
		};
	}
	//
    rl_replace_line(i->line, 0);
	rl_point = strlen(i->line);
}

static inline void next_history_proxy(){
	HIST_ENTRY* i = next_history();
	if(i){
        rl_replace_line(i->line, 0);
		rl_point = strlen(i->line);
	}else if(!partial_line.is_active){
        rl_replace_line(partial_line.line, 0);
		rl_point = partial_line.pos;
		partial_line.is_active = true;
	}
}

void rlinit() {
	rl_readline_name = PROGRAM_NAME;

	using_history();

	rl_catch_signals		= 0;
	rl_catch_sigwinch		= 0;
	rl_prep_term_function	= NULL;
	rl_deprep_term_function = NULL;
	rl_change_environment	= 0;

	rl_getc_function		= getc_function;
	rl_input_available_hook = input_available_hook;
	rl_redisplay_function	= redisplay_function;
	rl_callback_handler_install("", callback_handler);
}
