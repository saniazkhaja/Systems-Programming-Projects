#include "a3.h"

#define _DEFAULT_SOURCE
#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main() {
    int m_size = 128;

    // Allocates something so that the  heap doesn't become empty
    // Empty heaps can be weird...
    malloc(1000);

    // Allocates one chunk and fills it with 'a'
    char * p1 = malloc(m_size);

    for (int i = 0 ; i < m_size ; i++)
        p1[i] = 'a';

    // Allocates one chunk and fills it with 'b'
    char * p2 = malloc(m_size);

    for (int i = 0 ; i < m_size ; i++)
        p2[i] = 'b';

    // Allocates one chunk and fills it with 'c'
    char * p3 = malloc(m_size);

    for (int i = 0 ; i < m_size ; i++)
        p3[i] = 'c';

    // Allocates one chunk and fills it with 'd'
    char * p4 = malloc(m_size);

    for (int i = 0 ; i < m_size ; i++)
        p4[i] = 'd';

    // Frees some memory
    free(p1);
    free(p3);

    {
        // Calls the API
        void * i3 = next_free_mem(p1);

        // Makes sure we got the right pointers, skipping over the alloc'ed memory in the middle
        assert(i3 != p1);
        assert(i3 != p2);
        assert(i3 == p3);
    }

    return 0;
}
