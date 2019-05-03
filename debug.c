#include "debug.h"
#include <stdint.h>

/*
 * Struct or Enum Declarations and Definition
 */

// Instruction 정보를 저장함.
typedef struct instruction{
    bool extend;
    unsigned char opcode;
    struct {
        union{
            struct{
                uint16_t r2     : 4;
                uint16_t r1     : 4;
                uint16_t opcode : 8;
            } p2;

            struct{
                uint32_t address: 12;
                uint32_t e      : 1;
                uint32_t p      : 1;
                uint32_t b      : 1;
                uint32_t x      : 1;
                uint32_t i      : 1;
                uint32_t n      : 1;
                uint32_t opcode : 6;
            } p3;

            struct{
                uint32_t address: 20;
                uint32_t e      : 1;
                uint32_t p      : 1;
                uint32_t b      : 1;
                uint32_t x      : 1;
                uint32_t i      : 1;
                uint32_t n      : 1;
                uint32_t opcode : 6;
            } p4;
            uint32_t val;
        } param;
        bool extend;
    } param;
} Instruction;

// Operator 이름과 opcode 를 매핑
typedef enum {
    LDA = 0x00,
    LDB = 0x68,
    LDT = 0x74,
    LDX = 0x04,
    LDCH = 0x50,
    STA = 0x0C,
    STL = 0x14,
    STX  = 0x10,
    STCH = 0x54,
    JSUB = 0x48,
    JEQ = 0x30,
    JGT = 0x34,
    JLT = 0x38,
    J = 0x3C,
    COMP = 0x28,
    TD = 0xE0,
    RD = 0xD8,
    RSUB = 0x4C,
    WD = 0xDC,
    CLEAR = 0xB4,
    COMPR = 0xA0,
    TIXR = 0xB8
} Operator;

// Addressing mode
typedef enum {
    ENUM_IMMEDIATE_ADDRESSING,
    ENUM_SIMPLE_ADDRESSING,
    ENUM_INDIRECT_ADDRESSING,
    ENUM_ADDRESSING_ERROR
} ADDRESSING_MODE;

/*
 * Static Function Declarations
 */

/* loader_linker 함수 구현을 위한 함수들 */

// pass1
static bool loader_linker_pass1(Debugger *debugger);

// pass2
static bool loader_linker_pass2(Debugger *debugger, Memories *memories);

// pass1 sub function
static bool loader_linker_pass1_one(Debugger *debugger, int file_num, int *csaddr);

// pass2 sub function
static bool loader_linker_pass2_one(Debugger *debugger, Memories *memories, int file_num , int *csaddr);

// 생성자 함수
static LoadInfoList* construct_load_info_list();

// load 된 정보를 출력함.
static void print_load_infos(LoadInfoList *load_infos);

/* run 함수를 구현을 위한 함수들 */

// 레지스터 상태를 출력함.
static void print_registers(Registers* registers);

// 명령을 실행함.
static bool execute_operator(Debugger *debugger, Memories *memories, Instruction *instruction);

// TA 계산함.
static uint32_t calculate_TA(Instruction* instruction, Registers* registers);

// Addressing Mode 계산함
static ADDRESSING_MODE calculate_addressing_mode(Instruction* instruction, bool jump_op);

// reg_id 로 레지스터를 찾아서 리턴함.
static uint32_t *get_reg_by_id(Registers *registers, int reg_id);

// BP 를 핸들링함.
static bool handling_bp(Debugger *debugger, int instruction_size);

// 메모리로 부터 값을 가져옴.
static bool load_from_memory(Debugger *debugger, Memories *memories, Instruction *instruction, uint32_t *value,
                             size_t bytes, bool jump_op);

// 메모리에 값을 저장함.
static bool store_to_memory(Debugger *debugger, Memories *memories, Instruction *instruction, uint32_t value,
                            size_t bytes);

// 레지터로 부터 값을 가져옴.
static bool load_from_register(Debugger *debugger, int reg_id, uint32_t *val);

// 레지스터에 값을 저장함.
static bool store_to_register(Debugger *debugger, int reg_id, uint32_t val);

/*
 * 함수 상세 설명(주석)은 declaration 파트 참고
 * Function Definitions (public)
 */
