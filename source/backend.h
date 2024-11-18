#ifndef BACKEND_H
#define BACKEND_H

typedef struct {
    const char * name;
    int (*build)(void);
    int (*get_result)(char * * file, char * * function, char * * linenum, char * * tempstring);
} backend_t;

enum {
	CSCOPE_BACKEND = 0,
	CTAGS_BACKEND  = 1,
};

extern backend_t backend;
extern int backend_mode;

void change_backend(int backend);

#endif
