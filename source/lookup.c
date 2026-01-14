/*    cscope - interactive C symbol cross-reference
 *
 *    keyword look-up routine for the C symbol scanner
 */

#include "global.h"
#include "lookup.h"

/* keyword text for fast testing of keywords in the scanner */
const char enumtext[]	 = "enum";
const char externtext[]  = "extern";
const char structtext[]  = "struct";
const char typedeftext[] = "typedef";
const char uniontext[]   = "union";

/* This keyword table is also used for keyword text compression.  Keywords
 * with an index less than the numeric value of a space are replaced with the
 * control character corresponding to the index, so they cannot be moved
 * without changing the database file version and adding compatibility code
 * for old databases.
 */
struct keystruct keyword[] = {
    {"",          '\0', NULL}, /* dummy entry */
    {"#define",   ' ',  NULL}, /* must be table entry 1 */
    {"#include",  ' ',  NULL}, /* must be table entry 2 */
    {"break",     '\0', NULL}, /* rarely in cross-reference */
    {"case",      ' ',  NULL},
    {"char",      ' ',  NULL},
    {"continue",  '\0', NULL}, /* rarely in cross-reference */
    {"default",   '\0', NULL}, /* rarely in cross-reference */
    {"double",    ' ',  NULL},
    {"\t",        '\0', NULL}, /* must be the table entry 9 */
    {"\n",        '\0', NULL}, /* must be the table entry 10 */
    {"else",      ' ',  NULL},
    {enumtext,    ' ',  NULL},
    {externtext,  ' ',  NULL},
    {"float",     ' ',  NULL},
    {"for",       '(',  NULL},
    {"goto",      ' ',  NULL},
    {"if",        '(',  NULL},
    {"int",       ' ',  NULL},
    {"long",      ' ',  NULL},
    {"register",  ' ',  NULL},
    {"return",    '\0', NULL},
    {"short",     ' ',  NULL},
    {"sizeof",    '\0', NULL},
    {"static",    ' ',  NULL},
    {structtext,  ' ',  NULL},
    {"switch",    '(',  NULL},
    {typedeftext, ' ',  NULL},
    {uniontext,   ' ',  NULL},
    {"unsigned",  ' ',  NULL},
    {"void",      ' ',  NULL},
    {"while",     '(',  NULL},
 /* these keywords are not compressed */
    {"do",        '\0', NULL},
    {"auto",      ' ',  NULL},
    {"fortran",   ' ',  NULL},
    {"const",     ' ',  NULL},
    {"signed",    ' ',  NULL},
    {"volatile",  ' ',  NULL},
};
#define KEYWORDS (sizeof(keyword) / sizeof(keyword[0]))

#define HASHMOD (KEYWORDS * 2 + 1)

static struct keystruct *hashtab[HASHMOD]; /* pointer table */

/* put the keywords into the symbol table */
void initsymtab(void) {
	for(unsigned i = 1; i < KEYWORDS; i++) {
		struct keystruct *p	= keyword + i;
		int j		        = hash(p->text) % HASHMOD;
		p->next	   = hashtab[j];
		hashtab[j] = p;
	}
}

/* see if this identifier is a keyword */
char * lookup(char *ident, bool do_compressed) {
	struct keystruct *p;
	int				  c;

	/* look up the identifier in the keyword table */
	for(p = hashtab[hash(ident) % HASHMOD]; p != NULL; p = p->next) {
		if(strequal(ident, p->text)) {
			if(compress == true && (c = p - keyword) < ' ') {
				ident[0] = c; /* compress the keyword */
			}
			return (p->text);
		}
	}
	/* this is an identifier */
	return NULL;
}

int hash(const char * ss) {
	int			   i = 0;
	unsigned char *s = (unsigned char *)ss;

    while (*s != '\0') {
		i += *s++; /* += is faster than <<= for cscope */

    }

	return (i);
}
