#ifndef BACKEND_H
#define BACKEND_H

#include "global.h"

typedef struct {
    const char * name;
    int (*build)(void);
    symbol_t * (*get_result)(symbol_t * symbol);
    int (*search)(const char * query);
} backend_t;

enum {
	CSCOPE_BACKEND = 0,
	CTAGS_BACKEND  = 1,
};

extern backend_t backend;
extern int backend_mode;

void change_backend(int backend);

#endif
