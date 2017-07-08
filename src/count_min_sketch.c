/*******************************************************************************
***     Author: Tyler Barrus
***     email:  barrust@gmail.com
***     Version: 0.1.5
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
static int __setup_cms(CountMinSketch *cms, uint32_t width, uint32_t depth, double error_rate, double confidence, cms_hash_function hash_function);
static void __write_to_file(CountMinSketch *cms, FILE *fp, short on_disk);
static void __read_from_file(CountMinSketch *cms, FILE *fp, short on_disk, char *filename);
static uint64_t* __default_hash(int32_t num_hashes, char *key);
static uint64_t __fnv_1a(char *key);
int __compare (const void * a, const void * b);
int32_t __safe_add(int32_t a, int32_t b);
int32_t __safe_sub(int32_t a, int32_t b);


int cms_init_optimal_alt(CountMinSketch *cms, double error_rate, double confidence, cms_hash_function hash_function) {
    /* https://cs.stackexchange.com/q/44803 */
    uint32_t width = ceil(2 / error_rate);
    uint32_t depth = ceil((-1 * log(1 - confidence)) / LOG_TWO);
    return __setup_cms(cms, width, depth, error_rate, confidence, hash_function);
}

int cms_init_alt(CountMinSketch *cms, uint32_t width, uint32_t depth, cms_hash_function hash_function) {
    double confidence = 1 - (1 / pow(2, depth));
    double error_rate = 2 / (double) width;
    return __setup_cms(cms, width, depth, error_rate, confidence, hash_function);
}

int cms_destroy(CountMinSketch *cms) {
    free(cms->bins);
    cms->width = 0;
    cms->depth = 0;
    cms->confidence = 0.0;
    cms->error_rate = 0.0;
    cms->elements_added = 0;
    cms->hash_function = NULL;
    cms->bins = NULL;

    return CMS_SUCCESS;
}

int cms_clear(CountMinSketch *cms) {
    uint32_t i, j = cms->width * cms->depth;
    for (i = 0; i < j; i++) {
        cms->bins[i] = 0;
    }
    cms->elements_added = 0;
    return CMS_SUCCESS;
}

int32_t cms_add_inc_alt(CountMinSketch *cms, uint64_t* hashes, int32_t num_hashes, uint32_t x) {
    if (num_hashes < cms->depth) {
        fprintf(stderr, "Insufficient hashes to complete the addition of the element to the count-min sketch!");
        return CMS_ERROR;
    }
    int i, num_add = INT_MAX;
    for (i = 0; i < cms->depth; i++) {
        uint64_t bin = (hashes[i] % cms->width) + (i * cms->width);
        cms->bins[bin] = __safe_add(cms->bins[bin], x);
        /* currently a standard min strategy */
        if (cms->bins[bin] < num_add) {
            num_add = cms->bins[bin];
        }
    }
    cms->elements_added = __safe_add(cms->elements_added, (int64_t)x);
    return num_add;
}

int32_t cms_add_inc(CountMinSketch *cms, char* key, unsigned int x) {
    uint64_t* hashes = cms_get_hashes(cms, key);
    int32_t num_add = cms_add_inc_alt(cms, hashes, cms->depth, x);
    free(hashes);
    return num_add;
}

int32_t cms_remove_inc_alt(CountMinSketch *cms, uint64_t* hashes, int num_hashes, unsigned int x) {
    if (num_hashes < cms->depth) {
        fprintf(stderr, "Insufficient hashes to complete the removal of the element to the count-min sketch!");
        return CMS_ERROR;
    }
    int32_t i, num_add = INT_MAX;
    for (i = 0; i < cms->depth; i++) {
        uint32_t bin = (hashes[i] % cms->width) + (i * cms->width);
        cms->bins[bin] = __safe_sub(cms->bins[bin], x);
        if (cms->bins[bin] < num_add) {
            num_add = cms->bins[bin];
        }
    }
    cms->elements_added = __safe_sub(cms->elements_added, (int64_t)x);
    return num_add;
}

