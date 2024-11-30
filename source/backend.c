#include "backend.h"

#include "build.h"
#include "ctags.h"

int backend_mode = CSCOPE_BACKEND;
backend_t backend;

void change_backend(int backend_mode) {
    switch (backend_mode) {
        case CSCOPE_BACKEND: {
            backend.name = "CScope";
            backend.build = build;
            backend.get_result = get_next_symbol;
            backend.search = search;
        } break;
        case CTAGS_BACKEND: {
            backend.name  = "CTags";
            backend.build = gen_tags_file;
            backend.get_result = tags_get_next_symbol;
            backend.search = tags_search;
        } break;
    }
}
