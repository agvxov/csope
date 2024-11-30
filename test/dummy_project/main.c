// @BAKE gcc -o dummy.out main.c h.c
#include <stdlib.h>
#include "h.h"

int f(void) {
    int r = 0;
    for (int i = 0; i < 100; i++) {
        r += (int)h(r);
    }
    return r;
}


signed main(void) {
    return f();
}
