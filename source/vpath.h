/*
 *    VPATH assumptions:
 *        VPATH is the environment variable containing the view path
 *        where each path name is followed by ':', '\n', or '\0'.
 *        Embedded blanks are considered part of the path.
 */

#ifndef CSCOPE_VP_H
#define CSCOPE_VP_H

#define MAXPATH 200 /* max length for entire name */

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define HAVE_FCNTL_H 1 /* in case of doubt, assume it's there */
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h> /* needed for O_... open flags */
#endif

#include <sys/types.h>
#include <sys/stat.h>

extern char **vpdirs; /* directories (including current) in view path */
extern int vpndirs;	  /* number of directories in view path */

void vpinit(char *current_dir);
int	 vpopen(const char * path, const int oflag);
int	 vpaccess(const char * path, const mode_t amode);
FILE * vpfopen(const char * filename, const char * type);

#endif
