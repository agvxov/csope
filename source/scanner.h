#ifndef CSCOPE_SCANNER_H
#define CSCOPE_SCANNER_H

#include <stdio.h>

#undef YYLMAX
#define YYLMAX STMTMAX + PATLEN + 1 /* scanner line buffer size */

/* cross-reference database mark characters (when new ones are added,
 * update the cscope.out format description in cscope.1)
 */
#define CLASSDEF  'c'
#define DEFINE	  '#'
#define DEFINEEND ')'
#define ENUMDEF	  'e'
#define FCNCALL	  '`'
#define FCNDEF	  '$'
#define FCNEND	  '}'
#define GLOBALDEF 'g'
#define INCLUDE	  '~'
#define MEMBERDEF 'm'
#define NEWFILE	  '@'
#define STRUCTDEF 's'
#define TYPEDEF	  't'
#define UNIONDEF  'u'

/* other scanner token types */
#define LEXEOF	0
#define LEXERR	1
#define IDENT	2
#define NEWLINE 3

/* scanner.l global data */
extern int	 first;		/* buffer index for first char of symbol */
extern int	 last;		/* buffer index for last char of symbol */
extern int	 lineno;	/* symbol line number */
extern FILE *yyin;		/* input file descriptor */
extern FILE *yyout;		/* output file */
extern int	 myylineno; /* input line number */

extern char	 *my_yytext; /* private copy of input line */
extern size_t my_yyleng; /* ... and current length of it */

/* The master function exported by scanner.l */
int	 yylex(void);
void initscanner(char *srcfile);

#endif /* CSCOPE_SCANNER_H ends */
