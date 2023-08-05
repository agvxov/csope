# Csope
Fork of Cscope, with various improvements, because cscope is good and shall not be forgotten.

# Usacases
Csope shines at exploring stranger and obsecure code bases due to its TUI.
Cscope used to be advertized a 
For example, the Cscope codebase used to be a total mess,
fixing it would have been a lost cause, if not for Cscope itself.

# Demo
 TODO: fill in

# Interface
	            <-- Tab -->
	  +------------Message-------------+           +--------------------------------+
	A |+--------------+---------------+|           |+------------------------------+|
	| || Input Window | Result window ||           ||                              ||
	| |+--------------+               ||     ?     ||                              ||
	  || Mode  Window |               ||   ---->   ||            Help              ||
	% ||              |               ||   <----   ||                              ||
	  ||              |               ||    ...    ||                              ||
	| ||              |               ||           ||                              ||
	| ||              |               ||           ||                              ||
	V |+--------------+---------------+|           |+------------------------------+|
	  +-----------Tool Tips------------+           +--------------------------------+

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
+ use stdbool instead of YES/NO macros
+ saved kilobytes by stripping trailing whitespace

# Future features / contributor wishlist
+ providing support for other languages by integrating new lexers (e.g. ctag's)
