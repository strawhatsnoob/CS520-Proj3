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

        /* Copy data from fetch latch to decode latch*/
        cpu->decode = cpu->fetch;

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

static void update_with_forwarded_value(APEX_CPU *cpu) {
    if(cpu->writeback.has_insn) {
                    if(strcmp(cpu->writeback.opcode_str, "LOADP") == 0) {
                        printf("loadedP value %d", cpu->writeback.data_forward);
                        if(cpu->writeback.rd == cpu->decode.rs1 && cpu->writeback.rd == cpu->decode.rs2) {
                            cpu->decode.rs1_value = cpu->writeback.data_forward;
                            cpu->decode.rs2_value = cpu->writeback.data_forward;
                            cpu->is_data_forwarded = 1;
                        } else if(cpu->writeback.rd == cpu->decode.rs1) {
                            cpu->decode.rs1_value = cpu->writeback.data_forward;
                            cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                            cpu->is_data_forwarded = 1;
                        } else if(cpu->writeback.rd == cpu->decode.rs2) {
                            cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                            cpu->decode.rs2_value = cpu->writeback.data_forward;
                            cpu->is_data_forwarded = 1;
                        } else if(cpu->writeback.rs1 == cpu->decode.rs1 && cpu->writeback.rs1 == cpu->decode.rs2) {
                            cpu->decode.rs1_value = cpu->writeback.updated_register_src1;
                            cpu->decode.rs2_value = cpu->writeback.updated_register_src1;
                            cpu->is_data_forwarded = 1;
                        } else if(cpu->writeback.rs1 == cpu->decode.rs1) {
                            cpu->decode.rs1_value = cpu->writeback.updated_register_src1;
                            cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                            cpu->is_data_forwarded = 1;
                        } else if(cpu->writeback.rs1 == cpu->decode.rs2) {
                            cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                            cpu->decode.rs2_value = cpu->writeback.updated_register_src1;
                            cpu->is_data_forwarded = 1;
                        } else {
                            cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                            cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                        }
                    } else if(strcmp(cpu->writeback.opcode_str, "STOREP") == 0) {
                        if(cpu->writeback.rs2 == cpu->decode.rs1 && cpu->writeback.rs1 == cpu->decode.rs2) {
                            cpu->decode.rs1_value = cpu->writeback.data_forward;
                            cpu->decode.rs2_value = cpu->writeback.data_forward;
                            cpu->is_data_forwarded = 1;
                        } else if(cpu->writeback.rs2 == cpu->decode.rs1) {
                            cpu->decode.rs1_value = cpu->writeback.data_forward;
                            cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                            cpu->is_data_forwarded = 1;
                        } else if(cpu->writeback.rs2 == cpu->decode.rs2) {
                            cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                            cpu->decode.rs2_value = cpu->writeback.data_forward;
                            cpu->is_data_forwarded = 1;
                        } else {
                            cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                            cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                        }
                    } else {
                        if(cpu->writeback.rd == cpu->decode.rs1 && cpu->writeback.rd == cpu->decode.rs2) {
                            cpu->decode.rs1_value = cpu->writeback.data_forward;
                            cpu->decode.rs2_value = cpu->writeback.data_forward;
                            cpu->is_data_forwarded = 1;
                        } else if(cpu->writeback.rd == cpu->decode.rs1) {
                            cpu->decode.rs1_value = cpu->writeback.data_forward;
                            cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                            cpu->is_data_forwarded = 1;
                        } else if(cpu->writeback.rd == cpu->decode.rs2) {
                            cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                            cpu->decode.rs2_value = cpu->writeback.data_forward;
                            cpu->is_data_forwarded = 1;
                        } else {
                            cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                            cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                        }
                    }
                }
                if(cpu->memory.has_insn) {
                    if(strcmp(cpu->memory.opcode_str, "LOADP") == 0) {
                        printf("loadedP value %d", cpu->memory.data_forward);
                        if(cpu->memory.rs1 == cpu->decode.rs1 && cpu->memory.rs1 == cpu->decode.rs2) {
                            cpu->decode.rs1_value = cpu->memory.data_forward;
                            cpu->decode.rs2_value = cpu->memory.data_forward;
                        } else if(cpu->memory.rs1 == cpu->decode.rs1) {
                            cpu->decode.rs1_value = cpu->memory.data_forward;
                            if(!cpu->is_data_forwarded)
                                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                        } else if(cpu->memory.rs1 == cpu->decode.rs2) {
                            if(!cpu->is_data_forwarded)
                                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                            cpu->decode.rs2_value = cpu->memory.data_forward;
                        } else if(!cpu->is_data_forwarded){
                            cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                            cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                        }
                    } else if(strcmp(cpu->memory.opcode_str, "STOREP") == 0) {
                        if(cpu->memory.rs2 == cpu->decode.rs1 && cpu->memory.rs1 == cpu->decode.rs2) {
                            cpu->decode.rs1_value = cpu->memory.data_forward;
                            cpu->decode.rs2_value = cpu->memory.data_forward;
                        } else if(cpu->memory.rs2 == cpu->decode.rs1) {
                            cpu->decode.rs1_value = cpu->memory.data_forward;
                            if(!cpu->is_data_forwarded)
                                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                        } else if(cpu->memory.rs2 == cpu->decode.rs2) {
                            if(!cpu->is_data_forwarded)
                                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                            cpu->decode.rs2_value = cpu->memory.data_forward;
                        } else if(!cpu->is_data_forwarded){
                            cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                            cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                        }
                    } else {
                        if(cpu->memory.rd == cpu->decode.rs1 && cpu->memory.rd == cpu->decode.rs2) {
                            cpu->decode.rs1_value = cpu->memory.data_forward;
                            cpu->decode.rs2_value = cpu->memory.data_forward;
                        } else if(cpu->memory.rd == cpu->decode.rs1) {
                            cpu->decode.rs1_value = cpu->memory.data_forward;
                            if(!cpu->is_data_forwarded)
                                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                        } else if(cpu->memory.rd == cpu->decode.rs2) {
                            if(!cpu->is_data_forwarded)
                                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                            cpu->decode.rs2_value = cpu->memory.data_forward;
                        } else if(!cpu->is_data_forwarded){
                            cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                            cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                        }
                    }
                }
}


