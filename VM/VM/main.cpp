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
enum operators {ADD, AND, BR, JMP, RET, JSR, JSRR,
                LD, LDI, LDR, LEA, NOT, RTI, ST,
                STI, STR, TRAP,
};

uint16_t executeInstruction(uint16_t instruction){
    
    
    return NULL;
}





int main(int argc, const char * argv[]) {
    // insert code here...
    uint16_t instructions[24]; //TODO
    for(auto instruction : instructions){
        executeInstruction(instruction);
    }
    
    std::cout << "Hello, World!\n";

    uint16_t memory[INSTRUCTION_SIZE];
    //uint16_t registers[];
    
    
    return 0;
}
