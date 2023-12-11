/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apex_cpu.h"
#include "apex_macros.h"

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}

//mb_LSQ

static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_MOVC:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
            break;
        }

        case OPCODE_LOAD:
        case OPCODE_LOADP:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }

        case OPCODE_STORE:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
                   stage->imm);
            break;
        }

        case OPCODE_STOREP:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
                   stage->imm);
            break;
        }

        case OPCODE_BZ:
        case OPCODE_BNZ:
        case OPCODE_BNP:
        case OPCODE_BP:
        case OPCODE_BN:
        case OPCODE_BNN:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }

        case OPCODE_HALT:
        case OPCODE_NOP:
        {
            printf("%s", stage->opcode_str);
            break;
        }

        case OPCODE_ADDL:
        case OPCODE_SUBL:
        case OPCODE_JALR:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }

        case OPCODE_CMP:
        {
            printf("%s,R%d,R%d", stage->opcode_str, stage->rs1, stage->rs2);
            break;
        }

        case OPCODE_CML:
        case OPCODE_JUMP:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rs1, stage->imm);
            break;
        }
    }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

static void
print_renamed_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        {
            printf("%s,P%d,P%d,P%d ", stage->opcode_str, stage->pd, stage->ps1,
                   stage->ps2);
            break;
        }

        case OPCODE_MOVC:
        {
            printf("%s,P%d,#%d ", stage->opcode_str, stage->pd, stage->imm);
            break;
        }

        case OPCODE_LOAD:
        case OPCODE_LOADP:
        {
            printf("%s,P%d,P%d,#%d ", stage->opcode_str, stage->pd, stage->ps1,
                   stage->imm);
            break;
        }

        case OPCODE_STORE:
        {
            printf("%s,P%d,P%d,#%d ", stage->opcode_str, stage->ps1, stage->ps2,
                   stage->imm);
            break;
        }

        case OPCODE_STOREP:
        {
            printf("%s,P%d,P%d,#%d ", stage->opcode_str, stage->ps1, stage->ps2,
                   stage->imm);
            break;
        }

        case OPCODE_BZ:
        case OPCODE_BNZ:
        case OPCODE_BNP:
        case OPCODE_BP:
        case OPCODE_BN:
        case OPCODE_BNN:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }

        case OPCODE_HALT:
        case OPCODE_NOP:
        {
            printf("%s", stage->opcode_str);
            break;
        }

        case OPCODE_ADDL:
        case OPCODE_SUBL:
        case OPCODE_JALR:
        {
            printf("%s,P%d,P%d,#%d ", stage->opcode_str, stage->pd, stage->ps1,
                   stage->imm);
            break;
        }

        case OPCODE_CMP:
        {
            printf("%s,P%d,P%d", stage->opcode_str, stage->ps1, stage->ps2);
            break;
        }

        case OPCODE_CML:
        case OPCODE_JUMP:
        {
            printf("%s,P%d,#%d ", stage->opcode_str, stage->ps1, stage->imm);
            break;
        }
    }
}

static void
display_stage_content(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_renamed_instruction(stage);
    printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
    int i;

    printf("----------\n%s\n----------\n", "Registers:");

    for (int i = 0; i < REG_FILE_SIZE / 2; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");

    for (i = (REG_FILE_SIZE / 2); i < REG_FILE_SIZE; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");
}

int generate_hash_index(APEX_CPU *cpu) {
    if((cpu->fetch.opcode == OPCODE_BNZ) || (cpu->fetch.opcode == OPCODE_BP) || 
    (cpu->fetch.opcode == OPCODE_BZ) || (cpu->fetch.opcode == OPCODE_BNP)) {
        if(cpu->index == 0) {
        cpu->index += 1;
        return 0;
    } else if(cpu->index < 4) {
        for(int i = 0; i < 4; i++) {
            if(cpu->fetch.pc == cpu->branch_target_buffer[i].pc_address) {
                return cpu->branch_target_buffer[i].index;
            }
        }
        int index = cpu->index;
        cpu->index += 1;
        return index;
    } else {
        cpu->index = 1;
        return 0;
    }
    }
    return 0;
}

int generate_hash_tag(int pc_address) {
    return pc_address / 4;
}

/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;

    if (cpu->fetch.has_insn && !cpu->decode.has_insn)
    {
        /* This fetches new branch target instruction from next cycle */
        if (cpu->fetch_from_next_cycle == TRUE)
        {
            cpu->fetch_from_next_cycle = FALSE;

            /* Skip this cycle*/
            return;
        }

        /* Store current PC in fetch latch */
        cpu->fetch.pc = cpu->pc;

        /* Index into code memory using this pc and copy all instruction fields
         * into fetch latch  */
        current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
        strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
        // printf("fetch %s \n", &cpu->fetch);
        cpu->fetch.opcode = current_ins->opcode;
        cpu->fetch.rd = current_ins->rd;
        cpu->fetch.rs1 = current_ins->rs1;
        cpu->fetch.rs2 = current_ins->rs2;
        cpu->fetch.imm = current_ins->imm;
        cpu->fetch.is_empty_rd = current_ins->is_empty_rd;
        cpu->fetch.is_empty_rs1 = current_ins->is_empty_rs1;
        cpu->fetch.is_empty_rs2 = current_ins->is_empty_rs2;

        int index = generate_hash_index(cpu);

        if(generate_hash_tag(cpu->fetch.pc) == generate_hash_tag(cpu->branch_target_buffer[index].pc_address)) {
            if((cpu->fetch.opcode == OPCODE_BNZ) || (cpu->fetch.opcode == OPCODE_BP)) {
                cpu->fetch.btb_index = index;
                if(cpu->branch_target_buffer[index].branch_prediction == 01 || 
                cpu->branch_target_buffer[index].branch_prediction == 11) {
                    cpu->pc = cpu->branch_target_buffer[index].target_address;
                    cpu->branch_target_buffer[index].is_used = 1;
                    cpu->fetch.is_btb_hit = 1;
                } else {
                    cpu->pc += 4;
                    cpu->fetch.is_btb_hit = 0;
                }
            } else if((cpu->fetch.opcode == OPCODE_BZ) || (cpu->fetch.opcode == OPCODE_BNP)) {
                cpu->fetch.btb_index = index;
                if(cpu->branch_target_buffer[index].branch_prediction == 11) {
                    cpu->pc = cpu->branch_target_buffer[index].target_address;
                    cpu->branch_target_buffer[index].is_used = 1;
                    cpu->fetch.is_btb_hit = 1;
                } else {
                    cpu->pc += 4;
                    cpu->fetch.is_btb_hit = 0;
                }
            } else {
                 cpu->pc += 4;
                cpu->fetch.is_btb_hit = 0;
            }
        } else {
            if((cpu->fetch.opcode == OPCODE_BNZ) || (cpu->fetch.opcode == OPCODE_BP) || 
            (cpu->fetch.opcode == OPCODE_BZ) || (cpu->fetch.opcode == OPCODE_BNP)) {
                cpu->fetch.btb_index = index;
            }

             /* Update PC for next instruction */
            cpu->pc += 4;
            cpu->fetch.is_btb_hit = 0;
        }

        // Check for BQ instructions and set is_bq to 1 or is_iq to 1
        if (cpu->fetch.has_insn) {
            if (cpu->fetch.opcode == OPCODE_BZ || cpu->fetch.opcode == OPCODE_BNZ || cpu->fetch.opcode == OPCODE_BN || cpu->fetch.opcode == OPCODE_BNN || cpu->fetch.opcode == OPCODE_BP || cpu->fetch.opcode == OPCODE_BNP || cpu->fetch.opcode == OPCODE_JUMP || cpu->fetch.opcode == OPCODE_JALR) {
                cpu->fetch.is_bq = 1;
            }
            else {
                cpu->fetch.is_iq = 1;
            }
        }

        // Check for BQ and IQ instructions and set is_bq to 1 or is_iq to 1
        if (cpu->fetch.has_insn) {
            if (cpu->fetch.opcode == OPCODE_BZ || cpu->fetch.opcode == OPCODE_BNZ || cpu->fetch.opcode == OPCODE_BN || cpu->fetch.opcode == OPCODE_BNN || cpu->fetch.opcode == OPCODE_BP || cpu->fetch.opcode == OPCODE_BNP || cpu->fetch.opcode == OPCODE_JUMP || cpu->fetch.opcode == OPCODE_JALR) {
                cpu->fetch.is_bq = 1;
            }
            else {
                cpu->fetch.is_iq = 1;
            }
        }
        
        /* Copy data from fetch latch to decode latch*/
        cpu->decode = cpu->fetch;
        printf("fetch %d", cpu->decode.has_insn);

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Fetch", &cpu->fetch);
        }

        /* Stop fetching new instructions if HALT is fetched */
        if (cpu->fetch.opcode == OPCODE_HALT)
        {
            cpu->fetch.has_insn = FALSE;
        }
    }
}

static void remove_from_btb(APEX_CPU *cpu) {
    for(int i = 0; i < 3; i++) {
        cpu->branch_target_buffer[i] = cpu->branch_target_buffer[i + 1];
    }
}

int isEmpty(APEX_CPU *cpu) {
    return (cpu->ROB_queue.capacity == 0);
}

int isFull(APEX_CPU *cpu) {
    return (cpu->ROB_queue.capacity >= 32);
}

void enqueue(APEX_CPU *cpu) {
    if(isFull(cpu)) {
        printf("ROB Queue is full.");
        return;
    }
    if(isEmpty(cpu)) {
        cpu->ROB_queue.ROB_head = cpu->ROB_queue.ROB_tail = 0;
    } else {
        cpu->ROB_queue.ROB_tail = (cpu->ROB_queue.ROB_tail + 1) % 32;
    }
    cpu->ROB_queue.capacity++;
    cpu->ROB_queue.rob_entries[cpu->ROB_queue.ROB_tail] = cpu->rob_entry;
}

