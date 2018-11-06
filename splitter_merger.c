#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main (int argc, char const *argv[]) {
    printf("This is the Splitter/Merger program\n");
    int height = atoi(argv[0]);

    if (height == 1) {
        int pid = fork();
        if (pid == 0) {     //if child process
            execl("./searcher", (char*) NULL);
        }
        else {             //if parent process
            //wait for child to finish
            wait(NULL);
        }
        return 0;
    }

    height--;
    printf("%d\n", height);
    //fork two new splitter_mergers
    for (int i = 0; i < 2; i++) {
        int pid = fork();

        if (pid == 0) {     //if child process
            //make height from integer to string and pass it to splitter/merger
            //for convenience I assume height is maximum a 4 digit number
            char heightStr[4];
            sprintf(heightStr, "%d", height);
            execlp("./splitter_merger", heightStr, NULL);
        }
        else {             //if parent process
            //wait for child to finish
            wait(NULL);
        }
    }

    return 0;
}
