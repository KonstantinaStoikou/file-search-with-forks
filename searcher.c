#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "record.h"

int main (int argc, char const *argv[]) {
    printf("This is the Searcher program %d with parent %d\n", getpid(), getppid());
    char *datafile = malloc(sizeof(strlen(argv[0]) + 1));
    strcpy(datafile, argv[0]);
    char *pattern = malloc(sizeof(strlen(argv[1]) + 1));
    strcpy(pattern, argv[1]);
    int skew = atoi(argv[2]);
    int position = atoi(argv[3]);
    int numOfrecords = atoi(argv[4]);
    printf("numofrecords = %d, position = #%d\n", numOfrecords, position);

    FILE *fpb;
    Record rec;
    long lSize;

    fpb = fopen(argv[0], "rb");
    if (fpb == NULL) {
        perror("Cannot open binary file: ");
        exit(1);
    }

    // check number of records
    fseek(fpb, position, SEEK_SET);

    for(int i = 0; i < numOfrecords; i++) {
        fread(&rec, sizeof(rec), 1, fpb);
        int cur_pos = ftell(fpb);
        char str[200];
        sprintf(str, "%ld %s %s  %s %d %s %s %-9.2f\n", \
        rec.custid, rec.LastName, rec.FirstName, \
        rec.Street, rec.HouseID, rec.City, rec.postcode, \
        rec.amount);
        if (strstr(str, pattern) != NULL) {
            printf("%s\n", str);
        }
    }

    fclose (fpb);

    exit(0);
}
