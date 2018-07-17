//
//  main.cpp
//  VM
//
//  Created by Hunter Rasmussen on 6/30/18.
//  Copyright © 2018 Hunter Rasmussen. All rights reserved.
//

#include <iostream>
#include <stdint.h>

const int   INSTRUCTION_SIZE = 16;
uint16_t memory[INSTRUCTION_SIZE];
uint16_t registers[10];

 enum class Op {ADD, AND, BR, JMP, JSR, JSRR,
                LD, LDI, LDR, LEA, NOT, RTI, ST,
                STI, STR, TRAP, REV,
};




Op opTable[] = { Op::BR, Op::ADD, Op::LD, Op::ST, Op::JSR, Op::AND,
                 Op::LDR, Op::STR, Op::RTI, Op::NOT, Op::LDI, Op::STI,
                 Op::JMP, Op::REV, Op::LEA, Op::TRAP };


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



int main(int argc, const char * argv[]) {
    uint16_t instruction = 0;
    
     uint16_t instructionType = instruction >>  12;
    
    Op opType = opTable[instructionType];
    
    uint16_t sr1 = (instruction >> 6) & 0x7;
    
    uint16_t dr = (instruction >> 9) & 0x7;
    
    uint16_t sr2 = instruction &  0x7;
    
    uint16_t imm5 = instruction & 0x1F;
    
    uint16_t immFlag = (instruction >> 5 ) & 0x1;
    
    uint16_t brFlags = (instruction >> 9) & 0x7;
    
    uint16_t pcOffset9 = instruction & 0x1FF;
    
    uint16_t pcOffset11 = instruction & 0x7FF;
    
    uint16_t jsrFlag = (instruction >> 11) & 0x1;
    
    
    
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
        default:
            break;
    }

    
    
    return 0;
}
