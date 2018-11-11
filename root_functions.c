#include <stdio.h>
#include <stdlib.h>
#include "root_functions.h"
#include "record.h"

// open binary file and find number of records
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
