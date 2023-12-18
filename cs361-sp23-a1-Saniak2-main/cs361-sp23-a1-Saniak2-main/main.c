#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static void functions() {
    // TODO printf all global and weak functions in lib.o, one per line, in alphabetic order
    printf("fingerprint\n");
    printf("handle_error\n");
    printf("is_input_string_valid\n");
    printf("usage\n");
   // assert(0); // Not implemented
}

static void variables() {
    // TODO printf all global variables in lib.o, one per line, in alphabetic order
    printf("DEFAULT_MODE\n");
    printf("error\n");
    printf("EXTENDED_MODE\n");
    printf("mode\n");
    //assert(0); // Not implemented
}

// TODO define functions and variables you found in lib.o
extern int fingerprint(char * input, int len);
extern void handle_error();
extern int is_input_string_valid(char * input, int len);
extern void usage();

extern int DEFAULT_MODE;
extern int error;
extern int EXTENDED_MODE;
extern int* mode;

void handle_error() {
    error = 2;
}

int main(int argc, char * argv[]) {
    char * arg = argv[1];
    if (!strcmp("functions", argv[1])) {
        functions();
        return 0;
    } else if (!strcmp("variables", argv[1])) {
        variables();
        return 0;
    } else if (!strcmp("help", argv[1])) {
        // TODO call the library function in lib.o here that shows a help message
        usage();

    } else {
        int result = -1;

        if (is_input_string_valid(argv[1], strlen(argv[1])) == 1) {
            mode = &DEFAULT_MODE;
        } else {
            mode = &EXTENDED_MODE;
        }

       
        // TODO call the library function in lib.o here with argv[1] as the argument
        result = fingerprint(argv[1], strlen(argv[1]));

        if (error == 1) {
            printf("%s\n", argv[1]);
        } else if (error == 0) {
            printf("%d\n", result);
        } else if (error == 2) {
            printf("%s\n", argv[1]);
        }
        
        
       // assert(0); // Not implemented
    }

    return 0;
}