static void update_rs1_with_forwarded_value(APEX_CPU *cpu) {
  if (cpu->writeback.has_insn &&
      (strcmp(cpu->writeback.opcode_str, "BZ") != 0 &&
       strcmp(cpu->writeback.opcode_str, "BNZ") != 0 &&
       strcmp(cpu->writeback.opcode_str, "BP") != 0 &&
       strcmp(cpu->writeback.opcode_str, "BNP") != 0 &&
       strcmp(cpu->writeback.opcode_str, "BN") != 0 &&
       strcmp(cpu->writeback.opcode_str, "BNN") != 0)) {
    printf("rs1 %d", cpu->decode.rs1_value);
    if (strcmp(cpu->writeback.opcode_str, "LOADP") == 0) {
      printf("loadedP value %d", cpu->writeback.data_forward);
      if (cpu->writeback.rd == cpu->decode.rs1) {
        cpu->decode.rs1_value = cpu->writeback.data_forward;
        cpu->is_data_forwarded = 1;
      } else if (cpu->writeback.rs1 == cpu->decode.rs1) {
        cpu->decode.rs1_value = cpu->writeback.updated_register_src1;
        cpu->is_data_forwarded = 1;
      } else {
        cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
      }
    } else if (strcmp(cpu->writeback.opcode_str, "STOREP") == 0) {
      if (cpu->writeback.rs2 == cpu->decode.rs1) {
        cpu->decode.rs1_value = cpu->writeback.data_forward;
      } else {
        cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
      }
    } else {
      if (cpu->writeback.rd == cpu->decode.rs1) {
        cpu->decode.rs1_value = cpu->writeback.data_forward;
        cpu->is_data_forwarded = 1;
      } else {
        cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
      }
      printf("rs1 %d", cpu->decode.rs1_value);
    }
  }
  if (cpu->memory.has_insn && (strcmp(cpu->memory.opcode_str, "BZ") != 0 &&
                               strcmp(cpu->memory.opcode_str, "BNZ") != 0 &&
                               strcmp(cpu->memory.opcode_str, "BP") != 0 &&
                               strcmp(cpu->memory.opcode_str, "BNP") != 0 &&
                               strcmp(cpu->memory.opcode_str, "BN") != 0 &&
                               strcmp(cpu->memory.opcode_str, "BNN") != 0)) {
    if (strcmp(cpu->memory.opcode_str, "LOADP") == 0) {
      printf("loadedP value %d", cpu->memory.data_forward);
      if (cpu->memory.rs1 == cpu->decode.rs1) {
        cpu->decode.rs1_value = cpu->memory.data_forward;
      } else {
        cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
      }
    } else if (strcmp(cpu->memory.opcode_str, "STOREP") == 0) {
      if (cpu->memory.rs2 == cpu->decode.rs1) {
        cpu->decode.rs1_value = cpu->memory.data_forward;
      } else {
        cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
      }
    } else {
      if (cpu->memory.rd == cpu->decode.rs1) {
        cpu->decode.rs1_value = cpu->memory.data_forward;
      } else {
        if (!cpu->is_data_forwarded)
          cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
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
    if (cpu->decode.has_insn && ((cpu->decode.is_empty_rd || cpu->scoreBoarding[cpu->decode.rd] == 0) &&  (cpu->decode.is_empty_rs1 || cpu->scoreBoarding[cpu->decode.rs1] == 0) && (cpu->decode.is_empty_rs2 || cpu->scoreBoarding[cpu->decode.rs2] == 0)))
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
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                update_with_forwarded_value(cpu);
                cpu->is_data_forwarded = 0;
                break;
            }

            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_JALR:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];

                update_rs1_with_forwarded_value(cpu);
                // printf("rs1 %d", cpu->decode.rs1_value);
                cpu->is_data_forwarded = 0;
                break;
            }

            case OPCODE_LOAD:
            case OPCODE_LOADP:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];

                update_rs1_with_forwarded_value(cpu);
                cpu->is_data_forwarded = 0;
                cpu->scoreBoarding[cpu->decode.rd] = 1;
                cpu->scoreBoarding[cpu->decode.rs1] = 1;
                break;
            }


            case OPCODE_MOVC:
            {
                /* MOVC doesn't have register operands */
                // cpu->scoreBoarding[cpu->decode.rd] = 1;
                break;
            }

            case OPCODE_NOP:
            {
                break;
            }

            case OPCODE_STORE:
            case OPCODE_STOREP:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                update_with_forwarded_value(cpu);
                cpu->is_data_forwarded = 0;
                cpu->scoreBoarding[cpu->decode.rs1] = 1;
                cpu->scoreBoarding[cpu->decode.rs2] = 1;
                break;   
            }

            case OPCODE_CMP:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                update_with_forwarded_value(cpu);
                cpu->is_data_forwarded = 0;
                break;
            }

            case OPCODE_CML:
            case OPCODE_JUMP:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];

                update_rs1_with_forwarded_value(cpu);
                cpu->is_data_forwarded = 0;
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

        /* Copy data from decode latch to execute latch*/
        cpu->execute = cpu->decode;
        cpu->decode.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Decode/RF", &cpu->decode);
        }
    }
}

