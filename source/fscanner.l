%{
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

/*	cscope - interactive C symbol cross-reference
 *
 *	C symbol scanner
 */
#include "global.h"

#include "scanner.h"
#include "lookup.h"

/* the line counting has been moved from character reading for speed */
/* comments are discarded */

#ifndef FLEX_SCANNER
# error Sorry, this scanner needs flex. It is not usable with AT&T Lex.
#endif

#define	IFLEVELINC	5	/* #if nesting level size increment */
#define YY_false_TOP_STATE 1

int	first;	/* buffer index for first char of symbol */
int	last;	/* buffer index for last char of symbol */
int	lineno;	/* symbol line number */
int	myylineno = 1;

/* HBB 20001007: new variables, emulating yytext in a way that allows
 * the yymore() simulation, my_yymore(), to be used even in the presence of
 * yyless(). */
size_t my_yyleng = 0;
char *my_yytext = NULL;

enum {
    FILETYPE_IRRELEVANT,
    FILETYPE_LEX,
    FILETYPE_SDL,
    FILETYPE_BPLISTING,
} filetype = FILETYPE_IRRELEVANT;

static	bool	arraydimension;		/* inside array dimension declaration */
static	int	braces;			/* unmatched left brace count */
static	bool	classdef;		/* c++ class definition */
static	bool	elseelif;		/* #else or #elif found */
static	bool	esudef;			/* enum/struct/union global definition */
static	bool	external;		/* external definition */
static	int	externalbraces;		/* external definition outer brace count */
static	bool	fcndef;			/* function definition */
static	bool	global;			/* file global scope (outside functions) */
static	size_t	iflevel;		/* #if nesting level */
static	bool	initializer;		/* data initializer */
static	int	initializerbraces;	/* data initializer outer brace count */
static	size_t	miflevel = IFLEVELINC;	/* maximum #if nesting level */
static	int	*maxifbraces;		/* maximum brace count within #if */
static	int	*preifbraces;		/* brace count before #if */
static	int	parens;			/* unmatched left parenthesis count */
static	bool	ppdefine;		/* preprocessor define statement */
static	bool	pseudoelif;		/* pseudo-#elif */
static	bool	oldtype;		/* next identifier is an old type */
static	bool	rules;			/* lex/yacc rules */
static	bool	structfield;		/* structure field declaration */
static	int	tagdef;			/* class/enum/struct/union tag definition */
static	bool	template;		/* function template */
static	int	templateparens;		/* function template outer parentheses count */
static	int	typedefbraces = -1;	/* initial typedef brace count */
static	int	token;			/* token found */
static	int	ident_start;		/* begin of preceding identifier */

static	void	my_yymore(void);

%}
%x IN_PREPROC WAS_ENDIF WAS_IDENTIFIER WAS_ESU IN_DQUOTE IN_SQUOTE COMMENT

/* XXX:
 *  the {comment} pattern seems unnecesarry as the COMMENT state unputs a ' ',
 *  but i do not dare to touch it until we have tests
 */
identifier	[a-zA-Z_$][a-zA-Z_0-9$]*
number		\.?[0-9][.0-9a-fA-FlLuUxX]*
comment		"/*"([^*]*("*"+[^/])?)*"*/"|"//"[^\n]*\n
ws		    [ \t\r\v\f]
wsnl		[ \t\r\v\f\n]|{comment}

