#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main (int argc, char const *argv[]) {
    printf("This is the Splitter/Merger program %d with parent %d\n", getpid(), getppid());
    int height = atoi(argv[0]);
    char *datafile = malloc(sizeof(strlen(argv[1]) + 1));
    strcpy(datafile, argv[1]);
    int skew = atoi(argv[2]);
    printf("%d\n", height);

    if (height == 1) {
        // fork two searchers
        for (int i = 0; i < 2; i++) {
            pid_t pid = fork();

            if (pid == 0) {     // if child process
                char skewStr[4];
                sprintf(skewStr, "%d", skew);
                execlp("./searcher", datafile, skewStr, NULL);
            }
            else if (pid == -1) {
                perror("fork");
                exit(1);
            }

            pid_t wpid;
            int status = 0;
            while ((wpid = wait(&status)) > 0)
            {
                printf("Exit status of %d was %d\n", (int)wpid, status);
            }
        }
        return 0;
    }

    height--;
    // fork two new splitter_mergers
    for (int i = 0; i < 2; i++) {
        pid_t pid = fork();

        if (pid == 0) {     // if child process
            // make height and skew from integer to string and pass it to splitter/merger
            // for convenience I assume height is maximum a 4 digit number
            char heightStr[4];
            sprintf(heightStr, "%d", height);
            char skewStr[4];
            sprintf(skewStr, "%d", skew);
            execlp("./splitter_merger", heightStr, datafile, skewStr, NULL);
        }
        else if (pid == -1) {
            perror("fork");
            exit(1);
        }

        pid_t wpid;
        int status = 0;
        while ((wpid = wait(&status)) > 0)
        {
            printf("Exit status of %d was %d\n", (int)wpid, status);
        }
    }

    exit(0);
}
