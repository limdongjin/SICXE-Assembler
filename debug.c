#include "debug.h"
#include <stdint.h>
/* Struct or Enum */
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
/* Function Declarations */

bool loader_linker_pass1(Debugger *debugger);
bool loader_linker_pass2(Debugger *debugger, Memories *memories);

bool loader_linker_pass1_one(Debugger *debugger, int file_num, int *csaddr);
bool loader_linker_pass2_one(Debugger *debugger, Memories *memories, int file_num , int *csaddr);
LoadInfoList* construct_load_info_list();
bool destroy_load_info_list(LoadInfoList** load_infos);
void print_load_map (LoadInfoList* load_infos);
void print_registers(Registers* registers);
bool execute_operator(Debugger *debugger, Memories *memories, Instruction *instruction);

static bool load_mem(Debugger *debugger, Memories *memories, Instruction *instruction, uint32_t *value,
                     size_t bytes, bool jump_op);
static bool store_mem (Debugger* debugger, Memories* memories, Instruction* instruction, uint32_t value, size_t bytes);
static bool load_reg (Debugger* debugger, int reg_no, uint32_t *reg_val);
static bool store_reg (Debugger* debugger, int reg_no, uint32_t reg_val);
/* Function Definitions */

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

    print_load_map(debugger->load_infos);

    destroy_load_info_list(&(debugger->load_infos));
    destroy_symbol_table(&(debugger->estab));

    return true;
}

bool destroy_load_info_list(LoadInfoList** load_infos){
    assert(load_infos);
    free(*load_infos);

    return true;
}

bool loader_linker_pass1(Debugger *debugger) {
    debugger->estab = construct_symbol_table();
    debugger->load_infos = construct_load_info_list();

    int csaddr = debugger->start_address;

    for(int i = 0; i < debugger->file_count; i++){
        int status;
        status = loader_linker_pass1_one(debugger, i, &csaddr);
        if(!status) return false;
    }

    return true;
}

