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

/*    cscope - interactive C symbol or text cross-reference
 *
 *    searching functions
 */

#include "global.h"

#include "build.h"
#include "scanner.h" /* for token definitions */

#include <assert.h>
#include <signal.h>
#include <ncurses.h>
#include <regex.h>
#include <setjmp.h> /* jmp_buf */

/* most of these functions have been optimized so their innermost loops have
 * only one test for the desired character by putting the char and
 * an end-of-block marker (\0) at the end of the disk block buffer.
 * When the inner loop exits on the char, an outer loop will see if
 * the char is followed by a \0.  If so, it will read the next block
 * and restart the inner loop.
 */

char *blockp;							/* pointer to current char in block */
char  block[BUFSIZ + 2];				/* leave room for end-of-block mark */
int	  blocklen;							/* length of disk block read */
char  blockmark;						/* mark character to be searched for */
long  blocknumber;						/* block number */

static char		global[] = "<global>";	/* dummy global function name */
static char		cpattern[PATLEN + 1];	/* compressed pattern */
static long		lastfcnoffset;			/* last function name offset */
static POSTING *postingp;				/* retrieved posting set pointer */
static long		postingsfound;			/* retrieved number of postings */
static regex_t	regexp;					/* regular expression */
static bool		isregexp_valid = false; /* regular expression status */

static bool		match(void);
static bool		matchrest(void);
static POSTING *getposting(void);
static char	   *lcasify(const char *s);
static void		findcalledbysub(const char *file, bool macro);
static void		findterm(const char *pattern);
static void		putline(FILE *output);
static char	   *find_symbol_or_assignment(const char *pattern, bool assign_flag);
static bool		check_for_assignment(void);
static void		putpostingref(POSTING *p, const char *pat);
static void		putref(int seemore, const char *file, const char *func);
static void		putsource(int seemore, FILE *output);
static FILE	   *nonglobalrefs; /* non-global references file */

static sigjmp_buf env;		   /* setjmp/longjmp buffer */

typedef enum {				   /* findinit return code */
	NOERROR,
	NOTSYMBOL,
	REGCMPERROR
} FINDINIT;

static char *findsymbol(const char *pattern);
static char *finddef(const char *pattern);
static char *findcalledby(const char *pattern);
static char *findcalling(const char *pattern);
static char *findstring(const char *pattern);
static char *findregexp(const char *egreppat);
static char *findfile(const char *dummy);
static char *findinclude(const char *pattern);
static char *findassign(const char *pattern);
static char *findallfcns(const char *dummy);


static inline void trim_trailing_ws(char *str, size_t len);
static inline bool is_valid_c_symbol(char *str);

typedef char *(*FP)(const char *); /* pointer to function returning a character pointer */
/* Paralel array to "fields", indexed by "field" */
FP field_searchers[FIELDS + 1] = {
	findsymbol,
	finddef,
	findcalledby,
	findcalling,
	findstring,
	findstring,
	findregexp,
	findfile,
	findinclude,
	findassign,
	findallfcns /* samuel only */
};

/**/
struct TI {
	char *text1;
	char *text2;
};
extern struct TI fields[FIELDS + 1];

/* Internal prototypes: */
static void jumpback(int sig);

static void jumpback(int sig) {
	signal(sig, jumpback);
	siglongjmp(env, 1);
}

/* find the symbol in the cross-reference */
static
char *findsymbol(const char *pattern) {
	return find_symbol_or_assignment(pattern, false);
}

/* find the symbol in the cross-reference, and look for assignments */
static
char *findassign(const char *pattern) {
	return find_symbol_or_assignment(pattern, true);
}

/* Test reference whether it's an assignment to the symbol found at
 * (global variable) 'blockp' */
static
bool check_for_assignment(void) {
	/* Do the extra work here to determine if this is an
	 * assignment or not.  Do this by examining the next character
	 * or two in blockp */
	char *asgn_char = blockp;

	while(isspace((unsigned char)asgn_char[0])) {
		/* skip any whitespace or \n */
		asgn_char++;
		if(asgn_char[0] == '\0') {
			/* get the next block when we reach the end of
			 * the current block */
			if(NULL == (asgn_char = read_crossreference_block())){
				return false;
			}
		}
	}
	/* check for digraph starting with = */
	if((asgn_char[0] & 0x80) && (dichar1[(asgn_char[0] & 0177) / 8] == '=')) {
		return true;
	}
	/* check for plain '=', not '==' */
	if((asgn_char[0] == '=') &&
		(((asgn_char[1] != '=') && !(asgn_char[1] & 0x80)) ||
			((asgn_char[1] & 0x80) && (dichar1[(asgn_char[1] & 0177) / 8] != '=')))) {
		return true;
	}

	/* check for operator assignments: +=, ... ^= ? */
	if((asgn_char[1] == '=')
    || (((asgn_char[1] & 0x80) && (dichar1[(asgn_char[1] & 0177) / 8] == '='))
	   && ((asgn_char[0] == '+') || (asgn_char[0] == '-') || (asgn_char[0] == '*') ||
		   (asgn_char[0] == '/') || (asgn_char[0] == '%') || (asgn_char[0] == '&') ||
		   (asgn_char[0] == '|') || (asgn_char[0] == '^'))
	)) {
		return true;
	}

	/* check for two-letter operator assignments: <<= or >>= ? */
	if(((asgn_char[0] == '<') || (asgn_char[0] == '>')) &&
		(asgn_char[1] == asgn_char[0]) &&
		((asgn_char[2] == '=') ||
			((asgn_char[2] & 0x80) && (dichar1[(asgn_char[2] & 0177) / 8] == '=')))
	) {
		return true;
	}
	return false;
}


