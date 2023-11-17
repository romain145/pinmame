/***************************************************************************

	memory.h

	Functions which handle the CPU memory and I/O port access.

***************************************************************************/

#pragma once
#include <stdint.h>

void memory_init();
uint8_t cpu_readmem16(uint16_t address);
void cpu_writemem16(uint16_t address, uint8_t data);
uint8_t cpu_readop(uint16_t address);
uint8_t cpu_readop_arg(uint16_t address);
