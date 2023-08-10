# Csope
Fork of Cscope, with various improvements, because cscope is good and shall not be forgotten.
While the original's mainentence seems abandoned and as far as I can tell you need a PhD in autoconf to compile the latest version,
Csope is alive and well.

# Demo
 TODO: fill in

# Before/After
 TODO: fill in

# Features
#### Search for
 + C symbol
 + global definition
 + assignments to specified symbol
 + functions called by specified function
 + functions calling specified function
 + text string
 + egrep pattern
 + file
 + files #including specified file
#### ...and open with your editor.
#### Batch change search results **interactively**.
#### Save/load/pipe results.

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

# Usacases
Csope shines at exploring stranger and obsecure code bases due to its TUI.
It sometimes gets mislabeled as a code navigation tool, but the original documentation describes it best as a "code browsing tool".
Many tools can jump you to a definition or grep for patterns,
but Csope is unqie in that it allows for those and many other functionalities while providing you with a very comprehansible list of all results,
ready to fire up your editor at just the spot.
An example of its excelence is this project. The Cscope codebase used to be a total mess,
fixing it would have been a lost cause, if not for Cscope itself. Well, Csope now.

# Improvements/Changes
## User side
+ renamed the program, because "cscope" is annoying to type
+ improved gui	/*pending*/
+ GNU Readline integration (ie. VI/EMACS mode, command history) /*pending*/
## To the code
+ nuked autoconf, replaced with single Makefile
+ reorganized main()
+ encapsulated changes to the TUI into display.c
+ encapsulated searching into find.c
+ removed "scanner.l" which seems to be an anchient version (and redundant copy) of "fscanner.l" forgotten by all
+ removed macro hell put in place to allow compiling on a dead badger
+ replaced repeated inline #ifdef KEY_\*-s with guaranteed definitions
+ removed random commets giving tips for and refering to specific issues
+ use stdbool instead of YES/NO macros
+ saved kilobytes by stripping trailing whitespace
+ FILE\* refsfound used to be rewind()-ed everytime the reads were not sequencial

# Project structure	/*probably move to documentation*/
| Component | Purpose |
| :-------: | :-----: |
| main.c | generic init functions, main() and primary event loops (and junk)
| display.c | all functions directly dealing with NCurses |
| input.c | top layer of functions dealing with user input; migth dispatch to readline |
| find.c | searching functions |
| globals.h | an inherited curse; global var/prototype hell |
| readline.c | all functions directly dealing with GNU Readline; responsible for line editing in *input mode* |
| help.c | all functions dealing with help messages |

# TODO /*move soon*/
 + recursive macro function to assign KEY_\* default values; look for a new and shiny preprocessor?
 + sort out constants.h
 + scrollbar() uses magic int literals?
 + Ordering function declarations in global.h by alpha order is not smart
 + lineflagafterfile is stupid
 + library.h...; "private library", in a program using 90 globals; ffs
 + sort out the global hell
 + changestring() forks sh to execute an ed script...
 + was there really ever a scrollbar?
## Original
+ Display the current case mode (^C) onscreen
+ emacs like key bindings
    ^S for searching (^Y)
    Up/dwn Arrow support Next/Prev field. ??
    Inline editing on Input fields ( ??^B/^F )
    ^X^C to quit ( ^Q ??)
    Pagdwn/PageUp/+/- 
+ Same capabilities as interactive in non interactive (one shot) mode
+ Provide some how-do-I-use-this-thing doc.
+ Replace invlib.[ch] by real database.  Failing that, at least sanitize it.


# BUGS
 + Changing text double frees:
 	free(): double free detected in tcache 2
 	Aborted
 + Changing text can crash without replacing text and leaving the console ncursed

# Future features / contributor wishlist
+ providing support for other languages by integrating new lexers (e.g. ctag's)
