//
// Created by peterill on 17/03/24.
//
#include "types.h"
#include "date.h"
#include "user.h"

int
main(int argc, char *argv[])
{
    struct rtcdate r;
    //int encode;
    /*if (!(encode = date())) {
        printf(2, "date failed\n");
        exit();
    }*/
    if (date(&r)) {
        printf(2, "date failed\n");
        exit();
    }

    //r = timeDecoding(encode);
    printf(2, "y: %d m: %d d: %d %d:%d:%d\n", r.year, r.month, r.day, r.hour, r.minute, r.second);
    // your code to print the time in any format you like...
    exit();
}

/*struct rtcdate
timeDecoding(int encode) {
    struct rtcdate r;
    r.second = encode & 0x3f;
    encode = encode >> 6;
    r.minute = encode & 0x3f;
    encode = encode >> 6;
    r.hour = encode & 0x1f;
    encode = encode >> 5;
    r.day = encode >> 0x1f;
    encode = encode >> 5;
    r.month = encode & 0xf;
    encode = encode >> 4;
    r.year = 2000 + (encode & 0x3f);
    return r;
}*/
