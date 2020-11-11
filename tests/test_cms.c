#include <stdio.h>
#include <stdlib.h>
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
    mu_assert_int_eq(too_large, cms.elements_added);
    mu_assert_int_eq(INT32_MAX, cms_add_inc(&cms, "this is a test", 2));
    mu_assert_int_eq(too_large + 2, cms.elements_added);
}

MU_TEST(test_insertion_error) {
    uint64_t* hashes = cms_get_hashes_alt(&cms, 2, "this is a test");
    int32_t res = cms_add_alt(&cms, hashes, 2);
    mu_assert_int_eq(CMS_ERROR, res);
    free(hashes);
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
    mu_assert_int_eq(too_large * -1, cms.elements_added);

    mu_assert_int_eq(INT32_MIN, cms_remove_inc(&cms, "this is a test", 2));
    mu_assert_int_eq((too_large + 2) * -1 , cms.elements_added);
}

MU_TEST(test_removal_error) {
    mu_assert_int_eq(4, cms_add_inc(&cms, "this is a test", 4));

    uint64_t* hashes = cms_get_hashes_alt(&cms, 2, "this is a test");
    int32_t res = cms_remove_alt(&cms, hashes, 2);
    mu_assert_int_eq(CMS_ERROR, res);
    free(hashes);
}

/*******************************************************************************
*   Test Estimation Strageties
*******************************************************************************/
MU_TEST(test_check) {
    cms_add_inc(&cms, "this is a test", 255);
    cms_add_inc(&cms, "this is another test", 189);
    cms_add_inc(&cms, "this is also a test", 16);
    cms_add_inc(&cms, "this is something to test", 5);

    mu_assert_int_eq(255 + 189 + 16 + 5, cms.elements_added);

    mu_assert_int_eq(255, cms_check(&cms, "this is a test"));
    mu_assert_int_eq(189, cms_check(&cms, "this is another test"));
    mu_assert_int_eq(16, cms_check(&cms, "this is also a test"));
    mu_assert_int_eq(5, cms_check(&cms, "this is something to test"));
}

MU_TEST(test_check_error) {
    uint64_t* hashes = cms_get_hashes_alt(&cms, 2, "this is a test");
    int32_t res = cms_check_alt(&cms, hashes, 2);
    mu_assert_int_eq(CMS_ERROR, res);
    free(hashes);
}

MU_TEST(test_check_min) {
    cms_add_inc(&cms, "this is a test", 255);
    cms_add_inc(&cms, "this is another test", 189);
    cms_add_inc(&cms, "this is also a test", 16);
    cms_add_inc(&cms, "this is something to test", 5);

    mu_assert_int_eq(255 + 189 + 16 + 5, cms.elements_added);

    mu_assert_int_eq(255, cms_check_min(&cms, "this is a test"));
    mu_assert_int_eq(189, cms_check_min(&cms, "this is another test"));
    mu_assert_int_eq(16, cms_check_min(&cms, "this is also a test"));
    mu_assert_int_eq(5, cms_check_min(&cms, "this is something to test"));
}

MU_TEST(test_check_min_error) {
    uint64_t* hashes = cms_get_hashes_alt(&cms, 2, "this is a test");
    int32_t res = cms_check_min_alt(&cms, hashes, 2);
    mu_assert_int_eq(CMS_ERROR, res);
    free(hashes);
}

MU_TEST(test_check_mean) {
    cms_add_inc(&cms, "this is a test", 255);
    cms_add_inc(&cms, "this is another test", 189);
    cms_add_inc(&cms, "this is also a test", 16);
    cms_add_inc(&cms, "this is something to test", 5);

    mu_assert_int_eq(255 + 189 + 16 + 5, cms.elements_added);

    mu_assert_int_eq(255, cms_check_mean(&cms, "this is a test"));
    mu_assert_int_eq(189, cms_check_mean(&cms, "this is another test"));
    mu_assert_int_eq(16, cms_check_mean(&cms, "this is also a test"));
    mu_assert_int_eq(5, cms_check_mean(&cms, "this is something to test"));
}

MU_TEST(test_check_mean_error) {
    uint64_t* hashes = cms_get_hashes_alt(&cms, 2, "this is a test");
    int32_t res = cms_check_mean_alt(&cms, hashes, 2);
    mu_assert_int_eq(CMS_ERROR, res);
    free(hashes);
}

MU_TEST(test_check_mean_min) {
    cms_add_inc(&cms, "this is a test", 255);
    cms_add_inc(&cms, "this is another test", 189);
    cms_add_inc(&cms, "this is also a test", 16);
    cms_add_inc(&cms, "this is something to test", 5);

    mu_assert_int_eq(255 + 189 + 16 + 5, cms.elements_added);

    mu_assert_int_eq(255, cms_check_mean_min(&cms, "this is a test"));
    mu_assert_int_eq(189, cms_check_mean_min(&cms, "this is another test"));
    mu_assert_int_eq(16, cms_check_mean_min(&cms, "this is also a test"));
    mu_assert_int_eq(5, cms_check_mean_min(&cms, "this is something to test"));
}

