/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_

#include "apex_macros.h"

/* Format of an APEX instruction  */
typedef struct APEX_Instruction
{
    char opcode_str[128];
    int opcode;
    int rd;
    int rs1;
    int rs2;
    int imm;
    int is_empty_rd;
    int is_empty_rs1;
    int is_empty_rs2;
} APEX_Instruction;

typedef struct IQ_Entries {
    int allocated;
    int opcode;
    int literal;
    int src1_valid_bit;
    int src1_tag;
    int src1_value;
    int src2_valid_bit;
    int src2_tag;
    int src2_value;
    int dest;
    int pc_address;
    int is_used;
    int dispatch_time;
    int elapsed_cycles_at_dispatch;
}IQ_Entries;

typedef struct BQ_Entry {
    int allocated;
    int opcode;
    int literal;
    int src1_valid_bit;
    int src1_tag;
    int src1_value;
    int src2_valid_bit;
    int src2_tag;
    int src2_value;
    int dest;
    int pc_address;
    int branch_prediction;
    int target_address;
    int is_used;
    int index;
    int elapsed_cycles_at_dispatch;
} BQ_Entry;

typedef struct AFU_Data_Forward {
    int physical_address;
    int data;
    int dest_data;
    int updated_src_data;
    int zero_flag;
    int positive_flag;
    int is_allocated;
}AFU_Data_Forward;

typedef struct BFU_Data_Forward {
    int physical_address;
    int data;
    int dest_data;
    int updated_src_data;
    int zero_flag;
    int positive_flag;
    int is_allocated;
}BFU_Data_Forward;

typedef struct MulFu_Data_Forward {
    int physical_address;
    int data;
    int dest_data;
    int updated_src_data;
    int zero_flag;
    int positive_flag;
    int is_allocated;
}MulFu_Data_Forward;

typedef struct IntFu_Data_Forward {
    int physical_address;
    int data;
    int dest_data;
    int updated_src_data;
    int zero_flag;
    int positive_flag;
    int is_allocated;
}IntFu_Data_Forward;

typedef struct MAU_Data_Forward {
    int physical_address;
    int data;
    int dest_data;
    int updated_src_data;
    int zero_flag;
    int positive_flag;
    int is_allocated;
}MAU_Data_Forward;

typedef struct LSQEntry{
    int lsqEntryEstablished;
    int isLoadStore;
    int validBitMemoryAddress;
    unsigned int memoryAddress;
    int destRegAddressForLoad;               
    int srcDataValidBit;
    int srcTag;  
    int entryIndex;
    int opcode;
} LSQEntry;

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
    int pc;
    char opcode_str[128];
    int opcode;
    int rs1;
    int rs2;
    int rd;
    int imm;
    int rs1_value;
    int rs2_value;
    int result_buffer;
    int memory_address;
    int has_insn;
    int updated_register_src1;
    int is_empty_rd;
    int is_empty_rs1;
    int is_empty_rs2;
    int simulate_counter;
    int counter;
    int simulator_flag;
    int data_forward;
    int btb_index;
    int is_btb_hit;
    int pd;
    int ps1;
    int ps2;
    int is_bq;
    int is_iq;
    int is_used;

    IQ_Entries iq_entry;
    LSQEntry dqLsq;
    IQ_Entries iq_afu;
    IQ_Entries iq_mulfu;
    IQ_Entries iq_intfu;
    BQ_Entry bq_bfu;
    // AFU_Data_Forward afu_data;
    // BFU_Data_Forward bfu_data;
    // IntFu_Data_Forward intfu_data;
    // MulFu_Data_Forward mulfu_data;
    // MAU_Data_Forward mau_data;
} CPU_Stage;

typedef struct BTB {
    int pc_address;
    int branch_prediction;
    int target_address;
    int allocated;
    int is_used;
    int index;
}BTB;

typedef struct Register_Rename {
    int allocated;
    int valid_bit;
    int data;
}Register_Rename;