Debugger* construct_debugger(){
    Debugger* debugger = (Debugger*)malloc(sizeof(*debugger));

    debugger->start_address = 0; // default: 0
    debugger->bp_count = 0;      // default: 0
    debugger->run_count = 0;  // default: 0
    debugger->registers = construct_registers();
    debugger->previous_bp = -1;
    debugger->end_address = MAX_BP_NUM - 1;
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
    registers->SW = 0;

    return registers;
}

bool destroy_registers(Registers** registers){
    free(*registers);
    return true;
}

void reset_registers(Registers* registers){
    memset (registers, 0, sizeof (*registers));
    registers->L = 0x00FFFFFF;
}

bool loader_linker(Debugger *debugger, Memories *memories){
    assert(debugger->filenames[0]);
    assert(debugger->file_count > 0);

    bool status;

    status = loader_linker_pass1(debugger);
    if(!status) return false;

    status = loader_linker_pass2(debugger, memories);
    if(!status) return false;

    print_load_infos(debugger->load_infos);

    destroy_symbol_table(&(debugger->estab));

    return true;
}

bool run(Debugger *debugger, Memories *memories){
    Registers* registers = debugger->registers;
    uint32_t tmp;
    bool is_continue = debugger->is_running;

    if(!debugger->is_running){
        tmp = registers->PC;
        reset_registers(registers);
        registers->PC = tmp;
    }

    debugger->is_running = true;

    while (registers->PC != 0x00FFFFFF){
        uint8_t opcode;
        uint32_t instruction_val = 0;
        uint8_t memory_val;
        int instruction_size;

        enum op_format format;
        memory_val = (uint8_t)memories->data[registers->PC].value;

        instruction_val = memory_val;
        opcode = memory_val & 0xFC;
        format = op_format_by_op_num(opcode);

        switch (format){
            case OP_FORMAT_1:
                instruction_size = 1;
                break;
            case OP_FORMAT_2_GEN:
                memory_val = (unsigned char)memories->data[registers->PC + 1].value;
                instruction_size = 2;
                instruction_val = (instruction_val << 8) + memory_val;
                break;
            case OP_FORMAT_3_4_GEN:
                memory_val = (unsigned char)memories->data[registers->PC + 1].value;
                instruction_val = (instruction_val << 8) + memory_val;
                memory_val = (unsigned char)memories->data[registers->PC + 2].value;
                instruction_val = (instruction_val << 8) + memory_val;
                instruction_size = 3;
                if (instruction_val & (1 << 12)){
                    memory_val = (unsigned char)memories->data[registers->PC + 3].value;
                    instruction_val = (instruction_val << 8) + memory_val;
                    ++instruction_size;
                }
                break;
            default:
                return false;
        }

        if (!is_continue){
            bool is_break = handling_bp(debugger, instruction_size);
            if(is_break) return true;
        }

        Instruction instruction;
        instruction.opcode = opcode;
        instruction.extend = (instruction_size == 4);
        instruction.param.param.val = instruction_val;

        bool status;
        registers->PC += instruction_size;

        status = execute_operator(debugger, memories, &instruction);
        if(!status) return false;
        is_continue = false;
    }

    registers->PC = debugger->start_address + (uint32_t)debugger->load_infos->list[0].length;
    print_registers(registers);

    debugger->previous_bp = -1;
    printf("End program\n");

    debugger->is_running = false;
    return true;
}


/*
 * 함수 상세 설명(주석)은 declaration 파트 참고
 * Static Function Definitions
 */
static bool loader_linker_pass1(Debugger *debugger) {
    debugger->estab = construct_symbol_table();
    debugger->load_infos = construct_load_info_list();

    int csaddr = (int)debugger->start_address;

    for(int i = 0; i < debugger->file_count; i++){
        int status;
        status = loader_linker_pass1_one(debugger, i, &csaddr);
        if(!status) return false;
    }

    return true;
}

