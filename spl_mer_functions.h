#ifndef SPL_MER_FUNCTIONS_HEADER
#define SPL_MER_FUNCTIONS_HEADER

void breakNumOfRecords(int skew, int numOfrecords, char *numOfrecordsStr, int i, int start, int end, int mod, int sum);
void increaseSearcherPosition(int skew, int *position, int numOfrecords, int start, int end, int sum);
void increaseSplitterMergerPosition(int skew, int *position, int numOfrecords, int start, int end, int sum);
void readFromChild(int fd, int fdw);
void waitChildren(void);
void calculateNewRange(int i, int *newStart, int *newEnd, int start, int end);
void writeTimeToParent(int fdw, struct timeval begin, struct timeval stop);

#endif
