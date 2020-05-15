
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include "timing.h"
#include "../src/count_min_sketch.h"

#define KEY_LEN 5

#define TEST_DEPTH 100
#define TEST_WIDTH 100000
#define TEST_MEAN_ERROR 0.015
#define TEST_MEAN_MIN_ERROR_MAX 5

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
    cms_init(&cms, 100000, 7);

    if (cms.width == 100000 && cms.depth == 7) {
        success_or_failure(0);
    } else {
        // printf("width: %d\tdepth: %d\n", cms.width, cms.depth);
        success_or_failure(1);
    }

    printf("Count-Min Sketch: auto set confidence and error rate: ");
    if (cms.confidence >= 0.9920 && cms.error_rate >= 0.000020) {
        success_or_failure(0);
    } else {
        // printf("confidence: %f\terror rate: %f\n", cms.confidence, cms.error_rate);
        success_or_failure(1);
    }

    printf("Count-Min Sketch: insertion: ");
    fflush(stdout);
    int i, j, res;
    result = 0;
    for (i = 0; i < TEST_DEPTH; ++i) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        // uint64_t* hashes = cms_get_hashes(&cms, key);
        for (j = 1; j <= TEST_WIDTH; ++j) {
            // res = cms_add_alt(&cms, hashes, cms.depth);
            res = cms_add(&cms, key);
            if (res != j) {
                result = 1;
                // printf("Error with key=%s\ti=%d\tres=%d\n", key, i, res);
            }
        }
        // free(hashes);
    }
    success_or_failure(result);

    printf("Count-Min Sketch: insertion total (%"PRIi64"): ", cms.elements_added);
    if (cms.elements_added == TEST_DEPTH * TEST_WIDTH) {
        success_or_failure(0);
    } else {
        success_or_failure(1);
    }

    /* test max check */
    printf("Count-Min Sketch: check number of insertions using min strategy: ");
    result = 0;
    for (i = 0; i < TEST_DEPTH; ++i) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        res = cms_check_min(&cms, key);
        if (res != TEST_WIDTH) {
            result = 1;
            // printf("Error with key=%s\ti=%d\tres=%d\n", key, i, res);
        }
    }
    success_or_failure(result);

    /* test mean check */
    printf("Count-Min Sketch: check number of insertions using mean strategy: ");
    result = 0;
    for (i = 0; i < TEST_DEPTH; ++i) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        int error_rate = (TEST_WIDTH + ceil(TEST_WIDTH * TEST_MEAN_ERROR));
        res = cms_check_mean(&cms, key);
        if (res < TEST_WIDTH || res > error_rate ) {
            // printf("Error with key=%s\ti=%d\tres=%d, error_rate=%d\n", key, i, res, error_rate);
            result = 1;
        }
    }
    success_or_failure(result);

    printf("Count-Min Sketch: check number of insertions using mean-min strategy (similar values): ");
    result = 0;
    for (i = 0; i < TEST_DEPTH; ++i) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        int error_rate = TEST_WIDTH * TEST_MEAN_ERROR;
        res = cms_check_mean_min(&cms, key);
        if (res >= TEST_WIDTH + error_rate || res <= TEST_WIDTH - error_rate) {
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
    fflush(stdout);
    result = 0;
    for (i = 0; i < TEST_DEPTH; ++i) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        for (j = TEST_WIDTH; j > 0; j--) {
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

    // test clear
    printf("Count-Min Sketch: clear: ");
    result = 0;
    cms_clear(&cms);
    for (i = 0; i < cms.depth * cms.width; ++i) {
        if (cms.bins[i] != 0) {
            result = 1;
        }
    }
    if (cms.elements_added != 0) {
        result = 1;
    }
    success_or_failure(result);

    printf("Count-Min Sketch: insertions using mean-min strategy (diverse values): ");
    result = 0;
    for (i = 0; i < TEST_DEPTH; ++i) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        int t = TEST_DEPTH * (i + 1);
        res = cms_add_inc(&cms, key, t);
        if (res != t) {
            result = 1;
        }
    }
    success_or_failure(result);

    printf("Count-Min Sketch: check number of insertions using mean-min strategy (diverse values): ");
    result = 0;
    for (i = 0; i < TEST_DEPTH; ++i) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        int t = TEST_DEPTH * (i + 1);
        res = cms_check_mean_min(&cms, key);
        if (res > t + TEST_MEAN_MIN_ERROR_MAX || res < t - TEST_MEAN_MIN_ERROR_MAX) {
            // printf("i: %d\tt: %d\tres: %d\terr: %d\n", i, t, res, TEST_MEAN_MIN_ERROR_MAX);
            result = 1;
        }
    }
    success_or_failure(result);

    printf("cms->confidence: %f\n", cms.confidence);
    result = cms_export(&cms, "./dist/c_test.cms");

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
    if (cms.width != 100000 && cms.depth != 7) {
        printf("width: %d\tdepth: %d\n", cms.width, cms.depth);
        result = 1;
    } else if (cms.confidence < 0.992 && cms.error_rate < 0.000200) {
        printf("confidence: %f\terror rate: %f\n", cms.confidence, cms.error_rate);
        result = 1;
    }
    success_or_failure(result);

    printf("Count-Min Sketch: after import check max number of insertions using the min strategy: ");
    result = 0;
    for (i = 0; i < TEST_DEPTH; ++i) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        res = cms_check_min(&cms, key);
        if (res != TEST_WIDTH) {
            result = 1;
            // printf("Error with key=%s\ti=%d\tres=%d\n", key, i, res);
        }
    }
    success_or_failure(result);

    CountMinSketch cmsmerged;
    printf("Count-Min Sketch: merge: ");
    result = cms_merge(&cmsmerged, 2, &cms, &cms);
    success_or_failure(result);

    /* test max check */
    printf("Count-Min Sketch: check number of insertions using min strategy: ");
    result = 0;
    for (i = 0; i < TEST_DEPTH; ++i) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        res = cms_check_min(&cmsmerged, key);
        if (res != (2 * TEST_WIDTH)) {
            result = 1;
            printf("Error with key=%s\ti=%d\tres=%d\n", key, i, res);
        }
    }
    success_or_failure(result);

    /* test mean check */
    printf("Count-Min Sketch: check number of insertions using mean strategy: ");
    result = 0;
    for (i = 0; i < TEST_DEPTH; ++i) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        int error_rate = ((2 * TEST_WIDTH) + ceil((2 * TEST_WIDTH) * TEST_MEAN_ERROR));
        res = cms_check_mean(&cmsmerged, key);
        if (res < (2 * TEST_WIDTH) || res > error_rate ) {
            printf("Error with key=%s\ti=%d\tres=%d, error_rate=%d\n", key, i, res, error_rate);
            result = 1;
        }
    }
    success_or_failure(result);

    printf("Count-Min Sketch: check number of insertions using mean-min strategy (similar values): ");
    result = 0;
    for (i = 0; i < TEST_DEPTH; ++i) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        int error_rate = (2 * TEST_WIDTH) * TEST_MEAN_ERROR;
        res = cms_check_mean_min(&cmsmerged, key);
        if (res >= (2 * TEST_WIDTH) + error_rate || res <= (2 * TEST_WIDTH) - error_rate) {
            printf("Error with key=%s\ti=%d\tres=%d\n", key, i, res);
            result = 1;
        }
    }
    success_or_failure(result);

    cms_destroy(&cmsmerged);
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
