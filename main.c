#include <stdio.h>

#include "common.h"

int main(int argc, char **argv) {
    FILE *file = fopen(argv[0], "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }


    while (getc(file) != EOF) {

    }

    fclose(file);
    return 0;
}
