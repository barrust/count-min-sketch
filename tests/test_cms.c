#include <stdio.h>
#include <stdint.h>

#include <openssl/md5.h>

#include "minunit.h"
#include "../src/count_min_sketch.h"


CountMinSketch cms;
const int width = 1000;
const int depth = 5;


static int calculate_md5sum(const char* filename, char* digest);


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
    mu_assert_double_eq(0.002, cms.error_rate);
    mu_assert_double_eq(0.96875, cms.confidence);

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
    cms_init_optimal(&c, 0.002, 0.96875);
    mu_assert_double_eq(0.002, cms.error_rate);
    mu_assert_double_eq(0.96875, cms.confidence);
    mu_assert_int_eq(1000, c.width);
    mu_assert_int_eq(5, c.depth);
    mu_assert_not_null(c.bins);
    mu_assert_int_eq(0, cms.elements_added);
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
    mu_assert_int_eq(1, cms_add(&cms, "this is a test"));
    mu_assert_int_eq(2, cms_add(&cms, "this is a test"));
    mu_assert_int_eq(3, cms_add(&cms, "this is a test"));
    mu_assert_int_eq(4, cms_add(&cms, "this is a test"));

    mu_assert_int_eq(4, cms.elements_added);
}

MU_TEST(test_insertions_different) {
    mu_assert_int_eq(4, cms_add_inc(&cms, "this is a test", 4));
    mu_assert_int_eq(8, cms_add_inc(&cms, "this is a test", 4));
    mu_assert_int_eq(12, cms_add_inc(&cms, "this is a test", 4));
    mu_assert_int_eq(16, cms_add_inc(&cms, "this is a test", 4));

    mu_assert_int_eq(16, cms.elements_added);
}

MU_TEST(test_insertions_max) {
    uint32_t too_large = (uint32_t)INT32_MAX + 5;
    mu_assert_int_eq(INT32_MAX, cms_add_inc(&cms, "this is a test", too_large));
    mu_assert_int_eq(INT32_MAX, cms.elements_added);
    mu_assert_int_eq(INT32_MAX, cms_add_inc(&cms, "this is a test", 2));
    mu_assert_int_eq(INT32_MAX, cms.elements_added);
}

/*******************************************************************************
*   Test Removals
*******************************************************************************/
MU_TEST(test_removal_single) {
    mu_assert_int_eq(4, cms_add_inc(&cms, "this is a test", 4));
    mu_assert_int_eq(4, cms.elements_added);

    mu_assert_int_eq(3, cms_remove(&cms, "this is a test"));
    mu_assert_int_eq(3, cms.elements_added);
}

MU_TEST(test_removal_mult) {
    mu_assert_int_eq(16, cms_add_inc(&cms, "this is a test", 16));
    mu_assert_int_eq(16, cms.elements_added);

    mu_assert_int_eq(9, cms_remove_inc(&cms, "this is a test", 7));
    mu_assert_int_eq(9, cms.elements_added);
}

MU_TEST(test_removal_max_lower) {
    uint32_t too_large = (uint32_t)INT32_MAX + 5;
    mu_assert_int_eq(INT32_MIN, cms_remove_inc(&cms, "this is a test", too_large));
    mu_assert_int_eq(INT32_MIN, cms.elements_added);

    mu_assert_int_eq(INT32_MIN, cms_remove_inc(&cms, "this is a test", 2));
    mu_assert_int_eq(INT32_MIN, cms.elements_added);
}

/*******************************************************************************
*   Test Estimation Strageties
*******************************************************************************/


/*******************************************************************************
*   Test Clear / Reset
*******************************************************************************/
MU_TEST(test_clear) {
    cms_add_inc(&cms, "this is a test", 100);
    mu_assert_int_eq(100, cms.elements_added);
    mu_assert_int_eq(100, cms_check(&cms, "this is a test"));

    cms_clear(&cms);
    mu_assert_int_eq(0, cms.elements_added);
    mu_assert_int_eq(0, cms_check(&cms, "this is a test"));
}

/*******************************************************************************
*   Test Export / Import
*******************************************************************************/
MU_TEST(test_cms_export) {
    cms_add_inc(&cms, "this is a test", 100);
    cms_export(&cms, "./tests/test.cms");
    char digest[33] = {0};
    calculate_md5sum("./tests/test.cms", digest);
    mu_assert_string_eq("61d2ea9d0cb09b7bb284e1cf1a860449", digest);
    remove("./tests/test.cms");
}

MU_TEST(test_cms_import) {
    cms_add_inc(&cms, "this is a test", 100);
    cms_export(&cms, "./tests/test.cms");

    CountMinSketch imp;
    cms_import(&imp, "./tests/test.cms");
    mu_assert_int_eq(100, imp.elements_added);
    mu_assert_int_eq(100, cms_check(&imp, "this is a test"));
    mu_assert_int_eq(0, cms_check(&imp, "this is also a test"));
    cms_destroy(&imp);

    remove("./tests/test.cms");
}



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
    MU_RUN_TEST(test_removal_single);
    MU_RUN_TEST(test_removal_mult);
    MU_RUN_TEST(test_removal_max_lower);

    /* different estimation strategies mean, min, mean-min */

    /* clear / reset */
    MU_RUN_TEST(test_clear);

    /* export and import */
    MU_RUN_TEST(test_cms_export);
    MU_RUN_TEST(test_cms_import);
}

int main() {
    // we want to ignore stderr print statements
    freopen("/dev/null", "w", stderr);

    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    printf("Number failed tests: %d\n", minunit_fail);
    return minunit_fail;
}




/* private functions */
static int calculate_md5sum(const char* filename, char* digest) {
    //open file for calculating md5sum
    FILE *file_ptr;
    file_ptr = fopen(filename, "r");
    if (file_ptr==NULL) {
        perror("Error opening file");
        fflush(stdout);
        return 1;
    }

    int n;
    MD5_CTX c;
    char buf[512];
    ssize_t bytes;
    unsigned char out[MD5_DIGEST_LENGTH];

    MD5_Init(&c);
    do {
        bytes=fread(buf, 1, 512, file_ptr);
        MD5_Update(&c, buf, bytes);
    } while(bytes > 0);

    MD5_Final(out, &c);

    for (n = 0; n < MD5_DIGEST_LENGTH; n++) {
        char hex[3] = {0};
        sprintf(hex, "%02x", out[n]);
        digest[n*2] = hex[0];
        digest[n*2+1] = hex[1];
    }
    return 0;
}