bool loader_linker_pass1_one(Debugger *debugger, int file_num, int *csaddr) {
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

bool loader_linker_pass2(Debugger *debugger, Memories *memories){
    int csaddr = debugger->start_address;

    for(int i = 0; i < debugger->file_count; i++){
        int status;
        status = loader_linker_pass2_one(debugger, memories, i, &csaddr);
        if(!status) return false;
    }

    return true;
}

bool loader_linker_pass2_one(Debugger *debugger, Memories *memories, int file_num , int *csaddr){
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

LoadInfoList* construct_load_info_list(){
    LoadInfoList* loadInfoList;
    loadInfoList = (LoadInfoList*)malloc(sizeof(LoadInfoList));
    loadInfoList->count = 0;
    return loadInfoList;
}

void print_load_map (LoadInfoList* load_infos){
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

bool run(Debugger *debugger, Memories *memories){
    // op_format_by_op_num(0x11)

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
//        if(registers->PC == 0x00FF0000) break; // 이 코드 추가하면 progaddr 0 일때는 작동함
        uint8_t opcode;
        uint32_t inst_val = 0;
        uint8_t mem_val;
        int inst_size;

        printf("PC(16): %X\n", registers->PC);
//        assert(debugger->start_address <= registers->PC);
        enum op_format format;
        mem_val = (uint8_t)memories->data[registers->PC].value;

        inst_val = mem_val;
        opcode = mem_val & 0xFC;
        format = op_format_by_op_num(opcode);
        fprintf (stderr, "[DEBUG] running opcode %02X, PC = %08X, SW = %08X\n", mem_val, registers->PC, registers->SW);

        switch (format){
            case OP_FORMAT_1:
                inst_size = 1;
                break;
            case OP_FORMAT_2_GEN:
                mem_val = (unsigned char)memories->data[registers->PC + 1].value;
                inst_size = 2;
                inst_val = (inst_val << 8) + mem_val;
                break;
            case OP_FORMAT_3_4_GEN:
                mem_val = (unsigned char)memories->data[registers->PC + 1].value;
                inst_val = (inst_val << 8) + mem_val;
                mem_val = (unsigned char)memories->data[registers->PC + 2].value;
                inst_val = (inst_val << 8) + mem_val;
                inst_size = 3;
                if (inst_val & (1 << 12)){
                    mem_val = (unsigned char)memories->data[registers->PC + 3].value;
                    inst_val = (inst_val << 8) + mem_val;
                    ++inst_size;
                }
                break;
            default:
                return false;
        }

        unsigned int bp;
        if (!is_continue){
            bool is_break = false;
            for(unsigned int i = registers->PC; i < registers->PC + inst_size; i++){
                if(debugger->break_points[i]){
                    is_break = true;
                    bp = i;
                    break;
                }
            }
            if(is_break){
                // bp에 걸린 경우
                printf ("Stop at checkpoint[%04X]\n", bp);
                return true;
            }
        }

        Instruction instruction;
        instruction.opcode = opcode;
        instruction.extend = (inst_size == 4);
        instruction.param.param.val = inst_val;
        bool status;
        printf("inst size: %X\n", inst_size);
        printf("bef PC(16): %X\n", registers->PC);
        registers->PC += inst_size;
        printf("after PC(16): %X\n", registers->PC);

        status = execute_operator(debugger, memories, &instruction);
        if(!status) return false;
        printf("executed PC(16): %X\n", registers->PC);
        print_registers(registers);
        is_continue = false;
    }
    if(registers->PC == 0x00FF0000) registers->PC = 0x00FFFFFF;
    print_registers(registers);
    debugger->is_running = false;
    return true;
}

bool execute_operator(Debugger *debugger, Memories *memories, Instruction *instruction) {
    assert(debugger);
    assert(memories);
    assert(instruction);

    Registers* registers = debugger->registers;
    uint32_t value;
    uint32_t val1, val2;
    static size_t device_input_idx = 0;
    char inputDevice[] = "    I am Test Device\0\0";// virtual input device
//    char outputStream[13] = {'\0'};
    switch (instruction->opcode){
        case LDA:
            printf("lda\n");
            load_mem(debugger, memories, instruction, &registers->A, 3, false);
            break;
        case LDB:
            printf("ldb\n");
            load_mem(debugger, memories, instruction, &registers->B, 3, false);
            break;
        case LDT:
            printf("ldt\n");
            load_mem(debugger, memories, instruction, &registers->T, 3, false);
            break;
        case LDX:
            printf("ldx\n");
            load_mem(debugger, memories, instruction, &registers->X, 3, false);
            break;
        case LDCH:
            printf("ldch\n");
            load_mem(debugger, memories, instruction, &value, 1, false);
            registers->A = (registers->A & 0xFFFFFF00) + (value & 0xFF);
            break;
        case STA:
            printf("sta\n");
            store_mem(debugger, memories, instruction, registers->A, 3);
            break;
        case STL:
            printf("stl\n");
            store_mem (debugger, memories, instruction, registers->L, 3);
            break;
        case STX:
            printf("stx\n");
            store_mem (debugger, memories, instruction, registers->X, 3);
            break;
        case STCH:
            printf("stch\n");
            store_mem (debugger, memories, instruction, registers->A & 0xFF, 1);
            break;
        case JSUB:
            printf("jsub\n");
            load_mem (debugger, memories, instruction, &value, 3, true);
            registers->L = registers->PC;
            registers->PC = value;
            break;
        case JEQ:
            printf("jeq\n");
            if (registers->SW == 0) load_mem(debugger, memories, instruction, &registers->PC, 3, true);
            break;
        case JGT:
            printf("jgt\n");
            if ((int) registers->SW > 0) load_mem (debugger, memories, instruction, &registers->PC, 3, true);
            break;
        case JLT:
            printf("jlt\n");
            if ((int)registers->SW < 0) load_mem (debugger, memories, instruction, &registers->PC, 3, true);
            break;
        case J:
            printf("j\n");
            load_mem(debugger, memories, instruction, &registers->PC, 3, true);
            break;
        case COMP:
            printf("comp\n");
            load_mem (debugger, memories, instruction, &value, 3, false);
            if (registers->A > value)
                registers->SW = 1;
            else if (registers->A < value)
                registers->SW = -1;
            else
                registers->SW = 0;
            break;
        case TD:
            printf("td\n");
            registers->SW = 1;
            break;
        case RD:
            printf("rd\n");
            registers->A = (registers->A & 0xFFFFFF00) + (unsigned char)inputDevice[device_input_idx++];
            if (device_input_idx >= sizeof(inputDevice) / sizeof(char))
                device_input_idx = 0;
            break;
        case RSUB:
            printf("rsub\n");
            registers->PC = registers->L;
            break;
        case WD:
            printf("wd\n");
            // is done!
            break;
        case CLEAR:
            printf("clear\n");
            store_reg (debugger, instruction->param.param.p2.r1, 0);
            break;
        case COMPR:
            printf("compr\n");

//            uint32_t reg_val_1, reg_val_2;
            load_reg (debugger, instruction->param.param.p2.r1, &val1);
            load_reg (debugger, instruction->param.param.p2.r2, &val2);
//            load_reg (reg_set, inst_param.param.f2_param.r2, &val2);
            if (val1 > val2)
                registers->SW = 1;
            else if (val1 < val2)
                registers->SW = -1;
            else
                registers->SW = 0;
            break;
        case TIXR:
            printf("tixr\n");

            load_reg (debugger, instruction->param.param.p2.r1, &value);
            ++registers->X;

            if (registers->X > value)
                registers->SW = 1;
            else if (registers->X < value)
                registers->SW = -1;
            else
                registers->SW = 0;
            break;
        default:
            printf("default\n");
            return false;
    }

    return true;
}

// [TODO] value 이름 겹침으로 에러 나는걸로 예상됨.
static bool load_mem(Debugger *debugger, Memories *memories, Instruction *instruction, uint32_t *value,
                     size_t bytes, bool jump_op){
    uint32_t target_address;
    const Registers* registers = debugger->registers;

    uint32_t b = (instruction->extend ? instruction->param.param.p4.b : instruction->param.param.p3.b);
    uint32_t p = (instruction->extend ? instruction->param.param.p4.p : instruction->param.param.p3.p);
    uint32_t address = (instruction->extend ? instruction->param.param.p4.address : instruction->param.param.p3.address);
    uint32_t x = (instruction->extend ? instruction->param.param.p4.x : instruction->param.param.p3.x);
    uint32_t n = (instruction->extend ? instruction->param.param.p4.n : instruction->param.param.p3.n);
    uint32_t i = (instruction->extend ? instruction->param.param.p4.i : instruction->param.param.p3.i);

    if (b == 1 && p == 0){
        // Base relative
//        unsigned int address = (instruction->extend ? instruction->param.param.p4.address : instruction->param.param.p3.address);
        target_address = address + registers->B;
    }
    else if (b == 0 && p == 1){
        // PC relative
        int32_t val;
        uint32_t boundary;

        if(instruction->extend)
            boundary = (1 << 19);
        else
            boundary = (1 << 11);

        if(address >= boundary)
            val = address - (boundary << 1);
        else
            val = address;

        target_address = registers->PC + val;
    }else {
        target_address = address;
    }

    if (x == 1){
        target_address += registers->X;
    }

    // fprintf (stderr, "[DEBUG - load_mem] address = %08X, target_address = %08X\n", INST_PARAM (address), target_address);

    bool is_immediate, is_simple, is_indirect;
    if (jump_op){
        is_immediate = n == 1 && i == 1;
        is_simple = n == 1 && i == 0;
        is_indirect = false;
    }
    else{
        is_immediate = n == 0 && i == 1;
        is_simple = n == 1 && i == 1;
        is_indirect = n == 1 && i == 0;
    }

    if (is_immediate){
        // immediate
        //fprintf (stderr, "[DEBUG - load_mem - immediate]\n");
        *value = target_address;
    }
    else if (is_simple){
        // simple
        //fprintf (stderr, "[DEBUG - load_mem - simple]\n");
        uint8_t mem_val;
        *value = 0;
        for (size_t k = 0; k < bytes; ++k){
            mem_val = (uint8_t)memories->data[target_address + k].value;
//            memory_get (memory_manager, target_address + k, &mem_val);
            *value = (*value << 8) + mem_val;
        }
    }
    else if (is_indirect)  // indirect
    {
        //fprintf (stderr, "[DEBUG - load_mem - indirect]\n");
        uint32_t indirect_address = 0;
        uint8_t mem_val;

        for (int k = 0; k < 3; ++k){
            mem_val = (uint8_t)memories->data[target_address + k].value;
//            memory_get (memory_manager, target_address + k, &mem_val);
            indirect_address = (indirect_address << 8) + mem_val;
        }

        //fprintf (stderr, "[DEBUG - load_mem - indirect] final address = %08X\n", address);

        *value = 0;
        for (size_t k = 0; k < bytes; ++k){
            mem_val = (uint8_t)memories->data[indirect_address + k].value;
//            memory_get (memory_manager, address + k, &mem_val);
            *value = (*value << 8) + mem_val;
        }
    }
    else{
        return false;
    }

    return true;
}

static bool store_mem (Debugger* debugger, Memories* memories, Instruction* instruction, unsigned int value, size_t bytes){
    unsigned int target_address;
    unsigned int b = (instruction->extend ? instruction->param.param.p4.b : instruction->param.param.p3.b);
    unsigned int p = (instruction->extend ? instruction->param.param.p4.p : instruction->param.param.p3.p);
    uint32_t address = (instruction->extend ? instruction->param.param.p4.address : instruction->param.param.p3.address);
    unsigned int x = (instruction->extend ? instruction->param.param.p4.x : instruction->param.param.p3.x);
    unsigned int n = (instruction->extend ? instruction->param.param.p4.n : instruction->param.param.p3.n);
    unsigned int i = (instruction->extend ? instruction->param.param.p4.i : instruction->param.param.p3.i);
    const Registers* registers = debugger->registers;

    if (b == 1 && p == 0)  // Base relative
        target_address = address + registers->B;
    else if (b == 0 && p == 1)  // PC relative
    {
        int value;
        unsigned int boundary;

        if (instruction->extend)
            boundary = (1 << 19);
        else
            boundary = (1 << 11);

        if (address >= boundary)
            value = address - (boundary << 1);
        else
            value = address;

        target_address = registers->PC + value;
    }
    else
        target_address = address;

    if (x == 1)
    {
        target_address += registers->X;
    }

    // fprintf (stderr, "[DEBUG - store_mem] address = %08X, target_address = %08X\n", INST_PARAM (address), target_address);

    if (n == 0 && i == 1)  // immediate
    {
        return false;
    }
    else if (n == 1 && i == 1)  // simple
    {
        for (int k = bytes-1; k >= 0; --k)
        {
            edit_memory(memories, target_address + k, value);
//            memory_edit (memory_manager, target_address + k, value);
            value >>= 8;
        }
    }
    else if (n == 1 && i == 0)  // indirect
    {
        unsigned char mem_val;
        unsigned int address = 0;
        for (int k = 0; k < 3; ++k){
            mem_val = (unsigned char)memories->data[target_address + k].value;
//            memory_get (memory_manager, target_address + k, &mem_val);
            address = (address << 8) + mem_val;
        }
        for (int k = 2; k >= 0; --k){
            edit_memory(memories, address + k, value);
//            memory_edit (memory_manager, address + k, value);
            value >>= 8;
        }
    }
    else{
        return false;
    }

    return true;
}

static bool load_reg (Debugger* debugger, int reg_no, uint32_t *reg_val){
    const Registers* registers = debugger->registers;
    switch (reg_no){
        case 0:
            *reg_val = registers->A;
            break;
        case 1:
            *reg_val = registers->X;
            break;
        case 2:
            *reg_val = registers->L;
            break;
        case 3:
            *reg_val = registers->B;
            break;
        case 4:
            *reg_val = registers->S;
            break;
        case 5:
            *reg_val = registers->T;
            break;
        case 6:
            return false; // Not supported.
        case 8:
            *reg_val = registers->PC;
            break;
        case 9:
            *reg_val = registers->SW;
            break;
        default:
            return false;  // invalid
    }
    return true;
}

static bool store_reg (Debugger* debugger, int reg_no, uint32_t reg_val){
    Registers* registers = debugger->registers;
    switch (reg_no){
        case 0:
            registers->A = reg_val;
            break;
        case 1:
            registers->X = reg_val;
            break;
        case 2:
            registers->L = reg_val;
            break;
        case 3:
            registers->B = reg_val;
            break;
        case 4:
            registers->S = reg_val;
            break;
        case 5:
            registers->T = reg_val;
            break;
        case 6:
            return false; // Not supported.
        case 8:
            registers->PC = reg_val;
            break;
        case 9:
            registers->SW = reg_val;
            break;
        default:
            return false;
    }
    return true;
}

void print_registers(Registers* registers){
    printf (
            "A : %08X  X : %08X \n"
            "L : %08X  PC: %08X \n"
            "B : %08X  S : %08X \n"
            "T : %08X           \n",
            registers->A, registers->X,
            registers->L, registers->PC,
            registers->B, registers->S,
            registers->T
    );
}