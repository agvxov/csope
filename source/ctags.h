#ifndef CTAGS_H
#define CTAGS_H

int gen_tags_file(void);
int tags_search(const char * query);
symbol_t * tags_get_next_symbol(symbol_t * symbol);

// debug
void dump_symbol(symbol_t * symbol); // XXX move
void dump_tags_file(const char * filepath);

#endif
