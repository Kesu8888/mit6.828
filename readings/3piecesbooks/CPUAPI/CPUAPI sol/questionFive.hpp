#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//
// Created by peterill on 02/02/24.
//

/// No other child process can be waited by the child. and Hence the return value of wait is -1;
/// The wait return value of the parent is the pid of the waited child;
int questionFive() {
    printf("Hello (pid:%d)\n", (int) getpid());
    int rc = fork();
    if (rc < 0) {
        //fork failed
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        // child (new process)
        int x = wait(NULL);
        printf("child (pid:%d)\n", (int) getpid());
        printf("parent wait return value is %d\n", x);
    } else {
        int x = wait(NULL);
        printf("parent of %d (pid:%d)\n", rc, (int) (getpid()));
        printf("parent wait return value is %d\n", x);
    }
    return 0;
}