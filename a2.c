#define  _POSIX_C_SOURCE 200809L
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<assert.h>

#define exit(N) {fflush(stdout); fflush(stderr); _exit(N); }

void command(char* c) {
    char * sep = NULL;

    if((sep=strstr(c,"2>"))) {
        *sep=0;
        char * fileout = sep+2;

        if (fileout[strlen(fileout) - 1] == ' ') {
            fileout[strlen(fileout) - 1]='\0';
        }
        if (fileout[0] == ' ') {
            fileout = fileout + 1;
        }
        
        // Use the debugger to see the contents of c and fileout
        // assert(0); // Delete this assertion and implement stderr redirection below
        int write_fd = open(fileout, O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR | S_IRUSR);
        int res = dup2(write_fd, 2);
        close(write_fd);
    }

    if((sep=strstr(c,">"))) {
        *sep=0;
        char * fileout = sep+1;

        if (fileout[strlen(fileout) - 1] == ' ') {
            fileout[strlen(fileout)-1]='\0';
        }
        if (fileout[0] == ' ') {
            fileout = fileout + 1;
        }

        // Use the debugger to see the contents of c and fileout
        // assert(0); // Delete this assertion and implement stdout redirection below
        int write_fd = open(fileout, O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR | S_IRUSR);
        int res = dup2(write_fd, 1);
        close(write_fd);
    }

    if((sep=strstr(c,"<"))) {
        *sep=0;
        char * filein = sep+1;

        if (filein[strlen(filein) - 1] == ' ') {
            filein[strlen(filein)-1]='\0';
        }
        if (filein[0] == ' ') {
            filein = filein + 1;
        }
    //     // Use the debugger to see the contents of c and filein
    //  //   assert(0); // Delete this assertion and implement stdin redirection below
        int read_fd = open(filein, O_RDONLY | O_CREAT, S_IRUSR);
        int res = dup2(read_fd, 0);
        close(read_fd);
    }

    char* program = strtok(c," ");
    char *args[16]={program};

    // args[1] = strtok(NULL," ");
    
    if (args[0] != NULL) {
        // TODO: fill args with more arguments
        // Maybe it's a good idea to read on how function strtok works
        for (long unsigned int i = 1; i < 16; i++) {
            args[i] = strtok(NULL," "); 
        }

        if (!strcmp("echo",program)) {
            // TODO: implement built-in echo
            for (long unsigned int i = 1; i < 16; i++) {
                if (args[i] != NULL) {
                    char * tok;
                    tok = strtok(args[i], " ");

                    while (tok != NULL) {
                        printf("%s", tok);
                        tok = strtok(NULL, " ");
                    }
                    printf("\n");
                }
            }
            fflush(stdout);
        } else {
            // TODO: search the path instead of running "program directly"
            char* path = getenv("PATH");
                    
            if (execv(program, args) != 0) {
                char buffer[100];
                char * tok;
                tok = strtok(path, ":");

                while (tok != NULL) {
                    tok = strtok(NULL, ":");
                    if (tok == NULL) {
                        snprintf(buffer, 100, "%s/%s", path, program); 
                        execv(buffer, args);
                    }
                    snprintf(buffer, 100, "%s/%s", tok, program); 
                    execv(buffer, args);
                }
            }
            fprintf(stderr,"cs361sh: command not found: %s\n",program);
            fflush(stderr);
        }
    } 
}

void run(char*);

void pipeline(char* head, char* tail) {
    // fprintf(stderr,"Uh-oh, I don't know how to do pipes.");
    int fds[2]; 
    pipe(fds); 
    int read_fd = fds[0]; 
    int write_fd = fds[1];

    char *line = "";

    int ret = -1;
    int process_id = fork();

    if (process_id) {  // parent
        waitpid(process_id, &ret, 0);
        close(write_fd);
        dup2(read_fd, 0);
        run(tail);
    } else {  // child
        close(read_fd);
        dup2(write_fd, 1);
        run(head);
        exit(ret);
    }
    
    fflush(stderr);
    // assert(0);
}

void sequence(char* head, char* tail) {
    // fprintf(stderr,"Uh-oh, I don't know how to do sequences.");
    int ret = -1;
    int process_id = fork();
    
    if (process_id) {  // parent
        waitpid(process_id, &ret, 0);
    } else {  // child
        run(head);
        run(tail);
        exit(ret);
    }
    fflush(stderr);
    // assert(0);
}

void run(char *line) {
    char *sep = 0;

    // TODO find the separator character

    if((sep=strstr(line,";"))) {
        *sep=0;
        sequence(line,sep+1);
    }
    else if((sep=strstr(line,"|"))) {
        *sep=0;
        pipeline(line,sep+1);
    }
    else {
        // TODO fork to execute command
        int ret = -1;
        int process_id = fork();
        
        if (process_id) {  // parent
            waitpid(process_id, &ret, 0);
        } else {  // child
            command(line);
            exit(ret);
        }
    }
}

int main(int argc, char** argv) {
    char *line=0;
    size_t size=0;

    printf("cs361sh> ");
    fflush(stdout);

    // handy copies of original file descriptors
    int orig_in=dup(0);
    int orig_out=dup(1);
    int orig_err=dup(2);

    while(getline(&line,&size,stdin) > 0) {

        line[strlen(line)-1]='\0'; // remove the newline
        run(line);

        printf("cs361sh> ");
        fflush(stdout);
   }

    printf("\n");

    fflush(stdout);
    fflush(stderr);
    return 0;
}
