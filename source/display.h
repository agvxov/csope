#ifndef DISPLAY_H
#define DISPLAY_H

#include <ncurses.h> // XXX temp fix for global display access violations

/* bitmask type to mark which windows have to be rerendered by display()
 */
enum {
	CH_NONE	  = 0x0000,
	CH_RESULT = 0x0001 << 0,
	CH_INPUT  = 0x0001 << 1,
	CH_MODE	  = 0x0001 << 2,
	CH_CASE   = 0x0001 << 3,
	CH_HELP	  = 0x0001 << 4, /* do NOT add to CH_ALL */
	CH_ALL	  = CH_RESULT | CH_INPUT | CH_MODE | CH_CASE
};
/* Only display.c can directly update the screen.
 * Any other component must request the update of a particular
 *  screen area by OR'ing this variable with the appropriate mask.
 */
extern int window_change;

extern bool incurses;
extern void entercurses(void);
extern void exitcurses(void);

extern void dispinit(void);
extern void display(void);
extern void redisplay(void);
extern void drawscrollbar(int top, int bot);
extern void horswp_window(void);
extern void verswp_window(void);

#endif
