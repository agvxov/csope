#include "global.h"

#include <unistd.h>
#include <sys/wait.h>

int gen_tags_file(void) {
	const char * execve_args[nsrcfiles + 3];
	execve_args[0] = "/bin/env";
	execve_args[1] = "ctags";
	execve_args[nsrcfiles + 3 - 1] = NULL;
	for (int i = 0; i < nsrcfiles; i++) {
		execve_args[i + 2] = srcfiles[i];
	}

    pid_t pid = fork();
    if (pid == -1) { return -1; };

    if (pid == 0) {
        execvp(execve_args[0], (char *const *)execve_args);
        perror("execvp failed");
        exit(1);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid failed");
        return -1;
    }

	return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}
