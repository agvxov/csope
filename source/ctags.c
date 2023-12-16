#include "global.h"

#include <unistd.h>

int gen_tags_file(void) {
	const char * execve_args[nsrcfiles + 1];
	execve_args[0] = "ctags";
	for (int i = 0; i < nsrcfiles; i++) {
		execve_args[i + 1] = srcfiles[i];
	}

	execvp(execve_args[0], execve_args);

	return 0;
}