int32_t cms_remove_inc(CountMinSketch *cms, char* key, uint32_t x) {
    uint64_t* hashes = cms_get_hashes(cms, key);
    int32_t num_add = cms_remove_inc_alt(cms, hashes, cms->depth, x);
    free(hashes);
    return num_add;
}

int32_t cms_check_alt(CountMinSketch *cms, uint64_t* hashes, int num_hashes) {
    if (num_hashes < cms->depth) {
        fprintf(stderr, "Insufficient hashes to complete the min lookup of the element to the count-min sketch!");
        return CMS_ERROR;
    }
    int32_t i, num_add = INT_MAX;
    for (i = 0; i < cms->depth; i++) {
        uint32_t bin = (hashes[i] % cms->width) + (i * cms->width);
        if (cms->bins[bin] < num_add) {
            num_add = cms->bins[bin];
        }
    }
    return num_add;
}

int32_t cms_check(CountMinSketch *cms, char* key) {
    uint64_t* hashes = cms_get_hashes(cms, key);
    int32_t num_add = cms_check_alt(cms, hashes, cms->depth);
    free(hashes);
    return num_add;
}

int32_t cms_check_mean_alt(CountMinSketch *cms, uint64_t* hashes, int num_hashes) {
    if (num_hashes < cms->depth) {
        fprintf(stderr, "Insufficient hashes to complete the mean lookup of the element to the count-min sketch!");
        return CMS_ERROR;
    }
    int32_t i, num_add = 0;
    for (i = 0; i < cms->depth; i++) {
        uint32_t bin = (hashes[i] % cms->width) + (i * cms->width);
        num_add += cms->bins[bin];
    }
    return num_add / cms->depth;
}

int32_t cms_check_mean(CountMinSketch *cms, char* key) {
    uint64_t* hashes = cms_get_hashes(cms, key);
    int32_t num_add = cms_check_mean_alt(cms, hashes, cms->depth);
    free(hashes);
    return num_add;
}

int32_t cms_check_mean_min_alt(CountMinSketch *cms, uint64_t* hashes, int num_hashes) {
    if (num_hashes < cms->depth) {
        fprintf(stderr, "Insufficient hashes to complete the mean-min lookup of the element to the count-min sketch!");
        return CMS_ERROR;
    }
    int32_t i, num_add = 0;
    int64_t* mean_min_values = calloc(cms->depth, sizeof(long));
    for (i = 0; i < cms->depth; i++) {
        uint32_t bin = (hashes[i] % cms->width) + (i * cms->width);
        int32_t val = cms->bins[bin];
        mean_min_values[i] = val - ((cms->elements_added - val) / (cms->width - 1));
    }
    // return the median of the mean_min_value array... need to sort first
    qsort(mean_min_values, cms->depth, sizeof(long), __compare);
    int32_t n = cms->depth;
    if (n % 2 == 0) {
        num_add = (mean_min_values[n/2] + mean_min_values[n/2 - 1]) / 2;
    } else {
        num_add = mean_min_values[n/2];
    }
    free(mean_min_values);
    return num_add;
}

int32_t cms_check_mean_min(CountMinSketch *cms, char* key) {
    uint64_t* hashes = cms_get_hashes(cms, key);
    int32_t num_add = cms_check_mean_min_alt(cms, hashes, cms->depth);
    free(hashes);
    return num_add;
}

uint64_t* cms_get_hashes_alt(CountMinSketch *cms, int num_hashes, char* key) {
    return cms->hash_function(num_hashes, key);
}

int cms_export(CountMinSketch *cms, char* filepath) {
    FILE *fp;
    fp = fopen(filepath, "w+b");
    if (fp == NULL) {
        fprintf(stderr, "Can't open file %s!\n", filepath);
        return CMS_ERROR;
    }
    __write_to_file(cms, fp, 0);
    fclose(fp);
    return CMS_SUCCESS;
}

