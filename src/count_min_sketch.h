#ifndef BARRUST_SIMPLE_COUNT_MIN_SKETCH_H__
#define BARRUST_SIMPLE_COUNT_MIN_SKETCH_H__

/*******************************************************************************
***     Author: Tyler Barrus
***     email:  barrust@gmail.com
***     Version: 0.1.1
***     License: MIT 2017
*******************************************************************************/

#include <inttypes.h>       /* PRIu64 */

/* hashing function type */
typedef uint64_t* (*cms_hash_function) (int num_hashes, char *key);

typedef struct {
    int depth;
    int width;
    cms_hash_function hash_function;
    int** bins;
}  CountMinSketch, count_min_sketch;


int cms_init_alt(CountMinSketch *cms, int width, int depth, cms_hash_function hash_function);
static int __inline__  cms_init(CountMinSketch *cms, int width, int depth) {
    return cms_init_alt(cms, width, depth, NULL);
}

int cms_destroy(CountMinSketch *cms);

int cms_clear(CountMinSketch *cms);

int cms_add(CountMinSketch *cms, char* key);

int cms_remove(CountMinSketch *cms, char* key);

int cms_check(CountMinSketch *cms, char* key);
int cms_check_min(CountMinSketch *cms, char* key);

int cms_check_mean(CountMinSketch *cms, char* key);


// TODO: add additional functionality
// uint64_t* cms_get_hashes(CountMinSketch *cms);
// uint64_t* cms_get_hashes_alt(CountMinSketch *cms, int num_hashes);

#define CMS_SUCCESS 0

#endif
