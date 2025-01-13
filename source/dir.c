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
 *    directory searching functions
 */

#include "global.h"

#include "path.h"
#include "vpath.h" /* vpdirs and vpndirs */

#include <stdlib.h>
#include <sys/types.h> /* needed by stat.h and dirent.h */
#include <dirent.h>
#include <sys/stat.h>  /* stat */
#include <assert.h>

#define DIRSEPS " ,:"				   /* directory list separators */
#define DIRINC	10					   /* directory list size increment */
#define HASHMOD 2003				   /* must be a prime number */
#define SRCINC	HASHMOD				   /* source file list size increment */
									   /* largest known database had 22049 files */

char		  currentdir[PATHLEN + 1]; /* current directory */
char		**incdirs;				   /* #include directories */
char		**srcdirs;				   /* source directories */
char		**srcfiles;				   /* source files */
unsigned long nincdirs;				   /* number of #include directories */
unsigned long nsrcdirs;				   /* number of source directories */
unsigned long nsrcfiles;			   /* number of source files */
unsigned long msrcfiles = SRCINC;	   /* maximum number of source files */

static bool firstbuild = true;

static char		   **incnames;		   /* #include directory names without view pathing */
static unsigned long mincdirs = DIRINC; /* maximum number of #include directories */
static unsigned long msrcdirs;			/* maximum number of source directories */
static unsigned long nvpsrcdirs;		/* number of view path source directories */

static struct listitem {				/* source file names without view pathing */
		char			*text;
		struct listitem *next;
} *srcnames[HASHMOD];

/* Internal prototypes: */
static bool is_accessible_file(const char *file);
static bool is_source_file(char *file);
static void add_source_directory(char *dir);
static void add_include_directory(char *name, char *path);
static void scan_dir(const char *dirfile, bool recurse);
static void make_vp_source_directories(void);
static void read_listfile(FILE * listfile);

