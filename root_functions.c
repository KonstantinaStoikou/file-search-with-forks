#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "root_functions.h"
#include "record.h"
#include "statistic.h"

// Open binary file and find number of records
int findNumOfRecords(char *datafile) {
    FILE *fpb;
    Record rec;
    long lSize;
    int numOfrecords;

    fpb = fopen(datafile, "rb");
    if (fpb == NULL) {
        perror("Cannot open binary file");
        exit(1);
    }
    // check number of records
    fseek(fpb, 0, SEEK_END);
    lSize = ftell(fpb);
    rewind(fpb);
    numOfrecords = (int) lSize / sizeof(rec);
    fclose(fpb);

    return numOfrecords;
}

// Read command line arguments and initialize variables respectively
void readArguments(int argc, char const *argv[], int *height, char **datafile, char **pattern, int *skew) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            *height = atoi(argv[i+1]);
            // check if valid height
            if (*height < 1) {
                printf("Height should be at least 1\n");
                exit(1);
            } else if (*height > 5) {
                printf("Height should be maximum 5\n");
                exit(1);
            }
        }
        else if (strcmp(argv[i], "-d") == 0) {
            *datafile = malloc(strlen(argv[i+1]) + 1);
            strcpy(*datafile, argv[i+1]);
        }
        else if (strcmp(argv[i], "-p") == 0) {
            *pattern = malloc(strlen(argv[i+1]) + 1);
            strcpy(*pattern, argv[i+1]);
        }
        else if (strcmp(argv[i], "-s") == 0) {
            *skew = 1;
        }
    }
}

// read from pipe (where splitter/merger wrote) until there is nothing more to read
// and write only records to results.txt file and print time statistics
void readAndWriteResults(int fd, int *count, double *minSearcher, double *maxSearcher, double *averageSearcher, \
                        int *searcherCounter, double *minSplMerg, double *maxSlpMerg, double *averageSplMerg, \
                        int *splMergCounter) {
    Record rec;
    Statistic stat;

    // open file where results will be written
    FILE *fp = fopen("results.txt", "w");
    if (fp == NULL) {
        perror("Failed to open file: \n");
        exit(1);
    }

    int r = read(fd, &rec, sizeof(rec));
    do {
        // reading a record with negative id means that next thing to
        // read is a statistic so first write a record with negative id
        // for the parent to know when a statistic follows and then
        // write the statistic
        if (rec.custid == -1) {
            r = read(fd, &stat, sizeof(stat));
            if (stat.processType == SEARCHER) {
                findRunningTimes(minSearcher, maxSearcher, averageSearcher, searcherCounter, stat.time);
            } else {
                findRunningTimes(minSplMerg, maxSlpMerg, averageSplMerg, splMergCounter, stat.time);
            }
            r = read(fd, &rec, sizeof(rec));
        } else {
            // write record to file
            fprintf(fp, "%ld %s %s  %s %d %s %s %-9.2f\n", \
                rec.custid, rec.LastName, rec.FirstName, \
                rec.Street, rec.HouseID, rec.City, rec.postcode, \
                rec.amount);
            (*count)++;
            r = read(fd, &rec, sizeof(rec));
        }
    } while (r > 0);

    fclose(fp);

    *averageSearcher = *averageSearcher / *searcherCounter;
    *averageSplMerg = *averageSplMerg / *splMergCounter;

}

void findRunningTimes(double *min, double *max, double *average, int *count, double time) {
    if (*min > time) {
        *min = time;
    }
    if (*max < time) {
        *max = time;
    }
    *average += time;
    (*count)++;
}
