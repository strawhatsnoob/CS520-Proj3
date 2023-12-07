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
}IQ_Entries;

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

    /* Pipeline stages */
    CPU_Stage fetch;
    CPU_Stage decode;
    CPU_Stage dispatch;
    CPU_Stage execute;
    CPU_Stage memory;
    CPU_Stage writeback;
    BTB branch_target_buffer[4];
    Register_Rename physical_register[25];
    Register_Rename condition_code_register[16];
    Data_Forward data_forward[2];
    IQ_Entries iq_entries[16];

    // Add two queues for IQ and BQ
    APEX_Instruction iq[MAX_IQ_SIZE];
    APEX_Instruction bq[MAX_BQ_SIZE];
    
    int iq_size; // Number of instructions in IQ
    int bq_size; // Number of instructions in BQ
} APEX_CPU;

APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename);
void APEX_cpu_run(APEX_CPU *cpu);
void APEX_cpu_stop(APEX_CPU *cpu);
#endif
