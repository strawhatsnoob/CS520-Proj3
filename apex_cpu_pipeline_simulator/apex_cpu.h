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
<<<<<<< HEAD
    int pd;
    int ps1;
    int ps2;
=======
>>>>>>> origin/main

} CPU_Stage;

typedef struct BTB {
    int pc_address;
    int branch_prediction;
    int target_address;
    int allocated;
    int is_used;
    int index;
}BTB;

<<<<<<< HEAD
typedef struct Register_Rename {
    int allocated;
    int valid_bit;
    int data;
}Register_Rename;

=======
>>>>>>> origin/main
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
<<<<<<< HEAD
    int rename_table[41];
    int physical_queue[25];
    int physical_queue_length;
=======
>>>>>>> origin/main

    /* Pipeline stages */
    CPU_Stage fetch;
    CPU_Stage decode;
    CPU_Stage execute;
    CPU_Stage memory;
    CPU_Stage writeback;
    BTB branch_target_buffer[4];
<<<<<<< HEAD
    Register_Rename physical_register[25];
    Register_Rename condition_code_register[16];
=======
>>>>>>> origin/main

    // Branch Instruction Queue (BQ)
    CPU_Stage bq[4];
    int bq_size;
    int bq_head;
    int bq_tail;

    // Instruction Queue (IQ)
    CPU_Stage iq[16];
    int iq_size;
    int iq_head;
    int iq_tail;

    // Checkpointing for speculative execution
    int checkpointed;
    int checkpointed_free_list[REG_FILE_SIZE];
    int checkpointed_rename_table[REG_FILE_SIZE];
} APEX_CPU;

APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename);
void APEX_cpu_run(APEX_CPU *cpu);
void APEX_cpu_stop(APEX_CPU *cpu);

void APEX_cpu_issue_instructions(APEX_CPU *cpu);
void APEX_cpu_dispatch_instructions(APEX_CPU *cpu);
#endif
