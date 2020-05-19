#include <stdio.h>

#include "minunit.h"
#include "../src/count_min_sketch.h"


CountMinSketch cms;

void test_setup(void) {
    cms_init(&cms, 10000, 7);
}

void test_teardown(void) {
    cms_destroy(&cms);
}


/* Test Setup */
MU_TEST(test_default_setup) {
    mu_assert_int_eq(10000, cms.width);
    mu_assert_int_eq(7, cms.depth);
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


MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    /* setup */
    MU_RUN_TEST(test_default_setup);
    MU_RUN_TEST(test_bad_init);
    MU_RUN_TEST(test_init_optimal);
    MU_RUN_TEST(test_init_optimal_bad);



}

int main() {
    // we want to ignore stderr print statements
    freopen("/dev/null", "w", stderr);

    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    printf("Number failed tests: %d\n", minunit_fail);
    return minunit_fail;
}
