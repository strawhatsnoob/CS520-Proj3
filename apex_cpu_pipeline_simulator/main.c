/*
 * main.c
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>

#include "apex_cpu.h"

int
main(int argc, char const *argv[])
{
    APEX_CPU *cpu;

    fprintf(stderr, "APEX CPU Pipeline Simulator v%0.1lf\n", VERSION);

    if (argc != 2 && argc != 4)
    {
        fprintf(stderr, "APEX_Help: Usage %s <input_file>\n", argv[0]);
        exit(1);
    }
    cpu = APEX_cpu_init(argv[1]);
    if(argc == 4) {
        const char *number = argv[3];
        cpu->simulate_counter = atoi(number);
         if (cpu->simulate_counter == 0 || number[0] != '0') {
            printf("Error Occurred : Please enter a digit\n");
        }
        cpu->simulator_flag = 1;
    }

    if (!cpu)
    {
        fprintf(stderr, "APEX_Error: Unable to initialize CPU\n");
        exit(1);
    }

    //fprintf(stderr, "Instructions in IQ: %d\n", cpu->iq_size);
    //fprintf(stderr, "Instructions in BQ: %d\n", cpu->bq_size);

    APEX_cpu_run(cpu);
    APEX_cpu_stop(cpu);
    return 0;
}