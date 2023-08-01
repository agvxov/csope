#include <readline/readline.h>
#include "global.h"

int input_available = 0;
char input_char;

static void getc_function(FILE* ignore){
	input_available = 0;
	return (int)input_char;
}

static input_available_hook(){
	return input_available;
}

static void redisplay_function(){
	window_change |= CH_INPUT;
	//display();
}

static void callback_handler(char* line){
	// ?!
}

int proxy(int i, int h){
	horswp_field();
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

	rl_bind_key('\t', proxy);
}
