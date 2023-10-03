/*
 * trace.c: location of main() to start the simulator
 */

#include "loader.h"

// Global variable defining the current state of the machine
MachineState* CPU;

int main(int argc, char** argv)
{
    //instantiates the machine
    MachineState machineState = {
    .PC = 0x8200,
    .PSR = 0x8002,
    .R = {0},
    .rsMux_CTL = 0,
    .rtMux_CTL = 0,
    .rdMux_CTL = 0,
    .regFile_WE = 0,
    .NZP_WE = 0,
    .DATA_WE = 0,
    .regInputVal = 0,
    .NZPVal = 0,
    .dmemAddr = 0,
    .dmemValue = 0,
    .memory = {0}
    };
    CPU = &machineState;
    char* output_file = NULL;       //name of the output file
    FILE *fp;                       //file datatype of the current file
    int filename_len;               //length of filename
    //checks proper number of args
    if (argc < 3) {
        printf("error: you must specify the name of your output file and at least one object file\n");
        return -1;
    }
    //checks that destination file is a text file
    if (strstr(argv[1],".txt") == NULL) {
        printf("error: the destination file is not a text file\n");
        return -1;
    }
    //checks that all obj files exist
    for (int i = 2; i < argc; i++) {
        filename_len = strlen(argv[i]);
        fp = fopen(argv[i],"rb");
        if (fp == NULL || strstr(argv[i],".obj") == NULL) {
            printf("error: one or more specified object files do not exist\n");
            return -1;
        }
        fclose(fp);
    }
    //loads the programs into memory
    for (int i = 2; i < argc; i++) {
        ReadObjectFile(argv[i], CPU);
    }
    //executes the machine
    fp = fopen(argv[1], "w");
    if (fp == NULL) {
        printf("error: could not create file\n");
        return -1;
    }
    int success;
    while (CPU->PC != 0x80FF) {
        success = UpdateMachineState(CPU, fp);
        if (success != 0) {
            break;
        }
    }
    fclose(fp);
    return 0;
}