ROB_Entries dequeue(APEX_CPU *cpu) {
    if(isFull(cpu)) {
        printf("ROB Queue is full.");
    }

    ROB_Entries rob_entry = cpu->ROB_queue.rob_entries[cpu->ROB_queue.ROB_head];
    if(cpu->ROB_queue.ROB_head == cpu->ROB_queue.ROB_tail) {
        cpu->ROB_queue.ROB_head = -1;
        cpu->ROB_queue.ROB_tail = -1;
    } else {
        cpu->ROB_queue.ROB_head = (cpu->ROB_queue.ROB_head + 1) % 32;
    }
    cpu->ROB_queue.capacity--;
    return rob_entry;
}

static void update_src1_with_forwarded_data(APEX_CPU *cpu, int i) {
    if (!cpu->has_afu_data &&
        cpu->afu_data.physical_address == cpu->dispatch.ps1) {
        cpu->iq_entries[i].src1_valid_bit = 1;
        cpu->iq_entries[i].src1_value = cpu->afu_data.updated_src_data;
    }
    if (!cpu->has_mulfu_data &&
        cpu->mulfu_data.physical_address == cpu->dispatch.ps1) {
        cpu->iq_entries[i].src1_valid_bit = 1;
        cpu->iq_entries[i].src1_value = cpu->afu_data.dest_data;
    }
    if (!cpu->has_intfu_data &&
        cpu->intfu_data.physical_address == cpu->dispatch.ps1) {
        cpu->iq_entries[i].src1_valid_bit = 1;
        cpu->iq_entries[i].src1_value = cpu->afu_data.dest_data;
    }
}

static void update_src2_with_forwarded_data(APEX_CPU *cpu, int i) {
    if (!cpu->has_afu_data &&
        cpu->afu_data.physical_address == cpu->dispatch.ps2) {
        cpu->iq_entries[i].src2_valid_bit = 1;
        cpu->iq_entries[i].src2_value = cpu->afu_data.updated_src_data;
    }
    if (!cpu->has_mulfu_data &&
        cpu->mulfu_data.physical_address == cpu->dispatch.ps2) {
        cpu->iq_entries[i].src2_valid_bit = 1;
        cpu->iq_entries[i].src2_value = cpu->afu_data.dest_data;
    }
    if (!cpu->has_intfu_data &&
        cpu->intfu_data.physical_address == cpu->dispatch.ps2) {
        cpu->iq_entries[i].src2_valid_bit = 1;
        cpu->iq_entries[i].src2_value = cpu->afu_data.dest_data;
    }
}

static void rename_rd(APEX_CPU *cpu) {
    int length = cpu->physical_queue_length;
    for (int i = 0; i < length; i++) {
        if(cpu->rename_table[cpu->physical_queue[i]] == cpu->decode.rd) {
            cpu->prev_dest = cpu->physical_queue[i];
        }
        if (cpu->rename_table[cpu->physical_queue[i]] == -1) {
            cpu->rename_table[cpu->physical_queue[i]] = cpu->decode.rd;
            cpu->physical_register[cpu->physical_queue[i]].allocated = 1;
            cpu->physical_register[cpu->physical_queue[i]].valid_bit = 0;
            cpu->physical_register[cpu->physical_queue[i]].data = 0;
            cpu->decode.pd = cpu->physical_queue[i];
            cpu->free_list -= 1;
            break;
        }
    }
}

static void rename_rs1(APEX_CPU *cpu) {
    int length = cpu->physical_queue_length;
    int rs1_flag = 0;
    for(int i = 0; i < length; i++) {
      if (cpu->rename_table[cpu->physical_queue[i]] == cpu->decode.rs1) {
        rs1_flag = 1;
        cpu->decode.ps1 = cpu->physical_queue[i];
        cpu->physical_register[cpu->decode.ps1].allocated = 1;
        if(!cpu->has_afu_data && cpu->afu_data.physical_address == cpu->decode.ps1) {
            cpu->physical_register[cpu->decode.ps1].data = cpu->afu_data.updated_src_data;
            cpu->physical_register[cpu->decode.ps1].valid_bit = 1;
            // cpu->physical_register[cpu->decode.ps1].allocated = 1;
        }
        if(!cpu->has_mulfu_data && cpu->mulfu_data.physical_address == cpu->decode.ps1) {
            cpu->physical_register[cpu->decode.ps1].data = cpu->mulfu_data.dest_data;
            cpu->physical_register[cpu->decode.ps1].valid_bit = 1;
            // cpu->physical_register[cpu->decode.ps1].allocated = 1;
        }
        if(!cpu->has_intfu_data && cpu->intfu_data.physical_address == cpu->decode.ps1) {
            cpu->physical_register[cpu->decode.ps1].data = cpu->intfu_data.dest_data;
            cpu->physical_register[cpu->decode.ps1].valid_bit = 1;
            // cpu->physical_register[cpu->decode.ps1].allocated = 1;
        }
        break;
      }
    }
    if (rs1_flag == 0) {
      for (int i = 0; i < length; i++) {
        if (cpu->rename_table[cpu->physical_queue[i]] == -1) {
          cpu->rename_table[cpu->physical_queue[i]] = cpu->decode.rs1;
          cpu->physical_register[cpu->physical_queue[i]].allocated = 1;
          cpu->physical_register[cpu->physical_queue[i]].valid_bit = 0;
          cpu->physical_register[cpu->physical_queue[i]].data = 0;
          cpu->decode.ps1 = cpu->physical_queue[i];
          break;
        }
      }
    }
}

static void rename_rs2(APEX_CPU *cpu) {
    int length = cpu->physical_queue_length;
    int rs2_flag = 0;
    for(int i = 0; i < length; i++) {
      if (cpu->rename_table[cpu->physical_queue[i]] == cpu->decode.rs2) {
        rs2_flag = 1;
        cpu->decode.ps2 = cpu->physical_queue[i];
        cpu->physical_register[cpu->decode.ps2].allocated = 1;
        if(!cpu->has_afu_data && cpu->afu_data.physical_address == cpu->decode.ps2) {
            cpu->physical_register[cpu->decode.ps2].data = cpu->afu_data.updated_src_data;
            cpu->physical_register[cpu->decode.ps2].valid_bit = 1;
            // cpu->physical_register[cpu->decode.ps1].allocated = 1;
        }
        if(!cpu->has_mulfu_data && cpu->mulfu_data.physical_address == cpu->decode.ps2) {
            cpu->physical_register[cpu->decode.ps2].data = cpu->mulfu_data.dest_data;
            cpu->physical_register[cpu->decode.ps2].valid_bit = 1;
            // cpu->physical_register[cpu->decode.ps1].allocated = 1;
        }
        if(!cpu->has_intfu_data && cpu->intfu_data.physical_address == cpu->decode.ps2) {
            cpu->physical_register[cpu->decode.ps2].data = cpu->intfu_data.dest_data;
            cpu->physical_register[cpu->decode.ps2].valid_bit = 1;
            // cpu->physical_register[cpu->decode.ps1].allocated = 1;
        }

        break;
      }
    }
    if (rs2_flag == 0) {
      for (int i = 0; i < length; i++) {
        if (cpu->rename_table[cpu->physical_queue[i]] == -1) {
          cpu->rename_table[cpu->physical_queue[i]] = cpu->decode.rs2;
          cpu->physical_register[cpu->physical_queue[i]].allocated = 1;
          cpu->physical_register[cpu->physical_queue[i]].valid_bit = 0;
          cpu->physical_register[cpu->physical_queue[i]].data = 0;
          cpu->decode.ps2 = cpu->physical_queue[i];
          break;
        }
      }
    }
}

static void iq_entry_pd_ps1_ps2(APEX_CPU *cpu) {
    for(int i = 0; i < 24; i++) {
            if(cpu->iq_entries[i].allocated == 0) {
                cpu->iq_entries[i].allocated = 1;
                cpu->iq_entries[i].opcode = cpu->dispatch.opcode;

                cpu->iq_entries[i].dest = cpu->dispatch.pd;
                
                cpu->iq_entries[i].src1_tag = cpu->dispatch.ps1;
                if(cpu->physical_register[cpu->dispatch.ps1].allocated == 1 && 
                cpu->physical_register[cpu->dispatch.ps1].valid_bit == 1) {
                    cpu->iq_entries[i].src1_valid_bit = 1;
                    cpu->iq_entries[i].src1_value = cpu->physical_register[cpu->dispatch.ps1].data;
                }
                update_src1_with_forwarded_data(cpu, i);

                cpu->iq_entries[i].src2_tag = cpu->dispatch.ps2;
                if(cpu->physical_register[cpu->dispatch.ps2].allocated == 1 && 
                cpu->physical_register[cpu->dispatch.ps2].valid_bit == 1) {
                    cpu->iq_entries[i].src2_valid_bit = 1;
                    cpu->iq_entries[i].src2_value = cpu->physical_register[cpu->dispatch.ps2].data;
                } 
                update_src2_with_forwarded_data(cpu, i);
                break;
            }
        }
}

static void iq_entry_pd_ps1(APEX_CPU *cpu) {
    for(int i = 0; i < 24; i++) {
            if(cpu->iq_entries[i].allocated == 0) {
                cpu->iq_entries[i].allocated = 1;
                cpu->iq_entries[i].opcode = cpu->dispatch.opcode;

                cpu->iq_entries[i].dest = cpu->dispatch.pd;
                
                cpu->iq_entries[i].src1_tag = cpu->dispatch.ps1;
                if(cpu->physical_register[cpu->dispatch.ps1].allocated == 1 && 
                cpu->physical_register[cpu->dispatch.ps1].valid_bit == 1) {
                    cpu->iq_entries[i].src1_valid_bit = 1;
                    cpu->iq_entries[i].src1_value = cpu->physical_register[cpu->dispatch.ps1].data;
                } 
                update_src1_with_forwarded_data(cpu, i);
                break;
            }
        }
}

