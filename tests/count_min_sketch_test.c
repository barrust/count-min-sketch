
#include <stdio.h>
#include "../src/count_min_sketch.h"

int main(int argc, char** argv) {
    CountMinSketch cms;
    cms_init(&cms, 10000, 7);

    int i, res;
    for (i = 1; i <= 10; i++) {
        res = cms_add(&cms, "this is a test");
        if (res != i) {
            printf("Error with i=%d\tres=%d\n", i, res);
        }
    }

    res = cms_check(&cms, "this is a test");
    if (res != 10) {
        printf("Error with lookup: %d\n", res);
    }
    cms_destroy(&cms);
}
