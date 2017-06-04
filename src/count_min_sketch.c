/*******************************************************************************
***     Author: Tyler Barrus
***     email:  barrust@gmail.com
***     Version: 0.1.3
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
static int __setup_cms(CountMinSketch *cms, unsigned int width, unsigned int depth, double error_rate, double confidence, cms_hash_function hash_function);
static void __write_to_file(CountMinSketch *cms, FILE *fp, short on_disk);
static void __read_from_file(CountMinSketch *cms, FILE *fp, short on_disk, char *filename);
static uint64_t* __default_hash(int num_hashes, char *key);
static uint64_t __fnv_1a(char *key);



int cms_init_optimal_alt(CountMinSketch *cms, double error_rate, double confidence, cms_hash_function hash_function) {
    /* https://cs.stackexchange.com/q/44803 */
    unsigned int width = ceil(2 / error_rate);
    unsigned int depth = ceil((-1 * log(1 - confidence)) / LOG_TWO);
    return __setup_cms(cms, width, depth, error_rate, confidence, hash_function);
}

int cms_init_alt(CountMinSketch *cms, unsigned int width, unsigned int depth, cms_hash_function hash_function) {
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

    return CMS_SUCCESS;
}

int cms_clear(CountMinSketch *cms) {
    int i, j = cms->width * cms->depth;
    for (i = 0; i < j; i++) {
        cms->bins[i] = 0;
    }
    cms->elements_added = 0;
    return CMS_SUCCESS;
}

int cms_add_alt(CountMinSketch *cms, uint64_t* hashes, int num_hashes) {
    if (num_hashes < cms->depth) {
        fprintf(stderr, "Inssufecient hashes to complete the addition of the element to the count-min sketch!");
        return CMS_ERROR;
    }
    int i, num_add = INT_MAX;
    for (i = 0; i < cms->depth; i++) {
        int bin = (hashes[i] % cms->width) + (i * cms->width);
        if (cms->bins[bin] != INT_MAX) {
            cms->bins[bin]++;
        }
        /* currently a standard min strategy */
        if (cms->bins[bin] < num_add) {
            num_add = cms->bins[bin];
        }
    }
    cms->elements_added++;
    return num_add;
}

int cms_add(CountMinSketch *cms, char* key) {
    uint64_t* hashes = cms_get_hashes(cms, key);
    int num_add = cms_add_alt(cms, hashes, cms->depth);
    free(hashes);
    return num_add;
}

int cms_remove_alt(CountMinSketch *cms, uint64_t* hashes, int num_hashes) {
    if (num_hashes < cms->depth) {
        fprintf(stderr, "Inssufecient hashes to complete the removal of the element to the count-min sketch!");
        return CMS_ERROR;
    }
    int i, num_add = INT_MAX;
    for (i = 0; i < cms->depth; i++) {
        int bin = (hashes[i] % cms->width) + (i * cms->width);
        if (cms->bins[bin] != INT_MIN) {
            cms->bins[bin]--;
        }
        if (cms->bins[bin] < num_add) {
            num_add = cms->bins[bin];
        }
    }
    cms->elements_added--;
    return num_add;
}

int cms_remove(CountMinSketch *cms, char* key) {
    uint64_t* hashes = cms_get_hashes(cms, key);
    int num_add = cms_remove_alt(cms, hashes, cms->depth);
    free(hashes);
    return num_add;
}

int cms_check_alt(CountMinSketch *cms, uint64_t* hashes, int num_hashes) {
    if (num_hashes < cms->depth) {
        fprintf(stderr, "Inssufecient hashes to complete the min lookup of the element to the count-min sketch!");
        return CMS_ERROR;
    }
    int i, num_add = INT_MAX;
    for (i = 0; i < cms->depth; i++) {
        int bin = (hashes[i] % cms->width) + (i * cms->width);
        if (cms->bins[bin] < num_add) {
            num_add = cms->bins[bin];
        }
    }
    return num_add;
}

