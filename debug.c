#include "debug.h"

/* Function Declarations */

bool loader_linker_pass1(Debugger *debugger);
bool loader_linker_pass2(Debugger *debugger, Memories *memories);

bool loader_linker_pass1_one(Debugger *debugger, int file_num, int *csaddr);
bool loader_linker_pass2_one(Debugger *debugger, Memories *memories, int file_num , int *csaddr);
LoadInfoList* construct_load_info_list();
bool destroy_load_info_list(LoadInfoList** load_infos);
void print_load_map (LoadInfoList* load_infos);

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
