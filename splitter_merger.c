#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "record.h"

int main (int argc, char const *argv[]) {
    printf("This is the Splitter/Merger program %d with parent %d\n", getpid(), getppid());
    int height = atoi(argv[0]);
    char *datafile = malloc(sizeof(strlen(argv[1]) + 1));
    strcpy(datafile, argv[1]);
    int skew = atoi(argv[2]);
    int position = atoi(argv[3]);
    int numOfrecords = atoi(argv[4]);
    int mod;
    // if (skew == 0) {
    mod = numOfrecords % 2;
    numOfrecords /= 2;
    // }
    // else {}
    printf("height = %d\n", height);

    if (height == 1) {
        // fork two searchers
        for (int i = 0; i < 2; i++) {
            pid_t pid = fork();

            if (pid == 0) {     // if child process
                // make integers to strings and pass them to splitter/merger
                // for convenience I assume numOfrecords is maximum a 10 digit number
                // and position is maximum a 100 digit number
                char numOfrecordsStr[10];
                // if last forked searcher add to numOfrecords the remainder of the division
                if (i == 1) {
                    sprintf(numOfrecordsStr, "%d", numOfrecords + mod);
                } else {
                    sprintf(numOfrecordsStr, "%d", numOfrecords);
                }
                char positionStr[100];
                sprintf(positionStr, "%d", position);
                // arguments: datafile, skew, position, numOfrecords
                execlp("./searcher", datafile, argv[2], positionStr, numOfrecordsStr, NULL);
            }
            else if (pid == -1) {
                perror("fork");
                exit(1);
            }

            // add to last forked searchers the remainder of the division
            //if skeeeeeeewww
            numOfrecords += numOfrecords % 2;
            //the position will increase by the number of records
            position = position + numOfrecords * sizeof(Record);

            pid_t wpid;
            int status = 0;
            while ((wpid = wait(&status)) > 0) {
                // printf("Exit status of %d was %d\n", (int)wpid, status);
            }
        }
        exit(0);
    }

    height--;
    // fork two new splitter_mergers
    for (int i = 0; i < 2; i++) {
        pid_t pid = fork();

        if (pid == 0) {     // if child process
            // make integers to strings and pass them to splitter/merger
            // for convenience I assume height is maximum a 4 digit number,
            // numOfrecords is maximum a 10 digit number
            // and position is maximum a 100 digit number
            char heightStr[4];
            sprintf(heightStr, "%d", height);

            char numOfrecordsStr[10];
            // if last forked splitter_merger add to numOfrecords the remainder of the division
            if (i == 1) {
                sprintf(numOfrecordsStr, "%d", numOfrecords + mod);
            } else {
                sprintf(numOfrecordsStr, "%d", numOfrecords);
            }
            char positionStr[100];
            sprintf(positionStr, "%d", position);
            // arguments: height, datafile, skew, position, numOfrecords
            execlp("./splitter_merger", heightStr, argv[1], argv[2], positionStr, numOfrecordsStr, NULL);
        }
        else if (pid == -1) {
            perror("fork");
            exit(1);
        }

        //the position will increase by the number of records
        position = position + numOfrecords * sizeof(Record);

        pid_t wpid;
        int status = 0;
        while ((wpid = wait(&status)) > 0) {
            // printf("Exit status of %d was %d\n", (int)wpid, status);
        }
    }

    exit(0);
}
