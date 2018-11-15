#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "root_functions.h"
#include "record.h"

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
    printf("Records found in file %d of size %ld \n", numOfrecords, sizeof(rec));
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