class_definition    class{wsnl}+{identifier}({wsnl}|{identifier}|[():])*\{
function_definition {ws}*\(({wsnl}|{identifier}|{number}|[*&[\]=,.:])*\)([()]|{wsnl})*[:a-zA-Z_#{]
function_call       {ws}*\(([*&[\]=,.]|{identifier}|{number}|{wsnl})*

%option stack
%option noyywrap
/* unused */
%option noyy_top_state

%start SDL
%a 4000
%o 7000

%%

%\{		{	/* lex/yacc C declarations/definitions */
			global = true;
			goto more;
		}
%\}		{
			global = false;
			goto more;
		}
^%%		{	/* lex/yacc rules delimiter */
			braces = 0;
			if (rules == false) {
				/* this %% starts the section containing the rules */
				rules = true;

				/* Copy yytext to private buffer, to be able to add further
				 * content following it: */
				my_yymore();

				/* simulate a yylex() or yyparse() definition */
				(void) strcat(my_yytext, " /* ");
				first = strlen(my_yytext);
				if (filetype == FILETYPE_LEX) {
					(void) strcat(my_yytext, "yylex");
				} else {
					/* yacc: yyparse implicitly calls yylex */
					char *s = " yylex()";
					char *cp = s + strlen(s);
					while (--cp >= s) {
						unput(*cp);
					}
					(void) strcat(my_yytext, "yyparse");
				}
				last = strlen(my_yytext);
				(void) strcat(my_yytext, " */");
				my_yyleng = strlen(my_yytext);
				return(FCNDEF);
			} else {
				/* were in the rules section, now comes the closing one */
				rules = false;
				global = true;
				last = first;
				my_yymore();
				return(FCNEND);
			}
		}

<SDL>{
STATE[ \t]+({identifier}|\*)	{ /* sdl state, treat as function def */
			braces = 1;
			fcndef = true;
			token = FCNDEF;
			goto findident;
		}
ENDSTATE[ \t]	{ /* end of an sdl state, treat as end of a function */
			goto endstate;
		}
}

\{		{	/* count unmatched left braces for fcn def detection */
			++braces;

			/* mark an untagged enum/struct/union so its beginning
			   can be found */
			if (tagdef) {
				if (braces == 1) {
					esudef = true;
				}
				token = tagdef;
				tagdef = '\0';
				last = first;
				my_yymore();
				return(token);
			}
			goto more;
		}

\#{ws}*	{ /* start a preprocessor line */
			if (rules == false) {		/* don't consider CPP for lex/yacc rules */
				BEGIN(IN_PREPROC);
            }
			yyleng = 1;	/* get rid of the blanks, if any */
			goto more;
		}

<WAS_ENDIF>{
\n{wsnl}*#{ws}*if(ndef|def)?{ws}+		{
			/* attempt to correct erroneous brace count caused by:
			 *
			 * #if ...
			 * 	... {
			 * #endif
			 * #if ...
			 * 	... {
			 * #endif
			 */
			/* the current #if must not have an #else or #elif */
			if (elseelif == true) {
				goto endif;
				/* NOTREACHED */
			}
			pseudoelif = true;
			BEGIN(INITIAL);
			yyless(1);	/* rescan all but the line ending */
			yy_set_bol(1);
			goto eol;
			/* NOTREACHED */
		}
\n{wsnl}*		 { 	/* an #endif with no #if right after it */
		endif:
			if (iflevel > 0) {
				/* get the maximum brace count for this #if */
				if (braces < maxifbraces[--iflevel]) {
					braces = maxifbraces[iflevel];
				}
			}
			BEGIN(INITIAL);
			yyless(1);
			yy_set_bol(1);
			goto eol;
			/* NOTREACHED */
		}
}

\}		{
			/* could be the last enum member initializer */
			if (braces == initializerbraces) {
				initializerbraces = -1;
				initializer = false;
			}
			if (--braces <= 0) {
		endstate:
				braces = 0;
				classdef = false;
			}
			if (braces == 0 || (braces == 1 && classdef == true)) {

				/* if the end of an enum/struct/union definition */
				if (esudef == true) {
					esudef = false;
				}
				/* if the end of the function */
				else if (fcndef == true) {
					fcndef = false;
					last = first;
					my_yymore();
					return(FCNEND);
				}
			}
			goto more;
			/* NOTREACHED */
		}

\(		{	/* count unmatched left parentheses for function templates */
			++parens;
			goto more;
			/* NOTREACHED */
		}
\)		{
			if (--parens <= 0) {
				parens = 0;
			}
			/* if the end of a function template */
			if (parens == templateparens) {
				templateparens = -1;
				template = false;
			}
			goto more;
			/* NOTREACHED */
		}
=		{	/* if a global definition initializer */
			if (!my_yytext) { return(LEXERR); }
			if (global == true && ppdefine == false && my_yytext[0] != '#') {
				initializerbraces = braces;
				initializer = true;
			}
			goto more;
			/* NOTREACHED */
		}
:		{	/* a if global structure field */
			if (!my_yytext) { return(LEXERR); }
			if (global == true && ppdefine == false && my_yytext[0] != '#') {
				structfield = true;
			}
			goto more;
			/* NOTREACHED */
		}
\,		{
			if (braces == initializerbraces) {
				initializerbraces = -1;
				initializer = false;
			}
			structfield = false;
			goto more;
			/* NOTREACHED */
		}
;		{	/* if the enum/struct/union was not a definition */
			if (braces == 0) {
				esudef = false;
			}
			/* if the end of a typedef */
			if (braces == typedefbraces) {
				typedefbraces = -1;
			}
			/* if the end of a external definition */
			if (braces == externalbraces) {
				externalbraces = -1;
				external = false;
			}
			structfield = false;
			initializer = false;
			goto more;
			/* NOTREACHED */
		}

<IN_PREPROC>{
ifndef{ws}+	|
ifdef{ws}+	|
if{ws}+		{ /* #if directive */
			elseelif = false;
			if (pseudoelif == true) {
				pseudoelif = false;
				goto elif;
				/* NOTREACHED */
			}
			/* make sure there is room for the current brace count */
			if (iflevel == miflevel) {
				miflevel += IFLEVELINC;
				maxifbraces = realloc(maxifbraces, miflevel * sizeof(*maxifbraces));
				preifbraces = realloc(preifbraces, miflevel * sizeof(*preifbraces));
			}
			/* push the current brace count */
			preifbraces[iflevel] = braces;
			maxifbraces[iflevel++] = 0;
			BEGIN(INITIAL);
			goto more;
			/* NOTREACHED */
		}
else({ws}.*)?	{ /* #else --- eat up whole line */
			elseelif = true;
			if (iflevel > 0) {

				/* save the maximum brace count for this #if */
				if (braces > maxifbraces[iflevel - 1]) {
					maxifbraces[iflevel - 1] = braces;
				}
				/* restore the brace count to before the #if */
				braces = preifbraces[iflevel - 1];
			}
			BEGIN(INITIAL);
			goto more;
			/* NOTREACHED */
		}
elif{ws}+	{ /* #elif */
			/* elseelif = true; --- HBB I doubt this is correct */
		elif:
			if (iflevel > 0) {

				/* save the maximum brace count for this #if */
				if (braces > maxifbraces[iflevel - 1]) {
					maxifbraces[iflevel - 1] = braces;
				}
				/* restore the brace count to before the #if */
				braces = preifbraces[iflevel - 1];
			}
			BEGIN(INITIAL);
			goto more;
			/* NOTREACHED */
		}
endif([^a-zA-Z0-9_$\n].*)?	{	/* #endif */
			/* delay treatment of #endif depending on whether an
			 * #if comes right after it, or not */
			BEGIN(WAS_ENDIF);
			goto more;
		}

include{ws}*\"[^"\n]+\" |
include{ws}*<[^>\n]+> 	{ /* #include file */
			char	*s;
			char remember = yytext[yyleng-1];

			my_yymore();
			s = strpbrk(my_yytext, "\"<");
			if (!s) { return(LEXERR); }
			my_yytext[my_yyleng-1] = '\0';
			incfile(s + 1, s);
			my_yytext[my_yyleng-1] = remember;
			first = s - my_yytext;
			last = my_yyleng - 1;
			if (compress == true) {
				my_yytext[0] = '\2';	/* compress the keyword */
			}
			BEGIN(INITIAL);
			return(INCLUDE);
			/* NOTREACHED */
		}
define{ws}+{identifier}	{

			/* preprocessor macro or constant definition */
			ppdefine = true;
			token = DEFINE;
			if (compress == true) {
				my_yytext[0] = '\1';	/* compress the keyword */
			}
		findident:
			/* search backwards through yytext[] to find the identifier */
			/* falseTE: this had better be left to flex, by use of
			 * yet another starting condition */
			my_yymore();
			first = my_yyleng - 1;
			while (my_yytext[first] != ' ' && my_yytext[first] != '\t') {
				--first;
			}
			++first;
			last = my_yyleng;
			BEGIN(INITIAL);
			goto definition;
			/* NOTREACHED */
		}
\n	{   /* unknown preprocessor line */
			BEGIN(INITIAL);
                        ++myylineno;
			goto more;
			/* NOTREACHED */
		}
.		 |
{identifier}	{   /* unknown preprocessor line */
			BEGIN(INITIAL);
			goto more;
			/* NOTREACHED */
		}
}