static void iq_entry_ps1_ps2(APEX_CPU *cpu) {
    for(int i = 0; i < 24; i++) {
            if(cpu->iq_entries[i].allocated == 0) {
                cpu->iq_entries[i].allocated = 1;
                cpu->iq_entries[i].opcode = cpu->dispatch.opcode;

                cpu->iq_entries[i].src1_tag = cpu->dispatch.ps1;
                if(cpu->physical_register[cpu->dispatch.ps1].allocated == 1 && 
                cpu->physical_register[cpu->dispatch.ps1].valid_bit == 1) {
                    cpu->iq_entries[i].src1_valid_bit = 1;
                    cpu->iq_entries[i].src1_value = cpu->physical_register[cpu->dispatch.ps1].data;
                } 
                update_src1_with_forwarded_data(cpu, i);

                cpu->iq_entries[i].src2_tag = cpu->dispatch.ps2;
                if(cpu->physical_register[cpu->dispatch.ps2].allocated == 1 && 
                cpu->physical_register[cpu->dispatch.ps2].valid_bit == 1) {
                    cpu->iq_entries[i].src2_valid_bit = 1;
                    cpu->iq_entries[i].src2_value = cpu->physical_register[cpu->dispatch.ps2].data;
                } 
                update_src2_with_forwarded_data(cpu, i);
                break;
            }
        }
}

static void iq_entry_ps1(APEX_CPU *cpu) {
    for(int i = 0; i < 24; i++) {
            if(cpu->iq_entries[i].allocated == 0) {
                cpu->iq_entries[i].allocated = 1;
                cpu->iq_entries[i].opcode = cpu->dispatch.opcode;

                
                cpu->iq_entries[i].src1_tag = cpu->dispatch.ps1;
                if(cpu->physical_register[cpu->dispatch.ps1].allocated == 1 && 
                cpu->physical_register[cpu->dispatch.ps1].valid_bit == 1) {
                    cpu->iq_entries[i].src1_valid_bit = 1;
                    cpu->iq_entries[i].src1_value = cpu->physical_register[cpu->dispatch.ps1].data;
                }
                update_src1_with_forwarded_data(cpu, i);
            }
        }
}

static void iq_entry_pd(APEX_CPU *cpu) {
    for(int i = 0; i < 24; i++) {
            if(cpu->iq_entries[i].allocated == 0) {
                cpu->iq_entries[i].allocated = 1;
                cpu->iq_entries[i].opcode = cpu->dispatch.opcode;
                cpu->iq_entries[i].dest = cpu->dispatch.pd;
                cpu->iq_entries[i].src1_valid_bit = 1;
                cpu->iq_entries[i].src2_valid_bit = 1;
            }
        }
}

static void initialize_R2R_rob_entry(APEX_CPU *cpu) {
    if(!isFull(cpu)) {
        cpu->rob_entry.entry_bit = 1;
        cpu->rob_entry.dest_arch_register = cpu->dispatch.rd;
        cpu->rob_entry.dest_phsyical_register = cpu->dispatch.pd;
        cpu->rob_entry.lsq_index = 0;
        cpu->rob_entry.memory_error_code = 0;
        cpu->rob_entry.pc_value = cpu->dispatch.pc;
        cpu->rob_entry.opcode = cpu->dispatch.opcode;
        cpu->rob_entry.rename_table_entry = cpu->prev_dest;
        enqueue(cpu);
    }
}

static void initialize_load_rob_entry(APEX_CPU *cpu) {
    if(!isFull(cpu)) {
        cpu->rob_entry.entry_bit = 1;
        cpu->rob_entry.dest_arch_register = cpu->dispatch.rd;
        cpu->rob_entry.dest_phsyical_register = cpu->dispatch.pd;
        cpu->rob_entry.lsq_index = cpu->entry.entryIndex;
        cpu->rob_entry.memory_error_code = 0;
        cpu->rob_entry.pc_value = cpu->dispatch.pc;
        cpu->rob_entry.opcode = cpu->dispatch.opcode;
        cpu->rob_entry.rename_table_entry = cpu->prev_dest;
    }
}

static void initialize_store_rob_entry(APEX_CPU *cpu) {
    if(!isFull(cpu)) {
        cpu->rob_entry.entry_bit = 1;
        cpu->rob_entry.lsq_index = cpu->entry.entryIndex;
        cpu->rob_entry.pc_value = cpu->dispatch.pc;
        cpu->rob_entry.opcode = cpu->dispatch.opcode;
    }
}

static void do_commit(Register_Rename physical_entry, int dest_address, APEX_CPU *cpu) {
    /* Write result to register file based on instruction type */
        switch (cpu->writeback.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_DIV:
            {
                cpu->regs[dest_address] = physical_entry.data;
                break;
            }

            case OPCODE_ADDL:
            case OPCODE_SUBL:
            {
                cpu->regs[dest_address] = physical_entry.data;
                break;
            }

            case OPCODE_LOAD:
            {
                cpu->regs[dest_address] = physical_entry.data;
                break;
            }

            case OPCODE_MOVC: 
            {
                cpu->regs[dest_address] = physical_entry.data;
                break;
            }

            case OPCODE_LOADP:
            {
                cpu->regs[dest_address] = physical_entry.data;
                cpu->regs[cpu->writeback.rs1] = cpu->writeback.updated_register_src1;
                printf("src1 %d \n",cpu->regs[cpu->writeback.rs1]);
                break;
            }

            case OPCODE_NOP:
            {
                break;
            }

            case OPCODE_STORE:
            {
                printf("MEM[%d] : %d \n", cpu->writeback.memory_address, cpu->writeback.rs1_value);
                break;
            }

            case OPCODE_STOREP:
            {
                cpu->regs[cpu->writeback.rs2] = cpu->writeback.updated_register_src1;
                printf("MEM[%d] : %d \n", cpu->writeback.memory_address, cpu->writeback.rs1_value);
                break;
            }

            case OPCODE_CMP:
            {
                break;
            }

            case OPCODE_CML:
            {
                break;
            }

            case OPCODE_JALR:
            {
                cpu->regs[dest_address] = physical_entry.data;
                break;
            }
        }
}

static void APEX_ROB(APEX_CPU *cpu) {
    if(cpu->rob.has_insn) {
        for(int i = 0; i < 24; i++) {
            if(cpu->ROB_queue.rob_entries[cpu->ROB_queue.ROB_head].entry_bit == 1 && cpu->rename_table[cpu->physical_queue[i]] == cpu->ROB_queue.rob_entries[cpu->ROB_queue.ROB_head].dest_phsyical_register) {
                if(cpu->physical_register[cpu->physical_queue[i]].allocated == 1) {
                    ROB_Entries current_entry = dequeue(cpu);
                    do_commit(cpu->physical_register[cpu->physical_queue[i]], cpu->ROB_queue.rob_entries[cpu->ROB_queue.ROB_head].dest_arch_register ,
                    cpu);
                    cpu->rename_table[cpu->physical_queue[i]] = -1;
                    break;
                }
            }
        }
    }
}

static void fetch_LSQ_Entry(APEX_CPU *cpu){

        cpu->entry.lsqEntryEstablished = 1;
        cpu->entry.srcDataValidBit = 1;


        if(cpu->physical_register[cpu->dispatch.pd].allocated == 1 && 
            cpu->physical_register[cpu->dispatch.pd].valid_bit == 1) {
            cpu->entry.srcDataValidBit = 0;
            cpu->entry.destRegAddressForLoad = cpu->physical_register[cpu->dispatch.pd].data;
        } else if(cpu->data_forward[0].physical_address == cpu->decode.pd) {
            cpu->entry.srcDataValidBit = 0;
            cpu->entry.destRegAddressForLoad = cpu->data_forward[0].data;
            cpu->data_forward[0].is_allocated = 0;
        } else if(cpu->data_forward[1].physical_address == cpu->decode.pd) {
            cpu->entry.srcDataValidBit = 0;
            cpu->entry.destRegAddressForLoad = cpu->data_forward[1].data;
            cpu->data_forward[1].is_allocated = 0;
        }
        
        
        if(cpu->physical_register[cpu->dispatch.ps1].allocated == 1 && 
            cpu->physical_register[cpu->dispatch.ps1].valid_bit == 1) {
            cpu->entry.srcDataValidBit = 1;
            cpu->entry.srcTag = cpu->physical_register[cpu->dispatch.ps1].data;
        } else if(cpu->data_forward[0].physical_address == cpu->decode.ps1) {
            cpu->entry.srcDataValidBit = 1;
            cpu->entry.srcTag = cpu->data_forward[0].data;
            cpu->data_forward[0].is_allocated = 0;
        } else if(cpu->data_forward[1].physical_address == cpu->decode.ps1) {
            cpu->entry.srcDataValidBit = 1;
            cpu->entry.srcTag = cpu->data_forward[1].data;
            cpu->data_forward[1].is_allocated = 0;
        }
}

static int isLSQFull(APEX_CPU *cpu) {
    if (cpu->lsq.numberOfEntries < 16) {
        return 0;
    } else {
        return 1;
    }
}

static int isLSQEmpty(APEX_CPU *cpu) {
    return (cpu->lsq.numberOfEntries == 0);
}

static void LSQ_enqueue(APEX_CPU *cpu) {
    while(isLSQFull(cpu)) {
        //spin
    }

    if (isLSQEmpty(cpu)) {
        cpu->lsq.front = 0; // Set front to 0 for the first element
    }

    cpu->lsq.rear = (cpu->lsq.rear + 1) % 16; // Circular increment
    cpu->lsq.entries[cpu->lsq.rear] = cpu->entry;

}

static LSQEntry LSQ_dequeue(APEX_CPU *cpu){

    LSQEntry entry1 = cpu->lsq.entries[cpu->lsq.front];
    cpu->lsq.front = (cpu->lsq.front + 1) % 16; // Circular increment
    cpu->lsq.numberOfEntries--;

    return entry1;
}

static LSQEntry getEntryAtIndex(APEX_CPU *cpu, int index) {

    return cpu->lsq.entries[(cpu->lsq.front + index) % 16];
}



