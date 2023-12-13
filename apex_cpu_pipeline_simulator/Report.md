Project-3

Group members:
Karthick Gunasekar - B00976718
Mukunthan Sriram Balaji - B00946990
Shruti Dhande - B00983135

Contributions:

Mukunthan Sriram Balaji:
 I've implemented the Load/Store queue, Reorder buffer and Memory Access Unit in this project. The LSQ implementation was carried out through different stages of the out-of-order APEX simulator like dispatch, LSQ and MAU. LSQ and LSQEntry structs were initiated in the 'apex_cpu.h' file. A separate stage called LSQ has been implemented in 'apex_cpu.c' file to implement the LSQ stage logic.

I've initialized two functions called 'LSQEntryStore()' and 'LSQEntryLoad()' for the implementations of 'LOAD' and 'STORE' commands in the dispatch stage. If the value is available at the time of dispatching the STORE, the value is read out from the PRF and inserted into the LSQ entry that is being set up for the STORE this happens in the Dispatch stage. So after the instruction reaches the 'dispatch' stage, after reading the values via data forwarding/ after register renaming, IQ entries are initialized and the LSQ entry initialization and LSQ enqueue operation is executed in the LOAD/STORE case inside the dispatch section.

Post that, the the values are transfered from the dispatch stage to the LSQ stage. In the LSQ stage, the validity of the memory addess has been checked as the 'conditon 1' and 'ROB entry index' is compared with the 'LSQ entry index' to execute memory operations in the MAU stage.

Reorder Buffer:
I've implemented reorder buffer to maintain the instruction order. Allocation and Deallocation of physical registers implementing Deallocation logic. ROB is a circular FIFO queue implementation. An ROB entry is created accordingly and it has been co-ordinated with LSQ implementation to execute memory operations for LOAD/STORE instructions. The commit logic is integrated with the ROB logic.

The head and tail of ROB are used for the comparison operation with LSQ. The instruction is pitched to ROB, LSQ and IQ stages.

Shruti Dhande:
I've successfully incorporated separate Branch (BQ) and Instruction (IQ) Queues in the proposed processor architecture, preferring the earliest dispatched instruction for simultaneous readiness. For conditional branches based on BTB predictions, speculative execution is permitted, including checkpointing for recovery in the event of a misprediction.

I've included an elapsed cycle counter to handle functional unit contention, granting priority depending on the earliest dispatch time. The BQ and IQ entries are designed to enable a wide range of instruction types, including conditional branches and jumps.

A FIFO-based register allocation maintains physical registers effectively, and all registers are initialized as invalid at startup. The IQ and BQ are both set to zero. Forwarding is optimized, with tag and data broadcasts consuming one clock cycle each, and IQ logic efficiently picking and dispatching instructions.

The IQ efficiently requests functional unit execution upon operand availability, and the wakeup mechanism effectively broadcasts instructions for dependent calculations.

In conclusion, the implemented solution features efficient queuing, speculative execution, and optimized register allocation, exhibiting a well-structured and functioning processor architecture.

Karthick Gunasekar:
Following are my contributions in the project:
1)Register Renaming
2)AFU
3)MulFu
4)IntFu
5)Forwarding bus

1) Register Renaming:
I have a created a seperate struct for physical register with their respective fields. And I have a seperate queue to handle the physical register free list and a queue for rename table entries/
Decode : 
1) A new physical register will be allocated to the destination register and source resgiter will be updated with the recent physical register by looking up the rename table
2) Rename table will be updated with the new dest physical 

Dispatch:
1) IQ entry is created for the current instruction. If IQ is full, dispatch will be stalled.
2) BQ entry is created for the current instruction. If BQ is full, dispatch will be stalled.
3) LSQ entry is created for the current instruction. If LSQ is full, dispatch will be stalled.
4) ROB entry is created for the current instruction. If ROB is full, dispatch will be stalled.

AFU:
AFU is used for calculating the memory address for LOAD, LOADP, STORE, STOREP. And the calculated memory address will be forwarded into the corresponding IQ. It has latency of one cycle.

MulFu:
MulFu is used for multiplication operation. The result will be transferred using the forwarding buses to the IQ, dispatch, decode.

IntFu:
All of the operations other than branch instructions will be performed. The result will be transferred using the forwarding buses to the IQ, dispatch, decode.

Forwarding Bus:
Five forwarding buses are created each of one units. Seperate structs have been created


