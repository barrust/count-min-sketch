#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <openssl/evp.h>
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
    uint64_t too_large = (uint32_t)INT32_MAX + 5;
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
    mu_assert_string_eq("fb1c39dd1a73f1ef0d7fc79f60fc028e", digest);
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

MU_TEST(test_cms_merge_into_mismatch) {
    CountMinSketch c;
    cms_init(&c, width*2, depth);  // twice as wide!

    cms_add_inc(&cms, "this is a test", 255);

    int32_t res = cms_merge_into(&c, 1, &cms);
    mu_assert_int_eq(CMS_ERROR, res);
    cms_destroy(&c);
}

MU_TEST(test_cms_merge_mismatch) {
    CountMinSketch c;
    cms_init(&c, width*2, depth);  // twice as wide!

    cms_add_inc(&cms, "this is a test", 255);

    CountMinSketch d;
    int32_t res = cms_merge(&d, 2, &cms, &c);
    mu_assert_int_eq(CMS_ERROR, res);
    cms_destroy(&c);
}



/*******************************************************************************
*   Code review fixes
*******************************************************************************/
/* set the bin of each row that `key` maps to */
static void __set_key_bins(CountMinSketch* c, const char* key, const int32_t* values) {
    uint64_t* hashes = cms_get_hashes(c, key);
    for (unsigned int i = 0; i < c->depth; ++i) {
        c->bins[(hashes[i] % c->width) + (i * c->width)] = values[i];
    }
    free(hashes);
}

MU_TEST(test_cms_init_dimensions_overflow) {
    CountMinSketch c;
    /* width * depth overflowed an unsigned int, allocating too little; the first add crashed */
    mu_assert_int_eq(CMS_ERROR, cms_init(&c, 65536, 65537));
    mu_assert_int_eq(CMS_ERROR, cms_init(&c, 70000, 70000));
    mu_assert_int_eq(CMS_ERROR, cms_init(&c, 0, 5));
    mu_assert_int_eq(CMS_ERROR, cms_init(&c, 5, 0));
}

MU_TEST(test_cms_init_optimal_invalid) {
    CountMinSketch c;
    /* these converted infinity to an unsigned int, or gave a depth of 0 */
    mu_assert_int_eq(CMS_ERROR, cms_init_optimal(&c, 0.0, 0.5));
    mu_assert_int_eq(CMS_ERROR, cms_init_optimal(&c, 0.1, 1.0));
    mu_assert_int_eq(CMS_ERROR, cms_init_optimal(&c, 0.1, 0.0));
    mu_assert_int_eq(CMS_ERROR, cms_init_optimal(&c, 1e-12, 0.5));  /* a width over UINT32_MAX */
    /* a confidence too small to change 1 - confidence still gets one row */
    mu_assert_int_eq(CMS_SUCCESS, cms_init_optimal(&c, 0.1, 1e-20));
    mu_assert_int_eq(1, c.depth);
    cms_destroy(&c);
}

MU_TEST(test_cms_check_mean_saturated) {
    CountMinSketch c;
    cms_init(&c, 10, 4);
    cms_add_inc(&c, "a", 2000000000);
    cms_add_inc(&c, "a", 2000000000);  /* every row saturates at INT32_MAX */
    /* the mean summed in an int32, which overflowed (it returned 1073741823) */
    mu_assert_int_eq(INT32_MAX, cms_check_mean(&c, "a"));
    cms_destroy(&c);
}

MU_TEST(test_cms_mean_min_large_values) {
    CountMinSketch c;
    cms_init(&c, 3, 3);
    cms_add(&c, "a");
    /*  With 4e9 elements and these bins the mean-min values are 1e9, -5e9, and -2e9: their
        differences do not fit in an int, which the comparison truncated */
    const int32_t values[3] = {2000000000, -2000000000, 0};
    __set_key_bins(&c, "a", values);
    c.elements_added = 4000000000LL;
    mu_assert_int_eq(-2000000000, cms_check_mean_min(&c, "a"));

    /* a median beyond the int32 range is clamped (it wrapped around) */
    const int32_t high[3] = {2000000000, 2000000000, 2000000000};
    __set_key_bins(&c, "a", high);
    c.elements_added = -9000000000LL;  /* each mean-min value is 7.5e9 */
    mu_assert_int_eq(INT32_MAX, cms_check_mean_min(&c, "a"));
    cms_destroy(&c);
}

