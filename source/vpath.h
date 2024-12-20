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
