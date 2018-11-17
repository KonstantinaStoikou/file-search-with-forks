#ifndef ROOT_FUNCTIONS_HEADER
#define ROOT_FUNCTIONS_HEADER

int findNumOfRecords(char *datafile);
void readArguments(int argc, char const *argv[], int *height, char **datafile, char **pattern, int *skew);
void readAndWriteResults(int fd, int *count, double *minSearcher, double *maxSearcher, double *averageSearcher, \
                        int *searcherCounter, double *minSplMerg, double *maxSlpMerg, double *averageSplMerg, \
                        int *splMergCounter);
void findRunningTimes(double *min, double *max, double *average, int *count, double time);

#endif
