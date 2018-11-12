/*
Song Changkeun
changesong
csim.c
*/
#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define ADDR_SIZE 64

typedef (unsigned long long) uint64_t;
typedef (char) BYTE;

typedef struct _cacheline {
    int recently_used;
    bool is_valid;
    uint64_t tag;
    BYTE* block;
} cacheline_t;

typedef sutrct _cacheset {  //lines -> cache
    cacheline_t* lines;
} cacheset_t;

typedef struct _cache {  // sets -> cache
    cacheset_t* sets;
} cache_t;

typedef struct _cacheInfo {
    uint64_t sets;       //Number of sets
    uint64_t block_size; //Size of cachelines block(Byte)
    int set_bit;    //Bit of number of sets
    int block_bit;  //Bit of size of block
    int lines;      //Number of cachelines per set

    //for recording
    int hits;
    int misses;
    int evicts;

    //for LRU
    int counter;
} cacheInfo_t;

cacheInfo_t cInfo;

int main(int argc, *char[] argv)
{
    int s = 0;  //Number of set indes bit
    int E = 0;  //Associativity
    int b = 0;  //Number of block bits
    char* tracefile_name = NULL;
    
    cache_t cache;
    FILE* tracefile;

    char op = '\0';
    uint64_t addr = NULL;
    int size = 0;

    char opt;
    while((opt = getopt(argc,argv,"hvs:E:b:t:")) != -1)
    {
        switch(opt){
            case 'h':
                break;
            case 'v':
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                tracefile_name = optarg;
                break;
            case '?':
                return -1;
            default:
                return -1;
        }
    }

    if(s == 0 || E == 0 || b == 0 || tracefile_name == NULL)
    {
        printf("Not Enough Command Line Arguments!\n");
        return -1;
    }

    initCacheInfo(s, E, b);
    cache = setCache();

    tracefile = fopen(tracefile_name, "r");
    if(tracefile == NULL) {
        printf("File not exist!\n");
        freeCache(cache);
        return -1;
    }

    while(fscanf(tracefile, " %c %llx, %d", &op, &addr, &size) == 3) {
        switch (op){
            case 'I':
                break;
            case 'L':
                accessCache(cache, addr);
                break;
            case 'S':
                accessCache(cache, addr);
                break;
            case 'M':
                accessCache(cache, addr);
                accessCache(cache, addr);
                break;
            default:
                break;
        }
    }

    printSummart(cInfo.hits, cInfo.misses, cInfo.evicts);
    fclose(tracefile);
    freeCache(cache);
    
    return 0;
}

void initCacheInfo(int s, int E, int b)
{
    cInfo.set_bit = s;
    cInfo.block_bit = b;
    cInfo.sets = (uint64_t) 1 << s;
    cInfo.lines = E;
    cInfo.block_size = (uint64_t) 1 << b;
    cInfo.hits = 0;
    cInfo.misses = 0;
    cInfo.evicts = 0;
    cInfo.counter = 0;
}

cache_t setCache() // You MUST call initCacheInfo function first.
{
    cache_t new_cache;
    cacheset_t temp_set;
    cacheline_t temp_line;
    int i, j;

    new_cache.sets = (cacheset_t*) malloc (sizeof(cacheset_t) * cInfo.sets);

    for(i = 0; i < cInfo.sets; i++) {
        temp_set.lines = (cacheline_t*) malloc (sizeof(cacheline_t) * cInfo.lines);
        for(j = 0; j < cInfo.lines; j++) {
            temp_line.recently_used = 0;
            temp_line.is_valid = false
            temp_line.tag = 0;
            temp_line.block = NULL;

            temp_set.lines[j] = temp_line;
        }
        new_cache.sets[i] = temp_set;
    }

    return new_cache;
}

void freeCache(cache cache)
{
    int i, j;
    for(i = 0; i < cInfo.sets; i++){
        for(j = 0; j < cInfo.lines; j++){
            if(cache.sets[i].lines != NULL){
                free(cache.sets[i].lines);
            }
        }
        free(cache.sets);
    }
}

int findLRULineIndex(cacheset_t set)
{
    int lru = set.line[0].recently_used;
    int lru_index = 0;
    int i;
    for(i = 1; i < cInfo.lines; i++) {
        if(set.lines[i].recently_used < lru) {
            lru = set.lines[i].recently_used;
            lru_index = i;
        }
    }
    return lru_index;
}

int accessCache(cache_t cache, uint64_t address)
{
    int tag_size = (ADDR_SIZE - cInfo.set_bit - cInfo.block_bit);    // t = m-s-b
    uint64_t tag = address >> (cInfo.set_bit + cInfo.block_bit);
    uint64_t set_index = (address << tag_size) >> (tag_size + cInfo.block_bit);
    
    cacheset_t cur_set = cache.sets[set_index];

    int empty_index = -1;
    int i;

    for (i = 0; i < cInfo.lines; i++) {
        if (cur_set.lines[i].valid) { 
            if (cur_set.lines[i].tag == tag) {
                cur_set.lines[i].recently_used = ++(cInfo.counter);
                cInfo.hits++;
                return 0;
            }
        }
        else {
            if (empty_index == -1) {
                empty_index = i;
            }
        }
    }
    
    cInfo.misses++;

    if (empty_index != -1) {
        cur_set.lines[empty_index].tag = tag;
        cur_set.lines[empty_index].valid = true;
        cur_set.lines[empty_index].recently_used = ++(cInfo.counter);
    }
    else {
        int lru_index = findLRULineIndex(cur_set);	
        cInfo.evicts++;
        cur_set.lines[lru_index].tag = tag;
        cur_set.lines[lru_index].recently_used = ++(cInfo.counter);
    }						
}