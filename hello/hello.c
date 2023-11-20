#include <stdio.h>
#include <stdint.h>
#include "cpu/m6809/m6809.h"
#include "memory.h"

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("Hello, world!\n");

    memory_load_rom_from_file("../IJONE_L7.ROM");
    memory_init();

    m6809_reset(NULL);
    
    printf("info: %s\n", m6809_info(NULL, 129));
    
    m6809_execute(10);
    m6809_execute(5000);

    return 0;
}
