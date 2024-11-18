#ifndef EGREP_H
#define EGREP_H

extern int egrep(const char * file, FILE *output, char *format);
extern void egrepcaseless(int i);
extern int egrepinit(const char *egreppat);

#endif
