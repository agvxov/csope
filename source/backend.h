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
	/* if you plan to add more, you will have to modify logic,
	 *  because we depend on value flipping
     *  XXX
	 */
};

extern backend_t backend;
extern int backend_mode;

void change_backend(int backend);

#endif
