/*
 * LC4.h: Declares simulator functions for executing instructions
 */

#include "string.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    // PC the current value of the Program Counter register
    unsigned short int PC;

    // PSR : CPU Status Register, bit[0] = P, bit[1] = Z, bit[2] = N, bit[15] = privilege bit
    unsigned short int PSR;

    // Machine registers - all 8
    unsigned short int R[8];

    // Machine control signals
    // Note that all of the control signals are represented as unsigned 8 bit values although none of them use more than 3 bits
    // You should use the lower bits of the fields to store the mandated control bits.
    unsigned char rsMux_CTL;
    unsigned char rtMux_CTL;
    unsigned char rdMux_CTL;
    
    unsigned char regFile_WE;
    unsigned char NZP_WE;
    unsigned char DATA_WE;

    // These values can be used when writing to the output file
    unsigned short int regInputVal;
    unsigned short int NZPVal;
    unsigned short int dmemAddr;
    unsigned short int dmemValue;

    // Machine memory - all of it
    unsigned short int memory[65536];
} MachineState;


/*
 * This function should execute one LC4 datapath cycle.
 */
int UpdateMachineState(MachineState* CPU, FILE* output);


/*
 * This function should write out the current state of the CPU to the file output.
 */
void WriteOut(MachineState* CPU, FILE* output);


/*
 * This handles BRANCH instructions.
 */
void BranchOp(MachineState* CPU, FILE* output);


/*
 * This handles ARITHMETIC instructions.
 */
void ArithmeticOp(MachineState* CPU, FILE* output);


/*
 * This handles COMPARATIVE instructions.
 */
void ComparativeOp(MachineState* CPU, FILE* output);


/*
 * This handles LOGICAL instructions.
 */
void LogicalOp(MachineState* CPU, FILE* output);


/*
 * This handles JUMP instructions.
 */
void JumpOp(MachineState* CPU, FILE* output);


/*
 * This handles JSR instructions.
 */
void JSROp(MachineState* CPU, FILE* output);


/*
 * This handles SHIFT instructions.
 */
void ShiftModOp(MachineState* CPU, FILE* output);


/*
 * Set the NZP bits in the PSR.
 */
void SetNZP(MachineState* CPU, short result);


/*
 * Reset the machine state as Pennsim would do
 */
void Reset(MachineState* CPU);


/*
 * Clear all of the internal values (set to 0)
 */
void ClearSignals(MachineState* CPU);

int LoadOp(MachineState* CPU, FILE* output);

int StoreOp(MachineState* CPU, FILE* output);

void RtiOp(MachineState* CPU, FILE* output);

void TrapOp(MachineState* CPU, FILE* output);

void ConstOp(MachineState* CPU, FILE* output);

void HiconstOp(MachineState* CPU, FILE* output);

int CheckPermissions(MachineState* CPU, unsigned short int address);


unsigned short int extend_imm9(unsigned short int result);

unsigned short int extend_imm5(unsigned short int result);

unsigned short int extend_imm7(unsigned short int result);

unsigned short int extend_imm6(unsigned short int result);

unsigned short int extend_imm11(unsigned short int result);