/* 14-12-2024 21:24 yama
 * FIXME If we use the 'find assignments to this symbol' for a search such as 'pp'
   it will retrieve results of symbols named 'p'
 */
/* The actual routine that does the work for findsymbol() and
 * findassign() */
static
char *find_symbol_or_assignment(const char *pattern, bool assign_flag) {
	char   file[PATHLEN + 1];	 /* source file name */
	char   function[PATLEN + 1]; /* function name */
	char   macro[PATLEN + 1];	 /* macro name */
	char   symbol[PATLEN + 1];	 /* symbol name */
	char  *cp;
	char  *s;
	size_t s_len = 0;
	char   firstchar; /* first character of a potential symbol */
	bool   fcndef = false;

	if((invertedindex == true) && (assign_flag == false)) {
		long	 lastline = 0;
		POSTING *p;

		findterm(pattern);
		while((p = getposting()) != NULL) {
			if(p->type != INCLUDE && p->lineoffset != lastline) {
				putpostingref(p, 0);
				lastline = p->lineoffset;
			}
		}
		return NULL;
	}

	scanpast('\t');	  /* find the end of the header */
	skiprefchar();			  /* skip the file marker */
	fetch_string_from_dbase(file, sizeof(file));
	strcpy(function, global); /* set the dummy global function name */
	strcpy(macro, global);	  /* set the dummy global macro name */

	/* find the next symbol */
	/* note: this code was expanded in-line for speed */
	/* other macros were replaced by code using cp instead of blockp */
	cp = blockp;
	for(;;) {
		setmark('\n');
		do { /* innermost loop optimized to only one test */
			while(*cp != '\n') {
				++cp;
			}
		} while(*(cp + 1) == '\0' && (cp = read_crossreference_block()) != NULL);

		/* skip the found character */
		if(cp != NULL && *(++cp + 1) == '\0') { cp = read_crossreference_block(); }
		if(cp == NULL) { break; }
		/* look for a source file, function, or macro name */
		if(*cp == '\t') {
			blockp = cp;
			switch(getrefchar()) {

				case NEWFILE: /* file name */

					/* save the name */
					skiprefchar();
					fetch_string_from_dbase(file, sizeof(file));

					/* check for the end of the symbols */
					if(*file == '\0') { return NULL; }
					progress("Search", searchcount, nsrcfiles);
					/* FALLTHROUGH */

				case FCNEND:		 /* function end */
					(void)strcpy(function, global);
					goto notmatched; /* don't match name */

				case FCNDEF:		 /* function name */
					fcndef = true;
					s	   = function;
					s_len  = sizeof(function);
					break;

				case DEFINE: /* macro name */
					if(fileversion >= 10) {
						s	  = macro;
						s_len = sizeof(macro);
					} else {
						s	  = symbol;
						s_len = sizeof(symbol);
					}
					break;

				case DEFINEEND: /* macro end */
					(void)strcpy(macro, global);
					goto notmatched;

				case INCLUDE:		 /* #include file */
					goto notmatched; /* don't match name */

				default:			 /* other symbol */
					s	  = symbol;
					s_len = sizeof(symbol);
			}
			/* save the name */
			skiprefchar();
			fetch_string_from_dbase(s, s_len);

			/* see if this is a regular expression pattern */
			if(isregexp_valid == true) {
				if(caseless == true) { s = lcasify(s); }
				if(*s != '\0' && regexec(&regexp, s, (size_t)0, NULL, 0) == 0) {
					goto matched;
				}
			}
			/* match the symbol to the text pattern */
			else if(strequal(pattern, s)) {
				goto matched;
			}
			goto notmatched;
		}
		/* if this is a regular expression pattern */
		if(isregexp_valid == true) {

			/* if this is a symbol */

			/**************************************************
			 * The first character may be a digraph'ed char, so
			 * unpack it into firstchar, and then test that.
			 *
			 * Assume that all digraphed chars have the 8th bit
			 * set (0200).
			 **************************************************/
			if(*cp & 0200) { /* digraph char? */
				firstchar = dichar1[(*cp & 0177) / 8];
			} else {
				firstchar = *cp;
			}

			if(isalpha((unsigned char)firstchar) || firstchar == '_') {
				blockp = cp;
				fetch_string_from_dbase(symbol, sizeof(symbol));
				if(caseless == true) {
					s = lcasify(symbol); /* point to lower case version */
				} else {
					s = symbol;
				}

				/* match the symbol to the regular expression */
				if(*s != '\0' && regexec(&regexp, s, (size_t)0, NULL, 0) == 0) {
					goto matched;
				}
				goto notmatched;
			}
		}
		/* match the character to the text pattern */
		else if(*cp == cpattern[0]) {
			blockp = cp;

			/* match the rest of the symbol to the text pattern */
			if(matchrest()) {
				s = NULL;
			matched:
				/* if the assignment flag is set then
				 * we are looking for assignments and
				 * some extra filtering is needed */
				if(assign_flag == true && !check_for_assignment()) goto notmatched;


				/* output the file, function or macro, and source line */
				if(strcmp(macro, global) && s != macro) {
					putref(0, file, macro);
				} else if(fcndef == true || s != function) {
					fcndef = false;
					putref(0, file, function);
				} else {
					putref(0, file, global);
				}
			}
		notmatched:
			if(blockp == NULL) { return NULL; }
			fcndef = false;
			cp	   = blockp;
		}
	}
	blockp = cp;

	return NULL;
}

