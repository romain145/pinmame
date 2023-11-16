#include <stdio.h>
#include "cpu/m6809/m6809.h"

char s[50];

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("Hello, world!\n");

    m6809_reset(NULL);
    
    printf("info: %s\n", m6809_info(NULL, 129));
    
    
    printf("Hello, world!2\n");
    m6809_execute(3);

    return 0;
}
