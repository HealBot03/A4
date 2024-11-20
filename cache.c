#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "cache.h"
#include "jbod.h"

static cache_entry_t *cache = NULL;
static int cache_size = 0;
static int clock = 0;
static int num_queries = 0;
static int num_hits = 0;

/* Helper function to find a cache entry by disk and block number */
static int find_cache_entry(int disk_num, int block_num) {
    for (int i = 0; i < cache_size; i++) {
        if (cache[i].valid && cache[i].disk_num == disk_num && cache[i].block_num == block_num) {
            return i;
        }
    }
    return -1;
}

/* Helper function to find the MRU cache entry index */
static int find_mru_entry(void) {
    int mru_index = -1;
    int max_clock = -1;
    for (int i = 0; i < cache_size; i++) {
        if (cache[i].valid && cache[i].clock_accesses > max_clock) {
            max_clock = cache[i].clock_accesses;
            mru_index = i;
        }
    }
    return mru_index;
}

/* Helper function to find an invalid cache entry */
static int find_invalid_entry(void) {
    for (int i = 0; i < cache_size; i++) {
        if (!cache[i].valid) {
            return i;
        }
    }
    return -1;
}

int cache_create(int num_entries) {
    if (cache != NULL || num_entries < 2 || num_entries > 4096) {
        return -1;
    }
    cache = (cache_entry_t *)malloc(sizeof(cache_entry_t) * num_entries);
    if (cache == NULL) {
        return -1;
    }
    cache_size = num_entries;
    clock = 0;
    for (int i = 0; i < cache_size; i++) {
        cache[i].valid = false;
    }
    return 1;
}

int cache_destroy(void) {
    if (cache == NULL) {
        return -1;
    }
    free(cache);
    cache = NULL;
    cache_size = 0;
    clock = 0;
    num_queries = 0;
    num_hits = 0;
    return 1;
}

int cache_lookup(int disk_num, int block_num, uint8_t *buf) {
    if (!cache_enabled() || buf == NULL || disk_num < 0 || block_num < 0) {
        return -1;
    }
    num_queries++;
    int index = find_cache_entry(disk_num, block_num);
    if (index != -1) {
        memcpy(buf, cache[index].block, JBOD_BLOCK_SIZE);
        num_hits++;
        clock++;
        cache[index].clock_accesses = clock;
        return 1;
    }
    return -1;
}

void cache_update(int disk_num, int block_num, const uint8_t *buf) {
    if (!cache_enabled() || buf == NULL || disk_num < 0 || block_num < 0) {
        return;
    }
    int index = find_cache_entry(disk_num, block_num);
    if (index != -1) {
        memcpy(cache[index].block, buf, JBOD_BLOCK_SIZE);
        clock++;
        cache[index].clock_accesses = clock;
    }
}

int cache_insert(int disk_num, int block_num, const uint8_t *buf) {
    if (!cache_enabled() || buf == NULL || disk_num < 0 || block_num < 0) {
        return -1;
    }
    // Check for existing entry
    if (find_cache_entry(disk_num, block_num) != -1) {
        return -1;
    }
    // Find an invalid entry
    int index = find_invalid_entry();
    if (index == -1) {
        // Cache is full, replace MRU entry
        index = find_mru_entry();
        if (index == -1) {
            // Should not happen, but handle just in case
            return -1;
        }
    }
    // Insert the new entry
    cache[index].valid = true;
    cache[index].disk_num = disk_num;
    cache[index].block_num = block_num;
    memcpy(cache[index].block, buf, JBOD_BLOCK_SIZE);
    clock++;
    cache[index].clock_accesses = clock;
    return 1;
}

bool cache_enabled(void) {
    return (cache != NULL && cache_size > 0);
}

void cache_print_hit_rate(void) {
    fprintf(stderr, "num_hits: %d, num_queries: %d\n", num_hits, num_queries);
    if (num_queries > 0) {
        fprintf(stderr, "Hit rate: %5.1f%%\n", 100 * (float)num_hits / num_queries);
    } else {
        fprintf(stderr, "Hit rate: N/A\n");
    }
}

int cache_resize(int new_size) {
    if (new_size < 2 || new_size > 4096) {
        return -1;
    }
    if (!cache_enabled()) {
        return cache_create(new_size);
    }
    // Allocate new cache
    cache_entry_t *new_cache = (cache_entry_t *)malloc(sizeof(cache_entry_t) * new_size);
    if (new_cache == NULL) {
        return -1;
    }
    // Initialize new cache entries
    for (int i = 0; i < new_size; i++) {
        new_cache[i].valid = false;
    }
    // Copy existing entries to new cache
    int entries_to_copy = (new_size < cache_size) ? new_size : cache_size;
    for (int i = 0; i < entries_to_copy; i++) {
        new_cache[i] = cache[i];
    }
    // Free old cache and update pointers
    free(cache);
    cache = new_cache;
    cache_size = new_size;
    return 1;
}