/* find the function definition or #define */
static
char *finddef(const char *pattern) {
	char file[PATHLEN + 1]; /* source file name */

	if(invertedindex == true) {
		POSTING *p;

		findterm(pattern);
		while((p = getposting()) != NULL) {
			switch(p->type) {
				case DEFINE: /* could be a macro */
				case FCNDEF:
				case CLASSDEF:
				case ENUMDEF:
				case MEMBERDEF:
				case STRUCTDEF:
				case TYPEDEF:
				case UNIONDEF:
				case GLOBALDEF: /* other global definition */
					putpostingref(p, pattern);
			}
		}
		return NULL;
	}


	/* find the next file name or definition */
	while(scanpast('\t') != NULL) {
		switch(*blockp) {

			case NEWFILE:
				skiprefchar();		/* save file name */
				fetch_string_from_dbase(file, sizeof(file));
				if(*file == '\0') { /* if end of symbols */
					return NULL;
				}
				progress("Search", searchcount, nsrcfiles);
				break;

			case DEFINE: /* could be a macro */
			case FCNDEF:
			case CLASSDEF:
			case ENUMDEF:
			case MEMBERDEF:
			case STRUCTDEF:
			case TYPEDEF:
			case UNIONDEF:
			case GLOBALDEF:	   /* other global definition */
				skiprefchar(); /* match name to pattern */
				if(match()) {

					/* output the file, function and source line */
					putref(0, file, pattern);
				}
				break;
		}
	}

	return NULL;
}

/* find all function definitions (used by samuel only) */
static
char *findallfcns(const char *dummy) {
	char file[PATHLEN + 1];	   /* source file name */
	char function[PATLEN + 1]; /* function name */

	(void)dummy;			   /* unused argument */

	/* find the next file name or definition */
	while(scanpast('\t') != NULL) {
		switch(*blockp) {

			case NEWFILE:
				skiprefchar();		/* save file name */
				fetch_string_from_dbase(file, sizeof(file));
				if(*file == '\0') { /* if end of symbols */
					return NULL;
				}
				progress("Search", searchcount, nsrcfiles);
				/* FALLTHROUGH */

			case FCNEND: /* function end */
				(void)strcpy(function, global);
				break;

			case FCNDEF:
			case CLASSDEF:
				skiprefchar(); /* save function name */
				fetch_string_from_dbase(function, sizeof(function));

				/* output the file, function and source line */
				putref(0, file, function);
				break;
		}
	}
	return NULL;
}

