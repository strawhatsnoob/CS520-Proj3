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

static void update_src1_with_forwarded_data(APEX_CPU *cpu, int i) {
    // if (!cpu->has_afu_data &&
    //     cpu->afu_data.physical_address == cpu->dispatch.ps1) {
    //     cpu->iq_entries[i].src1_valid_bit = 1;
    //     cpu->iq_entries[i].src1_value = cpu->afu_data.updated_src_data;
    // }
    if (!cpu->has_mulfu_data[cpu->dispatch.ps1] &&
        cpu->mulfu_data[cpu->dispatch.ps1].physical_address == cpu->dispatch.ps1 
        && cpu->mulfu_data[cpu->dispatch.ps1].is_allocated) {
        cpu->iq_entries[i].src1_valid_bit = 1;
        cpu->iq_entries[i].src1_value = cpu->afu_data.dest_data;
    }
    if (!cpu->has_intfu_data[cpu->dispatch.ps1] &&
        cpu->intfu_data[cpu->dispatch.ps1].physical_address == cpu->dispatch.ps1
        && cpu->intfu_data[cpu->dispatch.ps1].is_allocated) {
        cpu->iq_entries[i].src1_valid_bit = 1;
        cpu->iq_entries[i].src1_value = cpu->intfu_data[cpu->dispatch.ps1].dest_data;
    }
}

static void update_src2_with_forwarded_data(APEX_CPU *cpu, int i) {
    // if (!cpu->has_afu_data &&
    //     cpu->afu_data.physical_address == cpu->dispatch.ps2) {
    //     cpu->iq_entries[i].src2_valid_bit = 1;
    //     cpu->iq_entries[i].src2_value = cpu->afu_data.updated_src_data;
    // }
    if (!cpu->has_mulfu_data[cpu->dispatch.ps2] &&
        cpu->mulfu_data[cpu->dispatch.ps2].physical_address == cpu->dispatch.ps2
        && cpu->mulfu_data[cpu->dispatch.ps2].is_allocated) {
        cpu->iq_entries[i].src2_valid_bit = 1;
        cpu->iq_entries[i].src2_value = cpu->afu_data.dest_data;
    }
    if (!cpu->has_intfu_data[cpu->dispatch.ps2] &&
        cpu->intfu_data[cpu->dispatch.ps2].physical_address == cpu->dispatch.ps2
        && cpu->intfu_data[cpu->dispatch.ps1].is_allocated) {
        cpu->iq_entries[i].src2_valid_bit = 1;
        cpu->iq_entries[i].src2_value = cpu->intfu_data[cpu->dispatch.ps1].dest_data;
    }
}

int check_wakeup_condition_issue(APEX_CPU *cpu, IQ_Entries *iq_entry) {
    // Check if the source operands are available and valid
    int src1_ready = 0;
    int src2_ready = 0;

    if (iq_entry->src1_valid_bit) {
        if (iq_entry->src1_value == -1) {
            // Source operand is a register
            if (cpu->rename_table[iq_entry->src1_tag] == -1) {
                src1_ready = 0; // Not available
            } else {
                src1_ready = 1; // Available
            }
        } else {
            // Source operand is a literal value
            src1_ready = 1; // Available
        }
        } else {
            // Source operand is not required
            // src1_ready = 1; // Available
        }

        if (iq_entry->src2_valid_bit) {
            if (iq_entry->src2_value == -1) {
                // Source operand is a register
                if (cpu->rename_table[iq_entry->src2_tag] == -1) {
                    src2_ready = 0; // Not available
                } else {
                    src2_ready = 1; // Available
                }
            } else {
                // Source operand is a literal value
                src2_ready = 1; // Available
            }
        } else {
            // Source operand is not required
            // src2_ready = 1; // Available
        }

        // Check if both source operands are available
        if (src1_ready && src2_ready) {

        return 1; // Instruction is ready to execute
        }
        // Updated: Check if source operands are available through bus monitoring
        if (cpu->data_forward[0].flag && (cpu->data_forward[0].physical_address == iq_entry->src1_tag)) {
            iq_entry->src1_value = cpu->data_forward[0].data;
            iq_entry->src1_valid_bit = 1;
        }

        if (cpu->data_forward[1].flag && (cpu->data_forward[1].physical_address == iq_entry->src2_tag)) {
            iq_entry->src2_value = cpu->data_forward[1].data;
            iq_entry->src2_valid_bit = 1;
        }

        return 0; // Instruction is not ready to execute
}
int check_wakeup_condition_branch(APEX_CPU *cpu, BQ_Entry *bq_entry) {
    // Check if the source operands are available and valid
    int src1_ready = 0;
    int src2_ready = 0;

    if (bq_entry->src1_valid_bit) {
        if (bq_entry->src1_value == -1) {
            // Source operand is a register
            if (cpu->rename_table[bq_entry->src1_tag] == -1) {
                src1_ready = 0; // Not available
            } else {
                src1_ready = 1; // Available
            }
        } else {
            // Source operand is a literal value
            src1_ready = 1; // Available
        }
        } else {
            // Source operand is not required
            src1_ready = 1; // Available
        }

        if (bq_entry->src2_valid_bit) {
            if (bq_entry->src2_value == -1) {
                // Source operand is a register
                if (cpu->rename_table[bq_entry->src2_tag] == -1) {
                    src2_ready = 0; // Not available
                } else {
                    src2_ready = 1; // Available
                }
            } else {
                // Source operand is a literal value
                src2_ready = 1; // Available
            }
        } else {
            // Source operand is not required
            src2_ready = 1; // Available
        }

        // Check if both source operands are available
        if (src1_ready && src2_ready) {

        return 1; // Instruction is ready to execute
        }
        // Updated: Check if source operands are available through bus monitoring
        if (cpu->data_forward[0].flag && (cpu->data_forward[0].physical_address == bq_entry->src1_tag)) {
            bq_entry->src1_value = cpu->data_forward[0].data;
            bq_entry->src1_valid_bit = 1;
        }

        if (cpu->data_forward[1].flag && (cpu->data_forward[1].physical_address == bq_entry->src2_tag)) {
            bq_entry->src2_value = cpu->data_forward[1].data;
            bq_entry->src2_valid_bit = 1;
        }

        return 0; // Instruction is not ready to execute
}

