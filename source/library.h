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

#ifndef CSCOPE_LIBRARY_H
#define CSCOPE_LIBRARY_H

#include <stdio.h> /* need FILE* type def. */

/* private library */
char	   *compress_path(const char *pathname_);
char	   *egrepinit(const char *egreppat);
char	   *logdir(char *name);
const char *basename(const char *path);
FILE	   *myfopen(const char * path, const char * mode);
int			myopen(const char * path, int flag, int mode);
FILE	   *mypopen(char *cmd, char *mode);
int			mypclose(FILE *ptr);
void		egrepcaseless(int i);

#endif /* CSCOPE_LIBRARY_H */