/* find the functions calling this function */
static
char *findcalling(const char *pattern) {
	char  file[PATHLEN + 1];	   /* source file name */
	char  function[PATLEN + 1];	   /* function name */
	char  tmpfunc[10][PATLEN + 1]; /* 10 temporary function names */
	char  macro[PATLEN + 1];	   /* macro name */
	char *tmpblockp;
	int	  morefuns, i;

	if(invertedindex == true) {
		POSTING *p;

		findterm(pattern);
		while((p = getposting()) != NULL) {
			if(p->type == FCNCALL) { putpostingref(p, 0); }
		}
		return NULL;
	}
	/* find the next file name or function definition */
	*macro	  = '\0'; /* a macro can be inside a function, but not vice versa */
	tmpblockp = 0;
	morefuns  = 0;	  /* one function definition is normal case */
	for(i = 0; i < 10; i++)
		*(tmpfunc[i]) = '\0';
	while(scanpast('\t') != NULL) {
		switch(*blockp) {

			case NEWFILE: /* save file name */
				skiprefchar();
				fetch_string_from_dbase(file, sizeof(file));
				if(*file == '\0') { /* if end of symbols */
					return NULL;
				}
				progress("Search", searchcount, nsrcfiles);
				(void)strcpy(function, global);
				break;

			case DEFINE: /* could be a macro */
				if(fileversion >= 10) {
					skiprefchar();
					fetch_string_from_dbase(macro, sizeof(macro));
				}
				break;

			case DEFINEEND:
				*macro = '\0';
				break;

			case FCNDEF: /* save calling function name */
				skiprefchar();
				fetch_string_from_dbase(function, sizeof(function));
				for(i = 0; i < morefuns; i++)
					if(!strcmp(tmpfunc[i], function)) break;
				if(i == morefuns) {
					(void)strcpy(tmpfunc[morefuns], function);
					if(++morefuns >= 10) morefuns = 9;
				}
				break;

			case FCNEND:
				for(i = 0; i < morefuns; i++)
					*(tmpfunc[i]) = '\0';
				morefuns = 0;
				break;

			case FCNCALL: /* match function called to pattern */
				skiprefchar();
				if(match()) {

					/* output the file, calling function or macro, and source */
					if(*macro != '\0') {
						putref(1, file, macro);
					} else {
						tmpblockp = blockp;
						for(i = 0; i < morefuns; i++) {
							blockp = tmpblockp;
							putref(1, file, tmpfunc[i]);
						}
					}
				}
		}
	}

	return NULL;
}

/* find the text in the source files */
static
char *findstring(const char *pattern) {
	char		egreppat[2 * PATLEN];
	char	   *cp = egreppat;
	const char *pp;

	/* translate special characters in the regular expression */
	for(pp = pattern; *pp != '\0'; ++pp) {
		if(strchr(".*[\\^$+?|()", *pp) != NULL) { *cp++ = '\\'; }
		*cp++ = *pp;
	}
	*cp = '\0';

	/* search the source files */
	return (findregexp(egreppat));
}

/* find this regular expression in the source files */
static
char *findregexp(const char *egreppat) {
	unsigned int i;
	char		*egreperror;

	/* compile the pattern */
	if((egreperror = egrepinit(egreppat)) == NULL) {

		/* search the files */
		for(i = 0; i < nsrcfiles; ++i) {
			const char * file = prepend_path(prependpath, srcfiles[i]);

			progress("Search", searchcount, nsrcfiles);
			if(egrep(file, refsfound, "%s <unknown> %ld ") < 0) {
				posterr("Cannot open file %s", file);
			}
		}
	}
	return (egreperror);
}

/* find matching file names */
static
char *findfile(const char *dummy) {
	UNUSED(dummy);

	unsigned int i;

	for(i = 0; i < nsrcfiles; ++i) {
		char *s;

		if(caseless == true) {
			s = lcasify(srcfiles[i]);
		} else {
			s = srcfiles[i];
		}
		if(regexec(&regexp, s, (size_t)0, NULL, 0) == 0) {
			(void)fprintf(refsfound, "%s <unknown> 1 <unknown>\n", srcfiles[i]);
		}
	}

	return NULL;
}

/* find files #including this file */
static
char *findinclude(const char *pattern) {
	char file[PATHLEN + 1]; /* source file name */

	if(invertedindex == true) {
		POSTING *p;

		findterm(pattern);
		while((p = getposting()) != NULL) {
			if(p->type == INCLUDE) { putpostingref(p, 0); }
		}
		return NULL;
	}

	/* find the next file name or function definition */
	while(scanpast('\t') != NULL) {
		switch(*blockp) {

			case NEWFILE: /* save file name */
				skiprefchar();
				fetch_string_from_dbase(file, sizeof(file));
				if(*file == '\0') { /* if end of symbols */
					return NULL;
				}
				progress("Search", searchcount, nsrcfiles);
				break;

			case INCLUDE:	   /* match function called to pattern */
				skiprefchar();
				skiprefchar(); /* skip global or local #include marker */
				if(match()) {

					/* output the file and source line */
					putref(0, file, global);
				}
		}
	}

	return NULL;
}

static inline
void trim_trailing_ws(char * str, size_t len) {
	char * end = str + len;

	while (end > str
       && isspace(*end)) {
		--end;
	}
	*end = '\0';
}


static inline
bool is_valid_c_symbol(char *str) {
    /* C symbols are of the following pattern:
     * [$_a-zA-Z][_a-zA-Z0-9]*
     */
	if ((!isalpha((unsigned char)*str))
    &&  (*str != '_')
    &&  (*str != '$')) {
		return false;
	}

	while (*++str != '\0') {
		if ((!isalnum((unsigned char)*str))
        &&  (*str != '_')) {
			return false;
		}
	}
	return true;
}

