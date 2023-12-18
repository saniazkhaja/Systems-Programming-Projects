#include "a3.h"

#define _DEFAULT_SOURCE
#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

int main() {
    int m_size = 128;

    srandom(time(NULL));

    // Allocates something so that the  heap doesn't become empty
    // Empty heaps can be weird...
    malloc(1000);

    void * data[500];
    int status[500];

    for (int i = 0 ; i < (sizeof(data)/sizeof(*data)) ; i++) {
        // Get a size at random
        int n = 128 + ((int) (random() % 1000));

        // Allocates memory with random size and saves it
        data[i] = malloc(n);

        // Set the status as alloc'ed
        status[i] = 0;
    }

    // Frees half data at random
    for (int i = 0 ; i < ((sizeof(data)/sizeof(*data)) >> 1) ; i++) {
        // Get a position at random
        int n = (int) (random() % (1 + ((sizeof(data)/sizeof(*data)) - 2)));

        // Make sure it is not free already
        if (status[n] == 1)
            continue;

        // Frees that data
        free(data[n]);

        // Set the status as free'ed
        status[n] = 1;
    }

    // Check data at random
    for (int i = 0 ; i < 2500000 ; i++) {
        int n;
        do {
            // Get an alloc'ed position at random
            n = (int) (random() % (1 + ((sizeof(data)/sizeof(*data)) - 2)));
        } while (status[n]);

        void * ptr = data[n];

        // Find next free memory
        void * next_free = NULL;
        int n_free = 0;
        for (int j = n+1 ; j < ((sizeof(data)/sizeof(*data)) - 2) ; j++) {
            if (status[j]) {
                n_free = j;
                next_free = data[j];
                break;
            }
        }

        // Find next used memory
        void * next_used = NULL;
        int n_used = 0;
        for (int j = n+1 ; j < ((sizeof(data)/sizeof(*data)) - 2) ; j++) {
            if (!status[j]) {
                n_used = j;
                next_used = data[j];
                break;
            }
        }

        if (next_free != NULL) {
            // Asks the API what is the next free memory
            void * actual = next_free_mem(ptr);

            if (actual != next_free) {
                printf("%d %d %d %d\n", i, n, n_free, n_used);
            }

            // Should be the same
            assert(actual == next_free);
        }

        if (next_used != NULL) {
            // Asks the API what is the next used memory
            void * actual = next_used_mem(ptr);

            if (actual != next_used) {
                printf("%d %d %d %d\n", i, n, n_free, n_used);
            }

            // Should be the same
            assert(actual == next_used);
        }


    }

    return 0;
}
