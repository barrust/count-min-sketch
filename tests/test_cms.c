#include <stdio.h>
#include <limits.h>         /* INT_MIN */

#include "minunit.h"
#include "../src/count_min_sketch.h"


CountMinSketch cms;
const int width = 10000;
const int depth = 7;


void test_setup(void) {
    cms_init(&cms, width, depth);
}

void test_teardown(void) {
    cms_destroy(&cms);
}


/*******************************************************************************
*   Test the setup
*******************************************************************************/
MU_TEST(test_default_setup) {
    mu_assert_int_eq(width, cms.width);
    mu_assert_int_eq(depth, cms.depth);
    mu_assert_not_null(cms.bins);
    mu_assert_int_eq(0, cms.elements_added);
    mu_assert_double_between(0.000000, 0.00100, cms.error_rate);
    mu_assert_double_between(0.990, 0.9950, cms.confidence);

    int num_bins = cms.depth * cms.width;
    int res = 0;
    for (int i = 0; i < num_bins; ++i)
        res += (cms.bins[i] == 0) ? 0 : 1;
    mu_assert_int_eq(0, res);
}

MU_TEST(test_bad_init) {
    CountMinSketch c;
    mu_assert_int_eq(CMS_ERROR, cms_init(&c, 0, 5));
    mu_assert_int_eq(CMS_ERROR, cms_init(&c, 10000, 0));
}

MU_TEST(test_init_optimal) {
    CountMinSketch c;
    cms_init_optimal(&c, 0.001, 0.99999);
    mu_assert_double_between(0.000000, 0.00110, c.error_rate);
    mu_assert_double_between(0.992, 0.999999, c.confidence);
    mu_assert_int_eq(2000, c.width);
    mu_assert_int_eq(17, c.depth);
    mu_assert_not_null(c.bins);
    int num_bins = cms.depth * cms.width;
    int res = 0;
    for (int i = 0; i < num_bins; ++i)
        res += (cms.bins[i] == 0) ? 0 : 1;
    mu_assert_int_eq(0, res);
    cms_destroy(&c);
}

MU_TEST(test_init_optimal_bad) {
    CountMinSketch c;
    mu_assert_int_eq(CMS_ERROR, cms_init_optimal(&c, -0.001, 0.99999));
    mu_assert_int_eq(CMS_ERROR, cms_init_optimal(&c, 0.001, -0.99999));
}

/*******************************************************************************
*   Test Insertions
*******************************************************************************/
MU_TEST(test_insertions_normal) {
    int failures = 0;

    for (int i = 0; i < width; ++i) {
        char key[6] = {0};
        sprintf(key, "%d", i);
        int res = cms_add(&cms, key);
        // printf("%d\t", res);
        if (res > 2) {
            ++failures;
        }
    }

    mu_assert_int_eq(0, failures);
}

MU_TEST(test_insertions_different) {
    int failures = 0;
    for (int i = 0; i < width; ++i) {
        char key[6] = {0};
        sprintf(key, "%d", i);
        int res = cms_add_inc(&cms, key, i + 1);
        if (res == CMS_ERROR) {
            ++failures;
        }
    }
    mu_assert_int_eq(0, failures);
}

MU_TEST(test_insertions_max) {
    int failures = 0;
    for (int i = 0; i < width; ++i) {
        char key[6] = {0};
        sprintf(key, "%d", i);
        int res = cms_add_inc(&cms, key, INT_MAX);
        if (res == CMS_ERROR || res != INT_MAX) {
            ++failures;
        }
    }
    mu_assert_int_eq(0, failures);
}

/*******************************************************************************
*   Test Removals
*******************************************************************************/


/*******************************************************************************
*   Test Estimation Strageties
*******************************************************************************/


/*******************************************************************************
*   Test Clear / Reset
*******************************************************************************/


/*******************************************************************************
*   Test Export / Import
*******************************************************************************/


MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    /* setup */
    MU_RUN_TEST(test_default_setup);
    MU_RUN_TEST(test_bad_init);
    MU_RUN_TEST(test_init_optimal);
    MU_RUN_TEST(test_init_optimal_bad);

    /* insertions (inc, add, etc) */
    MU_RUN_TEST(test_insertions_normal);
    MU_RUN_TEST(test_insertions_different);
    MU_RUN_TEST(test_insertions_max);

    /* removal of items (dec, remove, etc) */

    /* different estimation strategies mean, min, mean-min */

    /* clear / reset */

    /* export and import */

}

int main() {
    // we want to ignore stderr print statements
    freopen("/dev/null", "w", stderr);

    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    printf("Number failed tests: %d\n", minunit_fail);
    return minunit_fail;
}
