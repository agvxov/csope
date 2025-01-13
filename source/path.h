#ifndef PATH_H
#define PATH_H

#include "constants.h"

void shellpath(char *out, int limit, char *in);
const char * basename(const char * path);
const char * prepend_path(const char * prepand_with, const char * file);
char * pathcomponents(char *path, int components);
char * compress_path(const char *pathname_);
char * logdir(char *name);

#endif
