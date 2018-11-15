#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "record.h"

int main (int argc, char const *argv[]) {
    // printf("This is the Searcher program %d with parent %d\n", getpid(), getppid());
    int fdw = atoi(argv[1]);
    char *datafile = malloc(strlen(argv[2]) + 1);
    strcpy(datafile, argv[2]);
    char *pattern = malloc(strlen(argv[3]) + 1);
    strcpy(pattern, argv[3]);
    int skew = atoi(argv[4]);
    int position = atoi(argv[5]);
    int numOfrecords = atoi(argv[6]);

    FILE *fpb;
    Record rec;
    long lSize;

    fpb = fopen(argv[2], "rb");
    if (fpb == NULL) {
        perror("Cannot open binary file: ");
        exit(1);
    }

    // check number of records
    fseek(fpb, position, SEEK_SET);

    for(int i = 0; i < numOfrecords; i++) {
        fread(&rec, sizeof(rec), 1, fpb);
        int cur_pos = ftell(fpb);
        char recStr[150];
        sprintf(recStr, "%ld %s %s  %s %d %s %s %-9.2f\n", \
        rec.custid, rec.LastName, rec.FirstName, \
        rec.Street, rec.HouseID, rec.City, rec.postcode, \
        rec.amount);
        // check if current record includes the given substring
        if (strstr(recStr, pattern) != NULL) {
            printf("\nin searcher %d: %s\n", getpid(), recStr);
            // write it to parent's pipe
            if (write(fdw, recStr, strlen(recStr)+1) != strlen(recStr)+1) {
                printf("ERROR\n");
            }

        }
    }

    fclose(fpb);

    exit(0);
}
