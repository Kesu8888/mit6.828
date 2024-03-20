#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

//They can access the fd at the same time. The content both the parent and the child are
// writing won't overwrite. The content of first executed program will write first follow
// by another program's write


int questionTwo() {
    printf("Hello, myPID is %d\n", getpid());
    int fd = open("./qtwo.txt", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    int rc = fork();
    if (rc < 0) {
        printf("fork error");
    } else if (rc == 0) {
        write(fd, "child", 5);
        wait(NULL);
        printf ("Child PID is %d, FD is %d\n", getpid(), fd);
    } else {
        write(fd, "parent", 5);
        printf("Parent PID is %d, FD is %d\n", getpid(), fd);
    }
    return 0;
}