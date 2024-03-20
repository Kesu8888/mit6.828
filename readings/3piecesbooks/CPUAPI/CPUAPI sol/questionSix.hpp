#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//
// Created by peterill on 02/02/24.
//

/// When only one child is created.
int questionSix() {
    printf("Hello (pid:%d)\n", (int) getpid());
    int rc = fork();
    if (rc < 0) {
        //fork failed
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        // child (new process)
        printf("child (pid:%d)\n", (int) getpid());
    } else {
        int x = waitpid(rc, NULL, NULL);
        printf("parent of %d (pid:%d)\n", rc, (int) (getpid()));
        printf("parent wait return value is %d\n", x);
    }
    return 0;
}