/* initialize */
int findinit(const char *pattern_) {

	if (pattern_ == NULL
    ||  pattern_[0] == '\0') {
		return NOTSYMBOL;
	}

	char		 *pattern = strdup(pattern_);
	int			  r		  = NOERROR;
	char		  buf[PATLEN + 3];
	bool		  isregexp = false;
	int			  i;
	char		 *s;
	unsigned char c;
	const unsigned int truncation_len = 8;

	/* HBB: be nice: free regexp before allocating a new one */
	if(isregexp_valid == true) regfree(&regexp);

	isregexp_valid = false;

	/* Pattern length */
	size_t pattlen = strlen(pattern);

	/* 14-12-2024 23:27 yama
	 * NOTE: It is necessary to check the length because 'pattern' && 'pattern_'
	   could be non-null while still being 0 length.
	 */
	if (pattlen == 0) {
		free(pattern);
		return NOTSYMBOL;
	}

	/* Trim trailing whitespace */
	trim_trailing_ws(pattern, pattlen);
    if (pattern[0] == '\0') {
        return NOTSYMBOL;
    }

	/* Make sure pattern is lowercased. Curses
	 * mode gets this right all on its own, but at least -L mode
	 * doesn't */
	if(caseless == true) { pattern = lcasify(pattern); }

	/* allow a partial match for a file name */
	if(field == FILENAME || field == INCLUDES) {
		if(regcomp(&regexp, pattern, REG_EXTENDED | REG_NOSUB) != 0) {
			r = REGCMPERROR;
		} else {
			isregexp_valid = true;
		}
		goto end;
	}
	/* see if the pattern is a regular expression */
	if(strpbrk(pattern, "^.[{*+$|(") != NULL) {
		isregexp = true;
	} else {
		/* check for a valid C symbol */
		s = pattern;
		if (!is_valid_c_symbol(s)) {
			r = NOTSYMBOL;
			goto end;
		}

		/* look for use of the -T option (truncate symbol to 8
		   characters) on a database not built with -T */
		if(trun_syms == true && preserve_database == true && dbtruncated == false &&
			s - pattern >= truncation_len) {
			strcpy(pattern + truncation_len, ".*");
			isregexp = true;
		}
	}
	/* if this is a regular expression or letter case is to be ignored */
	/* or there is an inverted index */
	if(isregexp == true || caseless == true || invertedindex == true) {
		/* remove a leading ^ */
		s = pattern;
		if(*s == '^') {
			strcpy(newpat, s + 1);
			strcpy(s, newpat);
		}
		/* remove a trailing $ */
		i = strlen(s) - 1;
		if(s[i] == '$') {
			if(i > 0 && s[i - 1] == '\\') { s[i - 1] = '$'; }
			s[i] = '\0';
		}
		/* if requested, try to truncate a C symbol pattern */
		if(trun_syms == true && strpbrk(s, "[{*+") == NULL) { s[truncation_len] = '\0'; }
		/* must be an exact match */
		/* note: regcomp doesn't recognize ^*keypad$ as a syntax error
				 unless it is given as a single arg */
		snprintf(buf, sizeof(buf), "^%s$", s);
		if(regcomp(&regexp, buf, REG_EXTENDED | REG_NOSUB) != 0) {
			r = REGCMPERROR;
			goto end;
		} else {
			isregexp_valid = true;
		}
	} else {
		/* if requested, truncate a C symbol pattern */
		if(trun_syms == true && field <= CALLING) { pattern[truncation_len] = '\0'; }
		/* compress the string pattern for matching */
		s = cpattern;
		for(i = 0; (c = pattern[i]) != '\0'; ++i) {
			if(IS_A_DICODE(c, pattern[i + 1])) {
				c = DICODE_COMPRESS(c, pattern[i + 1]);
				++i;
			}
			*s++ = c;
		}
		*s = '\0';
	}

end:
	free(pattern);
	return r;
}

void findcleanup(void) {
	/* discard any regular expression */
}

/* match the pattern to the string */
static
bool match(void) {
	char string[PATLEN + 1];

	/* see if this is a regular expression pattern */
	if(isregexp_valid == true) {
		fetch_string_from_dbase(string, sizeof(string));
		if(*string == '\0') { return (false); }
		if(caseless == true) {
			return (regexec(&regexp, lcasify(string), (size_t)0, NULL, 0) ? false : true);
		} else {
			return (regexec(&regexp, string, (size_t)0, NULL, 0) ? false : true);
		}
	}
	/* it is a string pattern */
	return ((bool)(*blockp == cpattern[0] && matchrest()));
}

/* match the rest of the pattern to the name */
static
bool matchrest(void) {
	int i = 1;

	skiprefchar();
	do {
		while(*blockp == cpattern[i]) {
			++blockp;
			++i;
		}
	} while(*(blockp + 1) == '\0' && read_crossreference_block() != NULL);

	if(*blockp == '\n' && cpattern[i] == '\0') { return (true); }
	return (false);
}

