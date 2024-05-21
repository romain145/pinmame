#include <stdio.h>
#include <stdint.h>
#include "cpu/m6809/m6809.h"
#include "memory.h"
#include <time.h>

int main() {
    int cycles = 0;
    float elapsed_time = 0.0f;

    //setvbuf(stdout, NULL, _IONBF, 0);
    printf("Hello, world!\n");

    memory_load_rom_from_file("../IJONE_L7.ROM");
    memory_init();

    m6809_reset(NULL);
    
    while(1)
    {
        clock_t start_time = clock();
        cycles += m6809_execute(2048);
        clock_t end_time = clock();

        //m6809_set_irq_line(0, 1);

        elapsed_time += ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
        if(elapsed_time > 1.0f)
        {
            break;
        }
    }
    
    printf("executed %d cycles in %fs\n", cycles, elapsed_time);
    printf("Speed: %fMHz\n", (float)(cycles / elapsed_time) / 1000000.0f);

    return 0;
}
