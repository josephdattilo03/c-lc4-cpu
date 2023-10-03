/*
 * loader.h: Declares loader functions for opening and loading object files
 */

#include <stdio.h>
#include "LC4.h"

// Read an object file and modify the machine state as described in the writeup
int ReadObjectFile(char* filename, MachineState* CPU);
unsigned short int swap_endian(unsigned short int instruction);
int parse_code (MachineState* CPU, FILE *fp);
int write_to_file(MachineState* CPU, char* filename);