static
void read_listfile(FILE * names) {
	char line[PATHLEN * 10];
	char path[PATHLEN + 1];
	char  dir[PATHLEN + 1];
	char *s;
	/* get the names in the file */
	while(fgets(line, 10 * PATHLEN, names) != NULL) {
		char  *point_in_line	 = line;
		size_t length_of_name	 = 0;
		int	   unfinished_option = 0;
		bool   done				 = false;

		/* Kill away \n left at end of fgets()'d string: */
        {
            size_t last_char_index = strlen(line) - 1;
            if (line[last_char_index] == '\n') { line[last_char_index] = '\0'; }
        }

		/* Parse whitespace-terminated strings in line: */
		while(sscanf(point_in_line, "%" PATHLEN_STR "s", path) == 1) {
			/* Have to store this length --- inviewpath() will
			 * modify path, later! */
			length_of_name = strlen(path);

			if(*path == '-') { /* if an option */
				if(unfinished_option) {
					/* Can't have another option directly after an
					 * -I or -p option with no name after it! */
					fprintf(stderr,
						PROGRAM_NAME
						": Syntax error in namelist file %s: unfinished -I or -p option\n",
						namefile);
					unfinished_option = 0;
				}

				int i = path[1];
				switch(i) {
					case 'c': /* ASCII characters only in crossref */
						compress = false;
						break;
					case 'k': /* ignore DEFAULT_INCLUDE_DIRECTORY */
						kernelmode = true;
						break;
					case 'q': /* quick search */
						invertedindex = true;
						break;
					case 'T': /* truncate symbols to 8 characters */
						trun_syms = true;
						break;
					case 'I': /* #include file directory */
					case 'p': /* file path components to display */
						/* coverity[overwrite_var] */
						s = path + 2;	 /* for "-Ipath" */
						if(*s == '\0') { /* if "-I path" */
							unfinished_option = i;
							break;
						}

						/* this code block used several times in here
						 * --> make it a macro to avoid unnecessary
						 * duplication */
                        /* XXX: i dont think this does anything here?
                         *       if only we had tests
                         */
#define HANDLE_OPTION_ARGUMENT(i, s)                                                     \
	switch(i) {                                                                          \
		case 'I': /* #include file directory */                                          \
			if(firstbuild == true) {                                                     \
				/* expand $ and ~ */                                                     \
				shellpath(dir, sizeof(dir), (s));                                        \
				includedir(dir);                                                         \
			}                                                                            \
			unfinished_option = 0;                                                       \
			done			  = true;                                                    \
			break;                                                                       \
		case 'p': /* file path components to display */                                  \
			if(*(s) < '0' || *(s) > '9') {                                               \
				fprintf(stderr,                                                          \
					"csope: -p option in file %s: missing or invalid numeric value\n",   \
					namefile);                                                           \
			}                                                                            \
			dispcomponents	  = atoi(s);                                                 \
			unfinished_option = 0;                                                       \
			done			  = true;                                                    \
			break;                                                                       \
		default:                                                                         \
			done = false;                                                                \
	} /* switch(i) */

						/* ... and now call it for the first time */
						HANDLE_OPTION_ARGUMENT(i, s)
						break;
					default:
						fprintf(stderr,
							PROGRAM_NAME
							": only -I, -c, -k, -p, and -T options can be in file %s\n",
							namefile);
				} /* switch(i) */
			}	  /* if('-') */
			else if(*path == '"') {
				/* handle quoted filenames... */
				size_t in = 1, out = 0;
				char  *newpath = malloc(PATHLEN + 1);

				while(in < PATHLEN && point_in_line[in] != '\0') {
					if(point_in_line[in] == '"') {
						newpath[out] = '\0';
						/* Tell outer loop to skip over this entire
						 * quoted string */
						length_of_name = in + 1;
						break; /* found end of quoted string */
					} else if(point_in_line[in] == '\\' && in < PATHLEN - 1 &&
							  (point_in_line[in + 1] == '"' ||
								  point_in_line[in + 1] == '\\')) {
						/* un-escape \" or \\ sequence */
						newpath[out++] = point_in_line[in + 1];
						in += 2;
					} else {
						newpath[out++] = point_in_line[in++];
					}
				}					/* while(in) */
				if(in >= PATHLEN) { /* safeguard against almost-overflow */
					newpath[out] = '\0';
				}

				/* If an -I or -p arguments was missing before,
				 * treat this name as the argument: */
				HANDLE_OPTION_ARGUMENT(unfinished_option, newpath);
				if(!done) {
					/* coverity[overwrite_var] */
					if((s = inviewpath(newpath)) != NULL) {
						addsrcfile(s);
					} else {
						fprintf(stderr, PROGRAM_NAME ": cannot find file %s\n", newpath);
						errorsfound = true;
					}
				}
				free(newpath);
			} /* if(quoted name) */
			else {
				/* ... so this is an ordinary file name, unquoted */

				/* If an -I or -p arguments was missing before,
				 * treat this name as the argument: */
				HANDLE_OPTION_ARGUMENT(unfinished_option, path);
				if(!done) {
					if((s = inviewpath(path)) != NULL) {
						addsrcfile(s);
					} else {
						fprintf(stderr, PROGRAM_NAME ": cannot find file %s\n", path);
						errorsfound = true;
					}
				}
			} /* else(ordinary name) */

			point_in_line += length_of_name;
			while(isspace((unsigned char)*point_in_line)) { point_in_line++; }
		} /* while(sscanf(line)) */
	}	  /* while(fgets(line)) */

}

/* make the view source directory list */
static
void make_vp_source_directories(void) {
	/* return if this function has already been called */
	if(nsrcdirs > 0) { return; }
	/* get the current directory name */
	if(getcwd(currentdir, PATHLEN) == NULL) {
		fprintf(stderr, PROGRAM_NAME ": warning: cannot get current directory name\n");
		strcpy(currentdir, "<unknown>");
	}
	/* see if there is a view path and this directory is in it */
	vpinit(currentdir);
	if(vpndirs > 1) {
		nsrcdirs = vpndirs;
	} else {
		nsrcdirs = 1;
	}
	/* create the source directory list */
	msrcdirs = nsrcdirs + DIRINC;
	srcdirs	 = malloc(msrcdirs * sizeof(*srcdirs));
	*srcdirs = "."; /* first source dir is always current dir */
	for(int i = 1; i < vpndirs; ++i) {
		srcdirs[i] = vpdirs[i];
	}
	/* save the number of original source directories in the view path */
	nvpsrcdirs = nsrcdirs;
}

