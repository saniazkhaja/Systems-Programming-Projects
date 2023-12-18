#ifndef _A3_H_
#define _A3_H_

long distance_to_end_of_heap(void * ptr);

long chunk_size(void * ptr);

int is_chunk_free(void * ptr);

void * next_used_mem(void * ptr);

void * next_free_mem(void * ptr);

void free_everything(void * start, void * end, int size, long * stats);

#endif
