//
//  main.cpp
//  VM
//
//  Created by Hunter Rasmussen on 6/30/18.
//  Copyright © 2018 Hunter Rasmussen. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

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
        registers[9] = POS;
    }
}

struct termios original_tio;

void disable_input_buffering()
{
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

void handle_interrupt(int signal) {
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

uint16_t check_key() {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

uint16_t swap16(uint16_t x) { return (x << 8) | (x >> 8); }

void read_image(FILE* imageToRead){
    uint16_t memoryIndex;
    fread(&memoryIndex, sizeof(uint16_t), 1, imageToRead); //stores first line in image into memory index
    memoryIndex = swap16(memoryIndex);
    while(!feof(imageToRead)){
        fread(&memory[memoryIndex], sizeof(uint16_t), 1, imageToRead);
        memory[memoryIndex] = swap16(memory[memoryIndex]);
        memoryIndex++;
    }
    return;
}



int main(int argc, const char * argv[]) {
    
    disable_input_buffering();

    if(argc > 1){
        FILE* imageToRead = fopen(argv[1], "r");  //read only
        read_image(imageToRead);
    }
    else{
        return 0;
    }
    
    registers[8] = 0x3000;
    
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
                    imm5 = sign_extend(imm5, 5);
                    registers[dr] = registers[sr1] + imm5;
                }
                else{
                    registers[dr] = registers[sr1] + registers[sr2];
                    
                }
                updateFlags(dr);
                break;
                
            case Op::AND:
                if(immFlag){
                    imm5 = sign_extend(imm5, 5);
                    registers[dr] = registers[sr1] & imm5;
                }
                else{
                    registers[dr] = registers[sr1] & registers[sr2];
                }
                updateFlags(dr);
                break;
                
            case Op::BR:
                 pcOffset9 = sign_extend(pcOffset9, 9);
                if(brFlags & registers[9]){
                    registers[8] += pcOffset9;
                }
                break;
                
            case Op::JMP:
                registers[8] = registers[sr1];
                break;
                
            case Op::JSR:
                registers[7] = registers[8];
                if(jsrFlag){
                    pcOffset11 = sign_extend(pcOffset11, 11);
                    registers[8] += pcOffset11;
                }
                else{
                    registers[8] = registers[sr1];
                }
                break;
                
            case Op::LD:
                pcOffset9 = sign_extend(pcOffset9, 9);
                memoryAddress1 = registers[8] + pcOffset9;
                registers[dr] = memory[memoryAddress1];
                updateFlags(dr);
                break;
                
            case Op::LDI:
                pcOffset9 = sign_extend(pcOffset9, 9);
                memoryAddress1 = registers[8] + pcOffset9;
                memoryAddress2 = memory[memoryAddress1];
                registers[dr] = memory[memoryAddress2];
                updateFlags(dr);
                break;
                
            case Op::LDR:
                pcOffset6 = sign_extend(pcOffset6, 6);
                memoryAddress1 = registers[sr1] + pcOffset6;
                registers[dr] = memory[memoryAddress1];
                updateFlags(dr);
                break;
                
            case Op::LEA:
                pcOffset9 = sign_extend(pcOffset9, 9);
                registers[dr] = registers[8] + pcOffset9;
                updateFlags(dr);
                break;
                
            case Op::NOT:
                registers[dr] = ~registers[sr1];
                updateFlags(dr);
                break;
                
            case Op::ST:
                pcOffset9 = sign_extend(pcOffset9, 9);
                memoryAddress1 = registers[8] + pcOffset9;
                memory[memoryAddress1] = registers[dr];
                break;
                
            case Op::STI:
                pcOffset9 = sign_extend(pcOffset9, 9);
                memoryAddress1 = registers[8] + pcOffset9;
                memoryAddress2 = memory[memoryAddress1];
                memory[memoryAddress2] = registers[dr];
                break;
                
            case Op::STR:{
                pcOffset6 = sign_extend(pcOffset6, 6);
                uint16_t address = registers[sr1] + pcOffset6;
                memory[address] = registers[dr];
                break;
            }
                
            case Op::TRAP:
                trap(instruction & 0xFF);
                break;
                
            default:
                break;
        }
    }
    
   

    
    
    return 0;
}