/* add a source directory to the list */
static
void add_source_directory(char *dir) {
	struct stat statstruct;

	char *compressed_path = compress_path(dir);
	/* make sure it is a directory */
	if(lstat(compressed_path, &statstruct) == 0 && S_ISDIR(statstruct.st_mode)) {

		/* note: there already is a source directory list */
		if(nsrcdirs == msrcdirs) {
			msrcdirs += DIRINC;
			srcdirs = realloc(srcdirs, msrcdirs * sizeof(*srcdirs));
		}
		srcdirs[nsrcdirs++] = compressed_path;
	}
}

/* add a #include directory to the list */
static
void add_include_directory(char *name, char *path) {
	struct stat statstruct;

	char *compressed_path = compress_path(path);
	/* make sure it is a directory */
	if(lstat(compressed_path, &statstruct) == 0 && S_ISDIR(statstruct.st_mode)) {
		if(incdirs == NULL) {
			incdirs	 = malloc(mincdirs * sizeof(*incdirs));
			incnames = malloc(mincdirs * sizeof(*incnames));
		} else if(nincdirs == mincdirs) {
			mincdirs += DIRINC;
			incdirs	 = realloc(incdirs, mincdirs * sizeof(*incdirs));
			incnames = realloc(incnames, mincdirs * sizeof(*incnames));
		}
		incdirs[nincdirs]	 = compressed_path;
		incnames[nincdirs++] = strdup(name);
	}
}

/* scan a directory (recursively?) for source files */
static
void scan_dir(const char *adir, bool recurse_dir) {
	DIR *dirfile;
	int	 adir_len = strlen(adir);

	/* FIXME: no guards against adir_len > PATHLEN, yet */

	if((dirfile = opendir(adir)) != NULL) {
		struct dirent *entry;
		char		   path[PATHLEN + 1];

		while((entry = readdir(dirfile)) != NULL) {
			if((strcmp(".", entry->d_name) != 0) && (strcmp("..", entry->d_name) != 0)) {
				struct stat buf;

				snprintf(path,
					sizeof(path),
					"%s/%.*s",
					adir,
					PATHLEN - 2 - adir_len,
					entry->d_name);

				if(lstat(path, &buf)) { continue; }

				if(recurse_dir && S_ISDIR(buf.st_mode)) {
					scan_dir(path, recurse_dir);
				} else
                if(is_source_file(path)
                && !infilelist(path)
                && access(path, R_OK) == 0) {
					addsrcfile(path);
				}
			}
		}
		closedir(dirfile);
	}
	return;
}

/* check if a file is readable enough to be allowed in the
 * database */
static
bool is_accessible_file(const char *file) {

	if(access(file, READ) == 0) {
		struct stat stats;
		if(lstat(file, &stats) == 0 && S_ISREG(stats.st_mode)) {
			return true;
		}
	}
	return false;
}

/* see if this is a source file */
/* NOTE: these macros are somewhat faster than calling strcmp(),
 *        while not significantly uglier
 */
#define IS_SUFFIX_OF_2(s, suffix) (s[0] == suffix[0] && s[1] == suffix[1])
#define IS_SUFFIX_OF_3(s, suffix) (s[0] == suffix[0] && s[1] == suffix[1] && s[2] == suffix[2])
static
bool is_source_file(char *path) {
	struct stat statstruct;
	const char *file              = basename(path);
	char	   *suffix            = strrchr(file, '.');
	bool		looks_like_source = false;

	/* ensure there is some file suffix */
	if(suffix == NULL || *(++suffix) == '\0') { return false; }

	/* if an SCCS or versioned file */
	if(file[1] == '.' && file + 2 != suffix) { /* 1 character prefix */
		switch(*file) {
			case 's':
			case 'S':
				return false;
		}
	}

	if(suffix[1] == '\0') { /* 1 character suffix */
		switch(*suffix) {
			case 'c':
			case 'h':
			case 'l':
			case 'y':
			case 'C':
			case 'G':
			case 'H':
			case 'L':
				looks_like_source = true;
		}
	} else if((suffix[2] == '\0') /* 2 char suffix */
			  && (
                    /* breakpoint listing */
                    IS_SUFFIX_OF_2(suffix, "bp")
                    /* Ingres */
					|| IS_SUFFIX_OF_2(suffix, "qc")
					|| IS_SUFFIX_OF_2(suffix, "qh")
                    /* SDL */
					|| IS_SUFFIX_OF_2(suffix, "sd")
                    /* C++ source */
					|| IS_SUFFIX_OF_2(suffix, "cc")
                    /* C++ header */
					|| IS_SUFFIX_OF_2(suffix, "hh"))) {
		looks_like_source = true;
	} else if(suffix[3] == '\0' && ( /* 3 char suffix */
		      /* C++ template source */
		      IS_SUFFIX_OF_3(suffix, "tcc")
		      /* C++ source: */
		      || IS_SUFFIX_OF_3(suffix, "cpp")
		      || IS_SUFFIX_OF_3(suffix, "cxx")
		      || IS_SUFFIX_OF_3(suffix, "hpp")
		      || IS_SUFFIX_OF_3(suffix, "hxx")
              /**/
		      || IS_SUFFIX_OF_3(suffix, "inc")
    )) {
		looks_like_source = true;
	}

	if (!looks_like_source) { return false; }

	/* make sure it is a file */
	if(lstat(path, &statstruct) == 0 && S_ISREG(statstruct.st_mode)) { return true; }
	return false;
}

