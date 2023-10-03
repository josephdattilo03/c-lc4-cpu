/*
 * LC4.c: Defines simulator functions for executing instructions
 */

#include "LC4.h"
#include <stdio.h>

/*
 * Reset the machine state as Pennsim would do
 */
void Reset(MachineState* CPU)
{
    CPU->PSR = 0x8002;
    CPU->PC = 0x8200;
    for (int i = 0; i < sizeof(CPU->memory); i++) {
        CPU->memory[i] = 0;
    }
    for(int i = 0; i < sizeof(CPU->R);i++) {
        CPU->R[i] = 0;
    }
    ClearSignals(CPU);
}


/*
 * Clear all of the control signals (set to 0)
 */
void ClearSignals(MachineState* CPU)
{
    CPU->rsMux_CTL = 0;
    CPU->rtMux_CTL = 0;
    CPU->rdMux_CTL = 0;
    CPU->regFile_WE = 0;
    CPU->NZP_WE = 0;
    CPU->DATA_WE = 0;
    CPU->regInputVal = 0;
    CPU->NZPVal = 0;
    CPU->dmemAddr = 0;
    CPU->dmemValue = 0;
}


/*
 * This function should write out the current state of the CPU to the file output.
 */
void WriteOut(MachineState* CPU, FILE* output)
{
    fprintf(output, "%d", CPU->regFile_WE);
    fprintf(output, " ");
    fprintf(output, "%d", CPU->rdMux_CTL);
    fprintf(output, " ");
    if (CPU->regFile_WE == 1) {
        fprintf(output, "%04X", CPU->R[CPU->rdMux_CTL]);
        fprintf(output, " ");   
    }
    else {
        fprintf(output, "0000");
        fprintf(output, " "); 
    }

    fprintf(output,"%d", CPU->NZP_WE);
    fprintf(output, " ");
    fprintf(output, "%d", CPU->NZPVal);
    fprintf(output, " ");

    fprintf(output, "%d", CPU->DATA_WE);
    fprintf(output, " ");
    fprintf(output, "%04X", CPU->dmemAddr);
    fprintf(output, " ");
    fprintf(output, "%04X\n", CPU->dmemValue);



}


/*
 * This function should execute one LC4 datapath cycle.
 */
int UpdateMachineState(MachineState* CPU, FILE* output)
{
    //gets the opcode and checks if the current PC is in range
    int address_issue = CheckPermissions(CPU, CPU->PC);
    if(address_issue == 1) {
        ClearSignals(CPU);
        printf("error: address out of permitted range\n");
        return 1;
    }
    //saves the PC to be printed out later
    unsigned short int curr_pc = CPU->PC;
    unsigned short int opcode = CPU->memory[CPU->PC] >> 12u;
    //runs a function based on the opcode
    if (opcode == 1) {
        CPU->NZP_WE = 1;
        CPU->regFile_WE = 1;
        ArithmeticOp(CPU, output);
    }
    else if (opcode == 2) {
        CPU->NZP_WE = 1;
        ComparativeOp(CPU, output);
    }
    else if (opcode == 5) {
        CPU->NZP_WE = 1;
        CPU->regFile_WE = 1;
        LogicalOp(CPU, output);
    }
    else if (opcode == 0) {
        BranchOp(CPU, output);
    }
    else if (opcode == 12) {
        JumpOp(CPU, output);
    }
    else if (opcode == 4) {
        JSROp(CPU, output);
    }
    else if (opcode == 10) {
        CPU->NZP_WE = 1;
        CPU->regFile_WE = 1;
        ShiftModOp(CPU, output);
    }
    else if (opcode == 6) {
        CPU->NZP_WE = 1;
        CPU->regFile_WE = 1;
        int load_success = LoadOp(CPU, output);
        if (load_success != 0) {
            return 1;
        }
    }
    else if (opcode == 7) {
        CPU->DATA_WE = 1;
        int load_success = StoreOp(CPU, output);
        if (load_success != 0) {
            return 1;
        }
    }
    else if (opcode == 8) {
        RtiOp(CPU, output);
    }
    else if (opcode == 15) {
        CPU->NZP_WE = 1;
        CPU->regFile_WE = 1;
        TrapOp(CPU, output);
    }
    else if (opcode == 13) {
        CPU->NZP_WE = 1;
        CPU->regFile_WE = 1;
        HiconstOp(CPU, output);
    }
    else if (opcode == 9) {
        CPU->NZP_WE = 1;
        CPU->regFile_WE = 1;
        ConstOp(CPU, output);
    }
    else {
        printf("error: unrecognised instruction in program memory\n");
        return 1;
    }
    //prints the PC and calls Writeout
    fprintf(output, "%04X", curr_pc);
    fprintf(output, " ");
    unsigned int mask = 1 << (sizeof(unsigned short int) * 8 - 1);  
    while (mask > 0) {
        if (CPU->memory[curr_pc] & mask) {
            fprintf(output, "1");
        } else {
            fprintf(output, "0");
        }
        mask >>= 1; 
    }
    fprintf(output, " ");
    WriteOut(CPU, output);
    //clears necessary spots in the machine before the next run of the program
    ClearSignals(CPU);
    return 0;
}



