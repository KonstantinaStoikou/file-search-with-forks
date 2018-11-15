#include <stdio.h>
#include "spl_mer_functions.h"
#include "record.h"

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

void increaseSearcherPosition(int skew, int *position, int numOfrecords, int i, int start, int end, int sum) {
    if (skew == 0) {
        //the position will increase by the number of records
        *position = *position + numOfrecords * sizeof(Record);
    }
    else {
        if (i == 1) {
            *position = *position + (numOfrecords * start / sum) * sizeof(Record);
        } else {
            // afto mporei na min xrisimeusei pote
            *position = *position + (numOfrecords * end / sum) * sizeof(Record);
        }
    }
}