static bool loader_linker_pass1_one(Debugger *debugger, int file_num, int *csaddr) {
    assert(file_num >= 0 && file_num <= 2);
    assert(debugger->filenames[file_num]);

    FILE *fp;
    char buf[1010];
    bool is_header = false;
    int base_address = *csaddr;

    fp = fopen (debugger->filenames[file_num], "rt");
    if(!fp){
        fprintf(stderr, "[ERROR] Can't file open\n");
        return false;
    }

    while (fgets (buf, 1000, fp) != NULL){
        if(buf[0] != 'H' && buf[0] != 'D') continue;
        if (buf[0] == 'H'){

            if (is_header) return false;
            else is_header = true;

            int tmp;

            LoadInfoList* load_infos = debugger->load_infos;
            load_infos->list[load_infos->count].type = INFO_TYPE_CONTROL_SECTION;

            sscanf (buf, "H%6s%06X%06X",
                    load_infos->list[load_infos->count].name,
                    &tmp, &load_infos->list[load_infos->count].length);
            load_infos->list[load_infos->count].addr = base_address;

            *csaddr += load_infos->list[load_infos->count].length;
            load_infos->count += 1;

            continue;
        }
        if (buf[0] == 'D'){
            int rec_len = strlen(buf);
            rec_len -= 2;
            if (rec_len % 12 != 0) return false;
            LoadInfoList* load_infos = debugger->load_infos;

            for (int i = 0; i < rec_len / 12; i++){
                int offset;
                Symbol* symbol = construct_symbol();

                load_infos->list[load_infos->count].type = INFO_TYPE_SYMBOL;

                sscanf (buf + 1 + i * 12,
                        "%6s%06X",
                        load_infos->list[load_infos->count].name,
                        &offset);
                load_infos->list[load_infos->count].addr = base_address + offset;

                strncpy (symbol->label,
                        load_infos->list[load_infos->count].name,
                        10);

                symbol->location_counter = load_infos->list[load_infos->count].addr;

                insert_symbol(debugger->estab, symbol);

                load_infos->count += 1;
            }
            continue;
        }
    }

    if(!is_header){
        fprintf(stderr, "[ERROR] Object file is wrong\n");
        return false;
    }

    fclose(fp);

    return true;
}

static bool loader_linker_pass2(Debugger *debugger, Memories *memories){
    int csaddr = (int)debugger->start_address;

    for(int i = 0; i < debugger->file_count; i++){
        int status;
        status = loader_linker_pass2_one(debugger, memories, i, &csaddr);
        if(!status) return false;
    }

    return true;
}

static bool loader_linker_pass2_one(Debugger *debugger, Memories *memories, int file_num , int *csaddr){
    assert(file_num >= 0 && file_num <= 2);
    assert(debugger->filenames[file_num]);

    FILE *fp;

    fp = fopen (debugger->filenames[file_num], "rt");
    if(!fp){
        fprintf(stderr, "[ERROR] Can't file open\n");
        return false;
    }

    char ch_stat[128] = {};
    ch_stat['H'] = 0;
    ch_stat['R'] = 0;
    ch_stat['T'] = 1;
    ch_stat['M'] = 2;

    char buf[1001];
    int state = 0;
    int num_to_address[256];
    bool num_validation[256] = { false, };
    int base_addr = *csaddr;

    num_validation[1] = true;
    num_to_address[1] = base_addr;

    while (fgets (buf, 1000, fp) != NULL){
        if(buf[0] != 'H' && buf[0] != 'R' && buf[0] != 'T' && buf[0] != 'M')
            continue;

        if(state > ch_stat[(unsigned char)buf[0]]) return false;
        state = ch_stat[(unsigned char)buf[0]];

        if (buf[0] == 'H'){
            char name[8];
            unsigned int hex, length;

            sscanf (buf, "H%6s%06X%06X", name, &hex, &length);
            *csaddr += length;
            continue;
        }
        if (buf[0] == 'R'){
            int num;
            char label[8];
            int cnt;
            char *ptr = buf + 1;

            while (sscanf (ptr, "%02X%6s%n", &num, label, &cnt) > 0){
                ptr += cnt;

                const Symbol *symbol = find_symbol_by_name (debugger->estab, label);
                if (!symbol) return false;

                num_to_address[num] = symbol->location_counter;
                num_validation[num] = true;
            }
            continue;
        }
        if (buf[0] == 'T'){

            int start_offset;
            int length;

            sscanf (buf, "T%06X%02X", &start_offset, &length);
            for (int i = 0; i < length; ++i){
                unsigned int val_c;

                sscanf (buf + 9 + i * 2, "%02X", &val_c);
                edit_memory (memories, base_addr + start_offset + i, val_c);
            }
            continue;
        }
        if (buf[0] == 'M'){

            int offset;
            int length, num = 0xFFFFFFFF;
            char op_char = 0;

            sscanf (buf, "M%06X%02X%c%02X", &offset, &length, &op_char, &num);

            if ((unsigned)num == 0xFFFFFFFF){
                op_char = '+';
                num = 0x01;
            }

            if (!num_validation[num]) return false;

            const unsigned int old_value = num_to_address[num];
            unsigned int new_value = 0;

            for (int i = 0; i < 3; ++i){
                new_value <<= 8;
                new_value += (unsigned char)memories->data[base_addr+offset+i].value;
            }

            const int sum_new_old_value = new_value + old_value;
            const int sub_new_old_value = new_value - old_value;

            if (length == 5){
                if (op_char == '+')
                    new_value = (sum_new_old_value & 0xFFFFF) + ((new_value >> 20) << 20);
                else if (op_char == '-')
                    new_value = (sub_new_old_value & 0xFFFFF) + ((new_value >> 20) << 20);
                else return false;
            }else if (length == 6){
                if (op_char == '+') new_value = sum_new_old_value;
                else if (op_char == '-') new_value = sub_new_old_value;
                else return false;
            }

            edit_memory (memories, base_addr + offset, (unsigned char)(new_value >> 16));
            edit_memory (memories, base_addr + offset + 1, (unsigned char)(new_value >> 8));
            edit_memory (memories, base_addr + offset + 2,(unsigned char)new_value);
        }
    }

    fclose(fp);
    return true;
}

