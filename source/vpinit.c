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

/* vpinit - initialize vpdirs or update vpdirs based on currentdir */

#include <stdio.h> /* stderr */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "vp.h"

#include "library.h"
#include "global.h"
#include "constants.h"

char **vpdirs; /* directories (including current) in view path */
int vpndirs; /* number of directories in view path */

void vpinit(char *current_dir) {
	char *suffix; /* path from view path node */
	char *vpath;  /* VPATH environment variable value */
	char  buf[MAXPATH + 1];
	int	  i;
	char *s;

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
	if((vpath[i] != ':' && vpath[i] != '\0') ||
		(current_dir[i] != '/' && current_dir[i] != '\0')) {
		return;
	}
	suffix = &current_dir[i];

	/* count the nodes in the view path */
	vpndirs = 1;
	for(i = 0; vpath[i] != '\0'; ++i) {
		if(vpath[i] == ':' && vpath[i + 1]) { ++vpndirs; }
	}
	/* create the source directory list */
	vpdirs = malloc(vpndirs * sizeof(*vpdirs));

	/* don't change VPATH in the environment */
	vpath = strdup(vpath);

	/* split the view path into nodes */
	for(i = 0, s = vpath; *s != '\0'; ++i) {
		vpdirs[i] = s;
		while(*s != '\0' && *++s != ':') {
			if(*s == '\n') { *s = '\0'; }
		}
		if(*s != '\0') { *s++ = '\0'; }
	}
	/* convert the view path nodes to directories */
	for(i = 0; i < vpndirs; ++i) {
		s = malloc(strlen(vpdirs[i]) + strlen(suffix) + 1);
		(void)strcpy(s, vpdirs[i]);
		(void)strcat(s, suffix);
		vpdirs[i] = s;
	}
	free(vpath);
}