MU_TEST(test_cms_mean_min_width_one) {
    CountMinSketch c;
    cms_init(&c, 1, 3);
    cms_add(&c, "a");
    cms_add_inc(&c, "b", 2);
    /* with a single column there is no noise to estimate (it divided by zero) */
    mu_assert_int_eq(3, cms_check_mean_min(&c, "a"));
    cms_destroy(&c);
}

MU_TEST(test_cms_merge_keeps_saturation) {
    CountMinSketch a, b;
    cms_init(&a, 10, 3);
    cms_init(&b, 10, 3);
    cms_remove_inc(&a, "x", 10);
    cms_add_inc(&b, "x", UINT32_MAX);  /* saturates at INT32_MAX */
    mu_assert_int_eq(CMS_SUCCESS, cms_merge_into(&a, 1, &b));
    mu_assert_int_eq(INT32_MAX, cms_check(&a, "x"));  /* it became INT32_MAX - 10 */

    cms_clear(&a);
    cms_clear(&b);
    cms_add_inc(&a, "x", 10);
    cms_remove_inc(&b, "x", UINT32_MAX);  /* saturates at INT32_MIN */
    mu_assert_int_eq(CMS_SUCCESS, cms_merge_into(&a, 1, &b));
    mu_assert_int_eq(INT32_MIN, cms_check(&a, "x"));
    cms_destroy(&a);
    cms_destroy(&b);
}

MU_TEST(test_cms_import_invalid) {
    CountMinSketch c;
    char filepath[] = "./tests/test_invalid.cms";
    /* a footer with a width of 0 */
    FILE* fp = fopen(filepath, "wb");
    uint32_t width = 0, depth = 3;
    int64_t elements = 0;
    fwrite(&width, sizeof(uint32_t), 1, fp);
    fwrite(&depth, sizeof(uint32_t), 1, fp);
    fwrite(&elements, sizeof(int64_t), 1, fp);
    fclose(fp);
    mu_assert_int_eq(CMS_ERROR, cms_import(&c, filepath));

    /* a valid footer with too few bins (it called exit(1)) */
    width = 10;
    fp = fopen(filepath, "wb");
    int32_t bin = 0;
    fwrite(&bin, sizeof(int32_t), 1, fp);
    fwrite(&width, sizeof(uint32_t), 1, fp);
    fwrite(&depth, sizeof(uint32_t), 1, fp);
    fwrite(&elements, sizeof(int64_t), 1, fp);
    fclose(fp);
    mu_assert_int_eq(CMS_ERROR, cms_import(&c, filepath));

    /*  large dimensions in a small file: rejected before allocating, as width * depth * 4 bytes
        (16 GB here) is more than the file holds */
    width = 65536;
    depth = 65535;
    fp = fopen(filepath, "wb");
    fwrite(&bin, sizeof(int32_t), 1, fp);
    fwrite(&width, sizeof(uint32_t), 1, fp);
    fwrite(&depth, sizeof(uint32_t), 1, fp);
    fwrite(&elements, sizeof(int64_t), 1, fp);
    fclose(fp);
    mu_assert_int_eq(CMS_ERROR, cms_import(&c, filepath));

    /* a valid footer after one bin more than it describes (31 bins for 10 x 3): it was imported */
    width = 10;
    depth = 3;
    fp = fopen(filepath, "wb");
    for (int i = 0; i < 31; ++i) {
        fwrite(&bin, sizeof(int32_t), 1, fp);
    }
    fwrite(&width, sizeof(uint32_t), 1, fp);
    fwrite(&depth, sizeof(uint32_t), 1, fp);
    fwrite(&elements, sizeof(int64_t), 1, fp);
    fclose(fp);
    mu_assert_int_eq(CMS_ERROR, cms_import(&c, filepath));

    CountMinSketch valid;
    cms_init(&valid, 10, 3);
    cms_add(&valid, "a");

    /* and without it, it imports */
    cms_export(&valid, filepath);
    mu_assert_int_eq(CMS_SUCCESS, cms_import(&c, filepath));
    mu_assert_int_eq(1, cms_check(&c, "a"));
    cms_destroy(&c);
    cms_destroy(&valid);
    remove(filepath);
}

