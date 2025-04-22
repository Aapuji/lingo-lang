#ifndef UTIL_H
#define UTIL_H

#define RESIZING_FACTOR 2
#define RESIZING_APPEND(ARRAY, T, LEN, CAP, VALUE) \
if ((LEN) >= (CAP)) { \
    CAP *= RESIZING_FACTOR; \
    ARRAY = (T *) realloc(ARRAY, sizeof(T) * (CAP)); \
\
    if (ARRAY == NULL) { \
        perror("Realloc failed"); \
        exit(1); \
    } \
} \
\
ARRAY[(LEN)++] = VALUE;

#endif