//////////////// PARSING HELPER FUNCTIONS ///////////////////////////



/*
 * Parses rest of branch operation and updates state of machine.
 */
void BranchOp(MachineState* CPU, FILE* output)
{
    unsigned short int nzp = CPU->PSR & 0x0007;
    unsigned short int n,z,p;
    unsigned short int subop = (CPU->memory[CPU->PC] >> 9u) & 0x0007;
    unsigned short int imm9 = CPU->memory[CPU->PC] & 0x01FF;
    imm9 = extend_imm9(imm9);
    //gets current nzp and seperates it into individual values
    n = nzp >> 2u;
    z = (nzp >> 1u) & 0x0001;
    p = (nzp) & 0x0001;
    //depending on the sub_opcode compares the nzp and executes the program jump if fulfilled
    if (subop == 4 && (n == 1)) {
        CPU->PC = CPU->PC + 1 + imm9;
    }
    else if (subop == 6 && (n == 1 || z == 1)) {
        CPU->PC = CPU->PC + 1 + imm9;
    }
    else if (subop == 5 && (n == 1 || p == 1)) {
        CPU->PC = CPU->PC + 1 + imm9;
    }
    else if (subop == 2 && (z == 1)) {
        CPU->PC = CPU->PC + 1 + imm9;
    }
    else if (subop == 3 && (z == 1 || p == 1)) {
        CPU->PC = CPU->PC + 1 + imm9;
    }
    else if (subop == 1 && p == 1) {
        CPU->PC = CPU->PC + 1 + imm9;
    }
    else if (subop == 7 && (n == 1 || z == 1 || p == 1)) {
        CPU->PC = CPU->PC + 1 + imm9;
    }
    else {
        CPU->PC = CPU->PC + 1;
    }
    
}

/*
 * Parses rest of arithmetic operation and prints out.
 */
void ArithmeticOp(MachineState* CPU, FILE* output)
{
    unsigned short int subop;
    //gets the rd and rs values that are consistant in every arithmetic operation
    CPU->rdMux_CTL = (CPU->memory[CPU->PC] & 0x0E00) >> 9u;
    CPU->rsMux_CTL = (CPU->memory[CPU->PC] & 0x01C0) >> 6u;
    //operates on these values depending on the value of the sub_opcode
    subop = (CPU->memory[CPU->PC] & 0x0038) >> 3u;
    if (subop < 4) {
        CPU->rtMux_CTL = CPU->memory[CPU->PC] & 0x0007;
        if(subop == 0) {
            CPU->R[CPU->rdMux_CTL] = CPU->R[CPU->rsMux_CTL] + CPU->R[CPU->rtMux_CTL];
        }
        else if(subop == 1) {
            CPU->R[CPU->rdMux_CTL] = CPU->R[CPU->rsMux_CTL] * CPU->R[CPU->rtMux_CTL];
        }
        else if(subop == 2) {
            CPU->R[CPU->rdMux_CTL] = CPU->R[CPU->rsMux_CTL] - CPU->R[CPU->rtMux_CTL];
        }
        else if(subop == 3) {
            CPU->R[CPU->rdMux_CTL] = CPU->R[CPU->rsMux_CTL] / CPU->R[CPU->rtMux_CTL];
        }
    }
    //operates on rd and a provided 5-bit value
    else {
        unsigned short int imm5 = (CPU->memory[CPU->PC] & 0x001F);
        imm5 = extend_imm5(imm5);
        CPU->R[CPU->rdMux_CTL] = CPU->R[CPU->rsMux_CTL] + imm5;
    }
    SetNZP(CPU, CPU->R[CPU->rdMux_CTL]);
    CPU->PC = CPU->PC + 1;

}

