#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

int questionThree() {
    printf("Hello, myPID is %d\n", getpid());
    int rc = fork();
    if (rc < 0) {
        printf("fork error");
    } else if (rc == 0) {
        printf ("Hello, I am child: %d\n", getpid());
    } else {
        struct timespec ts;
        ts.tv_nsec = 1000000;
        ts.tv_sec = 0;
        nanosleep(&ts, NULL);
        printf("goodbye, I am parent: %d\n", getpid());
    }
    return 0;
}