static void LSQEntryStore(APEX_CPU *cpu){

    cpu->entry.lsqEntryEstablished = 1;
                cpu->entry.isLoadStore = 0;
                cpu->entry.validBitMemoryAddress = 1;
                cpu->entry.entryIndex = cpu->lsq.numberOfEntries+1;

                fetch_LSQ_Entry(cpu);

                if(cpu->physical_register[cpu->dispatch.ps1].allocated == 1 && 
                    cpu->physical_register[cpu->dispatch.ps1].valid_bit == 1) {
                    cpu->entry.srcDataValidBit = 1;
                    cpu->entry.srcTag = cpu->physical_register[cpu->dispatch.ps1].data;
                } else if(cpu->data_forward[0].physical_address == cpu->decode.ps1) {
                    cpu->entry.srcDataValidBit = 1;
                    cpu->entry.srcTag = cpu->data_forward[0].data;
                    cpu->data_forward[0].is_allocated = 0;
                } else if(cpu->data_forward[1].physical_address == cpu->decode.ps1) {
                    cpu->entry.srcDataValidBit = 1;
                    cpu->entry.srcTag = cpu->data_forward[1].data;
                    cpu->data_forward[1].is_allocated = 0;
                }

                for(int i = 0; i < 24; i++) {
                    if(cpu->iq_entries[i].allocated == 0) {
                        cpu->iq_entries[i].allocated = 1;
                        cpu->iq_entries[i].opcode = 0;

                        cpu->iq_entries[i].dest = 0;
                        
                        cpu->iq_entries[i].src1_tag = cpu->dispatch.ps1;
                        if(cpu->physical_register[cpu->dispatch.ps1].allocated == 1 && 
                        cpu->physical_register[cpu->dispatch.ps1].valid_bit == 1) {
                            cpu->iq_entries[i].src1_valid_bit = 1;
                            cpu->iq_entries[i].src1_value = cpu->physical_register[cpu->dispatch.ps1].data;
                        } else if(cpu->data_forward[0].physical_address == cpu->decode.ps1) {
                            cpu->iq_entries[i].src1_valid_bit = 1;
                            cpu->iq_entries[i].src1_value = cpu->data_forward[0].data;
                            cpu->data_forward[0].is_allocated = 0;
                        } else if(cpu->data_forward[1].physical_address == cpu->decode.ps1) {
                            cpu->iq_entries[i].src1_valid_bit = 1;
                            cpu->iq_entries[i].src1_value = cpu->data_forward[1].data;
                            cpu->data_forward[1].is_allocated = 0;
                        }

                        cpu->iq_entries[i].src2_tag = cpu->dispatch.ps2;
                        if(cpu->physical_register[cpu->dispatch.ps2].allocated == 1 && 
                        cpu->physical_register[cpu->dispatch.ps2].valid_bit == 1) {
                            cpu->iq_entries[i].src2_valid_bit = 1;
                            cpu->iq_entries[i].src2_value = cpu->physical_register[cpu->dispatch.ps2].data;
                        } else if(cpu->data_forward[0].physical_address == cpu->decode.ps2) {
                            cpu->iq_entries[i].src2_valid_bit = 1;
                            cpu->iq_entries[i].src2_value = cpu->data_forward[0].data;
                            cpu->data_forward[0].is_allocated = 0;
                        } else if(cpu->data_forward[1].physical_address == cpu->decode.ps2) {
                            cpu->iq_entries[i].src2_valid_bit = 1;
                            cpu->iq_entries[i].src2_value = cpu->data_forward[1].data;
                            cpu->data_forward[1].is_allocated = 0;
                        }
                        }
                    }

                    LSQ_enqueue(cpu);
}

static void LSQEntryLoad(APEX_CPU *cpu){

    iq_entry_pd_ps1(cpu);
                cpu->entry.lsqEntryEstablished = 1;
                cpu->entry.entryIndex = cpu->lsq.numberOfEntries+1;
                    for(int i = 0; i < 24; i++) {
                    if(cpu->iq_entries[i].allocated == 0) {
                        cpu->iq_entries[i].allocated = 1;
                        cpu->iq_entries[i].opcode = 0;

                        cpu->iq_entries[i].dest = 0;
                        cpu->iq_entries[i].src1_valid_bit = 1;
                        
                        cpu->iq_entries[i].src1_tag = cpu->dispatch.ps1;
                        if(cpu->physical_register[cpu->dispatch.ps1].allocated == 1 && 
                        cpu->physical_register[cpu->dispatch.ps1].valid_bit == 1) {
                            cpu->iq_entries[i].src1_value = cpu->physical_register[cpu->dispatch.ps1].data;
                        } else if(cpu->data_forward[0].physical_address == cpu->decode.ps1) {
                            cpu->iq_entries[i].src1_value = cpu->data_forward[0].data;
                            cpu->data_forward[0].is_allocated = 0;
                        } else if(cpu->data_forward[1].physical_address == cpu->decode.ps1) {
                            cpu->iq_entries[i].src1_value = cpu->data_forward[1].data;
                            cpu->data_forward[1].is_allocated = 0;
                        }

                        cpu->iq_entries[i].src2_tag = cpu->dispatch.ps2;
                        if(cpu->physical_register[cpu->dispatch.ps2].allocated == 1 && 
                        cpu->physical_register[cpu->dispatch.ps2].valid_bit == 1) {
                            cpu->iq_entries[i].src2_valid_bit = 1;
                            cpu->iq_entries[i].src2_value = cpu->physical_register[cpu->dispatch.ps2].data;
                        } else if(cpu->data_forward[0].physical_address == cpu->decode.ps2) {
                            cpu->iq_entries[i].src2_valid_bit = 1;
                            cpu->iq_entries[i].src2_value = cpu->data_forward[0].data;
                            cpu->data_forward[0].is_allocated = 0;
                        } else if(cpu->data_forward[1].physical_address == cpu->decode.ps2) {
                            cpu->iq_entries[i].src2_valid_bit = 1;
                            cpu->iq_entries[i].src2_value = cpu->data_forward[1].data;
                            cpu->data_forward[1].is_allocated = 0;
                        }
                        }
                    }

                    if(cpu->physical_register[cpu->dispatch.pd].allocated == 1 && 
                            cpu->physical_register[cpu->dispatch.pd].valid_bit == 1) {
                            cpu->entry.srcDataValidBit = 0;
                            cpu->entry.destRegAddressForLoad = cpu->physical_register[cpu->dispatch.pd].data;
                        } else if(cpu->data_forward[0].physical_address == cpu->decode.pd) {
                            cpu->entry.srcDataValidBit = 0;
                            cpu->entry.destRegAddressForLoad = cpu->data_forward[0].data;
                            cpu->data_forward[0].is_allocated = 0;
                        } else if(cpu->data_forward[1].physical_address == cpu->decode.pd) {
                            cpu->entry.srcDataValidBit = 0;
                            cpu->entry.destRegAddressForLoad = cpu->data_forward[1].data;
                            cpu->data_forward[1].is_allocated = 0;
                    }

                    cpu->entry.isLoadStore = 1;
                    cpu->entry.validBitMemoryAddress = 1;

                    fetch_LSQ_Entry(cpu);
                    LSQ_enqueue(cpu);
}

/*
 * Dispatch Stage of APEX Pipeline
 *
 */
static void
APEX_dispatch(APEX_CPU *cpu) {
    if(cpu->dispatch.has_insn && !isLSQFull(cpu) && !isFull(cpu)) {
        switch (cpu->dispatch.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_MUL:
            case OPCODE_DIV:
            {
                iq_entry_pd_ps1_ps2(cpu);
                initialize_R2R_rob_entry(cpu);
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                // cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                // update_with_forwarded_value(cpu);
                // cpu->is_data_forwarded = 0;
                break;
            }

            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_JALR:
            {
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                iq_entry_pd_ps1(cpu);
                initialize_R2R_rob_entry(cpu);

                // update_rs1_with_forwarded_value(cpu);
                // printf("rs1 %d", cpu->decode.rs1_value);
                // cpu->is_data_forwarded = 0;
                break;
            }

            case OPCODE_LOAD:
            case OPCODE_LOADP:
            {
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                iq_entry_pd_ps1(cpu);
                LSQEntryLoad(cpu);
                initialize_load_rob_entry(cpu);
                // update_rs1_with_forwarded_value(cpu);
                // cpu->is_data_forwarded = 0;
                // cpu->scoreBoarding[cpu->decode.rd] = 1;
                // cpu->scoreBoarding[cpu->decode.rs1] = 1;

                
                break;
            }


            case OPCODE_MOVC:
            {
                /* MOVC doesn't have register operands */
                // cpu->scoreBoarding[cpu->decode.rd] = 1;
                iq_entry_pd(cpu);
                initialize_R2R_rob_entry(cpu);
                break;
            }

            case OPCODE_NOP:
            {
                break;
            }

            case OPCODE_STORE:
            case OPCODE_STOREP:
            {
                iq_entry_ps1_ps2(cpu);
                LSQEntryStore(cpu);
                initialize_store_rob_entry(cpu);
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                // cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                // update_with_forwarded_value(cpu);
                // cpu->is_data_forwarded = 0;
                // cpu->scoreBoarding[cpu->decode.rs1] = 1;
                // cpu->scoreBoarding[cpu->decode.rs2] = 1;

                
                

                break;   
            }

            case OPCODE_CMP:
            {
                iq_entry_ps1_ps2(cpu);
                initialize_R2R_rob_entry(cpu);
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                // cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                // update_with_forwarded_value(cpu);
                // cpu->is_data_forwarded = 0;
                break;
            }

            case OPCODE_CML:
            case OPCODE_JUMP:
            {
                iq_entry_ps1(cpu);
                initialize_R2R_rob_entry(cpu);
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];

                // update_rs1_with_forwarded_value(cpu);
                // cpu->is_data_forwarded = 0;
                break;
            }

            case OPCODE_BZ:
            case OPCODE_BNZ:
            case OPCODE_BP:
            case OPCODE_BNP:
            {
                for(int i = 0; i < 16; i++) {
                    if(cpu->iq_entries[i].allocated == 0) {
                        cpu->iq_entries[i].allocated = 1;
                        cpu->iq_entries[i].opcode = cpu->dispatch.opcode;
                        cpu->iq_entries[i].dest = cpu->dispatch.pd;
                        cpu->iq_entries[i].src1_valid_bit = 1;
                        cpu->iq_entries[i].src2_valid_bit = 1;
                    }
                }
                initialize_R2R_rob_entry(cpu);
                break;
            }
        }
        if (cpu->bq_size > 0) {
            int index = cpu->bq[0].index;
            cpu->bq[index].is_used = 0;
            cpu->bq_size--;
            cpu->fetch.pc = cpu->bq[index].target_address;
        }

        /* Copy data from decode latch to execute latch*/
    
        cpu->lsqStage = cpu->dispatch;

        cpu->dispatch.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            display_stage_content("Dispatch/RF", &cpu->decode);
        }
    }
}