/*
 * Parses rest of comparative operation and prints out.
 */
void ComparativeOp(MachineState* CPU, FILE* output)
{
    unsigned short int subop;
    subop = (CPU->memory[CPU->PC] >> 7u) & 0x0003;
    CPU->rsMux_CTL = (CPU->memory[CPU->PC] >> 9u) & 0x0007;
    //executes comparison based on the sub_opcode
    if(subop == 0) {
        CPU->rtMux_CTL = CPU->memory[CPU->PC] & 0x0007;
        SetNZP(CPU, CPU->R[CPU->rsMux_CTL] - CPU->R[CPU->rtMux_CTL]);
    }
    else if(subop == 1) {
        unsigned short int nzp = 0;
        CPU->rtMux_CTL = CPU->memory[CPU->PC] & 0x0007;
        //statements to evaluate an unsigned comaprison
        if (CPU->R[CPU->rsMux_CTL] < CPU->R[CPU->rtMux_CTL]) {
            nzp = nzp + 4;
        }
        else if (CPU->R[CPU->rsMux_CTL] > CPU->R[CPU->rtMux_CTL]) {
            nzp = nzp + 1;
        }
        else {
            nzp = nzp + 2;
        }
        CPU->NZPVal = nzp;
        CPU->PSR = (CPU->PSR & 0xFFF8) + nzp;
    }
    else if(subop == 2) {
        unsigned short int imm7 = CPU->memory[CPU->PC] & 0x007F;
        imm7 = extend_imm7(imm7);
        SetNZP(CPU, CPU->R[CPU->rsMux_CTL] - imm7);
    }
    else if(subop == 3) {
        unsigned short int nzp;
        unsigned short int uimm7 = CPU->memory[CPU->PC] & 0x007F;
        if (CPU->R[CPU->rsMux_CTL] < uimm7) {
            nzp = nzp + 4;
        }
        else if (CPU->R[CPU->rsMux_CTL] > uimm7) {
            nzp = nzp + 1;
        }
        else {
            nzp = nzp + 2;
        }
        CPU->NZPVal = nzp;
        CPU->PSR = (CPU->PSR & 0xFFF8) + nzp;
    }
    CPU->PC = CPU->PC + 1;

}

/*
 * Parses rest of logical operation and prints out.
 */
void LogicalOp(MachineState* CPU, FILE* output)
{
    //gets the rs and rd consistant in every logical operation
    unsigned short int subop;
    CPU->rdMux_CTL = (CPU->memory[CPU->PC] & 0x0E00) >> 9u;
    CPU->rsMux_CTL = (CPU->memory[CPU->PC] & 0x01C0) >> 6u;
    //executes proper operation based on the sub_opcode
    subop = (CPU->memory[CPU->PC] & 0x0038) >> 3u;
    if (subop < 4) {
        CPU->rtMux_CTL = CPU->memory[CPU->PC] & 0x0007;
        if(subop == 0) {
            CPU->R[CPU->rdMux_CTL] = CPU->R[CPU->rsMux_CTL] & CPU->R[CPU->rtMux_CTL];
        }
        else if(subop == 1) {
            CPU->R[CPU->rdMux_CTL] = ~(CPU->R[CPU->rsMux_CTL]);
        }
        else if(subop == 2) {
            CPU->R[CPU->rdMux_CTL] = CPU->R[CPU->rsMux_CTL] | CPU->R[CPU->rtMux_CTL];
        }
        else if(subop == 3) {
            CPU->R[CPU->rdMux_CTL] = CPU->R[CPU->rsMux_CTL] ^ CPU->R[CPU->rtMux_CTL];
        }
    }
    else {
        unsigned short int imm5 = (CPU->memory[CPU->PC] & 0x001F);
        imm5 = extend_imm5(imm5);
        CPU->R[CPU->rdMux_CTL] = CPU->R[CPU->rsMux_CTL] & imm5;
    }
    SetNZP(CPU, CPU->R[CPU->rdMux_CTL]);
    CPU->PC = CPU->PC + 1;    
}

/*
 * Parses rest of jump operation and prints out.
 */
