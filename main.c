#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    printf("%li", __STDC_VERSION__);

    if (argc < 2) {
        fprintf(stderr, "Incorrect number of arguments.");
        exit(1);
    }
    
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    printf("File: %s\n", argv[1]);

    fclose(file);
    return 0;
}
