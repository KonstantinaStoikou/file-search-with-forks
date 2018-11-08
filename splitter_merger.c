#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <math.h>
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
    int x1, x2;

    if (skew == 0) {
        mod = numOfrecords % 2;
        numOfrecords /= 2;
    } else {
        // argv[5], argc[6] will be passed only if skew == 1
        x1 = atoi(argv[5]);
        x2 = atoi(argv[6]);
    }
    printf("height = %d\n", height);

    if (height == 1) {
        // fork two searchers
        for (int i = 1; i <= 2; i++) {
            pid_t pid = fork();
            if (pid == 0) {     // if child process
                char numOfrecordsStr[10];
                if (skew == 1) {
                    int sum = atoi(argv[7]);
                    if (i == 1) {
                        sprintf(numOfrecordsStr, "%d", numOfrecords * x1 / sum);
                        printf("%s\n", numOfrecordsStr);
                    } else {
                        sprintf(numOfrecordsStr, "%d", numOfrecords * x2 / sum);
                        printf("%s\n", numOfrecordsStr);
                    }
                } else {
                    if (i == 2) {
                        sprintf(numOfrecordsStr, "%d", numOfrecords + mod);
                    } else {
                        sprintf(numOfrecordsStr, "%d", numOfrecords);
                    }
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
    for (int i = 1; i <= 2; i++) {
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
            if (i == 2 && skew == 0) {
                sprintf(numOfrecordsStr, "%d", numOfrecords + mod);
            } else {
                sprintf(numOfrecordsStr, "%d", numOfrecords);
            }
            char positionStr[100];
            sprintf(positionStr, "%d", position);
            if (skew == 0) {
                // arguments: height, datafile, skew, position, numOfrecords
                execlp("./splitter_merger", heightStr, argv[1], argv[2], positionStr, numOfrecordsStr, NULL);
            } else {
                char x1Str[3];
                char x2Str[3];
                // e.g. for starting range 1-8 first forked splitter_merger will get 1-4 and second 5-8
                if (i == 1) {
                    sprintf(x1Str, "%d", x1);
                    sprintf(x2Str, "%d", x2 / 2);
                } else {
                    sprintf(x1Str, "%d", x2 / 2 + 1);
                    sprintf(x2Str, "%d", x2);
                }

                // arguments: height, datafile, skew, position, numOfrecords, x1Str, x2Str
                execlp("./splitter_merger", heightStr, argv[1], argv[2], positionStr, numOfrecordsStr, x1Str, x2Str, argv[7], NULL);
            }
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
