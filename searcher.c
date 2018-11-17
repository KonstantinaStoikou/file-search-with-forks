#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include "record.h"
#include "statistic.h"

int main (int argc, char const *argv[]) {
    struct timeval  begin, stop;
    gettimeofday(&begin, NULL);

    int rootPid = atoi(argv[1]);
    int fdw = atoi(argv[2]);
    char *datafile = malloc(strlen(argv[3]) + 1);
    strcpy(datafile, argv[3]);
    char *pattern = malloc(strlen(argv[4]) + 1);
    strcpy(pattern, argv[4]);
    int skew = atoi(argv[5]);
    int position = atoi(argv[6]);
    int numOfrecords = atoi(argv[7]);
    printf("root pid in searcher %d\n", rootPid);
    FILE *fpb;
    Record rec;
    long lSize;

    fpb = fopen(argv[3], "rb");
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
        sprintf(recStr, "%ld %s %s  %s %d %s %s %-9.2f", \
        rec.custid, rec.LastName, rec.FirstName, \
        rec.Street, rec.HouseID, rec.City, rec.postcode, \
        rec.amount);
        // check if current record includes the given substring
        if (strstr(recStr, pattern) != NULL) {
            write(fdw, &rec, sizeof(rec));
        }
    }
    fclose(fpb);

    // when all records are passed to the pipe, pass one last record with
    // negative id so that parent knows when records finish and statistics follow
    rec.custid = -1;
    write(fdw, &rec, sizeof(rec));

    Statistic stat;
    stat.processType = SEARCHER;
    gettimeofday(&stop, NULL);
    stat.time = (double) (stop.tv_usec - begin.tv_usec) / 1000000 + (double) (stop.tv_sec - begin.tv_sec);
    write(fdw, &stat, sizeof(stat));

    exit(0);
}
