# Csope
Fork of Cscope, with various improvements, because cscope is good and shall not be forgotten.

# Improvements/Changes
## User side
+ renamed the program, because "cscope" is annoying to type
+ improved gui	/*pending*/
+ GNU Readline integration (ie. VI/EMACS mode, command history) /*pending*/
## To the code
+ nuked autoconf, replaced with single Makefile
+ removed "scanner.l" which seems to be an anchient version (and redundant copy) of "fscanner.l" forgotten by all
+ encapsulated changes to the TUI into display.c
+ removed macro hell put in place to allow compiling on a dead badger
+ replaced repeated inline #ifdef KEY_\*-s with guaranteed definitions
+ removed random commets giving tips for and refering to specific issues

# Interface
	            <-- Tab -->
	  +------------Message-------------+          +--------------------------------+
	A |+--------------+---------------+|          |+------------------------------+|
	| || Input Window | Result window ||          ||                              ||
	| |+--------------+               ||     ?    ||                              ||
	  || Mode  Window |               ||   --->   ||            Help              ||
	% ||              |               ||   <---   ||                              ||
	  ||              |               ||    ESC   ||                              ||
	| ||              |               ||          ||                              ||
	| ||              |               ||          ||                              ||
	V |+--------------+---------------+|          |+------------------------------+|
	  +-----------Tool Tips------------+          +--------------------------------+
