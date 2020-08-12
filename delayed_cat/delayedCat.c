#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: delayedCat [filenames...]\n");
        return 1;
    }

    // sleep(atoi(argv[1]));

    // Wait till input
    while(1) {
        char _void[1];
        read(STDIN_FILENO, &_void, 1);

        for(int i = 1;i < argc;i++) {
            FILE *fp = fopen(argv[i], "r");
            if(fp == NULL) {
                fprintf(stderr, "Could not read file\n");
                return 2;
            }

            int readChar;
            while((readChar = fgetc(fp)) != EOF) {
                printf("%c", (unsigned char)readChar);
            }

            if(fclose(fp) != 0) {
                fprintf(stderr, "Could not close file\n");
                return 2;
            }
        }

        // FLUSH!
        fflush(stdout);
    }

    return 0;
}
