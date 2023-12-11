Project-3

Group members:
Karthick Gunasekar
Mukunthan Sriram Balaji - B00946990
Shruti Dhande - B00983135

Contributions:

Mukunthan Sriram Balaji:
I've implemented the Load/Store queue, Reorder buffer and Memory Access Unit in this project. The LSQ implementation was carried out through different stages of the out-of-order APEX simulator like dispatch, LSQ and MAU. LSQ and LSQEntry structs were initiated in the 'apex_cpu.h' file. A separate stage called LSQ has been implemented in 'apex_cpu.c' file to implement the LSQ stage logic.

Shruti Dhande:
I've successfully incorporated separate Branch (BQ) and Instruction (IQ) Queues in the proposed processor architecture, preferring the earliest dispatched instruction for simultaneous readiness. For conditional branches based on BTB predictions, speculative execution is permitted, including checkpointing for recovery in the event of a misprediction.

I've included an elapsed cycle counter to handle functional unit contention, granting priority depending on the earliest dispatch time. The BQ and IQ entries are designed to enable a wide range of instruction types, including conditional branches and jumps.

A FIFO-based register allocation maintains physical registers effectively, and all registers are initialized as invalid at startup. The IQ and BQ are both set to zero. Forwarding is optimized, with tag and data broadcasts consuming one clock cycle each, and IQ logic efficiently picking and dispatching instructions.

The IQ efficiently requests functional unit execution upon operand availability, and the wakeup mechanism effectively broadcasts instructions for dependent calculations.

In conclusion, the implemented solution features efficient queuing, speculative execution, and optimized register allocation, exhibiting a well-structured and functioning processor architecture.






