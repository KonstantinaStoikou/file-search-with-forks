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
    char *pattern = malloc(sizeof(strlen(argv[2]) + 1));
    strcpy(pattern, argv[2]);
    int skew = atoi(argv[3]);
    int position = atoi(argv[4]);
    int numOfrecords = atoi(argv[5]);
    int mod, start, end, sum;
    int fd = atoi(argv[6]);
    printf("fd %s\n", argv[6]);
    printf("fd %d\n", fd);
    printf("hii\n");
    char *string = "hi, this is in pipe";
    write(fd, string, (strlen(string)+1));

    if (skew == 0) {
        mod = numOfrecords % 2;
        numOfrecords /= 2;
    } else {
        start = atoi(argv[6]);
        end = atoi(argv[7]);
        sum = atoi(argv[8]);
    }

    printf("height = %d\n", height);

    if (height == 1) {
        // fork two searchers
        for (int i = 1; i <= 2; i++) {
            pid_t pid = fork();

            if (pid == 0) {     // if child process
                // make integers to strings and pass them to splitter/merger
                // for convenience I assume numOfrecords is maximum a 10 digit number
                // and position is maximum a 100 digit number
                char numOfrecordsStr[10];
                char positionStr[100];
                sprintf(positionStr, "%d", position);
                // if last forked searcher add to numOfrecords the remainder of the division
                if (skew == 0) {
                    if (i == 2) {
                        sprintf(numOfrecordsStr, "%d", numOfrecords + mod);
                    } else {
                        sprintf(numOfrecordsStr, "%d", numOfrecords);
                    }
                }
                else {
                    if (i == 1) {
                        sprintf(numOfrecordsStr, "%d", numOfrecords * start / sum);
                    } else {
                        int endSum = 0;
                        for (int j = 1; j <= end; j++) {
                            endSum += j;
                        }
                        // if this is the last searcher created pass all remaining records
                        if (endSum == sum) {
                            int remaining = numOfrecords;
                            // for each searcher that has been created find how much records
                            // has read and substract them from the initial number of records
                            // to find how many they remain because of modulos
                            for (int j = 1; j <= end; j++) {
                                remaining -= numOfrecords * j / sum;
                            }
                            sprintf(numOfrecordsStr, "%d", (numOfrecords * end / sum) + remaining);
                        } else {
                            sprintf(numOfrecordsStr, "%d", numOfrecords * end / sum);
                        }
                    }
                }

                // arguments: datafile, pattern, skew, position, numOfrecords
                execlp("./searcher", datafile, pattern, argv[3], positionStr, numOfrecordsStr, NULL);
            }
            else if (pid == -1) {
                perror("fork");
                exit(1);
            }

            if (skew == 0) {
                //the position will increase by the number of records
                position = position + numOfrecords * sizeof(Record);
            }
            else {
                if (i == 1) {
                    position = position + (numOfrecords * start / sum) * sizeof(Record);
                } else {
                    position = position + (numOfrecords * end / sum) * sizeof(Record);
                }
            }


        }
        pid_t wpid;
        int status = 0;
        while ((wpid = wait(&status)) > 0) {
            // printf("Exit status of %d was %d\n", (int)wpid, status);
        }
        exit(0);
    }

    height--;
    // fork two new splitter_mergers
    for (int i = 1; i <= 2; i++) {
        int newStart, newEnd;

        pid_t pid = fork();

        if (pid == 0) {     // if child process
            // make integers to strings and pass them to splitter/merger
            // for convenience I assume height is maximum a 4 digit number,
            // numOfrecords is maximum a 10 digit number
            // and position is maximum a 100 digit number
            char heightStr[4];
            sprintf(heightStr, "%d", height);
            char positionStr[100];
            sprintf(positionStr, "%d", position);

            char numOfrecordsStr[10];
            if (skew == 0) {
                // if last forked splitter_merger add to numOfrecords the remainder of the division
                if (i == 2) {
                    sprintf(numOfrecordsStr, "%d", numOfrecords + mod);
                } else {
                    sprintf(numOfrecordsStr, "%d", numOfrecords);
                }
                // arguments: height, datafile, pattern, skew, position, numOfrecords
                execlp("./splitter_merger", heightStr, argv[1], pattern, argv[3], positionStr, numOfrecordsStr, NULL);
            }
            else {
                if (i == 1) {
                    newStart = start;
                    newEnd = start - 1 + (end - start + 1) / 2;
                } else {
                    newStart = end + 1 - (end - start + 1) / 2;
                    newEnd = end;
                }
                char newStartStr[3];
                sprintf(newStartStr, "%d", newStart);
                char newEndStr[3];
                sprintf(newEndStr, "%d", newEnd);
                sprintf(numOfrecordsStr, "%d", numOfrecords);
                // arguments: height, datafile, pattern, skew, position, numOfrecords, start, end, sum
                execlp("./splitter_merger", heightStr, argv[1], pattern, argv[3], positionStr, numOfrecordsStr, newStartStr, newEndStr, argv[8], NULL);
            }
        }
        else if (pid == -1) {
            perror("fork");
            exit(1);
        }

        if (skew == 0) {
            //the position will increase by the number of records
            position = position + numOfrecords * sizeof(Record);
        }
        else {
            int rangeSum = 0;
            if (i == 1) {
                for (int j = start; j <= start - 1 + (end - start + 1) / 2; j++) {
                    rangeSum += numOfrecords * j / sum;
                }
            } else {
                for (int j = end + 1 - (end - start + 1) / 2; j <= end; j++) {
                    rangeSum +=  numOfrecords * j / sum;
                }
            }
            position = position + rangeSum * sizeof(Record);
        }


    }
    pid_t wpid;
    int status = 0;
    while ((wpid = wait(&status)) > 0) {
        // printf("Exit status of %d was %d\n", (int)wpid, status);
    }


    exit(0);
}
