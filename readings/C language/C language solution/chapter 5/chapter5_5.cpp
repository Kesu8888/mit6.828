//
// Created by peterill on 31/01/24.
//

void strcat(char *s, char *t) {
    while (*s != '\0')
        s ++;
    while ((*s++ = *t++) != '\0')
        ;
}

bool strend(char *s, char *t) {
    int i = 0;
    while (*t++ != '\0')
    i ++;
    t = t - i;
    s = s + i;
    while ((*s++ == *t++) && *s != '\0')
    ;
    return *s == '\0';
}

void strncpy(char *dest, const char *src, unsigned n) {
    while (n-- > 0)
        *dest++ = *src++;
}

void strncat(char *dest, const char *src, unsigned n) {
    while (*dest++ != '\0')
        ;
    while (n-- > 0)
        *dest++ = *src++;
}

int strncmp(char *dest, const char *src, unsigned n) {
    while (n-- > 0) {
        if (*dest < *src)
            return -1;
        if (*dest > *src)
            return 1;
        src++;
        dest++;
    }
    return 0;
}