/* add a source directory to the list for each view path source directory */
void sourcedir(const char * dirlist) {
	char path[PATHLEN + 1];
	make_vp_source_directories();		   /* make the view source directory list */
	char * mdirlist = strdup(dirlist); /* don't change environment variable text */

	/* parse the directory list */
	char * dir = strtok(mdirlist, DIRSEPS);
	while(dir != NULL) {
		int dir_len = strlen(dir);

		add_source_directory(dir);

		/* if it isn't a full path name and there is a
		   multi-directory view path */
		if(*mdirlist != '/' && vpndirs > 1) {

			/* compute its path from higher view path source dirs */
			for(unsigned i = 1; i < nvpsrcdirs; ++i) {
				snprintf(path,
					sizeof(path),
					"%.*s/%s",
					PATHLEN - 2 - dir_len,
					srcdirs[i],
					dir);
				add_source_directory(path);
			}
		}
		dir = strtok(NULL, DIRSEPS);
	}
	free(mdirlist);
}

// XXX: useless strdup
/* add a #include directory to the list for each view path source directory */
void includedir(const char * dirlist) {
	char		 path[PATHLEN + 1];
	char		*dir;

	make_vp_source_directories();		   /* make the view source directory list */
	char * mdirlist = strdup(dirlist); /* don't change environment variable text */

	/* parse the directory list */
	dir = strtok(mdirlist, DIRSEPS);
	while(dir != NULL) {
		size_t dir_len = strlen(dir);

		add_include_directory(dir, dir);

		/* if it isn't a full path name and there is a
		   multi-directory view path */
		if(*mdirlist != '/' && vpndirs > 1) {

			/* compute its path from higher view path source dirs */
			for(unsigned i = 1; i < nvpsrcdirs; ++i) {
				snprintf(path,
					sizeof(path),
					"%.*s/%s",
					(int)(PATHLEN - 2 - dir_len),
					srcdirs[i],
					dir);
				add_include_directory(dir, path);
			}
		}
		dir = strtok(NULL, DIRSEPS);
	}
	free(mdirlist); /* HBB 20000421: avoid leaks */
}

/* make the source file list */
void makefilelist(const char * const * const argv) {

	make_vp_source_directories(); /* make the view source directory list */

	/* if -i was NOT given and there are source file arguments */
	if(namefile == NULL && *argv) {
		/* put them in a list that can be expanded */
		for(unsigned i = 0; argv[i]; i++) {
			const char * file = argv[i];
			if (!infilelist(file)) {
                char * s = inviewpath(file);
				if (s) {
					addsrcfile(s);
				} else {
					fprintf(stderr, PROGRAM_NAME ": cannot find file %s\n", file);
					errorsfound = true;
				}
			}
		}
		return;
	}

	/* see if a file name file exists */
	if(namefile == NULL && vpaccess(NAMEFILE, READ) == 0) { namefile = NAMEFILE; }

	if(namefile == NULL) {
		/* No namefile --> make a list of all the source files
		 * in the directories */
		for(unsigned i = 0; i < nsrcdirs; i++) {
			scan_dir(srcdirs[i], recurse_dir);
		}
		return;
	}

	/* Came here --> there is a file of source file names */
	FILE *names;				/* name file pointer */

	if(strcmp(namefile, "-") == 0)
		names = stdin;
	else if((names = vpfopen(namefile, "r")) == NULL) {
		cannotopen(namefile);
		myexit(1);
	}

    read_listfile(names);

	if(names == stdin) {
		clearerr(stdin);
    } else {
		fclose(names);
    }

	firstbuild = false;
	return;
}