/* put the reference into the file */
static
void putref(int seemore, const char *file, const char *func) {
	FILE *output;

	if(strcmp(func, global) == 0) {
		output = refsfound;
	} else {
		output = nonglobalrefs;
	}
	fprintf(output, "%s %s ", file, func);
	putsource(seemore, output);
}

/* put the source line into the file */
static
void putsource(int seemore, FILE *output) {
	char *tmpblockp;
	char *cp, nextc = '\0';
	bool  Change = false, retreat = false;

	if(fileversion <= 5) {
		scanpast(' ');
		putline(output);
		putc('\n', output);
		return;
	}
	/* scan back to the beginning of the source line */
	cp = tmpblockp = blockp;
	while(*cp != '\n' || nextc != '\n') {
		nextc = *cp;
		if(--cp < block) {
			retreat = true;
			/* read the previous block */
			dbseek((blocknumber - 1) * BUFSIZ);
			cp = block + (BUFSIZ - 1);
		}
	}
	blockp = cp;
	if(*blockp != '\n' || getrefchar() != '\n' ||
		(!isdigit(getrefchar()) && fileversion >= 12)) {
		postfatal("Internal error: cannot get source line from database");
		/* NOTREACHED */
	}
	/* until a double newline is found */
	do {
		/* skip a symbol type */
		if(*blockp == '\t') {
			/* if retreat == true, that means tmpblockp and blockp
			 * point to different blocks.  Offset comparison should
			 * falseT be performed until they point to the same block.
			 */
			if(seemore && Change == false && retreat == false && blockp > tmpblockp) {
				Change = true;
				cp	   = blockp;
			}
			skiprefchar();
			skiprefchar();
		}
		/* output a piece of the source line */
		putline(output);
		if(retreat == true) retreat = false;
	} while(blockp != NULL && getrefchar() != '\n');
	putc('\n', output);
	if(Change == true) blockp = cp;
}

/* put the rest of the cross-reference line into the file */
static
void putline(FILE *output) {
	char	*cp;
	unsigned c;

	setmark('\n');
	cp = blockp;
	do {
		while((c = (unsigned)(*cp)) != '\n') {

			/* check for a compressed digraph */
			if(c > '\177') {
				c &= 0177;
				putc(dichar1[c / 8], output);
				putc(dichar2[c & 7], output);
			}
			/* check for a compressed keyword */
			else if(c < ' ') {
				fputs(keyword[c].text, output);
				if(keyword[c].delim != '\0') { putc(' ', output); }
				if(keyword[c].delim == '(') { putc('(', output); }
			} else {
				putc((int)c, output);
			}
			++cp;
		}
	} while(*(cp + 1) == '\0' && (cp = read_crossreference_block()) != NULL);
	blockp = cp;
}

/* put the rest of the cross-reference line into the string */
void fetch_string_from_dbase(char *s, size_t length) {
	char		*cp;
	unsigned int c;

	assert(length > sizeof(char *));

	setmark('\n');
	cp = blockp;
	do {
		while(length > 1 && (c = (unsigned int)(*cp)) != '\n') {
			if(c >= 0x80 && length > 2) {
				c &= 0x7f;
				*s++ = dichar1[c / 8];
				*s++ = dichar2[c & 7];
				length -= 2;
			} else {
				*s++ = c;
				length--;
			}
			++cp;
		}
	} while(length > 0 && cp[1] == '\0' && (cp = read_crossreference_block()) != NULL);
	blockp = cp;
	*s	   = '\0';
}

/* scan past the next occurence of this character in the cross-reference */
char *scanpast(char c) {
	char *cp;

	setmark(c);
	cp = blockp;
	do { /* innermost loop optimized to only one test */
		while(*cp != c) {
			++cp;
		}
	} while(*(cp + 1) == '\0' && (cp = read_crossreference_block()) != NULL);
	blockp = cp;
	if(cp != NULL) { skiprefchar(); /* skip the found character */ }
	return blockp;
}

/* read a block of the cross-reference */
char *read_crossreference_block(void) {
	/* read the next block */
	blocklen = read(symrefs, block, BUFSIZ);
	blockp	 = block;

	/* add the search character and end-of-block mark */
	block[blocklen]		= blockmark;
	block[blocklen + 1] = '\0';

	/* return NULL on end-of-file */
	if(blocklen == 0) {
		blockp = NULL;
	} else {
		++blocknumber;
	}
	return (blockp);
}

static
char *lcasify(const char *s) {
	static char ls[PATLEN + 1]; /* largest possible match string */
	char	   *lptr = ls;

	while(*s) {
		*lptr = tolower((unsigned char)*s);
		lptr++;
		s++;
	}
	*lptr = '\0';
	return ls;
}

