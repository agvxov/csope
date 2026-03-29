/*    cscope - interactive C symbol cross-reference
 *
 *    display help
 *
 */

#include "global.h"
#include <ncurses.h>
/*
	max num of lines of help screen -
	this number needs to be increased if more than n help items are needed
*/

const char tooltip_winput[] = "Search [Enter] -Mode [^k] +Mode [^j] Right [Tab] Down [%]";
const char tooltip_wmode[]	= "-Mode [Up] +Mode [Down] Right [Tab] Up [%]";
const char tooltip_wresult[] =
	"Edit [Enter] Up [Up] Down [Down] Left [Tab] Previous [-] Next [+]";

static char help_msg[] =
    "Select the desired input field by using ^J and ^K,\n"
    " or by foregrounding the window with % and using the arrow keys.\n"
	"Type the pattern to search for, and then press the RETURN key.\n"
    "For the first 4 and last 2 input fields,\n"
    " the pattern can be a regcomp(3) regular expression.\n"
	"If the search is successful, you can use these single-character commands:\n"
	"  0-9a-zA-Z\t"  ": Edit the file containing the displayed line.\n"
    "  PAGEUP   +\t" ": Display next set of matching lines.\n"
    "  PAGEDOWN -\t" ": Display previous set of matching lines.\n"
	"  ^E\t\t"       ": Open all results for editing.\n"
    "  \t\t"         "   You may break the iteration by exiting with non-zero (:cq).\n"
	"  >\t\t"        ": Write the list of lines being displayed to a file.\n"
	"  >>\t\t"       ": Append the list of lines being displayed to a file.\n"
	"  <\t\t"        ": Read lines from a file.\n"
	"  ^\t\t"        ": Filter all lines through a shell command.\n"
	"  |\t\t"        ": Pipe all lines to a shell command.\n"
	"At any time you can use these single-character commands:\n"
	"  TAB\t\t"      ": Foreground the next horizontal window.\n"
	"  %\t\t"        ": Foreground the next vectical window.\n"
	"  ^J\t\t"       ": Move to the next input field.\n"
	"  ^K\t\t"       ": Move to the previous input field.\n"
	"  ^S\t\t"       ": Toggle ignore/match letter case when searching.\n"
	"  ^R\t\t"       ": Rebuild the cross-reference.\n"
	"  !\t\t"        ": Start an interactive shell (exit to return).\n"
	"  ^L\t\t"       ": Redraw the screen.\n"
	"  ?\t\t"        ": Display this list of commands.\n"
    "\n"
	"NOTE: If the first character of the pattern you want to search for matches\n"
	"       a command, type a \\ character first.\n"
	"NOTE: Some control-key-combinations may be caught by your terminal.\n"
;

static char changeing_help_msg[] =
	"When changing text, you can use these single-character commands:\n\n"
	"  0-9a-zA-Z\t"  ": Mark or unmark the line to be changed.\n"
    "  PAGEUP   +\t" ": Display next set of matching lines.\n"
    "  PAGEDOWN -\t" ": Display previous set of matching lines.\n"
	"  *\t\t"        ": Mark or unmark all displayed lines to be changed.\n"
	"  ^A\t\t"       ": Mark or unmark all lines to be changed.\n"
	"  ^D\t\t"       ": Change the marked lines.\n"
	"  ESC\t\t"      ": Exit without changing the marked lines.\n"
	"  !\t\t"        ": Start an interactive shell (type ^D to return).\n"
	"  ^L\t\t"       ": Redraw the screen.\n"
	"  ?\t\t"        ": Display this list of commands.\n"
;

const char * help(void) {
	switch (input_mode) {
		case INPUT_CHANGE:
		case INPUT_CHANGE_TO:
			return changeing_help_msg;
		default:
			return help_msg;
	}
}

/* normal usage message */
void usage(void) {
	fputs(
        "Usage: " PROGRAM_NAME
		" [-bcCdehklLqRuUV] [-f file] [-F file] [-i file] [-I dir] [-s dir]\n"
		"              [-p number] [-P path] [-[0-8] pattern] [source files]\n"
		, stderr
    );
}

/* long usage message */
void longusage(void) {
	usage();
	fprintf(stderr,
        "\n"
        "-b --build-only        Build the cross-reference only.\n"
        "-C                     Ignore letter case when searching.\n"
        "-c                     Use only ASCII characters in the cross-ref file (don't compress).\n"
        "-d --preserve-database Do not update the cross-reference.\n"
        "-F symfile             Read symbol reference lines from symfile.\n"
        "-f reffile             Use reffile as cross-ref file name instead of %s.\n"
		, REFFILE
    );
	fprintf(stderr,
        "-h --help              This help screen.\n"
        "-I incdir              Look in incdir for any #include files.\n"
        "-i namefile            Browse through files listed in namefile, instead of %s\n"
		, NAMEFILE
    );
	fprintf(stderr,
        "-k --kernel-mode       Don't use %s for #include files.\n"
		, DEFAULT_INCLUDE_DIRECTORY
    );
	fputs(
        "-L --oneshot           Do a single search with line-oriented output.\n"
        "-l --line-oriented     Line-oriented interface.\n"
        "-num pattern           Go to input field num (counting from 0) and find pattern.\n"
        "-P path                Prepend path to relative file names in pre-built cross-ref file.\n"
        "-p n                   Display the last n file path components.\n"
        "-q                     Build an inverted index for quick symbol searching.\n"
        "-R --recursive         Recurse directories for files.\n"
        "-s dir                 Look in dir for additional source  files.\n"
        "-U                     Check file time stamps.\n"
        "-u --force-build       Unconditionally build the cross-reference file.\n"
        "-V --version           Print the version number.\n"
        "\n"
        "Please see the manpage for more information.\n"
		, stderr
    );
}
