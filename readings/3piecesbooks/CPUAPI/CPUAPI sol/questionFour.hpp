#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//
// Created by peterill on 02/02/24.
//
int questionFour() {
    printf("Hello\n");
    int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "Failed");
    } else if (rc == 0) {
        printf("I am child\n");
        /*execl("/bin/ls", NULL); */
        char *execvArray[2];
        /*execvArray[0] = "/bin/ls";
        execvArray[1] = NULL;
        execv(execvArray[0], NULL);*/
    } else {
        wait(NULL);
        printf("I am parent\n");
    }
    return 0;
}