void JumpOp(MachineState* CPU, FILE* output)
{
    unsigned short int subop = (CPU->memory[CPU->PC] & 0x0800) >> 11u;
    if (subop == 0) {
        CPU->rsMux_CTL = (CPU->memory[CPU->PC] & 0x01C0) >> 6u;
        CPU->PC = CPU->rsMux_CTL;
    }
    else {
        unsigned short int imm11 = (CPU->memory[CPU->PC] & 0x07FF);
        imm11 = extend_imm11(imm11);
        CPU->PC = CPU->PC + 1 + imm11;
    }
}

/*
 * Parses rest of JSR operation and prints out.
 */
void JSROp(MachineState* CPU, FILE* output)
{
    unsigned short int subop = (CPU->memory[CPU->PC] & 0x0800) >> 11u;
    if (subop == 1) {
        unsigned short int imm11 = (CPU->memory[CPU->PC] & 0x07FF);
        imm11 = extend_imm11(imm11);
        CPU->R[7] = CPU->PC +1;
        CPU->PC = (CPU->PC & 0x8000) | (imm11 << 4u);
    }
    else {
        CPU->rsMux_CTL = (CPU->memory[CPU->PC] & 0x01C0) >> 6u;
        CPU->R[7] = CPU->PC +1;
        CPU->PC = CPU->rsMux_CTL;
    }    
}

/*
 * Parses rest of shift/mod operations and prints out.
 */
void ShiftModOp(MachineState* CPU, FILE* output)
{
    //gets rd and rs and uimm4 used to shift
    unsigned short int subop, uimm4;
    subop = (CPU->memory[CPU->PC] & 0x0030) >> 4u;
    CPU->rdMux_CTL = (CPU->memory[CPU->PC] & 0x0E00) >> 9u;
    CPU->rsMux_CTL = (CPU->memory[CPU->PC] & 0x01C0) >> 6u;
    uimm4 = (CPU->memory[CPU->PC] & 0x000F);
    //based on the sub_opcode parses rest of shift or mod operation
    if (subop == 0) {
        CPU->R[CPU->rdMux_CTL] = CPU->R[CPU->rsMux_CTL] << uimm4;
    }
    else if (subop == 1) {
        unsigned short int sign = CPU->memory[CPU->PC] & 0x8000;
        if (sign > 0) {
            CPU->R[CPU->rdMux_CTL] = CPU->R[CPU->rsMux_CTL] >> uimm4;
            CPU->R[CPU->rdMux_CTL] |= ~(~0U >> uimm4);
        }
        else {
            CPU->R[CPU->rdMux_CTL] = CPU->R[CPU->rsMux_CTL] >> uimm4;
        }
    }
    else if (subop == 2) {
        CPU->R[CPU->rdMux_CTL] = CPU->R[CPU->rsMux_CTL] >> uimm4;
    }
    else if (subop == 3) {
        CPU->rtMux_CTL = uimm4 & 0x0007;
        CPU->R[CPU->rdMux_CTL] = CPU->R[CPU->rsMux_CTL] % CPU->R[CPU->rtMux_CTL];
    }
    SetNZP(CPU, CPU->R[CPU->rdMux_CTL]);
    CPU->PC = CPU->PC + 1;  
}


//parses the rest of a load operation
int LoadOp(MachineState* CPU, FILE* output) {
    unsigned short int imm6 = CPU->memory[CPU->PC] & 0x003F;
    imm6 = extend_imm6(imm6);
    CPU->rsMux_CTL = (CPU->memory[CPU->PC] >> 6u) & 0x0007;
    if (CheckPermissions(CPU, CPU->R[CPU->rsMux_CTL] + imm6) != 0) {
        return 1;
    }
    CPU->rdMux_CTL = (CPU->memory[CPU->PC] >> 9u) & 0x0007;
    CPU->dmemAddr = CPU->R[CPU->rsMux_CTL] + imm6;
    CPU->dmemValue = CPU->memory[CPU->R[CPU->rsMux_CTL] + imm6];
    CPU->R[CPU->rdMux_CTL] = CPU->memory[CPU->R[CPU->rsMux_CTL] + imm6];
    SetNZP(CPU, CPU->R[CPU->rdMux_CTL]);
    CPU->PC = CPU->PC + 1;
    return 0;
}


