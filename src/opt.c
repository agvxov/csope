#include "global.h"

#include "build.h" 
#include "vp.h"
#include "version.h"    /* FILEVERSION and FIXVERSION */
#include "scanner.h" 

#include <stdlib.h>    /* atoi */
#include <getopt.h>

BOOL remove_symfile_onexit = NO;
BOOL onesearch;        /* one search only in line mode */
char *reflines;        /* symbol reference lines file */

char ** parse_options(int *argc, char **argv)
{
    int opt;
    int longind;
    char path[PATHLEN + 1];     /* file path */
    char *s;
    int argcc = *argc;

    struct option lopts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argcc, argv,
           "hVbcCdeF:f:I:i:kLl0:1:2:3:4:5:6:7:8:9:P:p:qRs:TUuvX",
           lopts, &longind)) != -1) {
        switch(opt) {

        case '?':
        	usage();
        	myexit(1);
        	break;
        case 'X':
        	remove_symfile_onexit = YES;
        	break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        	/* The input fields numbers for line mode operation */
        	field = opt - '0';
        	if (strlen(optarg) > PATHLEN) {
        		    postfatal("\
        			cscope: pattern too long, cannot be > \
        			%d characters\n", PATLEN);
        	}
        	strcpy(input_line, optarg);	
        	break;
        case 'b':	/* only build the cross-reference */
        	buildonly = YES;
        	linemode  = YES;
        	break;
        case 'c':	/* ASCII characters only in crossref */
        	compress = NO;
        	break;
        case 'C':	/* turn on caseless mode for symbol searches */
        	caseless = YES;
        	egrepcaseless(caseless); /* simulate egrep -i flag */
        	break;
        case 'd':	/* consider crossref up-to-date */
        	isuptodate = YES;
        	break;
        case 'e':	/* suppress ^E prompt between files */
        	editallprompt = NO;
        	break;
        case 'h':
        	longusage();
        	myexit(1);
        	break;
        case 'k':	/* ignore DFLT_INCDIR */
        	kernelmode = YES;
        	break;
        case 'L':
        	onesearch = YES;
        	/* FALLTHROUGH */
        case 'l':
        	linemode = YES;
        	break;
        case 'v':
        	verbosemode = YES;
        	break;
        case 'V':
        	fprintf(stderr, "%s: version %d%s\n", argv0,
        		FILEVERSION, FIXVERSION);
        	myexit(0);
        	break;
        case 'q':	/* quick search */
        	invertedindex = YES;
        	break;
        case 'T':	/* truncate symbols to 8 characters */
        	trun_syms = YES;
        	break;
        case 'u':	/* unconditionally build the cross-reference */
        	unconditional = YES;
        	break;
        case 'U':	/* assume some files have changed */
        	fileschanged = YES;
        	break;
        case 'R':
        	recurse_dir = YES;
        	break;
        case 'f':	/* alternate cross-reference file */
        	reffile = optarg;
        	if (strlen(reffile) > sizeof(path) - 3) {
        		postfatal("\
        			cscope: reffile too long, cannot \
        			be > %d characters\n", sizeof(path) - 3);
        		/* NOTREACHED */
        	}
        	strcpy(path, reffile);

        	s = path + strlen(path);
        	strcpy(s, ".in");
        	/*coverity[overwrite_var]*/
        	invname = strdup(path);
        	strcpy(s, ".po");
        	/*coverity[overwrite_var]*/
        	invpost = strdup(path);
        	break;

        case 'F':	/* symbol reference lines file */
        	reflines = optarg;
        	break;
        case 'i':	/* file containing file names */
        	namefile = optarg;
        	break;
        case 'I':	/* #include file directory */
        	includedir(optarg);
        	break;
        case 'p':	/* file path components to display */
        	dispcomponents = atoi(optarg);
        	break;
        case 'P':	/* prepend path to file names */
        	prependpath = optarg;
        	break;
        case 's':	/* additional source file directory */
        	sourcedir(optarg);
        	break;
        }
    }
    /*
      * This adjusts argv so that we only see the remaining 
      * args.  Its ugly, but we need to do it so that the rest
      * of the main routine doesn't get all confused
      */
    *argc = *argc - optind;
    return argv + optind;
}
