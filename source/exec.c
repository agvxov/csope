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
 *    process execution functions
 */

#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h> /* pid_t */
#ifdef __DJGPP__
# include <process.h>
#endif

#include "global.h"

#include "constants.h"
#include "path.h"
#include "display.h"

typedef void (*sighandler_t)(int);
static sighandler_t oldsigquit; /* old value of quit signal */
static sighandler_t oldsighup;	/* old value of hangup signal */
static sighandler_t oldsigtstp; /* old value of SIGTSTP */

#ifndef __MSDOS__				/* none of these is needed, there */
static int	 join(pid_t p);
static int	 myexecvp(char * program_name, char **args);
static pid_t myfork(void);
#endif

// XXX
#ifndef BUFSIZ
# define BUFSIZ 8192
#endif

/* execute forks and executes a program or shell script, waits for it to
 * finish, and returns its exit code.
 */
int execute(char * program_name, ...) /* NOTE: "exec" is already defined on u370 */
{
	va_list ap;
	int		exitcode = -1;
	char   *argv[BUFSIZ];
	pid_t	p;

    // XXX: global display access
	/* fork and exec the program or shell script */
	endwin(); /* restore the terminal modes */
	//mousecleanup();
	fflush(stdout);
	va_start(ap, program_name);

	for(p = 0; (argv[p] = va_arg(ap, char *)) != 0; p++) { ; }

	if((p = myfork()) == 0) {	/* child */
		myexecvp(program_name, argv);
	}

	exitcode = join(p);

	entercurses();

	va_end(ap);
	return (exitcode);
}

#ifndef __MSDOS__ /* None of the following functions is used there */

/* myexecvp is an interface to the execvp system call to
 * modify argv[0] to reference the last component of its path-name.
 */
static int myexecvp(char *program_name, char **args) {
	char msg[MSGLEN + 1];

	/* modify argv[0] to reference the last component of its path name */
	args[0] = (char *)basename(args[0]);

	/* execute the program or shell script */
	execvp(program_name, args); /* returns only on failure */
	snprintf(msg, sizeof(msg), "\nCannot exec %s", program_name);
	perror(msg);	 /* display the reason */
	askforreturn();	 /* wait until the user sees the message */
	myexit(1);		 /* exit the child */

	return 0; // XXX
}

/* myfork acts like fork but also handles signals */

static pid_t myfork(void) {
	pid_t p; /* process number */

	p = fork();

	/* the parent ignores the interrupt, quit, and hangup signals */
	if(p > 0) {
		oldsigquit = signal(SIGQUIT, SIG_IGN);
		oldsighup  = signal(SIGHUP, SIG_IGN);
# ifdef SIGTSTP
		oldsigtstp = signal(SIGTSTP, SIG_DFL);
# endif
	}
	/* so they can be used to stop the child */
	else if(p == 0) {
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGHUP, SIG_DFL);
# ifdef SIGTSTP
		signal(SIGTSTP, SIG_DFL);
# endif
	}
	/* check for fork failure */
	if(p == -1) { postperror("Cannot fork"); }
	return p;
}

/* join is the compliment of fork */
static int join(pid_t p) {
	int	  status = -1;
	pid_t w;

	/* wait for the correct child to exit */
	do {
		w = wait(&status);
	} while(p != -1 && w != p);

	/* restore signal handling */
	signal(SIGQUIT, oldsigquit);
	signal(SIGHUP, oldsighup);
# ifdef SIGTSTP
	signal(SIGTSTP, oldsigtstp);
# endif

	/* return the child's exit code */
	return (status >> 8);
}

#endif /* !MSDOS */