static void
APEX_LSQ(APEX_CPU *cpu)
{
    if(cpu->lsqStage.has_insn && cpu->lsq.numberOfEntries > 0){
        LSQ_enqueue(cpu);

    //CHECKING CONDITION 1

    if(cpu->entry.validBitMemoryAddress == 0){
        //checking contition 2

            if(cpu->memory_address != -1){
                cpu->entry.memoryAddress = cpu->memory_address;
                cpu->memory_address = -1;
            }

            

            if(cpu->ROB_queue.rob_entries[cpu->ROB_queue.ROB_head].lsq_index == cpu->lsq.entries[cpu->lsq.front].entryIndex){

                if(isLSQEmpty(cpu)){
                    return;
                }

                LSQEntry checkedEntry = LSQ_dequeue(cpu);
                cpu->mau = cpu->lsqStage;
                cpu->lsqStage.has_insn = FALSE;

                if (ENABLE_DEBUG_MESSAGES)
                {
                    display_stage_content("LSQ/RF", &cpu->lsqStage);
                }
                

            }
        

        
        }
    }
    
}


/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu)
{
    if (cpu->decode.has_insn && (cpu->free_list != 0))
    // && (cpu->free_list != 0) &&
    // ((cpu->decode.is_empty_rd || cpu->scoreBoarding[cpu->decode.rd] == 0) &&  (cpu->decode.is_empty_rs1 || cpu->scoreBoarding[cpu->decode.rs1] == 0) && (cpu->decode.is_empty_rs2 || cpu->scoreBoarding[cpu->decode.rs2] == 0)))
    {
        /* Read operands from register file based on the instruction type */
        switch (cpu->decode.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_MUL:
            case OPCODE_DIV:
            {
                rename_rd(cpu);
                rename_rs1(cpu);
                rename_rs2(cpu);
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                // cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                // update_with_forwarded_value(cpu);
                // cpu->is_data_forwarded = 0;
                break;
            }

            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_JALR:
            {
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                rename_rd(cpu);
                rename_rs1(cpu);

                // update_rs1_with_forwarded_value(cpu);
                // printf("rs1 %d", cpu->decode.rs1_value);
                // cpu->is_data_forwarded = 0;
                break;
            }

            case OPCODE_LOAD:
            case OPCODE_LOADP:
            {
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                rename_rd(cpu);
                rename_rs1(cpu);

                // update_rs1_with_forwarded_value(cpu);
                // cpu->is_data_forwarded = 0;
                // cpu->scoreBoarding[cpu->decode.rd] = 1;
                // cpu->scoreBoarding[cpu->decode.rs1] = 1;
                break;
            }


            case OPCODE_MOVC:
            {
                /* MOVC doesn't have register operands */
                // cpu->scoreBoarding[cpu->decode.rd] = 1;
                rename_rd(cpu);
                break;
            }

            case OPCODE_NOP:
            {
                break;
            }

            case OPCODE_STORE:
            case OPCODE_STOREP:
            {
                rename_rs1(cpu);
                rename_rs2(cpu);
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                // cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                // update_with_forwarded_value(cpu);
                // cpu->is_data_forwarded = 0;
                // cpu->scoreBoarding[cpu->decode.rs1] = 1;
                // cpu->scoreBoarding[cpu->decode.rs2] = 1;
                break;   
            }

            case OPCODE_CMP:
            {
                rename_rs1(cpu);
                rename_rs2(cpu);
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                // cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                // update_with_forwarded_value(cpu);
                // cpu->is_data_forwarded = 0;
                break;
            }

            case OPCODE_CML:
            case OPCODE_JUMP:
            {
                rename_rs1(cpu);
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];

                // update_rs1_with_forwarded_value(cpu);
                // cpu->is_data_forwarded = 0;
                break;
            }

            case OPCODE_BZ:
            case OPCODE_BNZ:
            case OPCODE_BP:
            case OPCODE_BNP:
            {
                if(!cpu->decode.is_btb_hit) {
                    if(!cpu->branch_target_buffer[cpu->decode.btb_index].allocated) {
                        cpu->branch_target_buffer[cpu->decode.btb_index].pc_address = cpu->decode.pc;
                        cpu->branch_target_buffer[cpu->decode.btb_index].allocated = 1;
                        cpu->branch_target_buffer[cpu->decode.btb_index].index = cpu->decode.btb_index;
                        if((cpu->decode.opcode == OPCODE_BZ) || (cpu->decode.opcode == OPCODE_BNP)) {
                            cpu->branch_target_buffer[cpu->decode.btb_index].branch_prediction = 0;
                        } else {
                            cpu->branch_target_buffer[cpu->decode.btb_index].branch_prediction = 1;
                        }
                    } else if(cpu->branch_target_buffer[cpu->decode.btb_index].pc_address != cpu->decode.pc) {
                        int flag = 0;
                        for(int i = 0; i < 4; i++) {
                            if(cpu->branch_target_buffer->allocated != 1)
                                flag = 1;
                            else
                                flag = 0;
                        }
                        if(flag == 0) {
                            remove_from_btb(cpu);
                            cpu->branch_target_buffer[3].pc_address = cpu->decode.pc;
                            cpu->branch_target_buffer[3].allocated = 1;
                            cpu->decode.btb_index = 3;
                            cpu->branch_target_buffer[cpu->decode.btb_index].index = cpu->decode.btb_index;
                        }
                    }
                }
                break;
            }
        }

        if (cpu->decode.is_bq) {
            cpu->bq[cpu->bq_index].pc_address = cpu->decode.pc;
            cpu->bq[cpu->bq_index].branch_prediction = cpu->decode.result_buffer;
            cpu->bq[cpu->bq_index].target_address = cpu->decode.rs1_value;
            cpu->bq[cpu->bq_index].is_used = 1;
            cpu->bq[cpu->bq_index].index = cpu->bq_index;
            cpu->bq_index = (cpu->bq_index + 1) % MAX_BQ_SIZE;
            cpu->bq_size++;
        }
        if (cpu->decode.is_iq) {
            cpu->iq[cpu->iq_index].pc_address = cpu->decode.pc;
            cpu->iq[cpu->iq_index].opcode = cpu->decode.opcode;
            cpu->iq[cpu->iq_index].literal = cpu->decode.imm;
            cpu->iq[cpu->iq_index].src1_valid_bit = !cpu->decode.is_empty_rs1;
            cpu->iq[cpu->iq_index].src1_tag = cpu->decode.rs1;
            cpu->iq[cpu->iq_index].src1_value = cpu->decode.rs1_value;
            cpu->iq[cpu->iq_index].src2_valid_bit = !cpu->decode.is_empty_rs2;
            cpu->iq[cpu->iq_index].src2_tag = cpu->decode.rs2;
            cpu->iq[cpu->iq_index].src2_value = cpu->decode.rs2_value;
            cpu->iq[cpu->iq_index].dest = cpu->decode.rd;
            cpu->iq[cpu->iq_index].allocated = 1;
            cpu->iq_index = (cpu->iq_index + 1) % MAX_IQ_SIZE;
            cpu->iq_size++;
        }

        /* Copy data from decode latch to execute latch*/
        cpu->dispatch = cpu->decode;
        cpu->decode.has_insn = FALSE;


        if (ENABLE_DEBUG_MESSAGES)
        {
            display_stage_content("Decode/RF", &cpu->decode);
        }

    }
}

static void set_branch_flags(APEX_CPU *cpu) {
    if (cpu->intfu.result_buffer == 0) {
        cpu->zero_flag = TRUE;
        cpu->intfu_data.zero_flag = cpu->zero_flag;
    } else {
        cpu->zero_flag = FALSE;
        cpu->intfu_data.zero_flag = cpu->zero_flag;
    }
    if(cpu->intfu.result_buffer > 0) {
        cpu->positive_flag = TRUE;
        cpu->intfu_data.positive_flag = cpu->positive_flag;
    } else {
      cpu->positive_flag = FALSE;
      cpu->intfu_data.positive_flag = cpu->positive_flag;
    }
    // if (cpu->intfu.result_buffer < 0) {
    //   cpu->negative_flag = TRUE;
    // } else {
    //   cpu->negative_flag = FALSE;
    // }
}

static void do_branching(APEX_CPU *cpu) {
  /* Calculate new PC, and send it to fetch unit */
  cpu->pc = cpu->execute.pc + cpu->execute.imm;

  /* Since we are using reverse callbacks for pipeline stages,
   * this will prevent the new instruction from being fetched in the current
   * cycle*/
  cpu->fetch_from_next_cycle = TRUE;

  /* Flush previous stages */
  cpu->decode.has_insn = FALSE;

  /* Make sure fetch stage is enabled to start fetching from new PC */
  cpu->fetch.has_insn = TRUE;
}

// static void update_stalling_flags(APEX_CPU *cpu) {
//     if (cpu->execute.rd != cpu->execute.rs1) {
//         cpu->scoreBoarding[cpu->execute.rs1] = 0;
//     } else {
//         cpu->scoreBoarding[cpu->execute.rs1] = 1;
//     }
//     if (cpu->execute.rd != cpu->execute.rs2) {
//         cpu->scoreBoarding[cpu->execute.rs2] = 0;
//     } else {
//         cpu->scoreBoarding[cpu->execute.rs2] = 1;
//     }
// }