int cms_check(CountMinSketch *cms, char* key) {
    uint64_t* hashes = cms_get_hashes(cms, key);
    int num_add = cms_check_alt(cms, hashes, cms->depth);
    free(hashes);
    return num_add;
}

int cms_check_mean_alt(CountMinSketch *cms, uint64_t* hashes, int num_hashes) {
    if (num_hashes < cms->depth) {
        fprintf(stderr, "Inssufecient hashes to complete the mean lookup of the element to the count-min sketch!");
        return CMS_ERROR;
    }
    int i, num_add = 0;
    for (i = 0; i < cms->depth; i++) {
        int bin = (hashes[i] % cms->width) + (i * cms->width);
        num_add += cms->bins[bin];
    }
    return num_add / cms->depth;
}

int cms_check_mean(CountMinSketch *cms, char* key) {
    uint64_t* hashes = cms_get_hashes(cms, key);
    int num_add = cms_check_mean_alt(cms, hashes, cms->depth);
    free(hashes);
    return num_add;
}

int cms_check_mean_min_alt(CountMinSketch *cms, uint64_t* hashes, int num_hashes) {
    if (num_hashes < cms->depth) {
        fprintf(stderr, "Inssufecient hashes to complete the mean-min lookup of the element to the count-min sketch!");
        return CMS_ERROR;
    }
    int i, num_add = INT_MAX;
    for (i = 0; i < cms->depth; i++) {
        int bin = (hashes[i] % cms->width) + (i * cms->width);
        int val = cms->bins[bin];
        int mean_min = val - ((val - cms->bins[bin]) / (cms->width - 1));
        if (mean_min < num_add) {
            num_add = mean_min;
        }
    }
    return num_add;
}

int cms_check_mean_min(CountMinSketch *cms, char* key) {
    uint64_t* hashes = cms_get_hashes(cms, key);
    int num_add = cms_check_mean_min_alt(cms, hashes, cms->depth);
    free(hashes);
    return num_add;
}

int cms_check_mean(CountMinSketch *cms, char* key) {
    uint64_t* hashes = cms_get_hashes(cms, key);
    int num_add = cms_check_mean_alt(cms, hashes, cms->depth);
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
    cms->bins = calloc(width * depth, sizeof(int));
    cms->hash_function = (hash_function == NULL) ? __default_hash : hash_function;

    return CMS_SUCCESS;
}

static void __write_to_file(CountMinSketch *cms, FILE *fp, short on_disk) {
    unsigned long long i, length = cms->depth * cms->width;
    if (on_disk == 0) {
        for (i = 0; i < length; i++) {
            fwrite(&cms->bins[i], sizeof(int), 1, fp);
        }
    } else {
        // TODO: decide if this should be done directly on disk or not
        // will need to write out everything by hand
        uint64_t i;
        int q = 0;
        for (i = 0; i < length; i++) {
            fwrite(&q, sizeof(int), 1, fp);
        }
    }
    fwrite(&cms->width, sizeof(int), 1, fp);
    fwrite(&cms->depth, sizeof(int), 1, fp);
    fwrite(&cms->confidence, sizeof(double), 1, fp);
    fwrite(&cms->error_rate, sizeof(double), 1, fp);
    fwrite(&cms->elements_added, sizeof(long), 1, fp);
}

static void __read_from_file(CountMinSketch *cms, FILE *fp, short on_disk, char *filename) {
    /* read in the values from the file before getting the sketch itself */
    int offset = (sizeof(int) * 2) + (sizeof(double) * 2) + sizeof(long);
    fseek(fp, offset * -1, SEEK_END);
    size_t read;
    read = fread(&cms->width, sizeof(int), 1, fp);
    read = fread(&cms->depth, sizeof(int), 1, fp);
    read = fread(&cms->confidence, sizeof(double), 1, fp);
    read = fread(&cms->error_rate, sizeof(double), 1, fp);
    read = fread(&cms->elements_added, sizeof(long), 1, fp);

    rewind(fp);
    long length = cms->width * cms->depth;
    if (on_disk == 0) {
        cms->bins = malloc(length * sizeof(int));
        read = fread(cms->bins, sizeof(int), length, fp);
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
