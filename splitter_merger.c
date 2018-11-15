#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include "record.h"
#include "statistic.h"

#define READ 0
#define WRITE 1

int main (int argc, char const *argv[]) {
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
                clock_t begin = clock();

                close(fd[READ]);
                // make integers to strings and pass them to splitter/merger
                // for convenience I assume numOfrecords is maximum a 10 digit number
                // and position is maximum a 100 digit number
                char beginStr[10];
                sprintf(beginStr, "%f", (double)begin);
                char positionStr[100];
                sprintf(positionStr, "%d", position);
                char fdwStr[10];
                sprintf(fdwStr, "%d", fd[WRITE]);
                char numOfrecordsStr[10];
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

                // arguments: fd write end, datafile, pattern, skew, position, numOfrecords, begin time
                execlp("./searcher", "searcher", fdwStr, datafile, pattern, argv[5], positionStr, numOfrecordsStr, beginStr, NULL);
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
            close(fd[WRITE]);
            Record rec;
            Statistic stat;
            // read from pipe (where searcher wrote) until there is nothing more to read
            // and write it to parent's pipe
            int r = read(fd[READ], &rec, sizeof(rec));
            do {
                // reading a record with negative id means that next thing to
                // read is a statistic so first write a record with negative id
                // for the parent to know when a statistic follows and then
                // write the statistic
                if (rec.custid == -1) {
                    write(fdw, &rec, sizeof(rec));
                    r = read(fd[READ], &stat, sizeof(stat));
                    write(fdw, &stat, sizeof(stat));
                    r = read(fd[READ], &rec, sizeof(rec));
                } else {
                    write(fdw, &rec, sizeof(rec));
                    r = read(fd[READ], &rec, sizeof(rec));
                }
            } while (r > 0);
        }
        // wait for children to finish
        pid_t wpid;
        int status = 0;
        while ((wpid = wait(&status)) > 0);
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
                // arguments: fd write end, height, datafile, pattern, skew, position, numOfrecords, start, end, sum
                execlp("./splitter_merger", "splitter_merger", fdwStr, heightStr, argv[3], pattern, argv[5], positionStr, numOfrecordsStr, newStartStr, newEndStr, argv[10], NULL);
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
        close(fd[1]);
        Record rec;
        Statistic stat;
        // read from pipe (where splitter/merger wrote) until there is nothing more to read
        // and write it to parent's pipe
        int r = read(fd[READ], &rec, sizeof(rec));
        do {
            // reading a record with negative id means that next thing to
            // read is a statistic so first write a record with negative id
            // for the parent to know when a statistic follows and then
            // write the statistic
            if (rec.custid == -1) {
                write(fdw, &rec, sizeof(rec));
                r = read(fd[READ], &stat, sizeof(stat));
                write(fdw, &stat, sizeof(stat));
                r = read(fd[READ], &rec, sizeof(rec));
            } else {
                write(fdw, &rec, sizeof(rec));
                r = read(fd[READ], &rec, sizeof(rec));
            }
        } while (r > 0);
    }
    // wait for children to finish
    pid_t wpid;
    int status = 0;
    while ((wpid = wait(&status)) > 0);

    exit(0);
}
