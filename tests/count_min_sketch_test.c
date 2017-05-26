
#include <stdio.h>
#include "../src/count_min_sketch.h"

int main(int argc, char** argv) {
    CountMinSketch cms;
    cms_init(&cms, 10000, 7);
    printf("width: %d\n", cms.width);
    printf("depth: %d\n", cms.depth);
    printf("confidence: %f\n", cms.confidence);
    printf("error rate: %f\n", cms.error_rate);
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

    for (i = 1; i <= 10; i++) {
        res = cms_remove(&cms, "this is a test");
        if (res != 10 - i) {
            printf("Error with i=%d\tres=%d\n", i, res);
        }
    }

    res = cms_check(&cms, "this is a test");
    if (res != 0) {
        printf("Error with lookup: %d\n", res);
    }

    cms_destroy(&cms);

    CountMinSketch cmso;
    cms_init_optimal(&cmso, 0.001, 0.99999);
    printf("width: %d\n", cmso.width);
    printf("depth: %d\n", cmso.depth);
    printf("confidence: %f\n", cmso.confidence);
    printf("error rate: %f\n", cmso.error_rate);

    cms_destroy(&cmso);
}
