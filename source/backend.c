#include "backend.h"

#include "ctags.h"

int backend_mode;
backend_t backend = CSCOPE_BACKEND;

void change_backend(int backend_mode) {
    switch (backend_mode) {
        case CSCOPE_BACKEND: {
            backend.name = "CScope";
        } break;
        case CTAGS_BACKEND: {
            backend.name  = "CTags";
            backend.build = gen_tags_file;
        } break;
    }
}
