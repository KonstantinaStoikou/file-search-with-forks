#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>
#include <math.h>
#include "root_functions.h"
#include "record.h"

#define READ 0
#define WRITE 1

int main(int argc, char const *argv[]) {
    int height = 0;
    char *datafile;
    char *pattern;
    int skew = 0;       // flag for skew searching (initialized to false)

    // read command line arguments and initialize variables respectively
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            height = atoi(argv[i+1]);
            // check if valid height
            if (height < 1) {
                printf("Height should be at least 1\n");
                return 1;
            }
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


    printf("height: %d, datafile: %s, pattern: %s, skew: %d\n", height, datafile, pattern, skew);
    printf("This is root %d\n", getpid());

    // get number of records of binary file
    int numOfrecords = findNumOfRecords(datafile);

    // open pipe for reading
    int fd[2];
    if (pipe(fd) == -1) {
        perror("Error creating pipe");
        exit(1);
    }

    // fork splitter/merger processes
    pid_t pid = fork();

    if (pid == 0) {     // if child process
        close(fd[READ]);
        // make integers to strings and pass them to splitter/merger
        // for convenience I assume height is maximum a 4 digit number
        char heightStr[4];
        sprintf(heightStr, "%d", height);
        char skewStr[4];
        sprintf(skewStr, "%d", skew);
        char numOfrecordsStr[4];
        sprintf(numOfrecordsStr, "%d", numOfrecords);
        char fdwStr[10];
        sprintf(fdwStr, "%d", fd[WRITE]);
        // position where each searcher will start to read the file
        // position will change each time a splitter_merger is created
        char position[] = "0";

        if (skew == 0) {
            execlp("./splitter_merger", "splitter_merger", fdwStr, heightStr, datafile, pattern, skewStr, position, numOfrecordsStr, NULL);
        } else {
            // if skew == 1 pass extra parameters that show range of searchers each splitter has
            // and sum of numbers till 2^h
            char start[] = "1";
            char end[3];
            sprintf(end, "%d", (int)pow(2, height));
            // find sum from 1 to 2^h
            int sum = 0;
            for (int i = 1; i <= pow(2, height); i++) {
                sum += i;
            }
            char sumStr[4];
            sprintf(sumStr, "%d", sum);
            execlp("./splitter_merger", "splitter_merger", fdwStr, heightStr, datafile, pattern, skewStr, position, numOfrecordsStr, start, end, sumStr, NULL);
        }
    }
    else if (pid == -1) {
        perror("Failed to execute fork");
        exit(1);
    }
    else {             // if parent process
        // wait for child to finish
        wait(NULL);
        close(fd[WRITE]);
        char readbuffer[150];
        // read from pipe (where splitter/merger wrote) untill there is nothing more to read
        while (read(fd[READ], readbuffer, sizeof(readbuffer)) > 0) {
            printf("Received string in root: %s", readbuffer);
        }
    }

    return 0;
}