static LoadInfoList* construct_load_info_list(){
    LoadInfoList* loadInfoList;
    loadInfoList = (LoadInfoList*)malloc(sizeof(LoadInfoList));
    loadInfoList->count = 0;
    return loadInfoList;
}

static void print_load_infos(LoadInfoList *load_infos){
    unsigned int total_length = 0;
    const int count = load_infos->count;

    printf ("%-15s %-15s %-15s %-15s\n", "Control", "Symbol", "Address", "Length");
    printf ("%-15s %-15s %-15s %-15s\n", "Section", "Name", "", "");
    printf ("------------------------------------------------------------\n");
    for(int i=0; i<count; i++){
        const LoadInfoNode node = load_infos->list[i];
        if (node.type == INFO_TYPE_CONTROL_SECTION){
            printf ("%-15s %-15c %04X%11c %04X%11c\n",
                    node.name,
                    ' ', node.addr,
                    ' ', node.length, ' ');
            total_length += node.length;
            continue;
        }
        if (node.type == INFO_TYPE_SYMBOL){
            printf ("%-15c %-15s %04X%11c %-15c\n", ' ',
                    node.name, node.addr,
                    ' ', ' ');
        }
    }
    printf ("------------------------------------------------------------\n");
    printf ("%-15c %-15c %-15s %04X%11c\n", ' ', ' ', "Total Length", total_length, ' ');
}

