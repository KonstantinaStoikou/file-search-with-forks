#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "record.h"

int main (int argc, char const *argv[]) {
    printf("This is the Searcher program %d with parent %d\n", getpid(), getppid());
    char *datafile = malloc(sizeof(strlen(argv[0]) + 1));
    strcpy(datafile, argv[0]);
    int skew = atoi(argv[1]);

    FILE *fpb;
    Record rec;
    long lSize;
    int numOfrecords, i;

    fpb = fopen(argv[0], "rb");
    if (fpb == NULL) {
        perror("Cannot open binary file: ");
        exit(1);
    }

    // check number of records
    fseek(fpb, 0, SEEK_END);
    lSize = ftell(fpb);
    rewind(fpb);
    numOfrecords = (int) lSize / sizeof(rec);

    printf("Records found in file %d \n", numOfrecords);

    for(i = 0; i < numOfrecords; i++) {
        fread(&rec, sizeof(rec), 1, fpb);
        printf("%ld %s %s  %s %d %s %s %-9.2f\n", \
        rec.custid, rec.LastName, rec.FirstName, \
        rec.Street, rec.HouseID, rec.City, rec.postcode, \
        rec.amount);
    }

    fclose (fpb);

    exit(0);
}