static void APEX_AFU(APEX_CPU *cpu) {
    cpu->has_afu_data = FALSE;
    if(cpu->afu.has_insn) {
        int opcode = 0;
        switch (opcode) {
            case OPCODE_LOAD:
                //get rs1 and imm from lsq entry
                cpu->afu.memory_address = cpu->afu.iq_entry.src2_value + cpu->afu.iq_entry.literal;
                        // = cpu->afu.rs1_value + cpu->afu.imm;
                cpu->has_afu_data = TRUE;
                cpu->afu_data.dest_data = cpu->afu.memory_address;
                cpu->afu_data.physical_address = cpu->afu.iq_entry.dest;
                cpu->memory_address = cpu->afu.memory_address;
                break;
            case OPCODE_LOADP:
            //get rs1 and imm from lsq entry
                cpu->afu.memory_address = cpu->afu.iq_entry.src2_value + cpu->afu.iq_entry.literal;
                cpu->has_afu_data = TRUE;
                cpu->afu_data.dest_data = cpu->afu.memory_address;
                cpu->afu_data.physical_address = cpu->afu.iq_entry.dest;
                cpu->afu_data.updated_src_data = cpu->afu.iq_entry.src1_value + 4;
                cpu->memory_address = cpu->afu.memory_address;
                        // = cpu->afu.rs1_value + cpu->afu.imm;
                // cpu->memory_address = cpu->afu.memory_address;
                //update src2 + 4
                break;
            
            case OPCODE_STORE:
                //get rs1 and imm from lsq entry
                cpu->afu.memory_address = cpu->afu.iq_entry.src1_value + cpu->afu.iq_entry.src2_value;
                cpu->has_afu_data = TRUE;
                cpu->afu_data.dest_data = cpu->afu.memory_address;
                cpu->afu_data.physical_address = cpu->afu.iq_entry.dest;
                        // = cpu->afu.rs1_value + cpu->afu.rs2_value;
                cpu->memory_address = cpu->afu.memory_address;
            
            case OPCODE_STOREP:
            //get rs1 and imm from lsq entry
                cpu->afu.memory_address = cpu->afu.iq_entry.src1_value + cpu->afu.iq_entry.literal;
                cpu->has_afu_data = TRUE;
                cpu->afu_data.dest_data = cpu->afu.memory_address;
                cpu->afu_data.physical_address = cpu->afu.iq_entry.dest;
                cpu->afu_data.updated_src_data = cpu->afu.iq_entry.src2_value + 4;
                        // = cpu->afu.rs1_value + cpu->afu.imm;
                cpu->memory_address = cpu->afu.memory_address;
                //update src2 + 4
                break;

            case OPCODE_BZ:
            case OPCODE_BNZ:
            case OPCODE_BP:
            case OPCODE_BNP:
                //get imm from BQ entry
                cpu->memory_address = cpu->afu.iq_entry.pc_address + cpu->afu.iq_entry.literal;
                cpu->has_afu_data = TRUE;
                cpu->afu_data.dest_data = cpu->afu.memory_address;
                cpu->afu_data.physical_address = cpu->afu.iq_entry.dest;
                // cpu->afu.pc + cpu->afu.imm;
                break;

            case OPCODE_JUMP:
                //get imm from BQ entry
                cpu->memory_address = cpu->afu.iq_entry.src1_value + cpu->afu.iq_entry.literal;
                cpu->has_afu_data = TRUE;
                cpu->afu_data.dest_data = cpu->afu.memory_address;
                cpu->afu_data.physical_address = cpu->afu.iq_entry.dest;
                // cpu->afu.rs1_value + cpu->afu.imm;
                break;
            
            case OPCODE_JALR:
                //get imm from BQ entry
                cpu->memory_address = cpu->afu.iq_entry.pc_address + 4;
                cpu->has_afu_data = TRUE;
                cpu->afu_data.dest_data = cpu->afu.memory_address;
                cpu->afu_data.physical_address = cpu->afu.iq_entry.dest;
                // cpu->execute.pc + 4;
                break;
            
        }
        cpu->afu.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            display_stage_content("AFU/RF", &cpu->decode);
        }
        }
}
/*
 * Execute Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_execute(APEX_CPU *cpu)
{
    if (cpu->execute.has_insn)
    {
        /* Execute logic based on instruction type */
        switch (cpu->execute.opcode)
        {
            

        }

        /* Copy data from execute latch to memory latch*/
        cpu->memory = cpu->execute;
        cpu->execute.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            display_stage_content("Execute", &cpu->execute);
        }
    }
}

