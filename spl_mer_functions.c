#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "spl_mer_functions.h"
#include "record.h"
#include "statistic.h"

// Break number of records for each searcher depending on searching method (skew or not)
void breakNumOfRecords(int skew, int numOfrecords, char *numOfrecordsStr, int i, int start, int end, int mod, int sum) {
    if (skew == 0) {
        // if last forked searcher (with skew) add to numOfrecords the remainder
        // of the division else leave numOfrecords as it is
        if (i == 2) {
            sprintf(numOfrecordsStr, "%d", numOfrecords + mod);
        } else {
            sprintf(numOfrecordsStr, "%d", numOfrecords);
        }
    }
    else {
        // if first forked searcher (without skew)
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
}


void increaseSearcherPosition(int skew, int *position, int numOfrecords, int start, int end, int sum) {
    if (skew == 0) {
        //the position will increase by the number of records
        *position = *position + numOfrecords * sizeof(Record);
    } else {
        *position = *position + (numOfrecords * start / sum) * sizeof(Record);
    }
}


void increaseSplitterMergerPosition(int skew, int *position, int numOfrecords,int start, int end, int sum) {
    if (skew == 0) {
        //the position will increase by the number of records
        *position = *position + numOfrecords * sizeof(Record);
    } else {
        int rangeSum = 0;
        for (int j = start; j <= start - 1 + (end - start + 1) / 2; j++) {
            rangeSum += numOfrecords * j / sum;
        }
        *position = *position + rangeSum * sizeof(Record);
    }
}


void readFromChild(int fd, int fdw) {
    Record rec;
    Statistic stat;
    // read from pipe (where child wrote) until there is nothing more to read
    // and write it to parent's pipe
    int r = read(fd, &rec, sizeof(rec));
    do {
        // reading a record with negative id means that next thing to
        // read is a statistic, so in that case, first write a record with
        // negative id for the parent to know that a statistic follows and then
        // write the statistic
        if (rec.custid == -1) {
            write(fdw, &rec, sizeof(rec));
            r = read(fd, &stat, sizeof(stat));
            write(fdw, &stat, sizeof(stat));
            r = read(fd, &rec, sizeof(rec));
        } else {
            write(fdw, &rec, sizeof(rec));
            r = read(fd, &rec, sizeof(rec));
        }
    } while (r > 0);
}


// Wait for forked children to finish
void waitChildren(void) {
    pid_t wpid;
    int status = 0;
    while ((wpid = wait(&status)) > 0);
}


void calculateNewRange(int i, int *newStart, int *newEnd, int start, int end) {
    if (i == 1) {
        *newStart = start;
        *newEnd = start - 1 + (end - start + 1) / 2;
    } else {
        *newStart = end + 1 - (end - start + 1) / 2;
        *newEnd = end;
    }
}