static void set_branch_flags(APEX_CPU *cpu) {
    if (cpu->execute.result_buffer == 0) {
        cpu->zero_flag = TRUE;
    } else {
        cpu->zero_flag = FALSE;
    }
    if(cpu->execute.result_buffer > 0) {
        cpu->positive_flag = TRUE;
    } else {
      cpu->positive_flag = FALSE;
    }
    if (cpu->execute.result_buffer < 0) {
      cpu->negative_flag = TRUE;
    } else {
      cpu->negative_flag = FALSE;
    }
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
            case OPCODE_ADD:
            {
                    cpu->execute.result_buffer
                    = cpu->execute.rs1_value + cpu->execute.rs2_value;
                    /* Set the zero flag based on the result buffer */
                    set_branch_flags(cpu);
                    cpu->execute.data_forward = cpu->execute.result_buffer;
                    // update_stalling_flags(cpu);
                    printf("output is %d \n",cpu->execute.result_buffer);
                break;
            }

            case OPCODE_DIV:
            {
                    cpu->execute.result_buffer
                    = cpu->execute.rs1_value / cpu->execute.rs2_value;
                    cpu->execute.data_forward = cpu->execute.result_buffer;
                    /* Set the zero flag based on the result buffer */
                    set_branch_flags(cpu);
                    // update_stalling_flags(cpu);
                    printf("output is %d \n",cpu->execute.result_buffer);
                break;
            }

            case OPCODE_ADDL:
            {
                    cpu->execute.result_buffer
                    = cpu->execute.rs1_value + cpu->execute.imm;
                    cpu->execute.data_forward = cpu->execute.result_buffer;
                    /* Set the zero flag based on the result buffer */
                    set_branch_flags(cpu);
                    // if(cpu->execute.rd != cpu->execute.rs1) {
                    //     cpu->scoreBoarding[cpu->execute.rs1] = 0;
                    // } else {
                    //     cpu->scoreBoarding[cpu->execute.rs1] = 1;
                    // }
                    printf("output is %d \n",cpu->execute.result_buffer);
                break;
            }

            case OPCODE_SUB:
            {
                    cpu->execute.result_buffer
                    = cpu->execute.rs1_value - cpu->execute.rs2_value;
                    cpu->execute.data_forward = cpu->execute.result_buffer;
                    /* Set the zero flag based on the result buffer */
                    set_branch_flags(cpu);
                    // update_stalling_flags(cpu);
                    printf("output is %d \n",cpu->execute.result_buffer);
                break;
            }

            case OPCODE_SUBL:
            {
                    cpu->execute.result_buffer
                    = cpu->execute.rs1_value - cpu->execute.imm;
                    cpu->execute.data_forward = cpu->execute.result_buffer;
                    /* Set the zero flag based on the result buffer */
                    set_branch_flags(cpu);
                    // if(cpu->execute.rd != cpu->execute.rs1) {
                    //     cpu->scoreBoarding[cpu->execute.rs1] = 0;
                    // } else {
                    //     cpu->scoreBoarding[cpu->execute.rs1] = 1;
                    // }
                    printf("output is %d \n",cpu->execute.result_buffer);
                break;
            }

            case OPCODE_MUL:
            {
                    cpu->execute.result_buffer
                    = cpu->execute.rs1_value * cpu->execute.rs2_value;
                    cpu->execute.data_forward = cpu->execute.result_buffer;
                    /* Set the zero flag based on the result buffer */
                    set_branch_flags(cpu);
                    // update_stalling_flags(cpu);
                    printf("output is %d \n",cpu->execute.result_buffer);
                break;
            }

            case OPCODE_AND:
            {
                    cpu->execute.result_buffer
                    = cpu->execute.rs1_value & cpu->execute.rs2_value;
                    cpu->execute.data_forward = cpu->execute.result_buffer;
                    /* Set the zero flag based on the result buffer */
                    set_branch_flags(cpu);
                    // update_stalling_flags(cpu);
                    printf("output is %d \n",cpu->execute.result_buffer);
                break;
            }

            case OPCODE_OR:
            {
                    cpu->execute.result_buffer
                    = cpu->execute.rs1_value | cpu->execute.rs2_value;
                    cpu->execute.data_forward = cpu->execute.result_buffer;
                    /* Set the zero flag based on the result buffer */
                    set_branch_flags(cpu);
                    // update_stalling_flags(cpu);
                    printf("output is %d \n",cpu->execute.result_buffer);
                break;
            }

            case OPCODE_XOR:
            {
                    cpu->execute.result_buffer
                    = cpu->execute.rs1_value ^ cpu->execute.rs2_value;
                    cpu->execute.data_forward = cpu->execute.result_buffer;
                    /* Set the zero flag based on the result buffer */
                    set_branch_flags(cpu);
                    // update_stalling_flags(cpu);
                    printf("output is %d \n",cpu->execute.result_buffer);
                break;
            }

            case OPCODE_LOAD:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs1_value + cpu->execute.imm;
                cpu->execute.data_forward = cpu->execute.memory_address;
                cpu->scoreBoarding[cpu->execute.rd] = 1;
                break;
            }

            case OPCODE_LOADP:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs1_value + cpu->execute.imm;
                cpu->execute.updated_register_src1 = cpu->execute.rs1_value + 4;
                cpu->execute.data_forward = cpu->execute.updated_register_src1;
                // cpu->execute.data_forward = cpu->execute.updated_register_src1;
                cpu->scoreBoarding[cpu->execute.rs1] = 0;
                cpu->scoreBoarding[cpu->execute.rd] = 1;
                break;
            }

            case OPCODE_BP:
            {
                cpu->branch_target_buffer[cpu->execute.btb_index].target_address = cpu->execute.pc + cpu->execute.imm;
                printf("BP positiveflag %d", cpu->positive_flag);
                if (cpu->positive_flag == TRUE)
                {
                    switch (cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction) {
                    case 0:
                        do_branching(cpu);
                        cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 1;
                        break;
                    case 1:
                        if(cpu->execute.is_btb_hit == 0) {
                            do_branching(cpu);
                        } else {
                            cpu->decode.has_insn = TRUE;
                        }
                        cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 11;
                        break;
                    case 11:
                        cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 11;
                        cpu->decode.has_insn = TRUE;
                        break;
                    default:
                        break;
                }
                } else if(cpu->execute.is_btb_hit == 1){
                    switch (cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction) {
                    case 0:
                        break;
                    case 1:
                        cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 0;
                        break;
                    case 11:
                        cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 1;
                        break;
                    default:
                        break;
                    }
                    cpu->pc = cpu->branch_target_buffer[cpu->execute.btb_index].pc_address + 4;
                    cpu->decode.has_insn = FALSE;
                    cpu->fetch_from_next_cycle = TRUE;
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BNZ:
            {
                cpu->branch_target_buffer[cpu->execute.btb_index].target_address = cpu->execute.pc + cpu->execute.imm;
                printf("BNZ zeroflag %d", cpu->zero_flag);
                if (cpu->zero_flag == FALSE)
                {
                    switch (cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction) {
                    case 0:
                        do_branching(cpu);
                        cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 1;
                        break;
                    case 1:
                        if(cpu->execute.is_btb_hit == 0) {
                            do_branching(cpu);
                        } else {
                            cpu->decode.has_insn = TRUE;
                        }
                        cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 11;
                        break;
                    case 11:
                        cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 11;
                        cpu->decode.has_insn = TRUE;
                        break;
                    default:
                        break;
                }
                } else if(cpu->execute.is_btb_hit == 1){
                    switch (cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction) {
                    case 0:
                        break;
                    case 1:
                        cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 0;
                        break;
                    case 11:
                        cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 1;
                        break;
                    default:
                        break;
                    }
                    cpu->pc = cpu->branch_target_buffer[cpu->execute.btb_index].pc_address + 4;
                    cpu->decode.has_insn = FALSE;
                    cpu->fetch_from_next_cycle = TRUE;
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BNP:
            {
                cpu->branch_target_buffer[cpu->execute.btb_index].target_address = cpu->execute.pc + cpu->execute.imm;
                printf("BNP positiveflag %d", cpu->positive_flag);
                if (cpu->positive_flag == FALSE)
                {
                    switch (cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction) {
                    case 0:
                        do_branching(cpu);
                        cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 1;
                        break;
                    case 1:
                        do_branching(cpu);
                        cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 11;
                        break;
                    case 11:
                        cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 11;
                        cpu->decode.has_insn = TRUE;
                        break;
                    default:
                        break;
                }
                } else if(cpu->execute.is_btb_hit == 1){
                    switch (cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction) {
                        case 0:
                            break;
                        case 1:
                            cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 0;
                            break;
                        case 11:
                            cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 1;
                            break;
                        default:
                            break;
                    }
                    cpu->pc = cpu->branch_target_buffer[cpu->execute.btb_index].pc_address + 4;
                    cpu->decode.has_insn = FALSE;
                    cpu->fetch_from_next_cycle = TRUE;
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BZ:
            {
                cpu->branch_target_buffer[cpu->execute.btb_index].target_address = cpu->execute.pc + cpu->execute.imm;
                printf("BZ zeroflag %d", cpu->zero_flag);
              if (cpu->zero_flag == TRUE) 
                {
                    switch (cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction) {
                    case 0:
                        do_branching(cpu);
                        cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 1;
                        break;
                    case 1:
                        do_branching(cpu);
                        cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 11;
                        break;
                    case 11:
                        cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 11;
                        cpu->decode.has_insn = TRUE;
                        break;
                    default:
                        break;
                }
                } else if(cpu->execute.is_btb_hit == 1){
                    switch (cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction) {
                        case 0:
                            break;
                        case 1:
                            cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 0;
                            break;
                        case 11:
                            cpu->branch_target_buffer[cpu->execute.btb_index].branch_prediction = 1;
                            break;
                        default:
                            break;
                    }
                    cpu->pc = cpu->branch_target_buffer[cpu->execute.btb_index].pc_address + 4;
                    cpu->decode.has_insn = FALSE;
                    cpu->fetch_from_next_cycle = TRUE;
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BN:
            {
                if (cpu->negative_flag == TRUE)
                {
                    do_branching(cpu);
                }
                break;
            }

            case OPCODE_BNN:
            {
                if (cpu->negative_flag == FALSE)
                {
                    do_branching(cpu);
                }
                break;
            }

            case OPCODE_MOVC: 
            {
                cpu->execute.result_buffer = cpu->execute.imm + 0;
                cpu->execute.data_forward = cpu->execute.result_buffer;
                break;
            }

            case OPCODE_NOP:
            {
                break;
            }

            case OPCODE_STORE:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs2_value + cpu->execute.imm;
                // cpu->execute.data_forward = cpu->execute.memory_address;
                cpu->scoreBoarding[cpu->execute.rs1] = 0;
                cpu->scoreBoarding[cpu->execute.rs2] = 0;
                break;
            }

            case OPCODE_STOREP:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs2_value + cpu->execute.imm;
                cpu->execute.updated_register_src1 = cpu->execute.rs2_value + 4;
                cpu->execute.data_forward = cpu->execute.updated_register_src1;
                cpu->scoreBoarding[cpu->execute.rs1] = 0;
                cpu->scoreBoarding[cpu->execute.rs2] = 0;
                break;
            }

            case OPCODE_CMP:
            {
                if(cpu->execute.rs1_value > cpu->execute.rs2_value) {
                    cpu->positive_flag = TRUE;
                } else {
                    cpu->positive_flag = FALSE;
                }
                if(cpu->execute.rs1_value < cpu->execute.rs2_value) {
                    cpu->negative_flag = TRUE;
                } else {
                    cpu->negative_flag = FALSE;
                }
                if(cpu->execute.rs1_value == cpu->execute.rs2_value) {
                    cpu->zero_flag = TRUE;
                } else {
                    cpu->zero_flag = FALSE;
                }
                break;   
            }

            case OPCODE_CML:
            {
                if(cpu->execute.rs1_value > cpu->execute.imm) {
                    cpu->positive_flag = TRUE;
                } else {
                    cpu->positive_flag = FALSE;
                }
                if(cpu->execute.rs1_value < cpu->execute.imm) {
                    cpu->negative_flag = TRUE;
                } else {
                    cpu->negative_flag = FALSE;
                }
                if(cpu->execute.rs1_value == cpu->execute.imm) {
                    cpu->zero_flag = TRUE;
                } else {
                    cpu->zero_flag = FALSE;
                }
                break;   
            }

            case OPCODE_JALR:
            {
                /* Calculate new PC, and send it to fetch unit */
                cpu->execute.result_buffer = cpu->execute.pc + 4;
                // cpu->pc = cpu->regs[cpu->execute.rs1] + cpu->execute.imm;
                cpu->pc = cpu->execute.rs1_value + cpu->execute.imm;

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
                cpu->execute.result_buffer = cpu->execute.rs1_value + cpu->execute.imm;
                cpu->pc = cpu->execute.result_buffer;
                printf("New addres %d\n", cpu->execute.result_buffer);
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

        /* Copy data from execute latch to memory latch*/
        cpu->memory = cpu->execute;
        cpu->execute.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Execute", &cpu->execute);
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

            case OPCODE_LOAD:
            case OPCODE_LOADP:
            {
                /* Read from data memory */
                cpu->memory.result_buffer
                    = cpu->data_memory[cpu->memory.memory_address];
                // cpu->memory.data_forward = cpu->memory.result_buffer;
                printf("loadp %d", cpu->memory.data_forward);
                cpu->memory.data_forward = cpu->memory.result_buffer;
                cpu->scoreBoarding[cpu->memory.rd] = 0;                
                break;
            }

            case OPCODE_NOP:
            {
                break;
            }

            case OPCODE_STORE:
            {
                /* Read from data memory */
                cpu->data_memory[cpu->memory.memory_address] = cpu->memory.rs1_value;
                cpu->scoreBoarding[cpu->memory.rs1] = 0;
                break;
            }

            case OPCODE_STOREP:
            {
                /* Read from data memory */
                cpu->data_memory[cpu->memory.memory_address] = cpu->memory.rs1_value;
                cpu->memory.data_forward = cpu->memory.updated_register_src1;
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
            print_stage_content("Memory", &cpu->memory);
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

    cpu->bq_size = 4;
    cpu->bq_head = 0;
    cpu->bq_tail = 0;

    cpu->iq_size = 16;
    cpu->iq_head = 0;
    cpu->iq_tail = 0;


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

    cpu->counter = 0;
    cpu->index = 0;
    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
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
        APEX_decode(cpu);
        APEX_fetch(cpu);

        // Issue instructions from BQ and IQ
        APEX_cpu_issue_instructions(cpu);

        // Dispatch instructions to BQ and IQ
        APEX_cpu_dispatch_instructions(cpu);

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

        cpu->clock++;
        cpu->counter++;
    }
}

void APEX_cpu_dispatch(APEX_CPU *cpu, CPU_Stage *stage) {
    // Check for branch instructions (BZ, BNZ, BP, BNP, JUMP, JALR)
    if (stage->opcode == OPCODE_BZ || stage->opcode == OPCODE_BNZ || stage->opcode == OPCODE_BP ||
        stage->opcode == OPCODE_BNP || stage->opcode == OPCODE_JUMP || stage->opcode == OPCODE_JALR) {
        
        // Dispatch to Branch Instruction Queue (BQ)
        if (cpu->bq_tail < cpu->bq_size) {
            cpu->bq[cpu->bq_tail] = *stage;
            cpu->bq_tail++;
        } else {
            fprintf(stderr, "Error: Branch Instruction Queue (BQ) is full. Instruction cannot be dispatched.\n");
        }
    } else {
        // Dispatch to Instruction Queue (IQ)
        if (cpu->iq_tail < cpu->iq_size) {
            cpu->iq[cpu->iq_tail] = *stage;
            cpu->iq_tail++;
        } else {
            fprintf(stderr, "Error: Instruction Queue (IQ) is full. Instruction cannot be dispatched.\n");
        }
    }
}


void APEX_cpu_dispatch_instructions(APEX_CPU *cpu) {
    // Dispatch instructions from BQ
    for (int i = 0; i < cpu->bq_size; ++i) {
        if (cpu->bq[i].has_insn && !cpu->bq[i].simulator_flag) {
            APEX_cpu_dispatch(cpu, &cpu->bq[i]);
        }
    }

    // Dispatch instructions from IQ
    for (int i = 0; i < cpu->iq_size; ++i) {
        if (cpu->iq[i].has_insn && !cpu->iq[i].simulator_flag) {
            APEX_cpu_dispatch(cpu, &cpu->iq[i]);
        }
    }
}

void APEX_cpu_issue_instructions(APEX_CPU *cpu) {
    // Check if there are instructions in BQ
    if (cpu->bq_head != -1) {
        cpu->fetch = cpu->bq[cpu->bq_head];

        // Update BQ-related data structures
        if (cpu->bq_head == cpu->bq_tail) {
            cpu->bq_head = cpu->bq_tail = -1; // BQ becomes empty
        } else {
            cpu->bq_head = (cpu->bq_head + 1) % cpu->bq_size;
        }

    }

    // Check if there is space in IQ and there are instructions ready to issue
    if (cpu->iq_head != -1) {
        cpu->decode = cpu->iq[cpu->iq_head];

        // Update IQ-related data structures
        if (cpu->iq_head == cpu->iq_tail) {
            cpu->iq_head = cpu->iq_tail = -1; // IQ becomes empty
        } else {
            cpu->iq_head = (cpu->iq_head + 1) % cpu->iq_size;
        }

        /* if (cpu->rename_table[cpu->decode.rd] == -1) {
            // Physical register is available, use it
            cpu->rename_table[cpu->decode.rd] = cpu->free_physical_regs[0];
            cpu->free_physical_regs[0] = cpu->free_physical_regs[1];
            cpu->free_physical_regs[1] = cpu->free_physical_regs[2];
            // Clear the entry in the free list
            cpu->free_physical_regs[2] = -1;
        } */
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