static bool execute_operator(Debugger *debugger, Memories *memories, Instruction *instruction) {
    assert(debugger);
    assert(memories);
    assert(instruction);

    Registers* registers = debugger->registers;
    uint32_t value;
    uint32_t val1, val2;
    static size_t device_input_idx = 0;
    char inputDevice[] = "    I am Test Device\0\0";// virtual input device

    switch (instruction->opcode){
        case LDA:
            load_from_memory(debugger, memories, instruction, &registers->A, 3, false);
            break;
        case LDB:
            load_from_memory(debugger, memories, instruction, &registers->B, 3, false);
            break;
        case LDT:
            load_from_memory(debugger, memories, instruction, &registers->T, 3, false);
            break;
        case LDX:
            load_from_memory(debugger, memories, instruction, &registers->X, 3, false);
            break;
        case LDCH:
            load_from_memory(debugger, memories, instruction, &value, 1, false);
            registers->A = (registers->A & 0xFFFFFF00) + (value & 0xFF);
            break;
        case STA:
            store_to_memory(debugger, memories, instruction, registers->A, 3);
            break;
        case STL:
            store_to_memory(debugger, memories, instruction, registers->L, 3);
            break;
        case STX:
            store_to_memory(debugger, memories, instruction, registers->X, 3);
            break;
        case STCH:
            store_to_memory(debugger, memories, instruction, registers->A & 0xFF, 1);
            break;
        case JSUB:
            load_from_memory(debugger, memories, instruction, &value, 3, true);
            registers->L = registers->PC;
            registers->PC = value;
            break;
        case JEQ:
            if (registers->SW == 0) load_from_memory(debugger, memories, instruction, &registers->PC, 3, true);
            break;
        case JGT:
            if ((int) registers->SW > 0) load_from_memory(debugger, memories, instruction, &registers->PC, 3, true);
            break;
        case JLT:
            if ((int)registers->SW < 0) load_from_memory(debugger, memories, instruction, &registers->PC, 3, true);
            break;
        case J:
            load_from_memory(debugger, memories, instruction, &registers->PC, 3, true);
            break;
        case COMP:
            load_from_memory(debugger, memories, instruction, &value, 3, false);
            if (registers->A > value)
                registers->SW = 1;
            else if (registers->A < value)
                registers->SW = -1;
            else
                registers->SW = 0;
            break;
        case TD:
            registers->SW = 1;
            break;
        case RD:
            registers->A = (registers->A & 0xFFFFFF00) + (unsigned char)inputDevice[device_input_idx++];
            if (device_input_idx >= sizeof(inputDevice) / sizeof(char))
                device_input_idx = 0;
            break;
        case RSUB:
            registers->PC = registers->L;
            break;
        case WD:
            // is done!
            break;
        case CLEAR:
            store_to_register(debugger, instruction->param.param.p2.r1, 0);
            break;
        case COMPR:
            load_from_register(debugger, instruction->param.param.p2.r1, &val1);
            load_from_register(debugger, instruction->param.param.p2.r2, &val2);
            if (val1 > val2) registers->SW = 1;
            else if (val1 < val2) registers->SW = -1;
            else registers->SW = 0;
            break;
        case TIXR:
            load_from_register(debugger, instruction->param.param.p2.r1, &value);
            ++registers->X;

            if (registers->X > value) registers->SW = 1;
            else if (registers->X < value) registers->SW = -1;
            else registers->SW = 0;
            break;
        default:
            return false;
    }

    return true;
}
static uint32_t calculate_TA(Instruction* instruction, Registers* registers){
    uint32_t TA;

    uint32_t b = (instruction->extend ? instruction->param.param.p4.b : instruction->param.param.p3.b);
    uint32_t p = (instruction->extend ? instruction->param.param.p4.p : instruction->param.param.p3.p);
    uint32_t address = (instruction->extend ? instruction->param.param.p4.address : instruction->param.param.p3.address);
    uint32_t x = (instruction->extend ? instruction->param.param.p4.x : instruction->param.param.p3.x);

    if (b == 1 && p == 0){
        // Base Relative
        TA = address + registers->B;
    } else if (b == 0 && p == 1){
        // PC Relative
        int32_t val;
        uint32_t bound;

        if(instruction->extend) bound = (1 << 19);
        else bound = (1 << 11);

        if(address >= bound) val = address - (bound << 1);
        else val = address;

        TA = registers->PC + val;
    }else {
        TA = address;
    }

    if (x == 1){
        TA += registers->X;
    }

    return TA;
}
static bool handling_bp(Debugger *debugger, int instruction_size) {
    bool is_break = false;
    unsigned int bp;

    for(unsigned int i = debugger->registers->PC; i < debugger->registers->PC + instruction_size; i++){
        if(debugger->break_points[i]){
            if(debugger->previous_bp != -1 && i == (unsigned int)debugger->previous_bp)
                continue;
            is_break = true;
            bp = i;
            debugger->previous_bp = bp;
            break;
        }
    }
    if(is_break){
        print_registers(debugger->registers);
        printf ("Stop at checkpoint[%04X]\n", bp);
        return true;
    }
    return false;
}

