//
// Created by peterill on 30/01/24.
//

#include "chapter2_9.h"

// x = 100000'1111'01, y = 11010'0010'
// result will be 100000'0010'01
unsigned setbits(unsigned x, int p, int n, unsigned y) {
    unsigned rightmostbitsMask = ~(x & ~0 << (p + 1 - n));
    unsigned rightmostbitsInX = x & rightmostbitsMask;
    unsigned ErasePNbitsinX = (x >> (p+1) << (p+1)) | rightmostbitsInX;
    unsigned yWithNbitsMask = ~(~0 << n) & y << (p+1-n);
    return ErasePNbitsinX | yWithNbitsMask;
}

unsigned invert(unsigned x, int p, int n) {
    unsigned onlyRightNbitsAreOne = ~(~0 << n);
    unsigned mask = ~(x >> (p + 1 - n)) & onlyRightNbitsAreOne << (p + 1 - n);
    unsigned rightmostbitsMask = ~(x & ~0 << (p + 1 - n));
    unsigned rightmostbitsInX = x & rightmostbitsMask;
    unsigned ErasePNbitsinX = (x >> (p+1) << (p+1)) | rightmostbitsInX;
    return mask | ErasePNbitsinX;
}

unsigned rightrot(unsigned x, int n) {
    unsigned rightmostnbits = x & ~(~0 << n);
    unsigned mask = rightmostnbits << (32 - n);
    return (x >> n) | mask;
}