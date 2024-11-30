#include "global.h"

#include <unistd.h>
#include <sys/wait.h>

/* NOTE:
 *  We assume entry lines look something like this:
 *      siginit	source/main.c	/^void siginit(void) {$/;"	f	line:89
 *  (name|file|text|type|line)
 *  Notice that fiels are delimited by tabs.
 */

const char * results_file_name = "results.tags";
FILE * results_file = NULL;

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

static inline
int read_symbol(FILE * f, symbol_t * symbol) {
    char typebuf[2];
    read_field(f, symbol->name);
    read_field(f, symbol->filename);
    read_text_field(f, symbol->text);
    symbol->type = read_field(f, typebuf)[0];
    sprintf(symbol->linenum, "%d", read_line_field(f));

    if (symbol->type == '\00') {    // XXX
        symbol->type = 'u';
    }

    return 0;
}

static inline
int append_symbol(const symbol_t * symbol, FILE * f) {
    fprintf(f, "%s\t%s\t/%s/;\"\t%c\tline:%s\n",
        symbol->name,
        symbol->filename,
        symbol->text,
        symbol->type,
        symbol->linenum
    );

    return 0;
}

int tags_search(const char * query) {
    extern unsigned totallines;
    totallines = 0;

    symbol_t ctags_symbol;
    ctags_symbol.name     = alloca(100);
    ctags_symbol.filename = alloca(100);
    ctags_symbol.text     = alloca(100);
    ctags_symbol.linenum  = alloca(100);

    //FILE * f = fopen(filepath, "r");
    FILE * f = fopen("tags", "r");
    if (!f) {
        cannotopen("tags");
        return 1;
    }

    results_file = fopen(results_file_name, "w");
    if (!results_file) {
        cannotopen(results_file_name);
        return 1;
    }

    for (int c = getc(f); c != EOF; c = getc(f)) {
        // Ignore comment lines
        if (c == '!') {
            while ((c = getc(f)) != '\n') { ; }
            continue;
        } else {
            ungetc(c, f);
        }

        read_symbol(f, &ctags_symbol);

        if (!strcmp(query, ctags_symbol.name)) {
            append_symbol(&ctags_symbol, results_file);
            ++totallines;
        }
    }

    fclose(f);

    fclose(results_file);
    results_file = fopen(results_file_name, "r");

	window_change |= CH_RESULT;

    return 0;
}

symbol_t * tags_get_next_symbol(symbol_t * symbol) {
    int c = getc(results_file);
    if (c == EOF) {
        return NULL;
    } else {
        ungetc(c, results_file);
    }

    read_symbol(results_file, symbol);
    return symbol;
}

void dump_symbol(symbol_t * symbol) {
    if (symbol == NULL) {
        puts("(symbol_t)NULL");
        return;
    }

    printf("{ .name = %s, .filename = %s, .text = %s, .type = %c, .line = %s }\n",
        symbol->name,
        symbol->filename,
        symbol->text,
        symbol->type,
        symbol->linenum
    );
}

void dump_tags_file(const char * filepath) {
    symbol_t ctags_symbol;
    ctags_symbol.name     = alloca(100);
    ctags_symbol.filename = alloca(100);
    ctags_symbol.text     = alloca(100);
    ctags_symbol.linenum  = alloca(100);

    FILE * f = fopen(filepath, "r");
    if (!f) {
        cannotopen(filepath);
        return;
    }

    for (int c = getc(f); c != EOF; c = getc(f)) {
        // Ignore comment lines
        if (c == '!') {
            while ((c = getc(f)) != '\n') { ; }
            continue;
        } else {
            ungetc(c, f);
        }

        read_symbol(f, &ctags_symbol);

        dump_symbol(&ctags_symbol);
    }

    fclose(f);
}