static ADDRESSING_MODE calculate_addressing_mode(Instruction* instruction, bool jump_op){
    uint32_t n = (instruction->extend ? instruction->param.param.p4.n : instruction->param.param.p3.n);
    uint32_t i = (instruction->extend ? instruction->param.param.p4.i : instruction->param.param.p3.i);

    if(jump_op){
        if(n == 1 && i == 1) return ENUM_IMMEDIATE_ADDRESSING;
        if(n == 1 && i == 0) return ENUM_SIMPLE_ADDRESSING;
        return ENUM_ADDRESSING_ERROR;
    }

    if(n == 0 && i == 1) return ENUM_IMMEDIATE_ADDRESSING;
    if(n == 1 && i == 1) return ENUM_SIMPLE_ADDRESSING;
    if(n == 1 && i == 0) return ENUM_INDIRECT_ADDRESSING;

    return ENUM_ADDRESSING_ERROR;
}
static bool load_from_memory(Debugger *debugger, Memories *memories, Instruction *instruction, uint32_t *value,
                             size_t bytes, bool jump_op){
    uint32_t TA;
    ADDRESSING_MODE addr_mode;

    TA = calculate_TA(instruction, debugger->registers);
    addr_mode = calculate_addressing_mode(instruction, jump_op);

    if (addr_mode == ENUM_IMMEDIATE_ADDRESSING){
        *value = TA;
        return true;
    }

    if (addr_mode == ENUM_SIMPLE_ADDRESSING){
        uint8_t memory_val;
        *value = 0;
        for (size_t k = 0; k < bytes; ++k){
            memory_val = (uint8_t)memories->data[TA + k].value;

            *value = (*value << 8) + memory_val;
        }
        return true;
    }

    if (addr_mode == ENUM_INDIRECT_ADDRESSING) {
        uint32_t indirect_address = 0;
        uint8_t memory_val;

        for (int k = 0; k < 3; ++k){
            memory_val = (uint8_t)memories->data[TA + k].value;
            indirect_address = (indirect_address << 8) + memory_val;
        }

        *value = 0;
        for (size_t k = 0; k < bytes; ++k){
            memory_val = (uint8_t)memories->data[indirect_address + k].value;
            *value = (*value << 8) + memory_val;
        }
        return true;
    }

    return false;
}

static bool store_to_memory(Debugger *debugger, Memories *memories, Instruction *instruction, uint32_t value,
                            size_t bytes){
    uint32_t TA;
    ADDRESSING_MODE addr_mode;

    TA = calculate_TA(instruction, debugger->registers);
    addr_mode = calculate_addressing_mode(instruction, false);

    if(addr_mode == ENUM_IMMEDIATE_ADDRESSING){
        return false;
    }

    if(addr_mode == ENUM_SIMPLE_ADDRESSING){
        for (int k = bytes-1; k >= 0; --k){
            edit_memory(memories, TA + k, (uint8_t)value);
            value >>= 8;
        }
        return true;
    }

    if(addr_mode == ENUM_INDIRECT_ADDRESSING){
        uint8_t mem_val;
        uint32_t addr = 0;
        for (int k = 0; k < 3; ++k){
            mem_val = (uint8_t)memories->data[TA + k].value;
            addr = (addr << 8) + mem_val;
        }
        for (int k = 2; k >= 0; --k){
            edit_memory(memories, addr + k, (uint8_t)value);
            value >>= 8;
        }
        return true;
    }

    return false;
}

static bool load_from_register(Debugger *debugger, int reg_id, uint32_t *val){
    uint32_t* reg = get_reg_by_id(debugger->registers, reg_id);
    if(reg == NULL) return false;

    *val = *reg;

    return true;
}
static uint32_t *get_reg_by_id(Registers *registers, int reg_id) {
    if(reg_id == 6 || reg_id == 7) return NULL;
    if(reg_id < 0 || reg_id > 9) return NULL;

    uint32_t* regs[10] = {
            [0] = &registers->A,
            [1] = &registers->X,
            [2] = &registers->L,
            [3] = &registers->B,
            [4] = &registers->S,
            [5] = &registers->T,
            [6] = NULL,
            [7] = NULL,
            [8] = &registers->PC,
            [9] = &registers->SW
    };

    return regs[reg_id];
}
static bool store_to_register(Debugger *debugger, int reg_id, uint32_t val){
    uint32_t* reg = get_reg_by_id(debugger->registers, reg_id);
    if(reg == NULL) return false;

    *reg = val;
    return true;
}
static void print_registers(Registers* registers){
    printf (
            "A : %06X  X : %06X \n"
            "L : %06X  PC: %06X \n"
            "B : %06X  S : %06X \n"
            "T : %06X           \n",
            registers->A, registers->X,
            registers->L, registers->PC,
            registers->B, registers->S,
            registers->T
    );
}