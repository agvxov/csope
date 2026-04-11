#ifndef READLINE_H
#define READLINE_H

#include "global.h"

extern char *rl_line_buffer;
extern char	 input_line[PATLEN + 1];
extern int	 rl_point;

void rlinit(void);
bool interpret(int c);	// XXX: probably rename

#endif
