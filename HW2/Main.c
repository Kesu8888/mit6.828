//
// Created by peterill on 15/03/24.
//

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

int main(int i, char **argv) {
    open("a", O_WRONLY | O_CREAT, S_IRWXU);
}