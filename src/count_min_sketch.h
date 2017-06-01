#ifndef BARRUST_SIMPLE_COUNT_MIN_SKETCH_H__
#define BARRUST_SIMPLE_COUNT_MIN_SKETCH_H__

/*******************************************************************************
***     Author: Tyler Barrus
***     email:  barrust@gmail.com
***     Version: 0.1.1
***     License: MIT 2017
*******************************************************************************/

#include <inttypes.h>       /* PRIu64 */
#include <limits.h>         /* INT_MIN */

/* https://gcc.gnu.org/onlinedocs/gcc/Alternate-Keywords.html#Alternate-Keywords */
#ifndef __GNUC__
#define __inline__ inline
#endif

/* hashing function type */
typedef uint64_t* (*cms_hash_function) (int num_hashes, char *key);

typedef struct {
    int depth;
    int width;
    long elements_added;
    double confidence;
    double error_rate;
    cms_hash_function hash_function;
    int* bins;
}  CountMinSketch, count_min_sketch;


/* Initialize the count-min sketch based on user defined width and depth */
int cms_init_alt(CountMinSketch *cms, int width, int depth, cms_hash_function hash_function);
static __inline__ int cms_init(CountMinSketch *cms, int width, int depth) {
    return cms_init_alt(cms, width, depth, NULL);
}

/*  Initialize the count-min sketch based on user defined error rate and
    confidence values */
int cms_init_optimal_alt(CountMinSketch *cms, double error_rate, double confidence, cms_hash_function hash_function);
static __inline__ int cms_init_optimal(CountMinSketch *cms, float error_rate, float confidence) {
    return cms_init_optimal_alt(cms, error_rate, confidence, NULL);
}

/* Clean up memory used in the count-min sketch */
int cms_destroy(CountMinSketch *cms);

/* Reset the count-min sketch to zero */
int cms_clear(CountMinSketch *cms);

/* Add the provided key to the count-min sketch */
int cms_add(CountMinSketch *cms, char* key);
int cms_add_alt(CountMinSketch *cms, uint64_t* hashes, int num_hashes);

/*  Remove the provided key to the count-min sketch;
    NOTE: Values can be negative
    NOTE: Best check method when remove is used is `cms_check_mean` */
int cms_remove(CountMinSketch *cms, char* key);
int cms_remove_alt(CountMinSketch *cms, uint64_t* hashes, int num_hashes);

/* Determine the maximum number of times the key may have been inserted */
int cms_check(CountMinSketch *cms, char* key);
int cms_check_alt(CountMinSketch *cms, uint64_t* hashes, int num_hashes);
static __inline__ int cms_check_max(CountMinSketch *cms, char* key) {
    return cms_check(cms, key);
}
static __inline__ int cms_check_max_alt(CountMinSketch *cms, uint64_t* hashes, int num_hashes) {
    return cms_check_alt(cms, hashes, num_hashes);
}

/*  Determine the mean number of times the key may have been inserted
    NOTE: Mean check increases the over counting but is a `better` strategy
    when removes are added and negatives are possible */
int cms_check_mean(CountMinSketch *cms, char* key);
int cms_check_mean_alt(CountMinSketch *cms, uint64_t* hashes, int num_hashes);

/*  Return the hashes for the provided key based on the hashing function of
    the count-min sketch
    NOTE: Useful when multiple count-min sketches use the same hashing
    functions */
uint64_t* cms_get_hashes_alt(CountMinSketch *cms, int num_hashes, char* key);
static __inline__ uint64_t* cms_get_hashes(CountMinSketch *cms, char* key) {
    return cms_get_hashes_alt(cms, cms->depth, key);
}

#define CMS_SUCCESS 0
#define CMS_ERROR   INT_MIN

#endif
