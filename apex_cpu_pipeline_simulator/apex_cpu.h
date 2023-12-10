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
}IQ_Entries;

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
    IQ_Entries iq_entry;
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
    int flag;
    int is_allocated;
}Data_Forward;


typedef struct BQ_Entry {
    int pc_address;
    int branch_prediction;
    int target_address;
    int is_used;
    int index;
} BQ_Entry;

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

typedef struct LSQEntry{
    int lsqEntryEstablished;
    int isLoadStore;
    int validBitMemoryAddress;
    unsigned int memoryAddress;
    int destRegAddressForLoad;               
    int srcDataValidBit;
    int srcTag;  
} LSQEntry;

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
    CPU_Stage execute;
    CPU_Stage memory;
    CPU_Stage writeback;


    BTB branch_target_buffer[8];
    Register_Rename physical_register[25];
    Register_Rename condition_code_register[16];
    Data_Forward data_forward[2];
    IQ_Entries iq_entries[24];
    ROB_Entries rob_entry;
    ROB_Queue ROB_queue;
    LSQ lsq;
    LSQEntry entry;
    
    BQ_Entry bq[MAX_BQ_SIZE];
    int bq_size;
    int bq_index;
    IQ_Entries iq[MAX_IQ_SIZE];
    int iq_size;
    int iq_index;
} APEX_CPU;


APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename);
void APEX_cpu_run(APEX_CPU *cpu);
void APEX_cpu_stop(APEX_CPU *cpu);
void init_bq(APEX_CPU *cpu);
void init_iq(APEX_CPU *cpu);
#endif
