#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <sys/time.h>
#include "root_functions.h"

#define READ 0
#define WRITE 1

int sigCount = 0;

void handler() {
    signal(SIGUSR2,handler);
    sigCount++;
}


int main(int argc, char const *argv[]) {
    struct timeval  begin, stop;
    gettimeofday(&begin, NULL);

    int height = 0;
    char *datafile;
    char *pattern;
    int skew = 0;       // flag for skew searching (initialized to false)

    // variables for running time of processes (for min set a very high number)
    int count = 0;
    double minSearcher = 100000;
    double maxSearcher = 0;
    double averageSearcher;
    int searcherCounter = 0;
    double minSplMerg = 100000;
    double maxSlpMerg = 0;
    double averageSplMerg;
    int splMergCounter = 0;

    // initialize variables with command line arguments
    readArguments(argc, argv, &height, &datafile, &pattern, &skew);

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
        char heightStr[4];
        sprintf(heightStr, "%d", height);
        char skewStr[4];
        sprintf(skewStr, "%d", skew);
        char numOfrecordsStr[10];
        sprintf(numOfrecordsStr, "%d", numOfrecords);
        char fdwStr[10];
        sprintf(fdwStr, "%d", fd[WRITE]);
        char pidStr[10];
        sprintf(pidStr, "%d", getppid());
        // position where each searcher will start to read the file
        // position will change for each new splitter_merger
        char position[] = "0";

        if (skew == 0) {
            execlp("./splitter_merger", "splitter_merger", pidStr, fdwStr, heightStr, \
                datafile, pattern, skewStr, position, numOfrecordsStr, NULL);
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
            execlp("./splitter_merger", "splitter_merger", pidStr, fdwStr, heightStr, \
                datafile, pattern, skewStr, position, numOfrecordsStr, start, end, sumStr, NULL);
        }
    }
    else if (pid == -1) {
        perror("Failed to execute fork");
        exit(1);
    }
    else {             // if parent process
        // set signal handler
        signal(SIGUSR2,handler);
        close(fd[WRITE]);
        readAndWriteResults(fd[READ], &count, &minSearcher, &maxSearcher, &averageSearcher, \
            &searcherCounter, &minSplMerg, &maxSlpMerg, &averageSplMerg, &splMergCounter);

        // wait for child to finish
        wait(NULL);
    }

    // fork new process for sort execution so that records are printed before
    // statistics and other info
    pid_t pidSort = fork();
    if (pidSort == 0) {      // if child process
        // call sort for the results file that will print sorted results in console
        // (sorted by first column)
        execlp("sort", "sort", "-k", "1", "results.txt", NULL);
    }
    else if (pidSort == -1) {
        perror("Failed to execute fork");
        exit(1);
    }
    else {      // if parent process
        wait(NULL);
        printf("\n%d records found\n", count);
        printf("Root received %d SIGUSR2 signals\n", sigCount);
        printf("Min searcher running time: %f\n", minSearcher);
        printf("Max searcher running time: %f\n", maxSearcher);
        printf("Average searcher running time: %f\n", averageSearcher);
        printf("Min splitter/merger running time: %f\n", minSplMerg);
        printf("Max splitter/merger running time: %f\n", maxSlpMerg);
        printf("Average splitter/merger running time: %f\n", averageSplMerg);
        gettimeofday(&stop, NULL);
        double time_spent = (double) (stop.tv_usec - begin.tv_usec) / 1000000 + (double) (stop.tv_sec - begin.tv_sec);
        printf("Turnaround Time %f\n\n", time_spent);
    }

    free(pattern);
    free(datafile);
    remove("results.txt");
    return 0;
}
