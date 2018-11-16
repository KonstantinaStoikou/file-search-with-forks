#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "record.h"
#include "statistic.h"
#include "spl_mer_functions.h"

#define READ 0
#define WRITE 1

int main (int argc, char const *argv[]) {
    struct timeval  begin, stop;
    gettimeofday(&begin, NULL);

    printf("This is the Splitter/Merger program %d with parent %d\n", getpid(), getppid());
    int fdw = atoi(argv[1]);
    int height = atoi(argv[2]);
    char *datafile = malloc(strlen(argv[3]) + 1);
    strcpy(datafile, argv[3]);
    char *pattern = malloc(strlen(argv[4]) + 1);
    strcpy(pattern, argv[4]);
    int skew = atoi(argv[5]);
    int position = atoi(argv[6]);
    int numOfrecords = atoi(argv[7]);
    int mod, start, end, sum;

    if (skew == 0) {
        mod = numOfrecords % 2;
        numOfrecords /= 2;
    } else {
        start = atoi(argv[8]);
        end = atoi(argv[9]);
        sum = atoi(argv[10]);
    }

    if (height == 1) {
        // fork two searchers
        for (int i = 1; i <= 2; i++) {
            int fd[2];
            if (pipe(fd) == -1) {
                perror("Error creating pipe");
                exit(1);
            }

            pid_t pid = fork();

            if (pid == 0) {     // if child process
                close(fd[READ]);
                // make integers to strings and pass them to splitter/merger
                // for convenience I assume numOfrecords is maximum a 10 digit number
                // and position is maximum a 100 digit number
                char positionStr[100];
                sprintf(positionStr, "%d", position);
                char fdwStr[10];
                sprintf(fdwStr, "%d", fd[WRITE]);
                char numOfrecordsStr[10];

                // break numOfRecords for the process to take depending on skew
                breakNumOfRecords(skew, numOfrecords, numOfrecordsStr, i, start, end, mod, sum);

                // arguments: fd write end, datafile, pattern, skew, position, numOfrecords
                execlp("./searcher", "searcher", fdwStr, datafile, pattern, argv[5], positionStr, numOfrecordsStr, NULL);
            }
            else if (pid == -1) {
                perror("fork");
                exit(1);
            }
            increaseSearcherPosition(skew, &position, numOfrecords, start, end, sum);

            close(fd[WRITE]);
            readFromChild(fd[READ], fdw);
        }
        waitChildren();

        gettimeofday(&stop, NULL);
        writeTimeToParent(fdw, begin, stop);

        exit(0);
    }

    height--;

    // fork two new splitter_mergers
    for (int i = 1; i <= 2; i++) {
        int newStart, newEnd;
        // open pipe for reading
        int fd[2];
        if (pipe(fd) == -1) {
            perror("Error creating pipe");
            exit(1);
        }

        pid_t pid = fork();

        if (pid == 0) {     // if child process
            close(fd[READ]);
            // make integers to strings and pass them to splitter/merger
            // for convenience I assume height is maximum a 4 digit number,
            // numOfrecords is maximum a 10 digit number
            // and position is maximum a 100 digit number
            char heightStr[4];
            sprintf(heightStr, "%d", height);
            char positionStr[100];
            sprintf(positionStr, "%d", position);
            char fdwStr[10];
            sprintf(fdwStr, "%d", fd[WRITE]);

            char numOfrecordsStr[10];
            if (skew == 0) {
                // if last forked splitter_merger add to numOfrecords the remainder of the division
                if (i == 2) {
                    sprintf(numOfrecordsStr, "%d", numOfrecords + mod);
                } else {
                    sprintf(numOfrecordsStr, "%d", numOfrecords);
                }
                // arguments: fd write end, height, datafile, pattern, skew, position, numOfrecords
                execlp("./splitter_merger", "splitter_merger", fdwStr, heightStr, argv[3], pattern, argv[5], positionStr, numOfrecordsStr, NULL);
            }
            else {
                calculateNewRange(i, &newStart, &newEnd, start, end);

                char newStartStr[3];
                sprintf(newStartStr, "%d", newStart);
                char newEndStr[3];
                sprintf(newEndStr, "%d", newEnd);
                sprintf(numOfrecordsStr, "%d", numOfrecords);
                // arguments: fd write end, height, datafile, pattern, skew, position, numOfrecords, start, end, sum
                execlp("./splitter_merger", "splitter_merger", fdwStr, heightStr, argv[3], pattern, argv[5], positionStr, numOfrecordsStr, newStartStr, newEndStr, argv[10], NULL);
            }
        }
        else if (pid == -1) {
            perror("fork");
            exit(1);
        }
        increaseSplitterMergerPosition(skew, &position, numOfrecords,start, end, sum);

        close(fd[1]);
        readFromChild(fd[READ], fdw);
    }
    waitChildren();

    gettimeofday(&stop, NULL);
    writeTimeToParent(fdw, begin, stop);

    exit(0);
}