static void reinitialize_iq(APEX_CPU *cpu, int i) {
    cpu->iq_entries[i].allocated = 0;
    cpu->iq_entries[i].dest = 0;
    cpu->iq_entries[i].dispatch_time = 0;
    cpu->iq_entries[i].elapsed_cycles_at_dispatch = 0;
    cpu->iq_entries[i].is_used = 0;
    cpu->iq_entries[i].literal = 0;
    cpu->iq_entries[i].opcode = 0;
    cpu->iq_entries[i].pc_address = 0;
    cpu->iq_entries[i].src1_tag = 0;
    cpu->iq_entries[i].src1_valid_bit = 0;
    cpu->iq_entries[i].src1_value = 0;
    cpu->iq_entries[i].src2_valid_bit = 0;
    cpu->iq_entries[i].src2_value = 0;
    cpu->iq_entries[i].src2_tag = 0;
}

static void APEX_issue_queue(APEX_CPU *cpu) {
    if(cpu->iq_stage.has_insn) {
        // // Initialize IQ
    // for (int i = 0; i < 24; i++) {
    //     cpu->iq_entries[i].is_used = 0;
    // }
    // if (cpu->iq_size > 0) {
    //     // Issue instructions from the Instruction Queue
    //     int issue_ready = check_issue_ready(cpu->dispatch);
    //     if (issue_ready) {
    //         if (!(is_branch_instruction(cpu->dispatch.opcode))) {
    //             // Dispatch to Issue Queue (IQ)
    //             dispatch_to_IQ(cpu, &cpu->iq_entries[cpu->iq_index]);
    //             cpu->iq_index = (cpu->iq_index + 1) % 24;
    //             cpu->iq_size++;
    //         }
    //     }
    // }  
    // Check if there is forwarded data
        for (int i = 0; i < 24; i++) {
            if (cpu->iq_entries[i].allocated) {
                // if(!cpu->has_mulfu_data[cpu->iq_stage.ps1] && cpu->mulfu_data[cpu->iq_stage.ps1].physical_address == cpu->iq_entries[i].src1_tag
                // && cpu->mulfu_data[cpu->iq_stage.ps1].is_allocated) {
                //     cpu->iq_entries[i].src1_valid_bit = 1;
                //     cpu->iq_entries[i].src1_value = cpu->mulfu_data[cpu->iq_stage.ps1].dest_data;
                // }
                // if(!cpu->has_intfu_data[cpu->iq_stage.ps1] && cpu->intfu_data[cpu->iq_stage.ps1].physical_address == cpu->iq_entries[i].src1_tag
                //  && cpu->intfu_data[cpu->iq_stage.ps1].is_allocated) {
                //     cpu->iq_entries[i].src1_valid_bit = 1;
                //     cpu->iq_entries[i].src1_value = cpu->intfu_data[cpu->iq_stage.ps1].dest_data;
                // }

                if(!cpu->has_mulfu_data[i] && cpu->mulfu_data[i].physical_address == cpu->iq_entries[i].src1_tag
                && cpu->mulfu_data[i].is_allocated) {
                    cpu->iq_entries[i].src1_valid_bit = 1;
                    cpu->iq_entries[i].src1_value = cpu->mulfu_data[i].dest_data;
                }
                if(!cpu->has_intfu_data[i] && cpu->intfu_data[i].physical_address == cpu->iq_entries[i].src1_tag
                 && cpu->intfu_data[i].is_allocated) {
                    cpu->iq_entries[i].src1_valid_bit = 1;
                    cpu->iq_entries[i].src1_value = cpu->intfu_data[i].dest_data;
                }

                // if(!cpu->has_afu_data && cpu->afu_data.physical_address == cpu->iq_entries[i].src2_tag) {
                //     // cpu->physical_register[cpu->iq_stage.ps2].data = cpu->afu_data.updated_src_data;
                //     // cpu->physical_register[cpu->iq_stage.ps2].valid_bit = 1;
                //     cpu->iq_entries[i].src2_valid_bit = 1;
                //     cpu->iq_entries[i].src2_value = cpu->afu_data.dest_data;
                //     // cpu->physical_register[cpu->decode.ps1].allocated = 1;
                // }
                // if(!cpu->has_mulfu_data[cpu->iq_stage.ps2] && cpu->mulfu_data[cpu->iq_stage.ps2].physical_address == cpu->iq_entries[i].src2_tag
                // && cpu->mulfu_data[cpu->iq_stage.ps2].is_allocated) {
                //     cpu->iq_entries[i].src2_valid_bit = 1;
                //     cpu->iq_entries[i].src2_value = cpu->mulfu_data[cpu->iq_stage.ps2].dest_data;
                // }
                // if(!cpu->has_intfu_data[cpu->iq_stage.ps2] && cpu->intfu_data[cpu->iq_stage.ps2].physical_address == cpu->iq_entries[i].src2_tag
                // && cpu->intfu_data[cpu->iq_stage.ps2].is_allocated) {
                //     cpu->iq_entries[i].src2_valid_bit = 1;
                //     cpu->iq_entries[i].src2_value = cpu->intfu_data[cpu->iq_stage.ps2].dest_data;
                // }
                if(!cpu->has_mulfu_data[i] && cpu->mulfu_data[i].physical_address == cpu->iq_entries[i].src2_tag
                && cpu->mulfu_data[i].is_allocated) {
                    cpu->iq_entries[i].src2_valid_bit = 1;
                    cpu->iq_entries[i].src2_value = cpu->mulfu_data[i].dest_data;
                }
                if(!cpu->has_intfu_data[i] && cpu->intfu_data[i].physical_address == cpu->iq_entries[i].src2_tag
                && cpu->intfu_data[i].is_allocated) {
                    cpu->iq_entries[i].src2_valid_bit = 1;
                    cpu->iq_entries[i].src2_value = cpu->intfu_data[i].dest_data;
                }
            }
    }


        if (cpu->dispatch.is_used && cpu->dispatch.is_bq) {
        cpu->dispatch.simulate_counter = 1;
        }

        cpu->dispatch.simulate_counter = 1;  
        // Update wakeup logic for IQ entries
        for (int i = 0; i < 24; i++) {
            if (cpu->iq_entries[i].allocated) {
                // Check if the wakeup condition is met
                if (check_wakeup_condition_issue(cpu, &cpu->iq_entries[i])) {
                    // Set the instruction as ready to execute
                    switch(cpu->iq_entries[i].opcode) {
                        case OPCODE_LOAD:
                        case OPCODE_LOADP:
                        {
                            cpu->iq_stage.iq_afu = cpu->iq_entries[i];
                            cpu->afu = cpu->iq_stage;
                            cpu->VCount[cpu->iq_entries[i].src1_tag] -= 1;
                            break;
                        }
                        case OPCODE_STORE:
                        case OPCODE_STOREP:
                        {
                            cpu->iq_stage.iq_afu = cpu->iq_entries[i];
                            cpu->afu = cpu->iq_stage;
                            if(cpu->iq_entries[i].src1_tag == cpu->iq_entries[i].src2_tag){
                                cpu->VCount[cpu->iq_entries[i].src2_tag] -= 1;
                            }
                            else{
                                cpu->VCount[cpu->iq_entries[i].src2_tag] -= 1;
                                cpu->VCount[cpu->iq_entries[i].src1_tag] -= 1;
                            }
                            break;
                        }
                        case OPCODE_MUL:
                        {
                            // cpu->iq_entries[i].allocated = 0;
                            cpu->iq_stage.iq_mulfu = cpu->iq_entries[i];
                            cpu->mulfu = cpu->iq_stage;
                            if(cpu->iq_entries[i].src1_tag == cpu->iq_entries[i].src2_tag){
                                cpu->VCount[cpu->iq_entries[i].src2_tag] -= 1;
                            }
                            reinitialize_iq(cpu, i);
                            break;
                        }
                        case OPCODE_BP:
                        case OPCODE_BNP:
                        case OPCODE_BZ:
                        case OPCODE_BNZ:
                        case OPCODE_JUMP:
                        case OPCODE_JALR:
                            break;

                        case OPCODE_ADDL:
                        case OPCODE_SUBL:
                        {
                            cpu->iq_entries[i].allocated = 0;
                            cpu->iq_stage.iq_intfu = cpu->iq_entries[i];
                            cpu->intfu = cpu->iq_stage;
                            cpu->VCount[cpu->iq_entries[i].src1_tag] -= 1;
                        }
                        case OPCODE_CML:
                        {
                            cpu->iq_entries[i].allocated = 0;
                            cpu->iq_stage.iq_intfu = cpu->iq_entries[i];
                            cpu->intfu = cpu->iq_stage;
                            cpu->VCount[cpu->iq_entries[i].src1_tag] -= 1; 
                        }

                        default:
                        {
                            // reinitialize_iq(cpu, i);
                            cpu->iq_entries[i].allocated = 0;
                            cpu->iq_stage.iq_intfu = cpu->iq_entries[i];
                            cpu->intfu = cpu->iq_stage;
                            if(cpu->iq_entries[i].src1_tag == cpu->iq_entries[i].src2_tag){
                                cpu->VCount[cpu->iq_entries[i].src2_tag] -= 1;
                            }
                            else{
                                cpu->VCount[cpu->iq_entries[i].src2_tag] -= 1;
                                cpu->VCount[cpu->iq_entries[i].src1_tag] -= 1;
                            }
                            reinitialize_iq(cpu, i);
                            break;
                        }
                    }
                    
                }
            }
        }
        cpu->iq_stage.has_insn = FALSE;


        if (ENABLE_DEBUG_MESSAGES)
        {
            display_stage_content("Issue_Queue/RF", &cpu->iq_stage);
        }
    }

}

