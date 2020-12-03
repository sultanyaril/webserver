#include <stdio.h>
#include <string.h>
int main() {
    char h[] = "hello worald\n";
    printf("HTTP/1.1 200\ncontent-type: binary\ncontent-length: %ld\n\n",
        strlen(h));
    printf("%s", h);
    return 0;
}
