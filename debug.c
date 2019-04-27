#include "debug.h"

Debugger* construct_debugger(){
    Debugger* debugger = (Debugger*)malloc(sizeof(*debugger));

    debugger->start_address = 0; // default: 0
    debugger->bp_count = 0;      // default: 0
    debugger->previous_bp = -1;  // default: -1
    debugger->registers = construct_registers();

    return debugger;
}

bool destroy_debugger(Debugger** debugger){
    destroy_registers(&((*debugger)->registers));
    free(*debugger);
    return true;
}

Registers* construct_registers(){
    Registers* registers = (Registers*)malloc(sizeof(*registers));
    registers->A = 0;
    registers->B = 0;
    registers->L = 0;
    registers->PC = 0;
    registers->S = 0;
    registers->T = 0;
    registers->X = 0;

    return registers;
}

bool destroy_registers(Registers** registers){
    free(*registers);
    return true;
}