int cms_import_alt(CountMinSketch *cms, char* filepath, cms_hash_function hash_function) {
    FILE *fp;
    fp = fopen(filepath, "r+b");
    if (fp == NULL) {
        fprintf(stderr, "Can't open file %s!\n", filepath);
        return CMS_ERROR;
    }
    __read_from_file(cms, fp, 0, NULL);
    cms->hash_function = (hash_function == NULL) ? __default_hash : hash_function;
    fclose(fp);
    return CMS_SUCCESS;
}

/*******************************************************************************
*    PRIVATE FUNCTIONS
*******************************************************************************/
static int __setup_cms(CountMinSketch *cms, unsigned int width, unsigned int depth, double error_rate, double confidence, cms_hash_function hash_function) {
    cms->width = width;
    cms->depth = depth;
    cms->confidence = confidence;
    cms->error_rate = error_rate;
    cms->elements_added = 0;
    cms->bins = calloc(width * depth, sizeof(int32_t));
    cms->hash_function = (hash_function == NULL) ? __default_hash : hash_function;

    return CMS_SUCCESS;
}

static void __write_to_file(CountMinSketch *cms, FILE *fp, short on_disk) {
    unsigned long long i, length = cms->depth * cms->width;
    if (on_disk == 0) {
        for (i = 0; i < length; i++) {
            fwrite(&cms->bins[i], sizeof(int32_t), 1, fp);
        }
    } else {
        // TODO: decide if this should be done directly on disk or not
        // will need to write out everything by hand
        // uint64_t i;
        // int q = 0;
        // for (i = 0; i < length; i++) {
        //     fwrite(&q, sizeof(int), 1, fp);
        // }
    }
    fwrite(&cms->width, sizeof(int32_t), 1, fp);
    fwrite(&cms->depth, sizeof(int32_t), 1, fp);
    fwrite(&cms->elements_added, sizeof(int64_t), 1, fp);
}

static void __read_from_file(CountMinSketch *cms, FILE *fp, short on_disk, char *filename) {
    /* read in the values from the file before getting the sketch itself */
    int offset = (sizeof(int32_t) * 2) + sizeof(long);
    fseek(fp, offset * -1, SEEK_END);
    size_t read;
    read = fread(&cms->width, sizeof(int32_t), 1, fp);
    read = fread(&cms->depth, sizeof(int32_t), 1, fp);
    cms->confidence = 1 - (1 / pow(2, cms->depth));
    cms->error_rate = 2 / (double) cms->width;
    read = fread(&cms->elements_added, sizeof(int64_t), 1, fp);

    rewind(fp);
    long length = cms->width * cms->depth;
    if (on_disk == 0) {
        cms->bins = malloc(length * sizeof(int32_t));
        read = fread(cms->bins, sizeof(int32_t), length, fp);
        if (read != length) {
            perror("__read_from_file: ");
            exit(1);
        }
    } else {
        // TODO: decide if this should be done directly on disk or not
    }
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


int __compare (const void * a, const void * b) {
  return ( *(long*)a - *(long*)b );
}


int32_t __safe_add(int a, int b) {
    /* use the gcc macro if compiling with GCC, otherwise, simple overflow check */
    int32_t c = 0;
    #ifdef __GNUC__
        int bl = __builtin_add_overflow(a, b, &c);
        if (bl != 0) {
            c = INT_MAX;
        }
    #else
        if (b < INT_MIN + a) {
            c = INT_MIN;
        } else {
            c = a - b;
        }
    #endif

    return c;
}

int32_t __safe_sub(int32_t a, int32_t b) {
    /* use the gcc macro if compiling with GCC, otherwise, simple overflow check */
    int32_t c = 0;
    #ifdef __GNUC__
        int32_t bl = __builtin_sub_overflow(a, b, &c);
        if (bl != 0) {
            c = INT_MAX;
        }
    #else
        if (b > INT_MAX - a) {
            c = INT_MAX;
        } else {
            c = a + b;
        }
    #endif

    return c;
}
