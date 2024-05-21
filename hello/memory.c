/***************************************************************************

  memory.c

***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

#define MAX_ROM_BANKS (0x3F+1) // last ROM bankc is at 0x3F
#define ROM_BANK_SIZE 0x4000
#define RAM_SIZE 0x8000 // 32Kb

uint8_t* ram;
uint8_t* rom;
uint32_t rom_size;
uint8_t rom_banks[MAX_ROM_BANKS][ROM_BANK_SIZE];
uint8_t asic_rom_bank;

// ASIC registers
typedef enum {
  ASIC_SND_BACK = 0x3FDD,
  ASIC_REG_LED = 0x3FF2,
  ASIC_REG_FIRQ = 0x3FF8,
  ASIC_REG_ROM_BANK = 0x3FFC,
  ASIC_REG_RAM_LOCK = 0x3FFD,
  ASIC_REG_RAM_LOCKSIZE = 0x3FFE,
  ASIC_REG_IRQ = 0x3FFF
} asic_reg_t;

void memory_load_rom_from_file(char *filename) {
  FILE *fp;
  int i;
  uint8_t byte;

  fp = fopen(filename, "rb"); // r for read, b for binary
  if(fp == NULL) {
    printf("error: could not open file %s\n", filename);
    return;
  }

  // Seek to the end of the file
  fseek(fp, 0, SEEK_END);

  // Get the size of the file
  rom_size = ftell(fp);

  // Seek back to the start of the file
  fseek(fp, 0, SEEK_SET);

  // Allocate memory
  rom = malloc(rom_size);
  if(rom == NULL) {
    printf("error: could not allocate ROM\n");
    return;
  }

  // Read the file into memory
  i = 0;
  while(fread(&byte, 1, 1, fp) == 1) {
    rom[i] = byte;
    i++;
  }

  printf("File loaded %s\n", filename);
  fclose(fp);
}

void memory_init() {
  // Fill-in ROM banks
  // WPC splits the large ROM into smaller ROM banks
  // ROM banks are aligned to the end of the ROM. 0x3E and 0x3F are always last
  // ROM banks are 16Kb (0x4000) from 0x3F down to 0x00 for a 8Mb ROM
  // A 4Mb ROM has 32 banks from 0x3F down to 0x20
  // A 2Mb ROM has 16 banks from 0x3F down to 0x30
  // A 1Mb ROM has 8 banks from 0x3F down to 0x38
  for(int i=0x3F; i>= rom_size/0x4000; i--) {
    uint32_t src_addr = rom_size - (0x4000 * (0x3F - i + 1));
    memcpy(rom_banks[i], &rom[src_addr], 0x4000);
  }
  // We don't need the ROM anymore
  //free(rom);

  // allocate ram
  ram = malloc(RAM_SIZE);
  if(ram == NULL) {
    printf("error: could not allocate RAM\n");
    return;
  }
}

uint8_t mmu_read(uint16_t addr) {
  if(addr < 0x2000) {
    // Read from RAM
    uint8_t value = ram[addr];
    //printf("Read from RAM: %04X %02X\n", addr, value);
    return value;
  } else if(addr < 0x4000) {
    // Read from the hardware
    printf("Read from hardware: %04X\n", addr);
    switch(addr)
    {
      case ASIC_SND_BACK:
        //printf("ASIC_SND_BACK %02X\n", value);
        break;
      case ASIC_REG_FIRQ:
        printf("ASIC_REG_FIRQ\n");
        return 0x00;
        break;
      default:
        //printf("Unknown ASIC register: %04X\n", addr);
        return 0x00;
        break;
    }
  } else if(addr < 0x8000) {
    // Read from the switchable ROM bank
    uint16_t offset = 0x4000;
    uint8_t value = rom_banks[asic_rom_bank][addr-offset];
    //printf("Read from ROM bank %02X: %04X %02X\n", asic_rom_bank, addr-offset, value);
    return value;
  } else {
    // Read from ROM
    uint16_t offset = 0x8000;
    // System ROM is always in bank 0x3E
    uint8_t value = rom_banks[0x3E][addr-offset];
    //printf("Read from ROM: %04X %02X\n", addr, value);
    return value;
  }
}

void mmu_write(uint16_t addr, uint8_t value) {
    static int cpt=0;
  if(addr < 0x2000) {
    // Write to RAM
    //printf("Write to RAM: %04X %02X\n", addr, value);
    ram[addr] = value;
  } else if(addr < 0x4000) {
    // Write to the hardware
    //printf("Write to hardware: %04X = %02X cpt=%d\n", addr, value, cpt++);
    switch(addr)
    {
      case ASIC_SND_BACK:
        //printf("ASIC_SND_BACK %02X\n", value);
        break;
      case ASIC_REG_LED:
        printf("ASIC_REG_LED %02X\n", value);
        break;
      case ASIC_REG_ROM_BANK:
        //printf("ASIC_REG_ROM_BANK %02X\n", value);
        asic_rom_bank = value;
        break;
      case ASIC_REG_RAM_LOCK:
        printf("ASIC_REG_RAM_LOCK %02X\n", value);
        break;
      case ASIC_REG_RAM_LOCKSIZE:
        printf("ASIC_REG_RAM_LOCKSIZE %02X\n", value);
        break;
      case ASIC_REG_IRQ:
        //printf("ASIC_REG_IRQ\n");
        break;
      default:
        //printf("Unknown ASIC register: %04X\n", addr);
        break;
    }

  } else if(addr < 0x8000) {
    // Write to the ROM
  } else {
    // Write to ROM
  }
}

uint8_t cpu_readmem16(uint16_t addr) {
	return mmu_read(addr);
}

void cpu_writemem16(uint16_t addr, uint8_t value) {
	mmu_write(addr, value);
}

uint8_t cpu_readop(uint16_t addr) {
	return mmu_read(addr);
}

uint8_t cpu_readop_arg(uint16_t addr) {
	return mmu_read(addr);
}
