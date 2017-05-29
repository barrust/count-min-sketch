
#include <stdio.h>
#include "../src/count_min_sketch.h"

#define KEY_LEN 5

int main(int argc, char** argv) {
    CountMinSketch cms;
    cms_init(&cms, 10000, 7);
    printf("width: %d\n", cms.width);
    printf("depth: %d\n", cms.depth);
    printf("confidence: %f\n", cms.confidence);
    printf("error rate: %f\n", cms.error_rate);
    int i, j, res;
    for (i = 0; i < 10; i++) {
        for (j = 1; j <= 10; j++) {
            char key[KEY_LEN] = {0};
            sprintf(key, "%d", i);
            res = cms_add(&cms, key);
            if (res != j) {
                printf("Error with key=%s\ti=%d\tres=%d\n", key, i, res);
            }
        }
    }

    res = cms_check(&cms, "0");
    if (res != 10) {
        printf("Error with lookup: %d\n", res);
    }

    for (i = 0; i < 10; i++) {
        for (j = 10; j > 0; j--) {
            char key[KEY_LEN] = {0};
            sprintf(key, "%d", i);
            res = cms_remove(&cms, key);
            if (res != j - 1) {
                printf("Error with key=%s\ti=%d\tres=%d\n", key, j, res);
            }
        }
    }

    res = cms_check(&cms, "0");
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

    return 0;
}
