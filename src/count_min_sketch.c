/*******************************************************************************
***     Author: Tyler Barrus
***     email:  barrust@gmail.com
***     Version: 0.1.1
***     License: MIT 2017
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <inttypes.h>       /* PRIu64 */
#include <math.h>
#include "count_min_sketch.h"

#define LOG_TWO 0.6931471805599453

/* private functions */
static int __setup_cms(CountMinSketch *cms, int width, int depth, double error_rate, double confidence, cms_hash_function hash_function);
static uint64_t* __default_hash(int num_hashes, char *key);
static uint64_t __fnv_1a(char *key);



int cms_init_optimal_alt(CountMinSketch *cms, double error_rate, double confidence, cms_hash_function hash_function) {
    /* https://cs.stackexchange.com/q/44803 */
    int width = ceil(2 / error_rate);
    int depth = ceil((-1 * log(1 - confidence)) / LOG_TWO);
    return __setup_cms(cms, width, depth, error_rate, confidence, hash_function);
}

int cms_init_alt(CountMinSketch *cms, int width, int depth, cms_hash_function hash_function) {
    double confidence = 1 - (1 / pow(2, depth));
    double error_rate = 2 / (double) width;
    return __setup_cms(cms, width, depth, error_rate, confidence, hash_function);
}

int cms_destroy(CountMinSketch *cms) {
    int i;
    for (i = 0; i < cms->depth; i++) {
        free(cms->bins[i]);
    }
    free(cms->bins);
    cms->width = 0;
    cms->depth = 0;
    cms->confidence = 0.0;
    cms->error_rate = 0.0;
    cms->elements_added = 0;
    cms->hash_function = NULL;

    return CMS_SUCCESS;
}

int cms_clear(CountMinSketch *cms) {
    int i, j;
    for (i = 0; i < cms->depth; i++) {
        for (j = 0; j < cms->width; j++) {
            cms->bins[i][j] = 0;
        }
    }
    cms->elements_added = 0;
    return CMS_SUCCESS;
}

int cms_add(CountMinSketch *cms, char* key) {
    uint64_t* hashes = cms_get_hashes(cms, key);
    int i, num_add = 0;
    for (i = 0; i < cms->depth; i++) {
        int bin = hashes[i] % cms->width;
        if (cms->bins[i][bin] != INT_MAX) {
            cms->bins[i][bin]++;
        }
        if (cms->bins[i][bin] > num_add) {
            num_add = cms->bins[i][bin];
        }
    }
    cms->elements_added++;
    free(hashes);
    return num_add;
}

int cms_remove(CountMinSketch *cms, char* key) {
    uint64_t* hashes = cms_get_hashes(cms, key);
    int i, num_add = 0;
    for (i = 0; i < cms->depth; i++) {
        int bin = hashes[i] % cms->width;
        if (cms->bins[i][bin] != INT_MIN) {
            cms->bins[i][bin]--;
        }
        if (cms->bins[i][bin] > num_add) {
            num_add = cms->bins[i][bin];
        }
    }
    cms->elements_added--;
    free(hashes);
    return num_add;
}

int cms_check(CountMinSketch *cms, char* key) {
    uint64_t* hashes = cms_get_hashes(cms, key);
    int i, num_add = INT_MAX;
    for (i = 0; i < cms->depth; i++) {
        int bin = hashes[i] % cms->width;
        if (cms->bins[i][bin] < num_add) {
            num_add = cms->bins[i][bin];
        }
    }
    free(hashes);
    return num_add;
}

int cms_check_mean(CountMinSketch *cms, char* key) {
    uint64_t* hashes = cms_get_hashes(cms, key);
    int i, num_add = 0;
    for (i = 0; i < cms->depth; i++) {
        int bin = hashes[i] % cms->width;
        if (cms->bins[i][bin] < num_add) {
            num_add += cms->bins[i][bin];
        }
    }
    free(hashes);
    return num_add / cms->depth;
}

uint64_t* cms_get_hashes_alt(CountMinSketch *cms, int num_hashes, char* key) {
    return cms->hash_function(num_hashes, key);
}

/*******************************************************************************
*    PRIVATE FUNCTIONS
*******************************************************************************/
static int __setup_cms(CountMinSketch *cms, int width, int depth, double error_rate, double confidence, cms_hash_function hash_function) {
    cms->width = width;
    cms->depth = depth;
    cms->confidence = confidence;
    cms->error_rate = error_rate;
    cms->elements_added = 0;
    cms->bins = (int**) malloc(depth * sizeof(int*));
    int i;
    for (i = 0; i < cms->depth; i++) {
        cms->bins[i] = calloc(width, sizeof(int));
    }
    cms->hash_function = (hash_function == NULL) ? __default_hash : hash_function;

    return CMS_SUCCESS;
}

static uint64_t* __default_hash(int num_hashes, char *key) {
    uint64_t *results = calloc(num_hashes, sizeof(uint64_t));
    int i;
    char *str = calloc(17, sizeof(char));  // largest value is 7FFF,FFFF,FFFF,FFFF
    for (i = 0; i < num_hashes; i++) {
        if (i == 0) {
            results[i] = __fnv_1a(key);
        } else {
            uint64_t prev = results[i-1];
            memset(str, 0, 17);
            sprintf(str, "%" PRIx64 "", prev);
            results[i] = __fnv_1a(str);
        }
    }
    free(str);
    return results;
}

static uint64_t __fnv_1a(char *key) {
    // FNV-1a hash (http://www.isthe.com/chongo/tech/comp/fnv/)
    int i, len = strlen(key);
    uint64_t h = 14695981039346656073ULL; // FNV_OFFSET 64 bit
    for (i = 0; i < len; i++){
            h = h ^ (unsigned char) key[i];
            h = h * 1099511628211ULL; // FNV_PRIME 64 bit
    }
    return h;
}