/* find the functions called by this function */
char * findcalledby(const char *pattern) {
	char   file[PATHLEN + 1];	/* source file name */
	char * found_caller = NULL; /* seen calling function? */
	bool   macro        = false;

	if(invertedindex == true) {
		POSTING *p;

		findterm(pattern);
		while((p = getposting()) != NULL) {
			switch(p->type) {
				case DEFINE: /* could be a macro */
				case FCNDEF:
					if(dbseek(p->lineoffset) != -1 &&
						scanpast('\t') != NULL) { /* skip def */
						found_caller = (char*)0x01;
						findcalledbysub(srcfiles[p->fileindex], macro);
					}
			}
		}
		return found_caller;
	}
	/* find the function definition(s) */
	while(scanpast('\t') != NULL) {
		switch(*blockp) {

			case NEWFILE:
				skiprefchar();		/* save file name */
				fetch_string_from_dbase(file, sizeof(file));
				if(*file == '\0') { /* if end of symbols */
					return found_caller;
				}
				progress("Search", searchcount, nsrcfiles);
				break;

			case DEFINE: /* could be a macro */
				if(fileversion < 10) { break; }
				macro = true;
				/* FALLTHROUGH */

			case FCNDEF:
				skiprefchar(); /* match name to pattern */
				if(match()) {
					found_caller = (char*)0x01;
					findcalledbysub(file, macro);
				}
				break;
		}
	}

	return found_caller;
}

/* find this term, which can be a regular expression */
static
void findterm(const char *pattern) {
	char *s;
	int	  len;
	char  prefix[PATLEN + 1];
	char  term[PATLEN + 1];

	npostings	  = 0; /* will be non-zero after database built */
	lastfcnoffset = 0; /* clear the last function name found */
	boolclear();	   /* clear the posting set */

	/* get the string prefix (if any) of the regular expression */
	strcpy(prefix, pattern);
	if((s = strpbrk(prefix, ".[{*+")) != NULL) { *s = '\0'; }
	/* if letter case is to be ignored */
	if(caseless == true) {

		/* convert the prefix to upper case because it is lexically
		   less than lower case */
		s = prefix;
		while(*s != '\0') {
			*s = toupper((unsigned char)*s);
			++s;
		}
	}
	/* find the term lexically >= the prefix */
	UNUSED(invfind(&invcontrol, prefix));
	if(caseless == true) { /* restore lower case */
		UNUSED(strcpy(prefix, lcasify(prefix)));
	}
	/* a null prefix matches the null term in the inverted index,
	   so move to the first real term */
	if(*prefix == '\0') { invforward(&invcontrol); }
	len = strlen(prefix);
	do {
		UNUSED(invterm(&invcontrol, term)); /* get the term */
		s = term;
		if(caseless == true) { s = lcasify(s); /* make it lower case */ }
		/* if it matches */
		if(regexec(&regexp, s, (size_t)0, NULL, 0) == 0) {

			/* add its postings to the set */
			if((postingp = boolfile(&invcontrol, &npostings, bool_OR)) == NULL) { break; }
		}
		/* if there is a prefix */
		else if(len > 0) {

			/* if ignoring letter case and the term is out of the
			   range of possible matches */
			if(caseless == true) {
				if(strncmp(term, prefix, len) > 0) { break; /* stop searching */ }
			}
			/* if using letter case and the prefix doesn't match */
			else if(strncmp(term, prefix, len) != 0) {
				break; /* stop searching */
			}
		}
		/* display progress about every three seconds */
		if(++searchcount % 50 == 0) {
			progress("Symbols matched", searchcount, totalterms);
		}
	} while(invforward(&invcontrol)); /* while didn't wrap around */

	/* initialize the progress message for retrieving the references */
	searchcount	  = 0;
	postingsfound = npostings;
}

/* get the next posting for this term */
static
POSTING *getposting(void) {
	if(npostings-- <= 0) { return (NULL); }
	/* display progress about every three seconds */
	if(++searchcount % 100 == 0) {
		progress("Possible references retrieved", searchcount, postingsfound);
	}
	return postingp++;
}

/* put the posting reference into the file */

static
void putpostingref(POSTING *p, const char *pat) {
	// initialize function to "unknown" so that the first line of temp1
	// is properly formed if symbol matches a header file entry first time
	static char function[PATLEN + 1] = "unknown"; /* function name */

	if(p->fcnoffset == 0) {
		if(p->type == FCNDEF) { /* need to find the function name */
			if(dbseek(p->lineoffset) != -1) {
				scanpast(FCNDEF);
				fetch_string_from_dbase(function, sizeof(function));
			}
		} else if(p->type != FCNCALL) {
			strcpy(function, global);
		}
	} else if(p->fcnoffset != lastfcnoffset) {
		if(dbseek(p->fcnoffset) != -1) {
			fetch_string_from_dbase(function, sizeof(function));
			lastfcnoffset = p->fcnoffset;
		}
	}
	if(dbseek(p->lineoffset) != -1) {
		if(pat)
			putref(0, srcfiles[p->fileindex], pat);
		else
			putref(0, srcfiles[p->fileindex], function);
	}
}

/* seek to the database offset */

