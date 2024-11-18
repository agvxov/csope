#include "global.h"

#include <unistd.h>
#include <sys/wait.h>

int gen_tags_file(void) {
    const int NON_FILE_ARGUMENTS = 3;
	const char * execve_args[nsrcfiles + NON_FILE_ARGUMENTS + 1];
	execve_args[0] = "/bin/env";
	execve_args[1] = "ctags";
	execve_args[2] = "--fields=kn";
	execve_args[nsrcfiles + NON_FILE_ARGUMENTS] = NULL;
	for (int i = 0; i < nsrcfiles; i++) {
		execve_args[i + NON_FILE_ARGUMENTS] = srcfiles[i];
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

static inline
char * read_field(FILE * f, char * buffer) {
    int c;
    int buffer_empty_top = 0;
    while ((c = getc(f)) != '\t') {
        buffer[buffer_empty_top++] = c;
    }
    buffer[buffer_empty_top] = '\0';
    return buffer;
}

/* NOTE:
 *  The text field may contain '\t's and is delimited by '/',
 *   which we dont want to include.
 */
static inline
char * read_text_field(FILE * f, char * buffer) {
    int c;
    bool was_escaped = false;
    int buffer_empty_top = 0;
    UNUSED(getc(f)); // Skip leading '/'
    while ((c = getc(f))) {
        if (c == '/'
        && !was_escaped) {
            break;
        }

        buffer[buffer_empty_top++] = c;
        was_escaped = (c == '\\');
    }

    /* XXX:
     *  catgs seems to terminate text fields with '/;"'
     *  i have no clue why
     */
    fseek(f, strlen("/;\"\t"), SEEK_CUR);
    buffer[buffer_empty_top] = '\0';
    return buffer;
}

/* NOTE:
 *  We assume the line field is the last on the line.
 */
static inline
int read_line_field(FILE * f) {
    int r;
    fseek(f, strlen("line:"), SEEK_CUR);
    fscanf(f, "%d", &r);
    UNUSED(getc(f)); // eat '\n'
    return r;
}

int read_tags_file(const char * filepath) {
    /* NOTE:
     *  We assume entry lines look something like this:
     *      siginit	source/main.c	/^void siginit(void) {$/;"	f	line:89
     *  (name|file|text|type|line)
     *  Notice that fiels are delimited by tabs.
     */
    // XXX temp
    symbol_t ctags_symbol;

    FILE * f = fopen(filepath, "r");
    if (!f) {
        cannotopen(filepath);
        return 1;
    }

    char buffer[PATH_MAX];
    for (int c = getc(f); c != EOF; c = getc(f)) {
        // Ignore comment lines
        if (c == '!') {
            while ((c = getc(f)) != '\n') { ; }
            continue;
        } else {
            ungetc(c, f);
        }

        ctags_symbol.name     = strdup(read_field(f, buffer));
        ctags_symbol.filename = strdup(read_field(f, buffer));
        ctags_symbol.text     = strdup(read_text_field(f, buffer));
        ctags_symbol.type     = read_field(f, buffer)[0];
        asprintf(&ctags_symbol.linenum, "%d", read_line_field(f));

        printf("{ .name = %s, .filename = %s, .text = %s, .type = %c, .line = %s }\n",
            ctags_symbol.name,
            ctags_symbol.filename,
            ctags_symbol.text,
            ctags_symbol.type,
            ctags_symbol.linenum
        );

        // XXX: leak
    }

    fclose(f);
    return 0;
}
