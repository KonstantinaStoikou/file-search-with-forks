#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char const *argv[]) {
    //change height to zerooooooooooooooooooooooooooooooooooooooooooooooooooo
    int height = 1;
    char *datafile;
    char *pattern;
    int skew = 0;       //flag for skew searching (initialized to false)

    //read command line arguments and initialize variables respectively
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            height = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "-d") == 0) {
            datafile = malloc(sizeof(strlen(argv[i+1]) + 1));
            strcpy(datafile, argv[i+1]);
        }
        else if (strcmp(argv[i], "-p") == 0) {
            pattern = malloc(sizeof(strlen(argv[i+1]) + 1));
            strcpy(pattern, argv[i+1]);
        }
        else if (strcmp(argv[i], "-s") == 0) {
            skew = 1;
        }
    }

    //check if valid height
    if (height < 1) {
        printf("Height should be at least 1\n");
        return 1;
    }

    printf("height: %d, datafile: %s, pattern: %s, skew: %d\n", height, datafile, pattern, skew);
    int pid = fork();
    char hi[] = "hello";
    if (pid == 0) {
        printf("I am child\n");
        char child_program[] = "./test";
        // execvp(args[0],args);
        execlp(child_program, hi, (char *)0);
    }
    else {
        printf("I am parent\n");
    }

    printf("This is still root program %d %d\n", getpid(), pid);

    return 0;
}
