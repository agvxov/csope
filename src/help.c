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
const char tooltip_wmode[] = "-Mode [Up] +Mode [Down] Right [Tab] Up [%]";
const char tooltip_wresult[] = "Edit [Enter] Up [Up] Down [Down] Left [Tab] Previous [-] Next [+]";

static char help_msg[] = 
        "Press the RETURN key repeatedly to move to the desired input field, type the\n"
        "pattern to search for, and then press the RETURN key.  For the first 4 and\n"
        "last 2 input fields, the pattern can be a regcomp(3) regular expression.\n"
        "If the search is successful, you can use these single-character commands:\n\n"
        "0-9a-zA-Z\tEdit the file containing the displayed line.\n"
        "space bar\tDisplay next set of matching lines.\n"
        "+\t\tDisplay next set of matching lines.\n"
        "^V\t\tDisplay next set of matching lines.\n"
        "-\t\tDisplay previous set of matching lines.\n"
        "^E\t\tEdit all lines.\n"
        ">\t\tWrite the list of lines being displayed to a file.\n"
        ">>\t\tAppend the list of lines being displayed to a file.\n"
        "<\t\tRead lines from a file.\n"
        "^\t\tFilter all lines through a shell command.\n"
        "|\t\tPipe all lines to a shell command.\n"
        "\nAt any time you can use these single-character commands:\n\n"
        "TAB\t\tSwap positions between input and output areas.\n"
        "RETURN\t\tMove to the next input field.\n"
        "^N\t\tMove to the next input field.\n"
        "^P\t\tMove to the previous input field.\n"
        "^Y / ^A\t\tSearch with the last pattern typed.\n"
        "^B\t\tRecall previous input field and search pattern.\n"
        "^F\t\tRecall next input field and search pattern.\n"
        "^C\t\tToggle ignore/use letter case when searching.\n"
        "^R\t\tRebuild the cross-reference.\n"
        "!\t\tStart an interactive shell (type ^D to return).\n"
        "^L\t\tRedraw the screen.\n"
        "?\t\tDisplay this list of commands.\n"
        "^D\t\tExit the program.\n"
        "\nNote: If the first character of the pattern you want to search for matches\n"
        "a command, type a \\ character first.\n"
        "Note: Some ctrl keys may be occupied by your terminal configuration.\n"
;

static char changeing_help_msg[] = 
        "When changing text, you can use these single-character commands:\n\n"
        "0-9a-zA-Z\tMark or unmark the line to be changed.\n"
        "*\t\tMark or unmark all displayed lines to be changed.\n"
        "space bar\tDisplay next set of lines.\n"
        "+\t\tDisplay next set of lines.\n"
        "-\t\tDisplay previous set of lines.\n"
        "^A\t\tMark or unmark all lines to be changed.\n"
        "^D\t\tChange the marked lines and exit.\n"
        "ESC\t\tExit without changing the marked lines.\n"
        "!\t\tStart an interactive shell (type ^D to return).\n"
        "^L\t\tRedraw the screen.\n"
        "?\t\tDisplay this list of commands.\n"
;

const char*
help(void)
{
    if (input_mode == INPUT_CHANGE) {
		return help_msg;
    } else {
		return changeing_help_msg;
    }
}

/* error exit including short usage information */
void
error_usage(void)
{
    usage();
    fputs("Try the -h option for more information.\n", stderr);
    myexit(1);
}

/* normal usage message */
void
usage(void)
{
    fputs(
		"Usage: " PROGRAM_NAME " [-bcCdehklLqRTuUvV] [-f file] [-F file] [-i file] [-I dir] [-s dir]\n"
    	"              [-p number] [-P path] [-[0-8] pattern] [source files]\n",
		stderr
	);
}


/* long usage message */
void
longusage(void)
{
    usage();
    fprintf(stderr, "\
\n\
-b            Build the cross-reference only.\n\
-C            Ignore letter case when searching.\n\
-c            Use only ASCII characters in the cross-ref file (don't compress).\n\
-d            Do not update the cross-reference.\n\
-e            Suppress the <Ctrl>-e command prompt between files.\n\
-F symfile    Read symbol reference lines from symfile.\n\
-f reffile    Use reffile as cross-ref file name instead of %s.\n",
        REFFILE);
    fprintf(stderr, "\
-h            This help screen.\n\
-I incdir     Look in incdir for any #include files.\n\
-i namefile   Browse through files listed in namefile, instead of %s\n",
        NAMEFILE);
    fprintf(stderr, "\
-k            Kernel Mode - don't use %s for #include files.\n",
        DFLT_INCDIR);
    fputs("\
-L            Do a single search with line-oriented output.\n\
-l            Line-oriented interface.\n\
-num pattern  Go to input field num (counting from 0) and find pattern.\n\
-P path       Prepend path to relative file names in pre-built cross-ref file.\n\
-p n          Display the last n file path components.\n\
-q            Build an inverted index for quick symbol searching.\n\
-R            Recurse directories for files.\n\
-s dir        Look in dir for additional source  files.\n\
-T            Use only the first eight characters to match against C symbols.\n\
-U            Check file time stamps.\n\
-u            Unconditionally build the cross-reference file.\n\
-v            Be more verbose in line mode.\n\
-V            Print the version number.\n\
\n\
Please see the manpage for more information.\n",
          stderr);
}