static void APEX_BFU(APEX_CPU *cpu) {
    if(cpu->bfu.has_insn) {
        int opcode = 0;
        switch(opcode) {
            case OPCODE_BP:
            {
                cpu->branch_target_buffer[cpu->bfu.btb_index].target_address = cpu->memory_address;
                printf("BP positiveflag %d", cpu->positive_flag);
                if (cpu->positive_flag == TRUE)
                {
                    switch (cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction) {
                    case 0:
                        do_branching(cpu);
                        cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 1;
                        break;
                    case 1:
                        if(cpu->bfu.is_btb_hit == 0) {
                            do_branching(cpu);
                        } else {
                            cpu->decode.has_insn = TRUE;
                        }
                        cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 11;
                        break;
                    case 11:
                        cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 11;
                        cpu->decode.has_insn = TRUE;
                        break;
                    default:
                        break;
                }
                } else if(cpu->bfu.is_btb_hit == 1){
                    switch (cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction) {
                    case 0:
                        break;
                    case 1:
                        cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 0;
                        break;
                    case 11:
                        cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 1;
                        break;
                    default:
                        break;
                    }
                    cpu->pc = cpu->branch_target_buffer[cpu->bfu.btb_index].pc_address + 4;
                    cpu->decode.has_insn = FALSE;
                    cpu->fetch_from_next_cycle = TRUE;
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BNZ:
            {
                cpu->branch_target_buffer[cpu->bfu.btb_index].target_address = cpu->memory_address;
                printf("BNZ zeroflag %d", cpu->zero_flag);
                if (cpu->zero_flag == FALSE)
                {
                    switch (cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction) {
                    case 0:
                        do_branching(cpu);
                        cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 1;
                        break;
                    case 1:
                        if(cpu->bfu.is_btb_hit == 0) {
                            do_branching(cpu);
                        } else {
                            cpu->decode.has_insn = TRUE;
                        }
                        cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 11;
                        break;
                    case 11:
                        cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 11;
                        cpu->decode.has_insn = TRUE;
                        break;
                    default:
                        break;
                }
                } else if(cpu->bfu.is_btb_hit == 1){
                    switch (cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction) {
                    case 0:
                        break;
                    case 1:
                        cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 0;
                        break;
                    case 11:
                        cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 1;
                        break;
                    default:
                        break;
                    }
                    cpu->pc = cpu->branch_target_buffer[cpu->bfu.btb_index].pc_address + 4;
                    cpu->decode.has_insn = FALSE;
                    cpu->fetch_from_next_cycle = TRUE;
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BNP:
            {
                cpu->branch_target_buffer[cpu->bfu.btb_index].target_address = cpu->memory_address;
                printf("BNP positiveflag %d", cpu->positive_flag);
                if (cpu->positive_flag == FALSE)
                {
                    switch (cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction) {
                    case 0:
                        do_branching(cpu);
                        cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 1;
                        break;
                    case 1:
                        do_branching(cpu);
                        cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 11;
                        break;
                    case 11:
                        cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 11;
                        cpu->decode.has_insn = TRUE;
                        break;
                    default:
                        break;
                }
                } else if(cpu->bfu.is_btb_hit == 1){
                    switch (cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction) {
                        case 0:
                            break;
                        case 1:
                            cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 0;
                            break;
                        case 11:
                            cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 1;
                            break;
                        default:
                            break;
                    }
                    cpu->pc = cpu->branch_target_buffer[cpu->bfu.btb_index].pc_address + 4;
                    cpu->decode.has_insn = FALSE;
                    cpu->fetch_from_next_cycle = TRUE;
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BZ:
            {
                cpu->branch_target_buffer[cpu->bfu.btb_index].target_address = cpu->memory_address;
                printf("BZ zeroflag %d", cpu->zero_flag);
              if (cpu->zero_flag == TRUE) 
                {
                    switch (cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction) {
                    case 0:
                        do_branching(cpu);
                        cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 1;
                        break;
                    case 1:
                        do_branching(cpu);
                        cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 11;
                        break;
                    case 11:
                        cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 11;
                        cpu->decode.has_insn = TRUE;
                        break;
                    default:
                        break;
                }
                } else if(cpu->bfu.is_btb_hit == 1){
                    switch (cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction) {
                        case 0:
                            break;
                        case 1:
                            cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 0;
                            break;
                        case 11:
                            cpu->branch_target_buffer[cpu->bfu.btb_index].branch_prediction = 1;
                            break;
                        default:
                            break;
                    }
                    cpu->pc = cpu->branch_target_buffer[cpu->bfu.btb_index].pc_address + 4;
                    cpu->decode.has_insn = FALSE;
                    cpu->fetch_from_next_cycle = TRUE;
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_JALR:
            {
                /* Calculate new PC, and send it to fetch unit */
                cpu->bfu.result_buffer = cpu->memory_address;
                // cpu->pc = cpu->regs[cpu->execute.rs1] + cpu->execute.imm;
                cpu->pc = cpu->memory_address;

                /* Since we are using reverse callbacks for pipeline stages, 
                * this will prevent the new instruction from being fetched in the current cycle*/
                cpu->fetch_from_next_cycle = TRUE;

                /* Flush previous stages */
                cpu->decode.has_insn = FALSE;

                /* Make sure fetch stage is enabled to start fetching from new PC */
                cpu->fetch.has_insn = TRUE;
                break;
            }

            case OPCODE_JUMP:
            {
                /* Calculate new PC, and send it to fetch unit */
                cpu->bfu.result_buffer = cpu->memory_address;
                cpu->pc = cpu->memory_address;
                printf("New addres %d\n", cpu->bfu.result_buffer);
                /* Since we are using reverse callbacks for pipeline stages, 
                * this will prevent the new instruction from being fetched in the current cycle*/
                cpu->fetch_from_next_cycle = TRUE;

                /* Flush previous stages */
                cpu->decode.has_insn = FALSE;

                /* Make sure fetch stage is enabled to start fetching from new PC */
                cpu->fetch.has_insn = TRUE;
                break;
            }

        }
        cpu->bfu.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            display_stage_content("BFU", &cpu->bfu);
        }
    }
}


static void APEX_MulFu(APEX_CPU *cpu) {
    cpu->has_mulfu_data = FALSE;
    if(cpu->mulfu.has_insn) {
        int opcode = 0;
    switch(cpu->mulfu.has_insn) {
        case OPCODE_MUL:
        {
            cpu->mulfu.result_buffer = cpu->mulfu.iq_entry.src1_value * cpu->mulfu.iq_entry.src2_value;
            cpu->has_mulfu_data = TRUE;
            cpu->mulfu_data.dest_data = cpu->mulfu.result_buffer;
            cpu->mulfu_data.physical_address = cpu->mulfu.iq_entry.dest;
            if (cpu->mulfu.result_buffer == 0) {
                cpu->zero_flag = TRUE;
                cpu->mulfu_data.zero_flag = cpu->zero_flag;
            } else {
                cpu->zero_flag = FALSE;
                cpu->mulfu_data.zero_flag = cpu->zero_flag;
            }
            if(cpu->mulfu.result_buffer > 0) {
                cpu->positive_flag = TRUE;
                cpu->mulfu_data.positive_flag = cpu->positive_flag;
            } else {
                cpu->positive_flag = FALSE;
                cpu->mulfu_data.positive_flag = cpu->positive_flag;
            }
                // = cpu->mulfu.rs1_value * cpu->mulfu.rs2_value;
            /* Set the zero flag based on the result buffer */
            // set_branch_flags(cpu);
            // update_stalling_flags(cpu);
            printf("output is %d \n",cpu->mulfu.result_buffer);
            break;
            }
    }
    cpu->mulfu.has_insn = FALSE;

    if (ENABLE_DEBUG_MESSAGES)
    {
        display_stage_content("MulFu", &cpu->mulfu);
    }
}
}

static void APEX_MAU(APEX_CPU *cpu) {
    if(cpu->mau.has_insn) {
    int opcode = 0;
    switch(opcode) {
        case OPCODE_LOAD:
        case OPCODE_LOADP:
        {
                /* Read from data memory */
            cpu->mau.result_buffer
                = cpu->data_memory[cpu->memory_address];
            // cpu->memory.data_forward = cpu->memory.result_buffer;
            // printf("loadp %d", cpu->memory.data_forward);
            break;
        }

        case OPCODE_STORE:
        case OPCODE_STOREP:
        {
            /* Read from data memory */
            cpu->data_memory[cpu->memory_address] = cpu->mau.rs1_value;
            break;
        }
    }
    cpu->mau.has_insn = FALSE;

    if (ENABLE_DEBUG_MESSAGES)
    {
        display_stage_content("MAU", &cpu->mau);
    }
    }
}

static void APEX_IntFu(APEX_CPU *cpu) {
    cpu->has_intfu_data = FALSE;
    if(cpu->intfu.has_insn) {
        int opcode = 0;
        switch(opcode) {
            case OPCODE_ADD:
            {
                    cpu->intfu.result_buffer = cpu->intfu.iq_entry.src1_value + cpu->intfu.iq_entry.src2_value;
                    cpu->has_intfu_data = TRUE;
                    cpu->intfu_data.dest_data = cpu->intfu.result_buffer;
                    cpu->intfu_data.physical_address = cpu->intfu.iq_entry.dest;
                    // = cpu->execute.rs1_value + cpu->execute.rs2_value;
                    /* Set the zero flag based on the result buffer */
                    set_branch_flags(cpu);
                    // cpu->intfu.data_forward = cpu->intfu.result_buffer;
                    // update_stalling_flags(cpu);
                    printf("output is %d \n",cpu->intfu.result_buffer);
                break;
            }

            case OPCODE_DIV:
            {
                    cpu->intfu.result_buffer = cpu->intfu.iq_entry.src1_value / cpu->intfu.iq_entry.src2_value;
                    cpu->has_intfu_data = TRUE;
                    cpu->intfu_data.dest_data = cpu->intfu.result_buffer;
                    cpu->intfu_data.physical_address = cpu->intfu.iq_entry.dest;
                    // = cpu->execute.rs1_value / cpu->execute.rs2_value;
                    // cpu->intfu.data_forward = cpu->intfu.result_buffer;
                    /* Set the zero flag based on the result buffer */
                    set_branch_flags(cpu);
                    // update_stalling_flags(cpu);
                    printf("output is %d \n",cpu->intfu.result_buffer);
                break;
            }

            case OPCODE_ADDL:
            {
                    cpu->intfu.result_buffer = cpu->intfu.iq_entry.src1_value + cpu->intfu.iq_entry.literal;
                    cpu->has_intfu_data = TRUE;
                    cpu->intfu_data.dest_data = cpu->intfu.result_buffer;
                    cpu->intfu_data.physical_address = cpu->intfu.iq_entry.dest;
                    // = cpu->execute.rs1_value + cpu->execute.imm;
                    // cpu->intfu.data_forward = cpu->intfu.result_buffer;
                    /* Set the zero flag based on the result buffer */
                    set_branch_flags(cpu);
                    // if(cpu->execute.rd != cpu->execute.rs1) {
                    //     cpu->scoreBoarding[cpu->execute.rs1] = 0;
                    // } else {
                    //     cpu->scoreBoarding[cpu->execute.rs1] = 1;
                    // }
                    printf("output is %d \n",cpu->intfu.result_buffer);
                break;
            }

            case OPCODE_SUB:
            {
                    cpu->intfu.result_buffer = cpu->intfu.iq_entry.src1_value - cpu->intfu.iq_entry.src2_value;
                    cpu->has_intfu_data = TRUE;
                    cpu->intfu_data.dest_data = cpu->intfu.result_buffer;
                    cpu->intfu_data.physical_address = cpu->intfu.iq_entry.dest;
                    // = cpu->execute.rs1_value - cpu->execute.rs2_value;
                    // cpu->intfu.data_forward = cpu->intfu.result_buffer;
                    /* Set the zero flag based on the result buffer */
                    set_branch_flags(cpu);
                    // update_stalling_flags(cpu);
                    printf("output is %d \n",cpu->intfu.result_buffer);
                break;
            }

            case OPCODE_SUBL:
            {
                    cpu->intfu.result_buffer = cpu->intfu.iq_entry.src1_value - cpu->intfu.iq_entry.literal;
                    cpu->has_intfu_data = TRUE;
                    cpu->intfu_data.dest_data = cpu->intfu.result_buffer;
                    cpu->intfu_data.physical_address = cpu->intfu.iq_entry.dest;
                    // = cpu->execute.rs1_value - cpu->execute.imm;
                    // cpu->intfu.data_forward = cpu->intfu.result_buffer;
                    /* Set the zero flag based on the result buffer */
                    set_branch_flags(cpu);
                    // if(cpu->execute.rd != cpu->execute.rs1) {
                    //     cpu->scoreBoarding[cpu->execute.rs1] = 0;
                    // } else {
                    //     cpu->scoreBoarding[cpu->execute.rs1] = 1;
                    // }
                    printf("output is %d \n",cpu->intfu.result_buffer);
                break;
            }

            

            case OPCODE_AND:
            {
                    cpu->intfu.result_buffer = cpu->intfu.iq_entry.src1_value & cpu->intfu.iq_entry.src2_value;
                    cpu->has_intfu_data = TRUE;
                    cpu->intfu_data.dest_data = cpu->intfu.result_buffer;
                    cpu->intfu_data.physical_address = cpu->intfu.iq_entry.dest;
                    // = cpu->execute.rs1_value & cpu->execute.rs2_value;
                    // cpu->intfu.data_forward = cpu->intfu.result_buffer;
                    /* Set the zero flag based on the result buffer */
                    set_branch_flags(cpu);
                    // update_stalling_flags(cpu);
                    printf("output is %d \n",cpu->intfu.result_buffer);
                break;
            }

            case OPCODE_OR:
            {
                    cpu->intfu.result_buffer = cpu->intfu.iq_entry.src1_value | cpu->intfu.iq_entry.src2_value;
                    cpu->has_intfu_data = TRUE;
                    cpu->intfu_data.dest_data = cpu->intfu.result_buffer;
                    cpu->intfu_data.physical_address = cpu->intfu.iq_entry.dest;
                    // = cpu->execute.rs1_value | cpu->execute.rs2_value;
                    // cpu->intfu.data_forward = cpu->intfu.result_buffer;
                    /* Set the zero flag based on the result buffer */
                    set_branch_flags(cpu);
                    // update_stalling_flags(cpu);
                    printf("output is %d \n",cpu->intfu.result_buffer);
                break;
            }

            case OPCODE_XOR:
            {
                    cpu->execute.result_buffer = cpu->intfu.iq_entry.src1_value ^ cpu->intfu.iq_entry.src2_value;
                    cpu->has_intfu_data = TRUE;
                    cpu->intfu_data.dest_data = cpu->intfu.result_buffer;
                    cpu->intfu_data.physical_address = cpu->intfu.iq_entry.dest;
                    // = cpu->execute.rs1_value ^ cpu->execute.rs2_value;
                    cpu->intfu.data_forward = cpu->intfu.result_buffer;
                    /* Set the zero flag based on the result buffer */
                    set_branch_flags(cpu);
                    // update_stalling_flags(cpu);
                    printf("output is %d \n",cpu->intfu.result_buffer);
                break;
            }


            case OPCODE_MOVC: 
            {
                cpu->intfu.result_buffer = cpu->intfu.iq_entry.literal + 0;
                cpu->has_intfu_data = TRUE;
                cpu->intfu_data.dest_data = cpu->intfu.result_buffer;
                cpu->intfu_data.physical_address = cpu->intfu.iq_entry.dest;
                // cpu->intfu.data_forward = cpu->intfu.result_buffer;
                break;
            }

            case OPCODE_NOP:
            {
                break;
            }

            case OPCODE_CMP:
            {
                if(cpu->intfu.iq_entry.src1_value > cpu->intfu.iq_entry.src2_value) {
                    cpu->positive_flag = TRUE;
                    cpu->intfu_data.positive_flag = cpu->positive_flag;
                } else {
                    cpu->positive_flag = FALSE;
                    cpu->intfu_data.positive_flag = cpu->positive_flag;
                }
                if(cpu->intfu.iq_entry.src1_value == cpu->intfu.iq_entry.src2_value) {
                    cpu->zero_flag = TRUE;
                    cpu->intfu_data.zero_flag = cpu->zero_flag;
                } else {
                    cpu->zero_flag = FALSE;
                    cpu->intfu_data.zero_flag = cpu->zero_flag;
                }
                break;   
            }

            case OPCODE_CML:
            {
                if(cpu->intfu.iq_entry.src1_value > cpu->intfu.iq_entry.literal) {
                    cpu->positive_flag = TRUE;
                    cpu->intfu_data.positive_flag = cpu->positive_flag;
                } else {
                    cpu->positive_flag = FALSE;
                    cpu->intfu_data.positive_flag = cpu->positive_flag;
                }
                if(cpu->intfu.iq_entry.src1_value == cpu->intfu.iq_entry.literal) {
                    cpu->zero_flag = TRUE;
                    cpu->intfu_data.zero_flag = cpu->zero_flag;
                } else {
                    cpu->zero_flag = FALSE;
                    cpu->intfu_data.zero_flag = cpu->zero_flag;
                }
                break;   
            }
        }
        cpu->intfu.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            display_stage_content("IntFu", &cpu->intfu);
        }
    }
}

/*
 * Memory Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_memory(APEX_CPU *cpu)
{
    if (cpu->memory.has_insn)
    {
        switch (cpu->memory.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_AND:
            case OPCODE_DIV:
            {
                /* No work for ADD */
                // if(cpu->memory.rd != cpu->memory.rs1) {
                //     cpu->scoreBoarding[cpu->memory.rs1] = 0;
                // } else {
                //     cpu->scoreBoarding[cpu->memory.rs1] = 1;
                // }
                // if(cpu->memory.rd != cpu->memory.rs2) {
                //     cpu->scoreBoarding[cpu->memory.rs2] = 0;
                // } else {
                //     cpu->scoreBoarding[cpu->memory.rs2] = 1;
                // }
                cpu->memory.data_forward = cpu->memory.result_buffer;
                break;
            }

            case OPCODE_ADDL:
            case OPCODE_SUBL:
            {
                /* No work for ADD */
                // if(cpu->memory.rd != cpu->memory.rs1) {
                //     cpu->scoreBoarding[cpu->memory.rs1] = 0;
                // } else {
                //     cpu->scoreBoarding[cpu->memory.rs1] = 1;
                // }
                cpu->memory.data_forward = cpu->memory.result_buffer;
                cpu->scoreBoarding[cpu->memory.rs1] = 0;
                break;
            }


            case OPCODE_NOP:
            {
                break;
            }


            case OPCODE_MOVC:
            {
                cpu->memory.data_forward = cpu->memory.result_buffer;
            }
        }

        /* Copy data from memory latch to writeback latch*/
        cpu->writeback = cpu->memory;
        cpu->memory.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            display_stage_content("Memory", &cpu->memory);
        }
    }
}

/*
 * Writeback Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_writeback(APEX_CPU *cpu)
{
    if (cpu->writeback.has_insn)
    {
        /* Write result to register file based on instruction type */
        switch (cpu->writeback.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_DIV:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->scoreBoarding[cpu->writeback.rd] = 0;
                cpu->scoreBoarding[cpu->writeback.rs1] = 0;
                cpu->scoreBoarding[cpu->writeback.rs2] = 0;
                break;
            }

            case OPCODE_ADDL:
            case OPCODE_SUBL:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->scoreBoarding[cpu->writeback.rd] = 0;
                cpu->scoreBoarding[cpu->writeback.rs1] = 0;
                break;
            }

            case OPCODE_LOAD:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->scoreBoarding[cpu->writeback.rd] = 0;
                break;
            }

            case OPCODE_MOVC: 
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->scoreBoarding[cpu->writeback.rd] = 0;
                break;
            }

            case OPCODE_LOADP:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->regs[cpu->writeback.rs1] = cpu->writeback.updated_register_src1;
                cpu->scoreBoarding[cpu->writeback.rd] = 0;
                cpu->scoreBoarding[cpu->writeback.rs1] = 0;
                printf("src1 %d \n",cpu->regs[cpu->writeback.rs1]);
                break;
            }

            case OPCODE_NOP:
            {
                break;
            }

            case OPCODE_STORE:
            {
                printf("MEM[%d] : %d \n", cpu->writeback.memory_address, cpu->writeback.rs1_value);
                cpu->scoreBoarding[cpu->writeback.rs1] = 0;
                cpu->scoreBoarding[cpu->writeback.rs2] = 0;
                break;
            }

            case OPCODE_STOREP:
            {
                cpu->regs[cpu->writeback.rs2] = cpu->writeback.updated_register_src1;
                printf("MEM[%d] : %d \n", cpu->writeback.memory_address, cpu->writeback.rs1_value);
                cpu->scoreBoarding[cpu->writeback.rs1] = 0;
                cpu->scoreBoarding[cpu->writeback.rs2] = 0;
                break;
            }

            case OPCODE_CMP:
            {
                cpu->scoreBoarding[cpu->writeback.rs1] = 0;
                cpu->scoreBoarding[cpu->writeback.rs2] = 0;
                break;
            }

            case OPCODE_CML:
            {
                cpu->scoreBoarding[cpu->writeback.rs1] = 0;
                break;
            }

            case OPCODE_JALR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->scoreBoarding[cpu->writeback.rd] = 0;
                cpu->scoreBoarding[cpu->writeback.rs1] = 0;
                break;
            }
        }

        cpu->insn_completed++;
        cpu->writeback.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Writeback", &cpu->writeback);
        }

        if (cpu->writeback.opcode == OPCODE_HALT)
        {
            /* Stop the APEX simulator */
            return TRUE;
        }
    }

    /* Default */
    return 0;
}

