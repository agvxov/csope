#include "h.h"
#include <stdlib.h>

double h(int i) {
    if (i < 100000) {
        return rand() % i;
    } else {
        return i / 10;
    }
}