/* add an include file to the source file list */
void incfile(char *file, char *type) {
	assert(file != NULL); /* should never happen, but let's make sure anyway */

	/* see if the file is already in the source file list */
	if(infilelist(file) == true) { return; }

	/* look in current directory if it was #include "file" */
    {
        char * s;
        if(type[0] == '"' && (s = inviewpath(file)) != NULL) {
            addsrcfile(s);
            return;
        }
    }

	size_t file_len = strlen(file);

	/* search for the file in the #include directory list */
	for(unsigned i = 0; i < nincdirs; i++) {
        char name[PATHLEN + 1];
        char path[PATHLEN + 1];
		/* don't include the file from two directories */
		snprintf(name,
			sizeof(name),
			"%.*s/%s",
			(int)(PATHLEN - 2 - file_len),
			incnames[i],
			file);
		if(infilelist(name) == true) { break; }
		/* make sure it exists and is readable */
		snprintf(path,
			sizeof(path),
			"%.*s/%s",
			(int)(PATHLEN - 2 - file_len),
			incdirs[i],
			file);

		if(access(path, READ) == 0) {
			addsrcfile(path);
			break;
		}
	}
}

/* see if the file is already in the list */
bool infilelist(const char * path) {
	struct listitem *p;

	char *dir_path = compress_path(path);

	for(p = srcnames[hash(dir_path) % HASHMOD]; p != NULL; p = p->next) {
		if(strequal(path, p->text)) {
			free(dir_path);
			return (true);

		}
	}

	free(dir_path);
	return (false);
}

/* search for the file in the view path */
char *inviewpath(const char * file) {
	static char	path[PATHLEN + 1];

	/* look for the file */
	if(is_accessible_file(file)) {
        strcpy(path, file);
        return path;
    }

	/* if it isn't a full path name and there is a multi-directory
	 * view path */
	if(*file != '/'
    && vpndirs > 1) {
		int file_len = strlen(file);

		/* compute its path from higher view path source dirs */
		for(unsigned i = 1; i < nvpsrcdirs; ++i) {
			snprintf(path,
				sizeof(path),
				"%.*s/%s",
				PATHLEN - 2 - file_len,
				srcdirs[i],
				file
            );
			if(is_accessible_file(path)) { return path; }
		}
	}

	return NULL;
}

/* add a source file to the list */
void addsrcfile(char *path) {
	struct listitem *p;
	int				 i;

	/* make sure there is room for the file */
	if(nsrcfiles == msrcfiles) {
		msrcfiles += SRCINC;
		srcfiles = realloc(srcfiles, msrcfiles * sizeof(*srcfiles));
	}
	/* add the file to the list */
	char *dir_path = compress_path(path);
	srcfiles[nsrcfiles++] = strdup(dir_path);
	p					  = malloc(sizeof(*p));
	p->text				  = strdup(dir_path);
	free(dir_path);
	i					  = hash(p->text) % HASHMOD;
	p->next				  = srcnames[i];
	srcnames[i]			  = p;
}

/* free the memory allocated for the source file list */
void freefilelist(void) {
	struct listitem *p, *nextp;
	int				 i;

	/* if '-d' option is used a string space block is allocated */
	if(preserve_database == false) {
		while(nsrcfiles > 0) {
			free(srcfiles[--nsrcfiles]);
		}
	} else {
		/* for '-d' option free the string space block */
		/* protect against empty list */
		if(nsrcfiles > 0) free(srcfiles[0]);
		nsrcfiles = 0;
	}

	free(srcfiles); /* HBB 20000421: avoid leak */
	msrcfiles = 0;
	srcfiles  = 0;

	for(i = 0; i < HASHMOD; ++i) {
		for(p = srcnames[i]; p != NULL; p = nextp) {
			/* HBB 20000421: avoid memory leak */
			free(p->text);
			nextp = p->next;
			free(p);
		}
		srcnames[i] = NULL;
	}
}

void freeinclist() {
	if(!incdirs) return;
	while(nincdirs > 0) {
		free(incdirs[--nincdirs]);
		free(incnames[nincdirs]);
	}
	free(incdirs);
	free(incnames);
}

void freesrclist() {
	if(!srcdirs) { return; }
	while(nsrcdirs > 1) {
        free(srcdirs[--nsrcdirs]);
    }
	free(srcdirs);
}
