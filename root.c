#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char const *argv[]) {
    int height = 0;
    char *datafile;
    char *pattern;
    int skew = 0;       // flag for skew searching (initialized to false)

    // read command line arguments and initialize variables respectively
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            height = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "-d") == 0) {
            datafile = malloc(sizeof(strlen(argv[i+1]) + 1));
            strcpy(datafile, argv[i+1]);
        }
        else if (strcmp(argv[i], "-p") == 0) {
            pattern = malloc(sizeof(strlen(argv[i+1]) + 1));
            strcpy(pattern, argv[i+1]);
        }
        else if (strcmp(argv[i], "-s") == 0) {
            skew = 1;
        }
    }
    // check if valid height
    if (height < 1) {
        printf("Height should be at least 1\n");
        return 1;
    }

    printf("height: %d, datafile: %s, pattern: %s, skew: %d\n", height, datafile, pattern, skew);
    printf("This is root %d\n", getpid());
    // fork splitter/merger processes
    pid_t pid = fork();

    if (pid == 0) {     // if child process
        // make height and skew from integer to string and pass it to splitter/merger
        // for convenience I assume height is maximum a 4 digit number
        char heightStr[4];
        sprintf(heightStr, "%d", height);
        char skewStr[4];
        sprintf(skewStr, "%d", skew);
        execlp("./splitter_merger", heightStr, datafile, skewStr, NULL);
    }
    else if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else {             // if parent process
        // wait for child to finish
        wait(NULL);
    }

    return 0;
}
