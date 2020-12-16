#include <stdio.h>
#include <string.h>
int main(int argc, char *argv[]) {
    printf("content-length: ");

    long length = 0;
    for (int i = 0 ; i < argc ; i++) {
        length += strlen(argv[i]);
    }
    length += argc;  // for spaces
    printf("%ld\n\r\n\r", length);
    
    for (int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n\r");
}