{class_definition}	{	/* class definition */
			classdef = true;
			tagdef =  'c';
			yyless(5);		/* eat up 'class', and re-scan */
			yy_set_bol(0);
			goto more;
			/* NOTREACHED */
		}

("enum"|"struct"|"union")	{
			ident_start = first;
			BEGIN(WAS_ESU);
			goto more;
		}

<WAS_ESU>{
({wsnl}+{identifier}){wsnl}*\{		{ /* e/s/u definition */
			tagdef = my_yytext[ident_start];
			BEGIN(WAS_IDENTIFIER);
			goto ident;
		}
{wsnl}*\{		{ /* e/s/u definition without a tag */
			tagdef = my_yytext[ident_start];
			BEGIN(INITIAL);
			if (braces == 0) {
				esudef = true;
			}
			last = first;
			yyless(0);  /* re-scan all this as normal text */
			tagdef = '\0';
			goto more;
		}
({wsnl}+{identifier})?{wsnl}* 	|
.|\n										{   /* e/s/u usage */
			BEGIN(WAS_IDENTIFIER);
			goto ident;
		}
}

if{wsnl}*\(	{ 	/* ignore 'if' */
			yyless(2);
			yy_set_bol(0);
			goto more;
}

{identifier}	{	/* identifier found: do nothing, yet. (!) */
			BEGIN(WAS_IDENTIFIER);
			ident_start = first;
			goto more;
			/* NOTREACHED */
		}

<WAS_IDENTIFIER>{
{function_definition}	{
			/* a function definition */
			/* note: "#define a (b) {" and "#if defined(a)\n#"
			 * are not fcn definitions! */
			/* warning: "if (...)" must not overflow yytext,
			 * so the content of function argument definitions
			 * is restricted, in particular parentheses are
			 * not allowed */
			/* FIXME HBB 20001003: the above 'not allowed' may well be the
			 * reason for the parsing bug concerning function pointer usage,
			 * I suspect. --- I think my new special-case rule for 'if'
			 * could be helpful in removing that limitation */
			if ((braces == 0 && ppdefine == false && my_yytext[0] != '#' && rules == false) ||
			    (braces == 1 && classdef == true)) {
				fcndef = true;
				token = FCNDEF;
				goto fcn;
				/* NOTREACHED */
			}
			goto fcncal;
			/* NOTREACHED */
		}
{function_call}		{ 	/* function call */
		fcncal:	if (fcndef == true || ppdefine == true || rules == true) {
				token = FCNCALL;
				goto fcn;
				/* NOTREACHED */
			}
			if (template == false) {
				templateparens = parens;
				template = true;
			}
			goto ident;
			/* NOTREACHED */
		}
("*"|{wsnl})+{identifier}		{	/* typedef name or modifier use */
			goto ident;
			/* NOTREACHED */
		}
.|\n	{		/* general identifer usage */
			char	*s;

			if (global == true && ppdefine == false && my_yytext[0] != '#' &&
			    external == false && initializer == false &&
			    arraydimension == false && structfield == false &&
			    template == false && fcndef == false) {
				if (esudef == true) {
					/* if enum/struct/union */
					token = MEMBERDEF;
				} else {
					token = GLOBALDEF;
				}
			} else {
		ident:
				token = IDENT;
			}
		fcn:
			if (YYSTATE == WAS_IDENTIFIER) {
				/* Position back to the actual identifier: */
				last = first;
				first = ident_start;
				yyless(0);
				/* HBB 20001008: if the anti-backup-pattern above matched,
				 * and the matched context ended with a \n, then the scanner
				 * believes it's at the start of a new line. But the yyless()
				 * should feeds that \n back into the input, so that's
				 * wrong. --> force 'beginning-of-line' status off. */
				yy_set_bol(0);
				BEGIN(INITIAL);
			} else {
				my_yymore();
				last = my_yyleng;
			}
		definition:

			/* if a long line */
			if (yyleng > STMTMAX) {
				int	c;

				/* skip to the end of the line */
				warning("line too long");
				while ((c = input()) > LEXEOF) {
					if (c == '\n') {
						unput(c);
						break;
					}
				}
			}
			/* truncate a long symbol */
			if (yyleng > PATLEN) {
				warning("symbol too long");
				my_yyleng = first + PATLEN;
				my_yytext[my_yyleng] = '\0';
			}

			/* if found word was a keyword: */
			if ((s = lookup(my_yytext + first, compress)) != NULL) {
				first = my_yyleng;

				/* if the start of a typedef */
				if (s == typedeftext) {
					typedefbraces = braces;
					oldtype = true;
				}
				/* if an enum/struct/union */
				/* (needed for "typedef struct tag name;" so
				   tag isn't marked as the typedef name) */
				else if (s == enumtext || s == structtext || s == uniontext) {
					/* do nothing */
				} else if (s == externtext) {
					/* if an external definition */
					externalbraces = braces;
					external = true;
				} else if (templateparens == parens && template == true) {
					/* keyword doesn't start a function
					 * template */
					templateparens = -1;
					template = false;
				} else {
					/* identifier after typedef was a
					 * keyword */
					oldtype = false;
				}
			} else {
				/* not a keyword --> found an identifier */
				/* last = yyleng; */

				/* if a class/enum/struct/union tag definition */
				/* FIXME HBB 20001001: why reject "class"? */
				if (tagdef && strnotequal(my_yytext + first, "class")) {
					token = tagdef;
					tagdef = '\0';
					if (braces == 0) {
						esudef = true;
					}
				} else if (braces == typedefbraces && oldtype == false &&
				           arraydimension == false) {
					/* if a typedef name */
					token = TYPEDEF;
				} else {
					oldtype = false;
				}
				/* my_yymore(); */
				return(token);
				/* NOTREACHED */
			}
		}
}

\[		{	/* array dimension (don't worry or about subscripts) */
			arraydimension = true;
			goto more;
			/* NOTREACHED */
		}
\]		{
			arraydimension = false;
			goto more;
			/* NOTREACHED */
		}
\\\n		{	/* preprocessor statement is continued on next line */
			/* save the '\\' to the output file, but not the '\n': */
			yyleng = 1;
			my_yymore();
			goto eol;
			/* NOTREACHED */
		}
\n		{	/* end of the line */
			if (ppdefine == true) {	/* end of a #define */
				ppdefine = false;
				yyless(yyleng - 1);
				last = first;
				my_yymore();
				return(DEFINEEND);
			}
			/* skip the first 8 columns of a breakpoint listing line */
			/* and skip the file path in the page header */
			if (filetype == FILETYPE_BPLISTING) {
				int	c, i;

				/* FIXME HBB 20001007: should call input() instead */
				switch (input()) {	/* tab and EOF just fall through */
                    case ' ':	/* breakpoint number line */
                    case '[': {
                        for (i = 1; i < 8 && input() > LEXEOF; ++i) { ; }
                    } break;
                    case '.':	/* header line */
                    case '/':
                        /* skip to the end of the line */
                        while ((c = input()) > LEXEOF) {
                            if (c == '\n') {
                                unput(c);
                                break;
                            }
                        }
                        break;
                    case '\n': {	/* empty line */
                        unput('\n');
                    } break;
				}
			}
		eol:
			++myylineno;
			first = 0;
			last = 0;
			if (symbols > 0) {
				/* no my_yymore(): \n doesn't need to be in my_yytext */
				return(NEWLINE);
			}
			/* line ended --> flush my_yytext */
			if (my_yytext)
				*my_yytext = '\0';
			my_yyleng = 0;
			lineno = myylineno;
		}

\'		{	/* character constant */
			if (filetype == FILETYPE_SDL) { BEGIN(IN_SQUOTE); }
			goto more;
			/* NOTREACHED */
		}
<IN_SQUOTE>\'	{
			BEGIN(INITIAL);
			goto more;
			/* NOTREACHED */
		}
\"		{	/* string constant */
			BEGIN(IN_DQUOTE);
			goto more;
			/* NOTREACHED */
		}