MU_TEST(test_check_mean_min_even_depth) {
    CountMinSketch even;
    cms_init(&even, 5000, 4);
    cms_add_inc(&even, "this is a test", 255);
    cms_add_inc(&even, "this is another test", 189);
    cms_add_inc(&even, "this is also a test", 16);
    cms_add_inc(&even, "this is something to test", 5);

    mu_assert_int_eq(255 + 189 + 16 + 5, even.elements_added);

    mu_assert_int_eq(255, cms_check_mean_min(&even, "this is a test"));
    mu_assert_int_eq(189, cms_check_mean_min(&even, "this is another test"));
    mu_assert_int_eq(16, cms_check_mean_min(&even, "this is also a test"));
    mu_assert_int_eq(5, cms_check_mean_min(&even, "this is something to test"));
    cms_destroy(&even);
}

MU_TEST(test_check_mean_min_error) {
    uint64_t* hashes = cms_get_hashes_alt(&cms, 2, "this is a test");
    int32_t res = cms_check_mean_min_alt(&cms, hashes, 2);
    mu_assert_int_eq(CMS_ERROR, res);
    free(hashes);
}

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

MU_TEST(test_cms_import_error) {
    CountMinSketch imp;
    int32_t res = cms_import(&imp, "./tests/test.cms");
    mu_assert_int_eq(CMS_ERROR, res);
}


/*******************************************************************************
*   Test Merge
*******************************************************************************/
MU_TEST(test_cms_merge_simple) {
    cms_add_inc(&cms, "this is a test", 255);

    CountMinSketch n;
    int32_t res = cms_merge(&n, 2, &cms, &cms);
    mu_assert_int_eq(CMS_SUCCESS, res);

    mu_assert_int_eq(510, n.elements_added);
    mu_assert_int_eq(510, cms_check_min(&n, "this is a test"));

    cms_destroy(&n);
}

MU_TEST(test_cms_merge_overflow_up) {
    int64_t too_large = (int64_t)INT32_MAX + 5;
    cms_add_inc(&cms, "this is a test", too_large);

    CountMinSketch n;
    int32_t res = cms_merge(&n, 2, &cms, &cms);
    mu_assert_int_eq(CMS_SUCCESS, res);

    mu_assert_int_eq(too_large * 2, n.elements_added);
    mu_assert_int_eq(INT32_MAX, cms_check_min(&n, "this is a test"));

    cms_destroy(&n);
}

MU_TEST(test_cms_merge_overflow_down) {
    uint32_t too_large = (uint32_t)INT32_MAX + 5;
    mu_assert_int_eq(INT32_MIN, cms_remove_inc(&cms, "this is a test", too_large));
    mu_assert_int_eq(too_large * -1, cms.elements_added);

    CountMinSketch n;
    int32_t res = cms_merge(&n, 2, &cms, &cms);
    mu_assert_int_eq(CMS_SUCCESS, res);

    mu_assert_int_eq(too_large * -2, n.elements_added);
    mu_assert_int_eq(INT32_MIN, cms_check_min(&n, "this is a test"));

    cms_destroy(&n);
}

MU_TEST(test_cms_merge_into) {
    cms_add_inc(&cms, "this is a test", 255);

    int32_t res = cms_merge_into(&cms, 1, &cms);
    mu_assert_int_eq(CMS_SUCCESS, res);

    mu_assert_int_eq(510, cms.elements_added);
    mu_assert_int_eq(510, cms_check_min(&cms, "this is a test"));
}

MU_TEST(test_cms_merge_mismatch) {
    CountMinSketch c;
    cms_init(&c, width*2, depth);  // twice as wide!

    cms_add_inc(&cms, "this is a test", 255);

    int32_t res = cms_merge_into(&c, 1, &cms);
    mu_assert_int_eq(CMS_ERROR, res);
    cms_destroy(&c);
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
    MU_RUN_TEST(test_insertion_error);

    /* removal of items (dec, remove, etc) */
    MU_RUN_TEST(test_removal_single);
    MU_RUN_TEST(test_removal_mult);
    MU_RUN_TEST(test_removal_max_lower);
    MU_RUN_TEST(test_removal_error);

    /* different estimation strategies mean, min, mean-min */
    MU_RUN_TEST(test_check);
    MU_RUN_TEST(test_check_error);
    MU_RUN_TEST(test_check_min);
    MU_RUN_TEST(test_check_min_error);
    MU_RUN_TEST(test_check_mean);
    MU_RUN_TEST(test_check_mean_error);
    MU_RUN_TEST(test_check_mean_min);
    MU_RUN_TEST(test_check_mean_min_even_depth);
    MU_RUN_TEST(test_check_mean_min_error);

    /* clear / reset */
    MU_RUN_TEST(test_clear);

    /* export and import */
    MU_RUN_TEST(test_cms_export);
    MU_RUN_TEST(test_cms_import);
    MU_RUN_TEST(test_cms_import_error);

    /* merge */
    MU_RUN_TEST(test_cms_merge_simple);
    MU_RUN_TEST(test_cms_merge_overflow_up);
    MU_RUN_TEST(test_cms_merge_overflow_down);
    MU_RUN_TEST(test_cms_merge_into);
    MU_RUN_TEST(test_cms_merge_mismatch);
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
    FILE *file_ptr;
    file_ptr = fopen(filename, "r");
    if (file_ptr == NULL) {
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
        bytes = fread(buf, 1, 512, file_ptr);
        MD5_Update(&c, buf, bytes);
    } while(bytes > 0);

    MD5_Final(out, &c);

    for (n = 0; n < MD5_DIGEST_LENGTH; n++) {
        char hex[3] = {0};
        sprintf(hex, "%02x", out[n]);
        digest[n*2] = hex[0];
        digest[n*2+1] = hex[1];
    }

    fclose(file_ptr);

    return 0;
}
