#define  _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include<stdint.h>

#define exit(N) {fflush(stdout); fflush(stderr); _exit(N); }

long distance_to_end_of_heap(void * ptr) {
    void * end_of_heap_1 = sbrk(0);
    void * end_of_heap_2 = ptr;

    return (long) end_of_heap_1 - (long) end_of_heap_2;
}

long chunk_size(void * ptr) {
    void * ptr_with_header = (void*) (ptr - 8);
    long * ptr_header = (long*) ptr_with_header;
    
    return (*ptr_header & ~1);
}

int is_chunk_free(void * ptr) {
    void * next_header = (void*) (ptr + chunk_size(ptr) - 8);
    long * next_chunk = (long*) next_header;
    long chunk = *next_chunk;

    if (*next_chunk & 1){ 
        return 0; 
    } else {
        return 1;
    }
}

void * next_used_mem(void * ptr) {
    long moveAmount = chunk_size(ptr);
    void * currPtr = ptr + moveAmount;

    while (is_chunk_free(currPtr) != 0) {
        moveAmount = chunk_size(currPtr);
        currPtr += moveAmount; 
    }
    return currPtr;
}

void * next_free_mem(void * ptr) {
    long moveAmount = chunk_size(ptr);
    void * currPtr = ptr + moveAmount;

    while (is_chunk_free(currPtr) != 1) {
        moveAmount = chunk_size(currPtr);
        currPtr += moveAmount; 
    }
    return currPtr;
}

void free_everything(void * start, void * end, int size, long * stats) {
    void * currPtr = start;
    long countFreedChunks = 0;
    long amountFreed = 0;

    while (currPtr != end) {
        currPtr = next_used_mem(currPtr);
        if (is_chunk_free(currPtr) != 1 && currPtr != end && chunk_size(currPtr) > size) {
            countFreedChunks++;
            amountFreed += chunk_size(currPtr);
            free(currPtr);
        }
    }
    if (stats != NULL) {
        *stats = countFreedChunks;
        stats++;
        *stats = amountFreed;
    }
}
