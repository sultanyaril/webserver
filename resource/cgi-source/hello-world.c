#include <stdio.h>
#include <string.h>
#include <unistd.h>
int main() {
    char content[] = "<http> <h1> hello worald </h1> </http>";
    char HEAD_LENGTH[] = "content-length: ";
    write(1, HEAD_LENGTH, strlen(HEAD_LENGTH));
    char str_size[10];
    sprintf(str_size, "%ld", strlen(content));
    write(1, str_size, strlen(str_size));
    write(1, "\r\n\r\n", 4);
    write(1, content, strlen(content));
    write(1, "\r\n", 2);
    return 0;
}
