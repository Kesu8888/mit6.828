#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//
// Created by peterill on 02/02/24.
//

/// The child cannot print anything at the console after the stdout_fileno descriptor is closed
int questionSeven() {
    printf("Hello (pid:%d)\n", (int) getpid());
    int p[2];
    if (pipe(p) < 0) {
        fprintf(stderr, "fork failed\n");
        exit(NULL);
    }
    int rc = fork();
    if (rc < 0) {
        //fork failed
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        // child (new process)
        write(p[1], "I dont care", 10);
        write(p[1], "I really care", 13);
    } else {
        waitpid(rc, NULL, NULL);
        char content[14];
        read(p[0], content, 10);
        printf("The content is :\n%s\n", content);
        read(p[0], content, 13);
        printf("The content is :\n%s\n", content);
    }
    return 0;
}