void init_bq(APEX_CPU *cpu) {
    for (int i = 0; i < MAX_BQ_SIZE; i++) {
        cpu->bq[i].is_used = 0;
    }
    cpu->bq_size = 0;
}
void init_iq(APEX_CPU *cpu) {
    for (int i = 0; i < MAX_IQ_SIZE; i++) {
        cpu->iq[i].is_used = 0;
    }
    cpu->iq_size = 0;
}
/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename)
{
    int i;
    APEX_CPU *cpu;
    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }

    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    cpu->single_step = ENABLE_SINGLE_STEP;

    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }

    for(int i = 0; i < 4; i++) {
        cpu->branch_target_buffer[i].target_address = 0;
        cpu->branch_target_buffer[i].branch_prediction = 0;
        cpu->branch_target_buffer[i].pc_address = 0;
        cpu->branch_target_buffer[i].allocated = 0;
    }
    cpu->branch_target_buffer->branch_prediction = 00;

    int length = sizeof(cpu->rename_table) / sizeof(cpu->rename_table[0]);
    for(int i = 0; i < length; i++) {
        cpu->rename_table[i] = -1;
    }

    int physical_queue_length = sizeof(cpu->physical_queue) / sizeof(cpu->physical_queue[0]);
    for(int i = 0; i <physical_queue_length; i++) {
        cpu->physical_queue[i] = i;
    }
    cpu->physical_queue_length = physical_queue_length;


    cpu->bq_size = 0;
    cpu->bq_index = 0;
    cpu->iq_index = 0;
    init_bq(cpu);
    init_iq(cpu);


    cpu->counter = 0;
    cpu->index = 0;

    cpu->ROB_queue.ROB_head = -1;
    cpu->ROB_queue.ROB_tail = -1;
    cpu->ROB_queue.capacity = 0;

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
    cpu->free_list = 24;

    cpu->lsq.entries = (LSQEntry *)malloc(16 * sizeof(LSQEntry));
    cpu->lsq.front = -1;
    cpu->lsq.rear = -1;
    cpu->lsq.numberOfEntries = 0;
    return cpu;
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_run(APEX_CPU *cpu)
{
    char user_prompt_val;

    while (TRUE)
    {
        if(cpu->simulator_flag && cpu->counter >= cpu->simulate_counter) {
            break;
        }
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock);
            printf("--------------------------------------------\n");
            printf("P %d \n", cpu->positive_flag);
            printf("Z %d \n", cpu->zero_flag);
            printf("N %d \n", cpu->negative_flag);
        }

        if (APEX_writeback(cpu))
        {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
            break;
        }
        APEX_memory(cpu);
        APEX_execute(cpu);
        APEX_ROB(cpu);
        APEX_IntFu(cpu);
        APEX_MAU(cpu);
        APEX_MulFu(cpu);
        APEX_BFU(cpu);
        APEX_AFU(cpu);
        APEX_dispatch(cpu);
        APEX_LSQ(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);

        print_reg_file(cpu);
        printf("P %d \n", cpu->positive_flag);
        printf("Z %d \n", cpu->zero_flag);
        printf("N %d \n", cpu->negative_flag);

        if (!cpu->simulator_flag && cpu->single_step)
        {
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
        }
        cpu->bq_index = 0;
        cpu->bq_size = 0;
        cpu->iq_index = 0;
        cpu->iq_size = 0;

        cpu->clock++;
        cpu->counter++;
    }
}



/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}
