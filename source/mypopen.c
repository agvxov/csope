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
 IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
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

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "global.h"
#include "path.h"

#define tst(a, b) (*mode == 'r' ? (b) : (a))
#define RDR		  0
#define WTR		  1

#ifdef FD_CLOEXEC
# define CLOSE_ON_EXEC FD_CLOEXEC
#else
# define CLOSE_ON_EXEC 1
#endif

#ifdef HAVE_IO_H
# include <io.h> /* for setmode() */
#endif

static pid_t popen_pid[20];
static void (*tstat)(int);

int _myopen(const char * path, int flag, int mode) {
	/* opens a file descriptor and then sets close-on-exec for the file */
	int fd;

	/* If file is not explicitly in Binary mode, make
	 *  sure we override silly Cygwin behaviour of automatic binary
	 *  mode for files in "binary mounted" paths */
#if O_BINARY != O_TEXT
	if(!(flag | O_BINARY)) flag |= O_TEXT;
#endif
	if (mode) {
		fd = QUOTE(open)(path, flag, mode);
    } else {
		fd = QUOTE(open)(path, flag);
    }

	if (fd != -1
    && (fcntl(fd, F_SETFD, CLOSE_ON_EXEC) != -1)) {
		return fd;
    } else {
		/* Ensure that if the fcntl fails and fd is valid, then
		   the file is closed properly. In general this should
		   not happen. */
		if(fd != -1) { close(fd); }

		return (-1);
	}
}

FILE * _myfopen(const char * path, const char * mode) {
	/* opens a file pointer and then sets close-on-exec for the file */
	FILE *fp;

	fp = QUOTE(fopen)(path, mode);

	if(fp && (fcntl(fileno(fp), F_SETFD, CLOSE_ON_EXEC) != -1)) {
		return (fp);
	}

	if(fp) {
		fclose(fp);
	}
	return (NULL);
}

/* XXX: these functions should go as soon as build.c is sorted out
 */
FILE * _mypopen(char *cmd, char *mode) {
	int	   p[2];
	pid_t *poptr;
	int	   myside, yourside;
	pid_t  pid;

	if(pipe(p) < 0) return (NULL);
	myside	 = tst(p[WTR], p[RDR]);
	yourside = tst(p[RDR], p[WTR]);
	if((pid = fork()) == 0) {
		/* myside and yourside reverse roles in child */
		int stdio;

		/* close all pipes from other popen's */
		for(poptr = popen_pid; poptr < popen_pid + 20; poptr++) {
			if(*poptr) (void)close(poptr - popen_pid);
		}
		stdio = tst(0, 1);
		close(myside);
		close(stdio);
		fcntl(yourside, F_DUPFD, stdio);
		close(yourside);
		execlp(shell, basename(shell), "-c", cmd, (void *)0);
		_exit(1);
	} else if(pid > 0) {
		tstat = signal(SIGTSTP, SIG_DFL);
	}
	if(pid == -1) {
		return (NULL);
	}
	popen_pid[myside] = pid;
	(void)close(yourside);
	return (fdopen(myside, mode));
}

int _mypclose(FILE * ptr) {
	int			 f;
	pid_t		 r;
	int			 status = -1;
	sighandler_t hstat, istat, qstat;

	f = fileno(ptr);
	UNUSED(fclose(ptr));
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	hstat = signal(SIGHUP, SIG_IGN);
	while((r = wait(&status)) != popen_pid[f] && r != -1) { ; }
	if(r == -1) status = -1;
	UNUSED(signal(SIGINT, istat));
	UNUSED(signal(SIGQUIT, qstat));
	UNUSED(signal(SIGHUP, hstat));
	UNUSED(signal(SIGTSTP, tstat));
	/* mark this pipe closed */
	popen_pid[f] = 0;
	return (status);
}
