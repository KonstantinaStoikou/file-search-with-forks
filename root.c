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
#include <time.h>
#include "root_functions.h"
#include "record.h"
#include "statistic.h"

#define READ 0
#define WRITE 1

int main(int argc, char const *argv[]) {
    clock_t begin = clock();
    int height = 0;
    char *datafile;
    char *pattern;
    int skew = 0;       // flag for skew searching (initialized to false)

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
        Record rec;
        Statistic stat;
        printf("\n" );
        int count = 0;
        // read from pipe (where splitter/merger wrote) until there is nothing more to read
        int r = read(fd[READ], &rec, sizeof(rec));
        do {
            // reading a record with negative id means that next thing to
            // read is a statistic so first write a record with negative id
            // for the parent to know when a statistic follows and then
            // write the statistic
            if (rec.custid == -1) {
                r = read(fd[READ], &stat, sizeof(stat));
                printf("%d %f\n", stat.processType, stat.time);
                r = read(fd[READ], &rec, sizeof(rec));
            } else {
                printf("%ld %s %s  %s %d %s %s %-9.2f\n", \
            		rec.custid, rec.LastName, rec.FirstName, \
            		rec.Street, rec.HouseID, rec.City, rec.postcode, \
            		rec.amount);
                count++;
                r = read(fd[READ], &rec, sizeof(rec));
            }
        } while (r > 0);
        printf("Total records found %d\n", count);
    }

    clock_t stop = clock();
    double time_spent = (double)(stop - begin) / (double)CLOCKS_PER_SEC;
    printf("Turnaround Time %f\n", time_spent);

    return 0;
}
