#ifndef RECORD_HEADER
#define RECORD_HEADER

#define SIZEofBUFF 20
#define SSizeofBUFF 6

typedef struct Record {
    long  	custid;
    char 	FirstName[SIZEofBUFF];
    char 	LastName[SIZEofBUFF];
    char	Street[SIZEofBUFF];
    int 	HouseID;
    char	City[SIZEofBUFF];
    char	postcode[SSizeofBUFF];
    float  	amount;
} Record;

#endif
