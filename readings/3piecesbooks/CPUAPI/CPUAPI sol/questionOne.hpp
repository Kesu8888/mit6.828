#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//
// Created by peterill on 02/02/24.
//
int questionOne() {
    printf("Hello (pid:%d)\n", (int) getpid());
    int x = 100;
    int rc = fork();
    if (rc < 0) {
        //fork failed
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        // child (new process)
        printf("child (pid:%d)\n", (int) getpid());
        printf("variable x = %d\n", x);
    } else {
        printf("parent of %d (pid:%d)\n", rc, (int) (getpid()));
        printf("variable x = %d\n", x);
    }
    return 0;
}