typedef struct Data_Forward {
    int physical_address;
    int data;
    int dest_data;
    int updated_src_data;
    int flag;
    int is_allocated;
}Data_Forward;

typedef struct ROB_Entries {
    int entry_bit;
    int opcode;
    int pc_value;
    int dest_phsyical_register;
    int rename_table_entry;
    int dest_arch_register;
    int lsq_index;
    int memory_error_code;
}ROB_Entries;

typedef struct ROB_Queue {
    ROB_Entries rob_entries[32];
    int ROB_head;
    int ROB_tail;
    int capacity;
}ROB_Queue;

typedef struct LSQ {
    int numberOfEntries;
    int front;
    int rear;
    int sizeOfEntry;
    LSQEntry *entries;
} LSQ;

/* Model of APEX CPU */
typedef struct APEX_CPU
{
    int pc;                        /* Current program counter */
    int clock;                     /* Clock cycles elapsed */
    int insn_completed;            /* Instructions retired */
    int regs[REG_FILE_SIZE];       /* Integer register file */
    int scoreBoarding[REG_FILE_SIZE];
    int has_prev_instr;
    int code_memory_size;          /* Number of instruction in the input file */
    APEX_Instruction *code_memory; /* Code Memory */
    int data_memory[DATA_MEMORY_SIZE]; /* Data Memory */
    int single_step;               /* Wait for user input after every cycle */
    int zero_flag;                 /* {TRUE, FALSE} Used by BZ and BNZ to branch */
    int fetch_from_next_cycle;
    int positive_flag;
    int negative_flag;
    int simulate_counter;
    int counter;
    int simulator_flag;
    int is_data_forwarded;
    int index;
    int rename_table[41];
    int physical_queue[25];
    int physical_queue_length;
    int free_list;
    int prev_dest;
    int memory_address;
    int has_afu_data;
    int has_bfu_data[16];
    int has_intfu_data[24];
    int has_mau_data;
    int has_mulfu_data;
    int afu_entry;
    int VCount[24];
    int isRenamed[24];

    /* Pipeline stages */
    CPU_Stage fetch;
    CPU_Stage decode;
    CPU_Stage dispatch;
    CPU_Stage afu;
    CPU_Stage bfu;
    CPU_Stage mulfu;
    CPU_Stage mau;
    CPU_Stage intfu;
    CPU_Stage rob;
    CPU_Stage lsqStage;
    CPU_Stage execute;
    CPU_Stage memory;
    CPU_Stage writeback;
    CPU_Stage iq_stage;
    CPU_Stage bq_stage;
    // CPU_Stage data_forward_bus;


    BTB branch_target_buffer[8];
    Register_Rename physical_register[25];
    Register_Rename condition_code_register[16];
    Data_Forward data_forward[2];
    IQ_Entries iq_entries[24];
    ROB_Entries rob_entry;
    ROB_Queue ROB_queue;
    LSQ lsq;
    LSQEntry entry;
    AFU_Data_Forward afu_data;
    BFU_Data_Forward bfu_data;
    IntFu_Data_Forward intfu_data[24];
    MulFu_Data_Forward mulfu_data;
    MAU_Data_Forward mau_data;
    
    BQ_Entry bq[16];
    int bq_size;
    int bq_index;
    int iq_size;
    int iq_index;
    int forwarding_cycles;

    BFU_Data_Forward bfu_data_forward;
} APEX_CPU;


APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename);
void APEX_cpu_run(APEX_CPU *cpu);
void APEX_cpu_stop(APEX_CPU *cpu);
void init_bq(APEX_CPU *cpu);
void init_iq(APEX_CPU *cpu);
void dispatch_to_BQ(APEX_CPU *cpu, BQ_Entry *bq_entry);
void dispatch_to_IQ(APEX_CPU *cpu, IQ_Entries *iq_entry);
int is_branch_instruction(int opcode);
int check_issue_ready(CPU_Stage stage);
void APEX_branch_queue(APEX_CPU *cpu);
int check_wakeup_condition_issue(APEX_CPU *cpu, IQ_Entries *iq_entry);
#endif