<IN_DQUOTE>\"	{
			BEGIN(INITIAL);
			goto more;
			/* NOTREACHED */
		}
<IN_DQUOTE,IN_SQUOTE>{
\n	  	{	/* syntax error: unexpected EOL */
			BEGIN(INITIAL);
			goto eol;
			/* NOTREACHED */
		}
\\.	|
.		{
			goto more;
			/* NOTREACHED */
		}
\\\n 	{		/* line continuation inside a string! */
			myylineno++;
			goto more;
			/* NOTREACHED */
		}
}

^{ws}+		{		/* don't save leading white space */
		}

{ws}+\n 	{		/* eat whitespace at end of line */
			unput('\n');
		}

[\t\r\v\f]+	{	/* eat non-blank whitespace sequences, replace
			 * by single blank */
			unput(' ');
		}

{ws}{2,}	{   /* compress sequential whitespace here, not in putcrossref() */
			unput(' ');
 		}

"/*"	{
            yy_push_state(COMMENT);
        }

<COMMENT>{
[^*\n]*			|
"*"+[^*/\n]*	{ ; }
[^*\n]*\n		|
"*"+[^*/\n]*\n	{
                if (ppdefine == false) {
                    goto eol;
                } else {
                    ++myylineno;
                }
                /* NOTREACHED */
		    }
"*"+"/"	    {
                /* replace the comment by a single blank */
                unput(' ');
                yy_pop_state();
		    }
}