void APEX_branch_queue(APEX_CPU *cpu) {
    // Initialize BQ
    for (int i = 0; i < 16; i++) {
        cpu->bq[i].is_used = 0;
    }
    if (cpu->bq_size > 0) {
        // Issue instructions from the Branch Queue
        int issue_ready = check_issue_ready(cpu->dispatch);
        if (issue_ready) {
                if (is_branch_instruction(cpu->dispatch.opcode)) {
                    // Dispatch to Branch Queue (BQ)
                    dispatch_to_BQ(cpu, &cpu->bq[cpu->bq_index]);
                    cpu->bq_index = (cpu->bq_index + 1) % 16;
                    cpu->bq_size++;
                }
        }
    }
    for (int i = 0; i < 24; i++) {
        if (cpu->bq[i].is_used) {
            // Check if the wakeup condition is met
            if (check_wakeup_condition_branch(cpu, &cpu->bq[i])) {
                // Set the instruction as ready to execute
                switch(cpu->bq[i].opcode) {
                    case OPCODE_BP:
                    case OPCODE_BNP:
                    case OPCODE_BZ:
                    case OPCODE_BNZ:
                    case OPCODE_JUMP:
                    case OPCODE_JALR:
                        cpu->bq_stage.bq_bfu = cpu->bq[i];
                        cpu->bfu = cpu->bq_stage;
                        break;
                    default:
                    {
                        break;
                    }
                }
            }
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


static void rename_rd(APEX_CPU *cpu) {
    int length = cpu->physical_queue_length;
    if(cpu->isRenamed[cpu->decode.rd] == 0){
        cpu->isRenamed[cpu->decode.rd] = 1;
    }
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
        cpu->VCount[cpu->decode.rs1] += 1;
        if(!cpu->has_afu_data && cpu->afu_data.physical_address == cpu->decode.ps1) {
            cpu->physical_register[cpu->decode.ps1].data = cpu->afu_data.updated_src_data;
            cpu->physical_register[cpu->decode.ps1].valid_bit = 1;
            // cpu->physical_register[cpu->decode.ps1].allocated = 1;
        }
        if(!cpu->has_mulfu_data[cpu->decode.ps1] && cpu->mulfu_data[cpu->decode.ps1].physical_address == cpu->decode.ps1) {
            cpu->physical_register[cpu->decode.ps1].data = cpu->mulfu_data[cpu->decode.ps1].dest_data;
            cpu->physical_register[cpu->decode.ps1].valid_bit = 1;
            // cpu->physical_register[cpu->decode.ps1].allocated = 1;
        }
        if(!cpu->has_intfu_data[cpu->decode.ps1] && cpu->intfu_data[cpu->decode.ps1].physical_address == cpu->decode.ps1
        && cpu->intfu_data[cpu->decode.ps1].is_allocated) {
            cpu->physical_register[cpu->decode.ps1].data = cpu->intfu_data[cpu->decode.ps1].dest_data;
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
    cpu->VCount[cpu->decode.rs2] += 1;
    for(int i = 0; i < length; i++) {
      if (cpu->rename_table[cpu->physical_queue[i]] == cpu->decode.rs2) {
        rs2_flag = 1;
        cpu->decode.ps2 = cpu->physical_queue[i];
        cpu->physical_register[cpu->decode.ps2].allocated = 1;
        // if(!cpu->has_afu_data && cpu->afu_data.physical_address == cpu->decode.ps2) {
        //     cpu->physical_register[cpu->decode.ps2].data = cpu->afu_data.updated_src_data;
        //     cpu->physical_register[cpu->decode.ps2].valid_bit = 1;
        //     // cpu->physical_register[cpu->decode.ps1].allocated = 1;
        // }
        if(!cpu->has_mulfu_data[cpu->decode.ps2] && cpu->mulfu_data[cpu->decode.ps2].physical_address == cpu->decode.ps2
        && cpu->mulfu_data[cpu->decode.ps2].is_allocated) {
            cpu->physical_register[cpu->decode.ps2].data = cpu->mulfu_data[cpu->decode.ps2].dest_data;
            cpu->physical_register[cpu->decode.ps2].valid_bit = 1;
            // cpu->physical_register[cpu->decode.ps1].allocated = 1;
        }
        if(!cpu->has_intfu_data[cpu->decode.ps2] && cpu->intfu_data[cpu->decode.ps2].physical_address == cpu->decode.ps2
        && cpu->intfu_data[cpu->decode.ps2].is_allocated) {
            cpu->physical_register[cpu->decode.ps2].data = cpu->intfu_data[cpu->decode.ps2].dest_data;
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
                cpu->iq_entries[i].literal = cpu->dispatch.imm;
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
                cpu->iq_entries[i].literal = cpu->dispatch.imm;
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
                cpu->iq_entries[i].literal = cpu->dispatch.imm;
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
                cpu->iq_entries[i].literal = cpu->dispatch.imm;
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

static void iq_entry_pd(APEX_CPU *cpu) {
    for(int i = 0; i < 24; i++) {
            if(cpu->iq_entries[i].allocated == 0) {
                cpu->iq_entries[i].allocated = 1;
                cpu->iq_entries[i].opcode = cpu->dispatch.opcode;
                cpu->iq_entries[i].dest = cpu->dispatch.pd;
                cpu->iq_entries[i].src1_valid_bit = 1;
                cpu->iq_entries[i].src2_valid_bit = 1;
                cpu->iq_entries[i].literal = cpu->dispatch.imm;
                break;
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
            if(cpu->ROB_queue.rob_entries[cpu->ROB_queue.ROB_head].entry_bit == 1 && cpu->rename_table[cpu->physical_queue[i]] == cpu->ROB_queue.rob_entries[cpu->ROB_queue.ROB_head].dest_phsyical_register && cpu->VCount[cpu->rename_table[cpu->physical_queue[i]]] == 0) {
                if(cpu->physical_register[cpu->physical_queue[i]].allocated == 1) {
                    ROB_Entries current_entry = dequeue(cpu);
                    do_commit(cpu->physical_register[cpu->physical_queue[i]], cpu->ROB_queue.rob_entries[cpu->ROB_queue.ROB_head].dest_arch_register ,
                    cpu);
                    cpu->rename_table[cpu->physical_queue[i]] = -1;
                    cpu->rob.has_insn = FALSE;


                    if (ENABLE_DEBUG_MESSAGES)
                    {
                        display_stage_content("ROB/RF", &cpu->decode);
                    }
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
            case OPCODE_HALT:
            {
                iq_entry_pd(cpu);
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
                    if(cpu->bq[i].allocated == 0) {
                        cpu->bq[i].allocated = 1;
                        cpu->bq[i].opcode = cpu->dispatch.opcode;
                        cpu->bq[i].dest = cpu->dispatch.pd;
                        cpu->bq[i].src1_valid_bit = 1;
                        cpu->bq[i].src2_valid_bit = 1;
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

        if (cpu->dispatch.is_used && cpu->dispatch.data_forward) {
            for (int i = 0; i < 24; i++) {
                if (cpu->iq_entries[i].is_used && cpu->iq_entries[i].src1_valid_bit &&
                    cpu->iq_entries[i].src1_tag == cpu->dispatch.rd) {
                    // Forward the result to dependent instructions in IQ
                    cpu->iq_entries[i].src1_value = cpu->dispatch.result_buffer;
                    cpu->iq_entries[i].src1_valid_bit = 1;
                }
                if (cpu->iq_entries[i].is_used && cpu->iq_entries[i].src2_valid_bit &&
                    cpu->iq_entries[i].src2_tag == cpu->dispatch.rd) {
                    // Forward the result to dependent instructions in IQ
                    cpu->iq_entries[i].src2_value = cpu->dispatch.result_buffer;
                    cpu->iq_entries[i].src2_valid_bit = 1;
                }
            }
        }
       /*if (cpu->dispatch.has_insn) {
            int issue_ready = check_issue_ready(cpu->dispatch);

            if (issue_ready) {
                if (is_branch_instruction(cpu->dispatch.opcode)) {
                    // Dispatch to Branch Queue (BQ)
                    dispatch_to_BQ(cpu, &cpu->bq[cpu->bq_index]);
                    cpu->bq_index = (cpu->bq_index + 1) % 16;
                    cpu->bq_size++;
                } else {
                    // Dispatch to Issue Queue (IQ)
                    dispatch_to_IQ(cpu, &cpu->iq_entries[cpu->iq_index]);
                    cpu->iq_index = (cpu->iq_index + 1) % 24;
                    cpu->iq_size++;
                }
            }
        }*/

        /* // Check if there is forwarded data
        if (cpu->data_forward[0].flag || cpu->data_forward[1].flag) {
            for (int i = 0; i < 24; i++) {
                if (cpu->iq_entries[i].allocated) {
                    if (cpu->data_forward[0].flag && cpu->iq_entries[i].src1_tag == cpu->data_forward[0].physical_address) {
                        cpu->iq_entries[i].src1_value = cpu->data_forward[0].data;
                        cpu->iq_entries[i].src1_valid_bit = TRUE;
                    }

                    if (cpu->data_forward[1].flag && cpu->iq_entries[i].src2_tag == cpu->data_forward[1].physical_address) {
                        cpu->iq_entries[i].src2_value = cpu->data_forward[1].data;
                        cpu->iq_entries[i].src2_valid_bit = TRUE;
                    }
                }
            }

            // Clear forwarding information
            cpu->data_forward[0].flag = 0;
            cpu->data_forward[1].flag = 0;
        }

        if (cpu->dispatch.is_used && cpu->dispatch.is_bq) {
        cpu->dispatch.simulate_counter = 1;
        }

        cpu->dispatch.simulate_counter = 1; */


        /* Copy data from decode latch to execute latch*/
    
        cpu->lsqStage = cpu->dispatch;
        cpu->iq_stage = cpu->dispatch;
        cpu->bq_stage = cpu->dispatch;
        cpu->rob = cpu->dispatch;

        cpu->dispatch.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            display_stage_content("Dispatch/RF", &cpu->dispatch);
        }
    }
}

int is_branch_instruction(int opcode) {
    return (opcode == OPCODE_BZ || opcode == OPCODE_BNZ || opcode == OPCODE_JUMP || opcode == OPCODE_JALR);
}

int check_issue_ready(CPU_Stage stage) {
    return (stage.is_empty_rs1 || stage.rs1_value) &&
           (stage.is_empty_rs2 || stage.rs2_value);
}

void dispatch_to_IQ(APEX_CPU *cpu, IQ_Entries *iq_entry) {
    iq_entry->allocated = 1;
    iq_entry->opcode = cpu->dispatch.opcode;
    iq_entry->literal = cpu->dispatch.imm;
    iq_entry->src1_valid_bit = !cpu->dispatch.is_empty_rs1;
    iq_entry->src1_tag = cpu->dispatch.rs1;
    iq_entry->src1_value = cpu->dispatch.rs1_value;
    iq_entry->src2_valid_bit = !cpu->dispatch.is_empty_rs2;
    iq_entry->src2_tag = cpu->dispatch.rs2;
    iq_entry->src2_value = cpu->dispatch.rs2_value;
    iq_entry->dest = cpu->dispatch.rd;
    iq_entry->pc_address = cpu->dispatch.pc;
    iq_entry->is_used = 1;
    iq_entry->dispatch_time = cpu->counter;
}

void dispatch_to_BQ(APEX_CPU *cpu, BQ_Entry *bq_entry) {
    bq_entry->allocated = 1;
    bq_entry->opcode = cpu->dispatch.opcode;
    bq_entry->literal = cpu->dispatch.imm;
    bq_entry->src1_valid_bit = !cpu->dispatch.is_empty_rs1;
    bq_entry->src1_tag = cpu->dispatch.rs1;
    bq_entry->src1_value = cpu->dispatch.rs1_value;
    bq_entry->src2_valid_bit = !cpu->dispatch.is_empty_rs2;
    bq_entry->src2_tag = cpu->dispatch.rs2;
    bq_entry->src2_value = cpu->dispatch.rs2_value;
    bq_entry->dest = cpu->dispatch.rd;
    bq_entry->pc_address = cpu->dispatch.pc;
    bq_entry->branch_prediction = 0;
    bq_entry->target_address = 0;
    bq_entry->is_used = 1;
    bq_entry->index = cpu->counter;

    if (cpu->data_forward[0].flag) {
        bq_entry->src1_value = cpu->data_forward[0].data;
        bq_entry->src1_valid_bit = TRUE;
    }

    if (cpu->data_forward[1].flag) {
        bq_entry->src2_value = cpu->data_forward[1].data;
        bq_entry->src2_valid_bit = TRUE;
    }

    // Clear forwarding information
    cpu->data_forward[0].flag = 0;
    cpu->data_forward[1].flag = 0;

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

                cpu->lsqStage.dqLsq = LSQ_dequeue(cpu);
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
                if(cpu->decode.ps1 == cpu->decode.ps2){
                    cpu->VCount[cpu->decode.ps2] -= 1;
                }
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
                if(cpu->decode.ps1 == cpu->decode.ps2){
                    cpu->VCount[cpu->decode.ps2] -= 1;
                }
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
                if(cpu->decode.ps1 == cpu->decode.ps2){
                    cpu->VCount[cpu->decode.ps2] -= 1;
                }
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

        // Check if forwarding cycles are active
        if (cpu->forwarding_cycles > 0) {
            if (cpu->decode.is_used && cpu->decode.is_bq) {
                // Check if forwarding data is valid
                if (cpu->data_forward[0].flag) {
                    cpu->decode.rs1_value = cpu->data_forward[0].data;
                    cpu->decode.is_empty_rs1 = FALSE;
                }

                // Check if both sources have been forwarded
                if (cpu->data_forward[1].flag) {
                    cpu->decode.rs2_value = cpu->data_forward[1].data;
                    cpu->decode.is_empty_rs2 = FALSE;
                }

                // Clear forwarding information
                cpu->data_forward[0].flag = 0;
                cpu->data_forward[1].flag = 0;
            }

            // Decrement forwarding cycles
            cpu->forwarding_cycles--;
        } else {
            // Check if instruction is used and in the branch queue
            if (cpu->decode.is_used && cpu->decode.is_bq) {
                // Set forwarding cycles for tag and data broadcast
                cpu->forwarding_cycles = 2;
            }
        }
        cpu->decode.simulate_counter = 1;

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
        cpu->intfu_data[cpu->decode.ps1].zero_flag = cpu->zero_flag;
    } else {
        cpu->zero_flag = FALSE;
        cpu->intfu_data[cpu->decode.ps1].zero_flag = cpu->zero_flag;
    }
    if(cpu->intfu.result_buffer > 0) {
        cpu->positive_flag = TRUE;
        cpu->intfu_data[cpu->decode.ps1].positive_flag = cpu->positive_flag;
    } else {
      cpu->positive_flag = FALSE;
      cpu->intfu_data[cpu->decode.ps1].positive_flag = cpu->positive_flag;
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

        /* for (int i = 0; i < cpu->iq_size; ++i) {
            afu_entry = &cpu->iq_stage.entries[i];

            // Check if the instruction is ready to be issued
            if (iq_entry->is_ready) {
                // Issue the instruction to the AFU
                cpu->afu = cpu->afu_entry;
                cpu->afu_stage.has_insn = TRUE;

            }
            // Reset readiness status for the IQ entry
            iq_entry->is_ready = FALSE;
        }*/

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

        if (cpu->execute.is_used && cpu->execute.data_forward) {
            for (int i = 0; i < 24; i++) {
                if (cpu->iq_entries[i].is_used && cpu->iq_entries[i].src1_valid_bit &&
                    cpu->iq_entries[i].src1_tag == cpu->execute.rd) {
                    // Forward the result to dependent instructions in IQ
                    cpu->iq_entries[i].src1_value = cpu->execute.result_buffer;
                    cpu->iq_entries[i].src1_valid_bit = 1;
                }
                if (cpu->iq_entries[i].is_used && cpu->iq_entries[i].src2_valid_bit &&
                    cpu->iq_entries[i].src2_tag == cpu->execute.rd) {
                    // Forward the result to dependent instructions in IQ
                    cpu->iq_entries[i].src2_value = cpu->execute.result_buffer;
                    cpu->iq_entries[i].src2_valid_bit = 1;
                }
            }
        }

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
    cpu->has_mulfu_data[cpu->mulfu.iq_mulfu.dest] = FALSE;
    if(cpu->mulfu.has_insn) {
        int opcode = cpu->mulfu.iq_mulfu.opcode;
    switch(opcode) {
        case OPCODE_MUL:
        {
            cpu->mulfu.result_buffer = cpu->mulfu.iq_mulfu.src1_value * cpu->mulfu.iq_mulfu.src2_value;
            cpu->has_mulfu_data[cpu->mulfu.iq_mulfu.dest] = TRUE;
            cpu->mulfu_data[cpu->mulfu.iq_mulfu.dest].is_allocated = TRUE;
            cpu->mulfu_data[cpu->mulfu.iq_mulfu.dest].dest_data = cpu->mulfu.result_buffer;
            cpu->mulfu_data[cpu->mulfu.iq_mulfu.dest].physical_address = cpu->mulfu.iq_mulfu.dest;
            if (cpu->mulfu.result_buffer == 0) {
                cpu->zero_flag = TRUE;
                cpu->mulfu_data[cpu->mulfu.iq_mulfu.dest].zero_flag = cpu->zero_flag;
            } else {
                cpu->zero_flag = FALSE;
                cpu->mulfu_data[cpu->mulfu.iq_mulfu.dest].zero_flag = cpu->zero_flag;
            }
            if(cpu->mulfu.result_buffer > 0) {
                cpu->positive_flag = TRUE;
                cpu->mulfu_data[cpu->mulfu.iq_mulfu.dest].positive_flag = cpu->positive_flag;
            } else {
                cpu->positive_flag = FALSE;
                cpu->mulfu_data[cpu->mulfu.iq_mulfu.dest].positive_flag = cpu->positive_flag;
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
    cpu->has_mau_data = FALSE;
    if(cpu->mau.has_insn) {
    int opcode = 0;
    switch(opcode) {
        case OPCODE_LOAD:
        case OPCODE_LOADP:
        {

            cpu->has_mau_data = TRUE;
            cpu->mau_data.physical_address = cpu->mau.iq_entry.dest;
                /* Read from data memory */
            cpu->mau.result_buffer
                = cpu->data_memory[cpu->memory_address];
                cpu->mau_data.dest_data = cpu->mau.result_buffer;
            // cpu->memory.data_forward = cpu->memory.result_buffer;
            // printf("loadp %d", cpu->memory.data_forward);
            break;
        }

        case OPCODE_STORE:
        case OPCODE_STOREP:
        {
            /* Read from data memory */
            cpu->data_memory[cpu->memory_address] = cpu->mau.dqLsq.srcTag;
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
    cpu->has_intfu_data[cpu->intfu.iq_intfu.dest] = FALSE;
    if(cpu->intfu.has_insn) {
        int opcode = cpu->intfu.iq_intfu.opcode;
        switch(opcode) {
            case OPCODE_ADD:
            {
                    cpu->intfu.result_buffer = cpu->intfu.iq_intfu.src1_value + cpu->intfu.iq_intfu.src2_value;
                    cpu->has_intfu_data[cpu->intfu.iq_intfu.dest] = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].is_allocated = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].dest_data = cpu->intfu.result_buffer;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].physical_address = cpu->intfu.iq_intfu.dest;
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
                    cpu->intfu.result_buffer = cpu->intfu.iq_intfu.src1_value / cpu->intfu.iq_intfu.src2_value;
                    cpu->has_intfu_data[cpu->intfu.iq_intfu.dest] = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].is_allocated = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].dest_data = cpu->intfu.result_buffer;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].physical_address = cpu->intfu.iq_intfu.dest;
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
                    cpu->intfu.result_buffer = cpu->intfu.iq_intfu.src1_value + cpu->intfu.iq_intfu.literal;
                    cpu->has_intfu_data[cpu->intfu.iq_intfu.dest] = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].is_allocated = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].dest_data = cpu->intfu.result_buffer;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].physical_address = cpu->intfu.iq_intfu.dest;
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
                    cpu->intfu.result_buffer = cpu->intfu.iq_intfu.src1_value - cpu->intfu.iq_intfu.src2_value;
                    cpu->has_intfu_data[cpu->intfu.iq_intfu.dest] = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].is_allocated = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].dest_data = cpu->intfu.result_buffer;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].physical_address = cpu->intfu.iq_intfu.dest;
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
                    cpu->intfu.result_buffer = cpu->intfu.iq_intfu.src1_value - cpu->intfu.iq_intfu.literal;
                    cpu->has_intfu_data[cpu->intfu.iq_intfu.dest] = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].is_allocated = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].dest_data = cpu->intfu.result_buffer;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].physical_address = cpu->intfu.iq_intfu.dest;
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
                    cpu->intfu.result_buffer = cpu->intfu.iq_intfu.src1_value & cpu->intfu.iq_intfu.src2_value;
                    cpu->has_intfu_data[cpu->intfu.iq_intfu.dest] = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].is_allocated = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].dest_data = cpu->intfu.result_buffer;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].physical_address = cpu->intfu.iq_intfu.dest;
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
                    cpu->intfu.result_buffer = cpu->intfu.iq_intfu.src1_value | cpu->intfu.iq_intfu.src2_value;
                    cpu->has_intfu_data[cpu->intfu.iq_intfu.dest] = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].is_allocated = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].dest_data = cpu->intfu.result_buffer;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].physical_address = cpu->intfu.iq_intfu.dest;
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
                    cpu->execute.result_buffer = cpu->intfu.iq_intfu.src1_value ^ cpu->intfu.iq_intfu.src2_value;
                    cpu->has_intfu_data[cpu->intfu.iq_intfu.dest] = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].is_allocated = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].dest_data = cpu->intfu.result_buffer;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].physical_address = cpu->intfu.iq_intfu.dest;
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
                cpu->intfu.result_buffer = cpu->intfu.iq_intfu.literal + 0;
                cpu->has_intfu_data[cpu->intfu.iq_intfu.dest] = TRUE;
                cpu->intfu_data[cpu->intfu.iq_intfu.dest].is_allocated = TRUE;
                cpu->intfu_data[cpu->intfu.iq_intfu.dest].dest_data = cpu->intfu.result_buffer;
                cpu->intfu_data[cpu->intfu.iq_intfu.dest].physical_address = cpu->intfu.iq_intfu.dest;
                // cpu->intfu.data_forward = cpu->intfu.result_buffer;
                break;
            }

            case OPCODE_NOP:
            {
                break;
            }

            case OPCODE_CMP:
            {
                if(cpu->intfu.iq_intfu.src1_value > cpu->intfu.iq_intfu.src2_value) {
                    cpu->positive_flag = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].positive_flag = cpu->positive_flag;
                } else {
                    cpu->positive_flag = FALSE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].positive_flag = cpu->positive_flag;
                }
                if(cpu->intfu.iq_intfu.src1_value == cpu->intfu.iq_intfu.src2_value) {
                    cpu->zero_flag = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].zero_flag = cpu->zero_flag;
                } else {
                    cpu->zero_flag = FALSE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].zero_flag = cpu->zero_flag;
                }
                break;   
            }

            case OPCODE_CML:
            {
                if(cpu->intfu.iq_intfu.src1_value > cpu->intfu.iq_intfu.literal) {
                    cpu->positive_flag = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].positive_flag = cpu->positive_flag;
                } else {
                    cpu->positive_flag = FALSE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].positive_flag = cpu->positive_flag;
                }
                if(cpu->intfu.iq_intfu.src1_value == cpu->intfu.iq_intfu.literal) {
                    cpu->zero_flag = TRUE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].zero_flag = cpu->zero_flag;
                } else {
                    cpu->zero_flag = FALSE;
                    cpu->intfu_data[cpu->intfu.iq_intfu.dest].zero_flag = cpu->zero_flag;
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

void init_iq_stage(APEX_CPU *cpu) {
    cpu->iq_stage.is_used = 0;
}

void init_bq_stage(APEX_CPU *cpu) {
    cpu->bq_stage.is_used = 0;
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
    init_iq_stage(cpu);
    init_bq_stage(cpu);


    cpu->counter = 0;
    cpu->index = 0;

    cpu->forwarding_cycles = 0;

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
        
        APEX_execute(cpu);
        APEX_memory(cpu);
        APEX_ROB(cpu);
        APEX_MAU(cpu);
        APEX_LSQ(cpu);
        APEX_AFU(cpu);
        APEX_BFU(cpu);
        APEX_MulFu(cpu);
        APEX_IntFu(cpu);
        APEX_issue_queue(cpu);
        APEX_dispatch(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);

        for(int i = 0; i < 24; i++) {
            cpu->has_intfu_data[i] = FALSE;
            cpu->has_mulfu_data[i] = FALSE;
        }
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