//parses the rest of a store operation
int StoreOp(MachineState* CPU, FILE* output) {
    unsigned short int imm6 = CPU->memory[CPU->PC] & 0x003F;
    imm6 = extend_imm6(imm6);
    CPU->rsMux_CTL = (CPU->memory[CPU->PC] >> 6u) & 0x0007;
    if (CheckPermissions(CPU, CPU->R[CPU->rsMux_CTL] + imm6) != 0) {
        return 1;
    }
    CPU->rtMux_CTL = (CPU->memory[CPU->PC] >> 9u) & 0x0007;
    CPU->dmemAddr = CPU->R[CPU->rsMux_CTL] + imm6;
    CPU->memory[CPU->R[CPU->rsMux_CTL] + imm6] = CPU->R[CPU->rtMux_CTL];
    CPU->dmemValue = CPU->R[CPU->rtMux_CTL];
    CPU->PC = CPU->PC + 1;
    return 0;
}

//parses the constant operation
void ConstOp(MachineState* CPU, FILE* output) {
    unsigned short int imm9 = CPU->memory[CPU->PC] & 0x01FF;
    imm9 = extend_imm9(imm9);
    CPU->rdMux_CTL = (CPU->memory[CPU->PC] >> 9u) & 0x0007;
    CPU->R[CPU->rdMux_CTL] = imm9;
    SetNZP(CPU, CPU->R[CPU->rdMux_CTL]);
    CPU->PC = CPU->PC + 1;
}

//parses the hiconstant operation
void HiconstOp(MachineState* CPU, FILE* output) {
    unsigned short int uimm8 = CPU->memory[CPU->PC] & 0x00FF;
    CPU->rdMux_CTL = (CPU->memory[CPU->PC] >> 9u) & 0x0007;
    CPU->R[CPU->rdMux_CTL] = (CPU->R[CPU->rdMux_CTL] & 0x00FF)|(uimm8 << 8u);
    SetNZP(CPU, CPU->R[CPU->rdMux_CTL]);
    CPU->PC = CPU->PC + 1;
}

//parses the trap operation
void TrapOp(MachineState* CPU, FILE* output) {
    unsigned short int uimm8 = CPU->memory[CPU->PC] & 0x00FF;
    CPU->R[7] = CPU->PC + 1;
    CPU->rdMux_CTL = 7;
    SetNZP(CPU, CPU->PC + 1);
    CPU->PC = (0x8000 | uimm8);
    CPU->PSR = CPU->PSR | 0x8000;
}

//parses the rti operation
void RtiOp(MachineState* CPU, FILE* output) {
    CPU->PC = CPU->R[7];
    CPU->PSR = CPU->PSR & 0x7FFF;
}


/*
 * Set the NZP bits in the PSR.
 */
void SetNZP(MachineState* CPU, short result)
{
    unsigned short int nzp = 0;
    if ((result & 0x8000) > 0) {
        nzp = nzp + 4;
    }
    else if (result == 0) {
        nzp = nzp + 2;
    }
    else if ((result & 0x8000) == 0) {
        nzp = nzp + 1;
    }
    CPU->NZPVal = nzp;
    CPU->PSR = (CPU->PSR & 0xFFF8) + nzp;
}

//each of these functions evaluates whether the imm(x) value is positive or negative and extends the bits if necessary
unsigned short int extend_imm9(unsigned short int result) {
    if (result & 0x100) {
        result |= 0xFE00;
    }
    return result;
}

unsigned short int extend_imm5(unsigned short int result) {
    if (result & 0x10) {
        result |= 0xFFE0;
    }
    return result;
}

unsigned short int extend_imm7(unsigned short int result) {
    if (result & 0x40) {
        result |= 0xFF80;
    }
    return result;
}

unsigned short int extend_imm6(unsigned short int result) {
    if (result & 0x20) {
        result |= 0xFFC0;
    }
    return result;
}

unsigned short int extend_imm11(unsigned short int result) {
    if (result & 0x400) {
        result |= 0xF800;
    }
    return result;
}


//checks to see if the provided address is in the range of the system
int CheckPermissions(MachineState* CPU, unsigned short int address) {
    unsigned short int psr15 = CPU->PSR >> 15u;
    if (psr15 == 0 && address > 0x7FFF && address < 0xFFFF) {
        return 1;
    }
    if (psr15 == 1 && address < 0x8000 && address >= 0x0000) {
        return 1;
    }
    return 0;
}

