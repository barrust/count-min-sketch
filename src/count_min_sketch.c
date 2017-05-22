/*******************************************************************************
***
***     Author: Tyler Barrus
***     email:  barrust@gmail.com
***
***     Version: 0.0.1
***
***     License: MIT 2017
***
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <inttypes.h>       /* PRIu64 */
#include "count_min_sketch.h"

/* private functions */
static uint64_t* __default_hash(int num_hashes, char *key);
static uint64_t __fnv_1a(char *key);




int cms_init(CountMinSketch *cms, int width, int depth) {
    return cms_init_alt(cms, width, depth, NULL);
}

int cms_init_alt(CountMinSketch *cms, int width, int depth, cms_hash_function hash_function) {
    cms->width = width;
    cms->depth = depth;
    cms->bins = (unsigned int**) malloc(depth * sizeof(unsigned int*));
    int i;
    for (i = 0; i < cms->depth; i++) {
        cms->bins[i] = calloc(width, sizeof(unsigned int));
    }
    cms->hash_function = (hash_function == NULL) ? __default_hash : hash_function;

    return CMS_SUCCESS;
}

int cms_destroy(CountMinSketch *cms) {
    int i;
    for (i = 0; i < cms->depth; i++) {
        free(cms->bins[i]);
    }
    free(cms->bins);
    cms->width = 0;
    cms->depth = 0;
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
    return CMS_SUCCESS;
}

int cms_add(CountMinSketch *cms, char* key) {
    uint64_t* hashes = cms->hash_function(cms->depth, key);
    int i, num_add = 0;
    for (i = 0; i < cms->depth; i++) {
        int bin = hashes[i] % cms->width;
        cms->bins[i][bin]++;
        if (cms->bins[i][bin] > num_add) {
            num_add = cms->bins[i][bin];
        }
    }
    free(hashes);
    return num_add;
}

int cms_check(CountMinSketch *cms, char* key) {
    uint64_t* hashes = cms->hash_function(cms->depth, key);
    unsigned int i, num_add = UINT_MAX;
    for (i = 0; i < cms->depth; i++) {
        int bin = hashes[i] % cms->width;
        if (cms->bins[i][bin] < num_add) {
            num_add = cms->bins[i][bin];
        }
    }
    free(hashes);
    return num_add;
}

/*******************************************************************************
*    PRIVATE FUNCTIONS
*******************************************************************************/
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
