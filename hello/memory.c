/***************************************************************************

  memory.c

***************************************************************************/
#include <stdio.h>
#include "memory.h"

uint8_t memory[0xFFFF+1];

void memory_init() {
  memory[0x8D17] = 0x1A;
  memory[0x8D18] = 0x50;
  memory[0x8D19] = 0x86;
  memory[0x8D1A] = 0x00;
  memory[0x8D1B] = 0xB7;
  memory[0x8D1C] = 0x3F;
  memory[0x8D1D] = 0xF2;

	memory[0xFFFE] = 0x8D;
	memory[0xFFFF] = 0x17;
}

uint8_t cpu_readmem16(uint16_t addr) {
	return memory[addr];
}

void cpu_writemem16(uint16_t addr, uint8_t value) {
  if(addr == 0x3FF2 && value == 0x00) {
    printf("LED OFF\n");
  }
  if(addr == 0x3FF2 && value == 0x01) {
    printf("LED ON\n");
  }

	memory[addr] = value;
}

uint8_t cpu_readop(uint16_t addr) {
	return memory[addr];
}

uint8_t cpu_readop_arg(uint16_t addr) {
	return memory[addr];
}
