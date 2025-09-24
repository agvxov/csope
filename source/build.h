#ifndef CSCOPE_BUILD_H
#define CSCOPE_BUILD_H

#include "invlib.h"

/* types and macros of build.c to be used by other modules */

/* database output macros that update its offset */
#define dbputc(c)  (++dboffset, (void)putc(c, newrefs))
#define dbfputs(s) (dboffset += strlen(s), fputs(s, newrefs))

/* declarations for globals defined in build.c */

extern bool buildonly;		  /* only build the database */
extern bool unconditional;	  /* unconditionally build database */
extern bool fileschanged;	  /* assume some files changed */

extern char *tmpdir;		  /* temporary directory */
extern char *reffile;		  /* cross-reference file path name */
extern char *invname;		  /* inverted index to the database */
extern char *invpost;		  /* inverted index postings */
extern char *newreffile;	  /* new cross-reference file name */
extern FILE *newrefs;		  /* new cross-reference */
extern FILE *postings;		  /* new inverted index postings */
extern int	 symrefs;		  /* cross-reference file */
extern long  totalterms;	  /* total inverted index terms */

extern INVCONTROL invcontrol; /* inverted file control structure */

/* Prototypes of external functions defined by build.c */
extern void build(void);
extern void free_newbuildfiles(void);
extern void opendatabase(const char * const reffile);
extern void rebuild(void);
extern void setup_build_filenames(const char * const reffile);
extern void seek_to_trailer(FILE *f);
extern void read_old_reffile(const char * reffile);
extern void initcompress(void);

#endif
