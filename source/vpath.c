/* vpinit - initialize vpdirs or update vpdirs based on currentdir */

#include <stdio.h> /* stderr */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "vpath.h"

#include "constants.h"

char **vpdirs; /* directories (including current) in view path */
int vpndirs; /* number of directories in view path */

static inline int vp_element_count(const char * vpath);

int vpaccess(const char * path, const mode_t amode) {
	char buf[MAXPATH + 1];
	int	r = access(path, amode);

    if (r != -1
    ||  path[0] == '/') {
        return r;
    }

	vpinit(NULL);
	for(int i = 1; i < vpndirs; i++) {
		(void)snprintf(buf, sizeof(buf), "%s/%s", vpdirs[i], path);
        r = access(buf, amode);
		if(r != -1) { break; }
	}

	return r;
}

/* XXX
 * const arg
 * scope i
 * scope s
 * do not dare to get current directory here
 * have vpath on the stack
 */
void vpinit(char *current_dir) {
	char *suffix; /* path from view path node */
	char *vpath;  /* VPATH environment variable value */
	char  buf[MAXPATH + 1];
	int	  i;

	/* if an existing directory list is to be updated, free it */
	if(current_dir != NULL && vpndirs > 0) {
		for(i = 0; i < vpndirs; ++i) {
			free(vpdirs[i]);
		}
		free(vpdirs);
		vpndirs = 0;
	}

	/* return if the directory list has been computed */
	/* or there isn't a view path environment variable */
	if(vpndirs > 0 || (vpath = getenv("VPATH")) == NULL || *vpath == '\0') { return; }
	/* if not given, get the current directory name */
	if(current_dir == NULL && (current_dir = getcwd(buf, MAXPATH)) == NULL) {
		fprintf(stderr, PROGRAM_NAME ": cannot get current directory name\n");
		return;
	}
	/* see if this directory is in the first view path node */
	for(i = 0; vpath[i] == current_dir[i] && vpath[i] != '\0'; ++i) {
		;
	}
	if((vpath[i] != ':' && vpath[i] != '\0')
    || (current_dir[i] != '/' && current_dir[i] != '\0')) {
		return;
	}
	suffix = &current_dir[i];

	/* count the nodes in the view path */
    vpndirs = vp_element_count(vpath);

	/* create the source directory list */
	vpdirs = malloc(vpndirs * sizeof(*vpdirs));

	/* don't change VPATH in the environment */
	vpath = strdup(vpath);

	char * s = vpath;
	/* split the view path into nodes */
	for(int i = 0; *s != '\0'; ++i) {
		vpdirs[i] = s;
		while(*s != '\0' && *++s != ':') {
			if(*s == '\n') { *s = '\0'; }
		}
		if(*s != '\0') { *s++ = '\0'; }
	}
	/* convert the view path nodes to directories */
	for(int i = 0; i < vpndirs; ++i) {
		s = malloc(strlen(vpdirs[i]) + strlen(suffix) + 1);
		(void)strcpy(s, vpdirs[i]);
		(void)strcat(s, suffix);
		vpdirs[i] = s;
	}
	free(vpath);
}

#define OPENFLAG_READ 0
int vpopen(const char * path, const int oflag) {
	char buf[MAXPATH + 1];
	int	r = open(path, oflag, 0666);

    if (r != -1
    ||  path[0] == '/'
    ||  oflag != OPENFLAG_READ) {
        return r;
    }

	vpinit(NULL);
	for(int i = 1; i < vpndirs; i++) {
		(void)snprintf(buf, sizeof(buf), "%s/%s", vpdirs[i], path);
        r = open(buf, oflag, 0666);
		if(r != -1) { break; }
	}

	return (r);
}

FILE * vpfopen(const char *filename, const char * type) {
	char  buf[MAXPATH + 1];
	FILE * r = fopen(filename, type);

    if (r != NULL
    ||  filename[0] == '/'
    ||  type[0] != 'r') { /* && strcmp(type, "r") == 0 */ /* HBB: this breaks if type=="rb" */
        return r;
    }

	vpinit(NULL);
	for(int i = 1; i < vpndirs; i++) {
		(void)snprintf(buf, sizeof(buf), "%s/%s", vpdirs[i], filename);
        r = fopen(buf, type);
		if(r != NULL) { break; }
	}

	return r;
}

/* NOTE:
 *  the original code tested for a trailing ':',
 *  however, thats equivalent to having and empty path
 *  which could arise from "::"
 */
static inline
int vp_element_count(const char * vpath) {
    int r = strspn(vpath, ":");
    ++r;
    return r;
}
