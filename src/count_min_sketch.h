#ifndef BARRUST_SIMPLE_COUNT_MIN_SKETCH_H__
#define BARRUST_SIMPLE_COUNT_MIN_SKETCH_H__

/*******************************************************************************
***     Author: Tyler Barrus
***     email:  barrust@gmail.com
***     Version: 0.1.6
***     License: MIT 2017
*******************************************************************************/

#include <stdint.h>
#include <limits.h>         /* INT_MIN */

#define COUNT_MIN_SKETCH_VERSION "0.1.6"

/* https://gcc.gnu.org/onlinedocs/gcc/Alternate-Keywords.html#Alternate-Keywords */
#ifndef __GNUC__
#define __inline__ inline
#endif

/* hashing function type */
typedef uint64_t* (*cms_hash_function) (int num_hashes, const char* key);

typedef struct {
    uint32_t depth;
    uint32_t width;
    int64_t elements_added;
    double confidence;
    double error_rate;
    cms_hash_function hash_function;
    int32_t* bins;
}  CountMinSketch, count_min_sketch;


/* Initialize the count-min sketch based on user defined width and depth */
int cms_init_alt(CountMinSketch* cms, unsigned int width, unsigned int depth, cms_hash_function hash_function);
static __inline__ int cms_init(CountMinSketch* cms, unsigned int width, unsigned int depth) {
    return cms_init_alt(cms, width, depth, NULL);
}

/*  Initialize the count-min sketch based on user defined error rate and
    confidence values */
int cms_init_optimal_alt(CountMinSketch* cms, double error_rate, double confidence, cms_hash_function hash_function);
static __inline__ int cms_init_optimal(CountMinSketch* cms, float error_rate, float confidence) {
    return cms_init_optimal_alt(cms, error_rate, confidence, NULL);
}

// double cms_bias(CountMinSketch* cms);  // TODO: implement (?)

/* Clean up memory used in the count-min sketch */
int cms_destroy(CountMinSketch* cms);

/* Reset the count-min sketch to zero */
int cms_clear(CountMinSketch* cms);

/* Export count-min sketch to file */
int cms_export(CountMinSketch* cms, const char* filepath);

/*  Import count-min sketch from file
    NOTE: It is up to the caller to provide the correct hashing algorithm */
int cms_import_alt(CountMinSketch* cms, const char* filepath, cms_hash_function hash_function);
static __inline__ int cms_import(CountMinSketch* cms, const char* filepath) {
    return cms_import_alt(cms, filepath, NULL);
}

/* Add the provided key to the count-min sketch `x` times */
int32_t cms_add_inc(CountMinSketch* cms, const char* key, uint32_t x);
int32_t cms_add_inc_alt(CountMinSketch* cms, uint64_t* hashes, int num_hashes, uint32_t x);

/* Add the provided key to the count-min sketch */
static __inline__ int32_t cms_add(CountMinSketch* cms, const char* key) {
    return cms_add_inc(cms, key, 1);
}
static __inline__ int32_t cms_add_alt(CountMinSketch* cms, uint64_t* hashes, int num_hashes) {
    return cms_add_inc_alt(cms, hashes, num_hashes, 1);
}

/*  Remove the provided key to the count-min sketch `x` times;
    NOTE: Result Values can be negative
    NOTE: Best check method when remove is used is `cms_check_mean` */
int32_t cms_remove_inc(CountMinSketch* cms, const char* key, uint32_t x);
int32_t cms_remove_inc_alt(CountMinSketch* cms, uint64_t* hashes, int num_hashes, uint32_t x);

/*  Remove the provided key to the count-min sketch;
    NOTE: Result Values can be negative
    NOTE: Best check method when remove is used is `cms_check_mean` */
static __inline__ int32_t cms_remove(CountMinSketch* cms, const char* key) {
    return cms_remove_inc(cms, key, 1);
}
static __inline__ int32_t cms_remove_alt(CountMinSketch* cms, uint64_t* hashes, int num_hashes) {
    return cms_remove_inc_alt(cms, hashes, num_hashes, 1);
}

/* Determine the maximum number of times the key may have been inserted */
int32_t cms_check(CountMinSketch* cms, const char* key);
int32_t cms_check_alt(CountMinSketch* cms, uint64_t* hashes, int num_hashes);
static __inline__ int32_t cms_check_min(CountMinSketch* cms, const char* key) {
    return cms_check(cms, key);
}
static __inline__ int32_t cms_check_min_alt(CountMinSketch* cms, uint64_t* hashes, int num_hashes) {
    return cms_check_alt(cms, hashes, num_hashes);
}

/*  Determine the mean number of times the key may have been inserted
    NOTE: Mean check increases the over counting but is a `better` strategy
    when removes are added and negatives are possible */
int32_t cms_check_mean(CountMinSketch* cms, const char* key);
int32_t cms_check_mean_alt(CountMinSketch* cms, uint64_t* hashes, int num_hashes);

int32_t cms_check_mean_min(CountMinSketch* cms, const char* key);
int32_t cms_check_mean_min_alt(CountMinSketch* cms, uint64_t* hashes, int num_hashes);

/*  Return the hashes for the provided key based on the hashing function of
    the count-min sketch
    NOTE: Useful when multiple count-min sketches use the same hashing
    functions
    NOTE: Up to the caller to free the array of hash values */
uint64_t* cms_get_hashes_alt(CountMinSketch* cms, int num_hashes, const char* key);
static __inline__ uint64_t* cms_get_hashes(CountMinSketch* cms, const char* key) {
    return cms_get_hashes_alt(cms, cms->depth, key);
}

#define CMS_SUCCESS 0
#define CMS_ERROR   INT_MIN

#endif
