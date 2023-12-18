#include "a3.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main() {
    // Allocates one chunk
    void * ptr1 = malloc(4);

    // How far is it from the end of the heap?
    long d1 = distance_to_end_of_heap(ptr1);

    // Print some stuff to help debug
    printf("%lu\n", d1);

    // The distance should not be zero
    assert(d1 != 0);

    // The distance should be positive
    assert(d1 > 0);

    // Free used memory
    free(ptr1);
    return 0;
}