"//".*\n?	{
                /* C++-style one-line comment */
                goto eol;
                /* NOTREACHED */
		    }

{number}				|	/* number */
<SDL>STATE[ \t]+		|   /* ... and other syntax error catchers... */
.						{	/* punctuation and operators */
						more:
							my_yymore();
							first = my_yyleng;
						}

%%

void initscanner(char *srcfile) {
	if (maxifbraces == NULL) {
		maxifbraces = malloc(miflevel * sizeof(*maxifbraces));
		preifbraces = malloc(miflevel * sizeof(*preifbraces));
	}

	first = 0;		/* buffer index for first char of symbol */
	last = 0;		/* buffer index for last char of symbol */
	lineno = 1;		/* symbol line number */
	myylineno = 1;		/* input line number */
	arraydimension = false;	/* inside array dimension declaration */
	braces = 0;		/* unmatched left brace count */
	classdef = false;		/* c++ class definition */
	elseelif = false;		/* #else or #elif found */
	esudef = false;		/* enum/struct/union global definition */
	external = false;		/* external definition */
	externalbraces = -1;	/* external definition outer brace count */
	fcndef = false;		/* function definition */
	global = true;		/* file global scope (outside functions) */
	iflevel = 0;		/* #if nesting level */
	initializer = false;	/* data initializer */
	initializerbraces = -1;	/* data initializer outer brace count */
	parens = 0;		/* unmatched left parenthesis count */
	ppdefine = false;		/* preprocessor define statement */
	pseudoelif = false;	/* pseudo-#elif */
	oldtype = false;		/* next identifier is an old type */
	rules = false;		/* lex/yacc rules */
	structfield = false;	/* structure field declaration */
	tagdef = '\0';		/* class/enum/struct/union tag definition */
	template = false;		/* function template */
	templateparens = -1;	/* function template outer parentheses count */
	typedefbraces = -1;	/* initial typedef braces count */
	ident_start = 0;	/* start of previously found identifier */

	if (my_yytext) { *my_yytext = '\0'; }
	my_yyleng = 0;

	BEGIN(INITIAL);

	/* if this is not a C file */
	const char * s = strrchr(srcfile, '.');
	if (s) {
		switch (*++s) {	/* this switch saves time on C files */
            case 'b': {
                if (strcmp(s, "bp") == 0) {	/* breakpoint listing */
                    filetype = FILETYPE_BPLISTING;
                }
            } break;
            case 'l': {
                if (strcmp(s, "l") == 0) {	/* lex */
                    filetype = FILETYPE_LEX;
                    global = false;
                }
            } break;
            case 's': {
                if (strcmp(s, "sd") == 0) {	/* sdl */
                    filetype = FILETYPE_SDL;
                    BEGIN(SDL);
                }
            } break;
            case 'y': {
                if (strcmp(s, "y") == 0) {	/* yacc */
                    // NOTE: yacc has no filetype value
                    global = false;
                }
            } break;
        }
	}
}

#define MY_YY_ALLOCSTEP 1000
static
void my_yymore(void) {
	static size_t yytext_size = 0;

	/* my_yytext is an ever-growing buffer. It will not ever
	 * shrink, nor will it be freed at end of program, for now */
	while (my_yyleng + yyleng + 1 >= yytext_size) {
		my_yytext = realloc(my_yytext, yytext_size += MY_YY_ALLOCSTEP);
	}

	strncpy (my_yytext + my_yyleng, yytext, yyleng+1);
	my_yyleng += yyleng;
}
