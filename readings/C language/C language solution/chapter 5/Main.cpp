//
// Created by peterill on 07/02/24.
//

#include <cstdio>

int main(int argc, char *argv[]) {
    struct pageInfo {
        int i;
        int x;
    };
    struct pageInfo i = {.i = 0, .x = 5};
    struct pageInfo *ptr = &i;
    printf("i is %d, x is %d\n", i.i, i.x);
    printf("i is %d, x is %d\n", ptr[0].i, ptr[0].x);
}