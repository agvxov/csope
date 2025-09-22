#ifndef OPT_H
#define OPT_H

char * * parse_options(const int argc, const char * const * const argv);
void readenv(bool preserve_database);

#endif