long dbseek(long offset) {
	long n;
	int	 rc = 0;

	if((n = offset / BUFSIZ) != blocknumber) {
		if((rc = lseek(symrefs, n * BUFSIZ, 0)) == -1) {
			myperror("Lseek failed");
			sleep(3);
			return rc;
		}
		read_crossreference_block();
		blocknumber = n;
	}
	blockp = block + offset % BUFSIZ;
	return rc;
}

static
void findcalledbysub(const char *file, bool macro) {
	/* find the next function call or the end of this function */
	while(scanpast('\t') != NULL) {
		switch(*blockp) {

			case DEFINE:				/* #define inside a function */
				if(fileversion >= 10) { /* skip it */
					while(scanpast('\t') != NULL && *blockp != DEFINEEND)
						;
				}
				break;

			case FCNCALL: /* function call */

				/* output the file name */
				UNUSED(fprintf(refsfound, "%s ", file));

				/* output the function name */
				skiprefchar();
				putline(refsfound);
				UNUSED(putc(' ', refsfound));

				/* output the source line */
				putsource(1, refsfound);
				break;

			case DEFINEEND: /* #define end */

				if(invertedindex == false) {
					if(macro == true) { return; }
					break; /* inside a function */
				}
				/* FALLTHROUGH */

			case FCNDEF: /* function end (pre 9.5) */

				if(invertedindex == false) break;
				/* FALLTHROUGH */

			case FCNEND:  /* function end */
			case NEWFILE: /* file end */
				return;
		}
	}
}

/* open the references found file for writing */
bool writerefsfound(void) {
	if(refsfound == NULL) {
		if((refsfound = myfopen(temp1, "wb")) == NULL) {
			cannotopen(temp1);
			return false;
		}
	} else {
		fclose(refsfound);
		if((refsfound = myfopen(temp1, "wb")) == NULL) {
			postmsg("Cannot reopen temporary file");
			return false;
		}
	}
	return true;
}

/* Perform token search based on "field" */
bool search(const char *query) {
	char		 msg[MSGLEN + 1];
	char		*findresult = NULL;	   /* find function output */
	bool		 funcexist	= true;	   /* find "function" error */
	FINDINIT	 rc			= NOERROR; /* findinit return code */
	sighandler_t savesig;			   /* old value of signal */
	FP			 f;					   /* searching function */
	int			 c;

	/* open the references found file for writing */
	if(writerefsfound() == false) { return (false); }
	/* find the pattern - stop on an interrupt */
	if(linemode == false) { postmsg("Searching"); }
	searchcount = 0;
	savesig		= signal(SIGINT, jumpback);
	if(sigsetjmp(env, 1) == 0) {
		f = field_searchers[field];
		if(f == findregexp || f == findstring) {
			findresult = (*f)(query);
		} else {
			if((nonglobalrefs = myfopen(temp2, "wb")) == NULL) {
				cannotopen(temp2);
				return (false);
			}
			if((rc = findinit(query)) == NOERROR) {
				UNUSED(dbseek(0L)); /* read the first block */
				findresult = (*f)(query);
				if(f == findcalledby){
					funcexist = (bool)(findresult);
				}
				findcleanup();

				/* append the non-global references */
				UNUSED(fclose(nonglobalrefs));
				if((nonglobalrefs = myfopen(temp2, "rb")) == NULL) {
					cannotopen(temp2);
					return (false);
				}
				while((c = getc(nonglobalrefs)) != EOF) {
					UNUSED(putc(c, refsfound));
				}
			}
			UNUSED(fclose(nonglobalrefs));
		}
	}
	signal(SIGINT, savesig);

	/* rewind the cross-reference file */
	lseek(symrefs, (long)0, 0);

	/* reopen the references found file for reading */
	fclose(refsfound);
	if((refsfound = myfopen(temp1, "rb")) == NULL) {
		cannotopen(temp1);
		return (false);
	}
	totallines = 0;
	disprefs   = 0;

	/* see if it is empty */
	if((c = getc(refsfound)) == EOF) {
		if(findresult != NULL) {
			snprintf(msg,
				sizeof(msg),
				"Egrep %s in this pattern: %s",
				findresult,
				query);
		} else if(rc == NOTSYMBOL) {
			snprintf(msg, sizeof(msg), "This is not a C symbol: %s", query);
		} else if(rc == REGCMPERROR) {
			snprintf(msg,
				sizeof(msg),
				"Error in this regcomp(3) regular expression: %s",
				query);

		} else if(funcexist == false) {
			snprintf(msg,
				sizeof(msg),
				"Function definition does not exist: %s",
				query);
		} else {
			snprintf(msg,
				sizeof(msg),
				"Could not find the %s: %s",
				fields[field].text2,
				query);
		}
		postmsg(msg);
		return (false);
	}
	/* put back the character read */
	ungetc(c, refsfound);

	countrefs();

	window_change |= CH_RESULT;

	return (true);
}
