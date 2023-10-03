/*
 * loader.c : Defines loader functions for opening and loading object files
 */

#include "loader.h"

// memory array location
unsigned short memoryAddress;

/*
 * Read an object file and modify the machine state as described in the writeup
 */
int ReadObjectFile(char* filename, MachineState* CPU)
{
  FILE *fp;   //file to be read
  unsigned short int binary_instruction;
  fp = fopen(filename, "rb");
  if (fp == NULL) {
    printf("error: the file could not be opened\n");
    return -1;
  }
  while(!feof(fp)) {
    fread(&binary_instruction, sizeof(binary_instruction), 1, fp);
    binary_instruction = swap_endian(binary_instruction);
    if (binary_instruction == 0xCADE || binary_instruction == 0xDADA) {
      parse_code(CPU, fp);
    }
  }
  fclose(fp);
  return 0;
}


unsigned short int swap_endian (unsigned short int instruction) {
  unsigned short int temp = instruction;
  unsigned short int s0,s1,s2,s3;
  s0 = (temp & 0x000F) << 8u;
  s1 = (temp & 0x00F0) << 8u;
  s2 = (temp & 0x0F00) >> 8u;
  s3 = (temp & 0xF000) >> 8u;
  return s0 + s1 + s2 + s3;
}

int parse_code (MachineState* CPU, FILE *fp) {
  unsigned short int addr, amt_of_data, current_data;
  fread(&addr, sizeof(addr), 1, fp);
  fread(&amt_of_data, sizeof(amt_of_data), 1, fp);
  addr = swap_endian(addr);
  amt_of_data = swap_endian (amt_of_data);
  for (int i = 0; i < amt_of_data; i++) {
    if (addr > 65535) {
      printf("error: the address specified exceeds the memory of the system\n");
      return -1;
    }
    fread(&current_data, sizeof(current_data), 1, fp);
    current_data = swap_endian(current_data);
    CPU -> memory[addr] = current_data;
    addr++;
  }
  return 0;
}

int write_to_file(MachineState* CPU, char* filename) {
  FILE *fp;
  fp = fopen(filename, "w");
  unsigned short int addr;
  if (fp == NULL) {
    printf("error: could not create file\n");
    return -1;
  }
  for (int i = 0; i < 65536; i++) {
    if (CPU->memory[i] != 0) {
      addr = (unsigned short int) i;
      fprintf(fp, "address: %05hu", addr);
      fprintf(fp, " contents: 0x%04hX\n", CPU->memory[i]);
    }
  }
  fclose(fp);
  return 0;
}

