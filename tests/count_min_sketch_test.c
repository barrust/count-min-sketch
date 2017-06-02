
#include <stdio.h>
#include <math.h>
#include "timing.h"
#include "../src/count_min_sketch.h"

#define KEY_LEN 5

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KCYN  "\x1B[36m"

/* private functions */
void success_or_failure(int res);



int main(int argc, char** argv) {
    printf("Testing Count-Min Sketch version %s\n\n", COUNT_MIN_SKETCH_VERSION);
    Timing tm;
    timing_start(&tm);
    int result;

    CountMinSketch cms;
    printf("Count-Min Sketch: Creation using defined width and depth: ");
    cms_init(&cms, 10000, 7);
    if (cms.width == 10000 && cms.depth == 7) {
        success_or_failure(0);
    } else {
        success_or_failure(1);
    }

    printf("Count-Min Sketch: auto set confidence and error rate: ");
    if (cms.confidence >= 0.992 && cms.error_rate >= 0.000200) {
        success_or_failure(0);
    } else {
        success_or_failure(1);
    }

    printf("Count-Min Sketch: insertion: ");
    int i, j, res;
    result = 0;
    for (i = 0; i < 10; i++) {
        for (j = 1; j <= 10; j++) {
            char key[KEY_LEN] = {0};
            sprintf(key, "%d", i);
            res = cms_add(&cms, key);
            if (res != j) {
                result = 1;
                // printf("Error with key=%s\ti=%d\tres=%d\n", key, i, res);
            }
        }
    }
    success_or_failure(result);


    /* test max check */
    printf("Count-Min Sketch: check number of insertions using min strategy: ");
    result = 0;
    for (i = 0; i < 10; i++) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        res = cms_check_min(&cms, key);
        if (res != 10) {
            result = 1;
            // printf("Error with key=%s\ti=%d\tres=%d\n", key, i, res);
        }
    }
    success_or_failure(result);

    /* test mean check */
    printf("Count-Min Sketch: check mean insertions: ");
    result = 0;
    for (i = 0; i < 10; i++) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        int error_rate = (10 + ceil(10 * cms.error_rate));
        res = cms_check_mean(&cms, key);
        if (res < 10 || res > error_rate ) {
            // printf("Error with key=%s\ti=%d\tres=%d\n", key, i, res);
            result = 1;
        }
    }
    success_or_failure(result);

    printf("Count-Min Sketch: export: ");
    result = cms_export(&cms, "./dist/test_export.cms");
    success_or_failure(result);

    /* test remove */
    printf("Count-Min Sketch: check remove: ");
    result = 0;
    for (i = 0; i < 10; i++) {
        for (j = 10; j > 0; j--) {
            char key[KEY_LEN] = {0};
            sprintf(key, "%d", i);
            res = cms_remove(&cms, key);
            if (res != j - 1) {
                result = 1;
                // printf("Error with key=%s\ti=%d\tres=%d\n", key, j, res);
            }
        }
    }
    // do this too before declaring it a success
    res = cms_check(&cms, "0");
    if (res != 0) {
        result = 1;
        // printf("Error with lookup: %d\n", res);
    }
    success_or_failure(result);

    printf("Count-Min Sketch: destroy: ");
    result = 0;
    cms_destroy(&cms);
    if (cms.width != 0 || cms.depth != 0) {
        result = 1;
    } else if (cms.confidence != 0.0 || cms.error_rate != 0.0) {
        result = 1;
    }
    success_or_failure(result);


    printf("Count-Min Sketch: setup using confidence and error rate: ");
    cms_init_optimal(&cms, 0.001, 0.99999);
    if (cms.confidence >= 0.992 && cms.error_rate >= 0.000200) {
        success_or_failure(0);
    } else {
        success_or_failure(1);
    }

    printf("Count-Min Sketch: set up width and depth: ");
    if (cms.width == 2000 && cms.depth == 17) {
        success_or_failure(0);
    } else {
        success_or_failure(1);
    }

    cms_destroy(&cms);

    printf("Count-Min Sketch: import: ");
    result = 0;
    result = cms_import(&cms, "./dist/test_export.cms");
    success_or_failure(result);

    printf("Count-Min Sketch: import values correct: ");
    result = 0;
    if (cms.width != 10000 && cms.depth != 7) {
        result = 1;
    } else if (cms.confidence < 0.992 && cms.error_rate < 0.000200) {
        result = 1;
    }
    success_or_failure(result);

    printf("Count-Min Sketch: after import check max number of insertions using the min strategy: ");
    result = 0;
    for (i = 0; i < 10; i++) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        res = cms_check_min(&cms, key);
        if (res != 10) {
            result = 1;
            printf("Error with key=%s\ti=%d\tres=%d\n", key, i, res);
        }
    }
    success_or_failure(result);
    cms_destroy(&cms);

    timing_end(&tm);
    printf("\nCompleted Count-Min Sketch tests in %f seconds!\n", timing_get_difference(tm));
    printf("\nCompleted tests!\n");

    return 0;
}




/* PRIVATE FUNCTIONS */
void success_or_failure(int res) {
    if (res == 0) {
        printf(KGRN "success!\n" KNRM);
    } else {
        printf(KRED "failure!\n" KNRM);
    }
}