MU_TEST(test_cms_export_matches_bins) {
    /* the export is the bins in memory order, then the footer */
    CountMinSketch c;
    cms_init(&c, 100, 4);
    for (int i = 0; i < 1000; ++i) {
        char key[16] = {0};
        sprintf(key, "%d", i);
        cms_add_inc(&c, key, (uint32_t)(i % 5 + 1));
    }
    char filepath[] = "./tests/test_export_bins.cms";
    cms_export(&c, filepath);
    FILE* fp = fopen(filepath, "rb");
    int32_t* bins = (int32_t*)calloc(400, sizeof(int32_t));
    uint32_t width = 0, depth = 0;
    int64_t elements = 0;
    mu_assert_int_eq(400, fread(bins, sizeof(int32_t), 400, fp));
    mu_assert_int_eq(1, fread(&width, sizeof(uint32_t), 1, fp));
    mu_assert_int_eq(1, fread(&depth, sizeof(uint32_t), 1, fp));
    mu_assert_int_eq(1, fread(&elements, sizeof(int64_t), 1, fp));
    mu_assert_int_eq(EOF, fgetc(fp));
    fclose(fp);
    mu_assert_int_eq(0, memcmp(bins, c.bins, 400 * sizeof(int32_t)));
    mu_assert_int_eq(100, width);
    mu_assert_int_eq(4, depth);
    mu_assert(elements == c.elements_added, "elements added should match");
    free(bins);
    remove(filepath);
    cms_destroy(&c);
}

/* a hash function that cannot allocate its results */
static uint64_t* __null_hash(unsigned int num_hashes, const char* key) {
    (void)num_hashes;
    (void)key;
    return NULL;
}

MU_TEST(test_cms_hash_allocation_failure) {
    CountMinSketch c;
    mu_assert_int_eq(CMS_SUCCESS, cms_init_alt(&c, 10, 3, __null_hash));
    mu_assert_int_eq(CMS_ERROR, cms_add(&c, "a"));
    mu_assert_int_eq(CMS_ERROR, cms_remove(&c, "a"));
    mu_assert_int_eq(CMS_ERROR, cms_check(&c, "a"));
    mu_assert_int_eq(CMS_ERROR, cms_check_mean(&c, "a"));
    mu_assert_int_eq(CMS_ERROR, cms_check_mean_min(&c, "a"));
    mu_assert_int_eq(0, c.elements_added);
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
    MU_RUN_TEST(test_cms_init_dimensions_overflow);
    MU_RUN_TEST(test_cms_init_optimal_invalid);
    MU_RUN_TEST(test_cms_check_mean_saturated);
    MU_RUN_TEST(test_cms_mean_min_large_values);
    MU_RUN_TEST(test_cms_mean_min_width_one);
    MU_RUN_TEST(test_cms_merge_keeps_saturation);
    MU_RUN_TEST(test_cms_import_invalid);
    MU_RUN_TEST(test_cms_hash_allocation_failure);
    MU_RUN_TEST(test_cms_export_matches_bins);
    MU_RUN_TEST(test_cms_merge_into);
    MU_RUN_TEST(test_cms_merge_into_mismatch);
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
    char buf[512];
    ssize_t bytes;
    unsigned char out[MD5_DIGEST_LENGTH];
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) {
        perror("EVP_MD_CTX_new failed");
        fclose(file_ptr);
        return 1;
    }
    if (EVP_DigestInit_ex(mdctx, EVP_md5(), NULL) != 1) {
        perror("EVP_DigestInit_ex failed");
        EVP_MD_CTX_free(mdctx);
        fclose(file_ptr);
        return 1;
    }
    do {
        bytes = fread(buf, 1, 512, file_ptr);
        if (bytes > 0) {
            if (EVP_DigestUpdate(mdctx, buf, bytes) != 1) {
                perror("EVP_DigestUpdate failed");
                EVP_MD_CTX_free(mdctx);
                fclose(file_ptr);
                return 1;
            }
        }
    } while(bytes > 0);
    if (EVP_DigestFinal_ex(mdctx, out, NULL) != 1) {
        perror("EVP_DigestFinal_ex failed");
        EVP_MD_CTX_free(mdctx);
        fclose(file_ptr);
        return 1;
    }
    EVP_MD_CTX_free(mdctx);

    for (n = 0; n < MD5_DIGEST_LENGTH; n++) {
        char hex[3] = {0};
        sprintf(hex, "%02x", out[n]);
        digest[n*2] = hex[0];
        digest[n*2+1] = hex[1];
    }

    fclose(file_ptr);

    return 0;
}

