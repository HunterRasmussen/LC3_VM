//
//  main.cpp
//  VM
//
//  Created by Hunter Rasmussen on 6/30/18.
//  Copyright © 2018 Hunter Rasmussen. All rights reserved.
//

#include <iostream>
#include <stdint.h>

const int   MEM_SIZE = UINT16_MAX;
uint16_t memory[MEM_SIZE];
uint16_t registers[10];

//state of the program
int running = 1;

 enum class Op {ADD, AND, BR, JMP, JSR, JSRR,
                LD, LDI, LDR, LEA, NOT, RTI, ST,
                STI, STR, TRAP, REV,
};

// Trap Codes (OS Calls)
enum
{
    TRAP_GETC = 0x20,
    TRAP_OUT = 0x21,
    TRAP_PUTS = 0x22,
    TRAP_IN = 0x23,
    TRAP_PUTSP = 0x24,
    TRAP_HALT = 0x25
};




Op opTable[] = { Op::BR, Op::ADD, Op::LD, Op::ST, Op::JSR, Op::AND,
                 Op::LDR, Op::STR, Op::RTI, Op::NOT, Op::LDI, Op::STI,
                 Op::JMP, Op::REV, Op::LEA, Op::TRAP };


void trap(uint16_t code)
{
    switch (code)
    {
        case TRAP_GETC:
            // reads a single ASCII char
            registers[0] = (uint16_t)getchar();
            break;
        case TRAP_OUT:
            putc((char)registers[0], stdout);
            fflush(stdout);
            break;
        case TRAP_PUTS:
        {
            int address = registers[0];
            
            while (memory[address])
            {
                putc((char)memory[address], stdout);
                ++address;
            }
            fflush(stdout);
            break;
        }
        case TRAP_IN:
            printf("Enter a character: ");
            registers[0] = (uint16_t)getchar();
            break;
        case TRAP_PUTSP:
        {
            const char* str = (const char*)(memory + registers[0]);
            printf("%s", str);
            break;
        }
        case TRAP_HALT:
            puts("HALT");
            fflush(stdout);
            running = 0;
            break;
    }
}

uint16_t sign_extend(uint16_t x, int bit_count)
{
    // 1000
    // ->
    // 1111 1000
    
    // 0100
    // ->
    // 0000 0100
    if ((x >> (bit_count - 1)) && 1) {
        // extend 1s
        x |= (0xFFFF << bit_count);
    }
    return x;
}

void updateFlags(uint16_t drIndex){
    
    uint16_t POS = 1 << 0;
    uint16_t ZERO = 1 << 1;
    uint16_t NEG = 1 << 2;
    
    
    if(registers[drIndex] == 0x0){
        registers[9] = ZERO;
    }
    else if ((registers[drIndex] >> 15) == 0x1){
        registers[9] = NEG;
    }
    else{
        registers[drIndex] = POS;
    }
}

void read_image(FILE* imageToRead){
    
}



int main(int argc, const char * argv[]) {
    

    if(argc > 0){
        FILE* imageToRead = fopen(argv[1], "r");  //read only
        read_image(imageToRead);
    }
    else{
        return 0;
    }
    
    while (running){
        uint16_t instruction = memory[registers[8]];
        registers[8]++;
        
        uint16_t instructionType = instruction >>  12;
        
        Op opType = opTable[instructionType];
        
        uint16_t sr1 = (instruction >> 6) & 0x7;
        
        uint16_t dr = (instruction >> 9) & 0x7;
        
        uint16_t sr2 = instruction &  0x7;
        
        uint16_t imm5 = instruction & 0x1F;
        
        uint16_t immFlag = (instruction >> 5 ) & 0x1;
        
        uint16_t brFlags = (instruction >> 9) & 0x7;
        
        uint16_t pcOffset6 = instruction & 0x3F;
        
        uint16_t pcOffset9 = instruction & 0x1FF;
        
        uint16_t pcOffset11 = instruction & 0x7FF;
        
        uint16_t jsrFlag = (instruction >> 11) & 0x1;
        
        uint16_t memoryAddress1;
        
        uint16_t memoryAddress2;
        
        switch (opType) {
            case Op::ADD:
                if(immFlag){
                    registers[dr] = registers[sr1] + imm5;
                }
                else{
                    registers[dr] = registers[sr1] + registers[sr2];
                    
                }
                updateFlags(dr);
                break;
                
            case Op::AND:
                if(immFlag){
                    registers[dr] = registers[sr1] & imm5;
                }
                else{
                    registers[dr] = registers[sr1] & registers[sr2];
                }
                updateFlags(dr);
                break;
                
            case Op::BR:
                if((brFlags & registers[9]) != 0){
                    registers[8] += pcOffset9;
                }
                break;
                
            case Op::JMP:
                registers[8] = registers[sr1];
                break;
                
            case Op::JSR:
                registers[7] = registers[8];
                
                if(jsrFlag){
                    registers[8] += pcOffset11;
                }
                else{
                    registers[8] = registers[sr1];
                }
                break;
                
            case Op::LD:
                memoryAddress1 = registers[8] + pcOffset9;
                registers[dr] = memory[memoryAddress1];
                updateFlags(dr);
                break;
                
            case Op::LDI:
                memoryAddress1 = registers[8] + pcOffset9;
                memoryAddress2 = memory[memoryAddress1];
                registers[dr] = memory[memoryAddress2];
                updateFlags(dr);
                break;
                
            case Op::LDR:
                memoryAddress1 = registers[sr1] + pcOffset6;
                registers[dr] = memory[memoryAddress1];
                updateFlags(dr);
                break;
                
            case Op::LEA:
                registers[dr] = registers[8] + pcOffset9;
                updateFlags(dr);
                break;
                
            case Op::NOT:
                registers[dr] = ~registers[sr1];
                updateFlags(dr);
                break;
                
            case Op::ST:
                memoryAddress1 = registers[8] + pcOffset9;
                memory[memoryAddress1] = registers[dr];
                break;
                
            case Op::STI:
                memoryAddress1 = registers[8] + pcOffset9;
                memoryAddress2 = memory[memoryAddress1];
                memory[memoryAddress2] = registers[dr];
                break;
                
            case Op::STR:
                memory[registers[sr1]+pcOffset6] = registers[dr];
                break;
                
            case Op::TRAP:
                trap(instruction);
                break;
                
            default:
                break;
        }
    }
    
   

    
    
    return 0;
}
