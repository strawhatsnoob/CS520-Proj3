Project-3

Group members:
Karthick Gunasekar
Mukunthan Sriram Balaji - B00946990
Shruti Dhande

Contributions:

Mukunthan Sriram Balaji:
I've implemented the Load/Store queue, Reorder buffer and Memory Access Unit in this project. The LSQ implementation was carried out through different stages of the out-of-order APEX simulator like dispatch, LSQ and MAU. LSQ and LSQEntry structs were initiated in the 'apex_cpu.h' file. A separate stage called LSQ has been implemented in 'apex_cpu.c' file to implement the LSQ stage logic.

I've initialized two functions called 'LSQEntryStore()' and 'LSQEntryLoad()' for the implementations of 'LOAD' and 'STORE' commands in the dispatch stage. If the value is available at the time of dispatching the STORE, the value is read out from the PRF and inserted into the LSQ entry that is being set up for the STORE this happens in the Dispatch stage. So after the instruction reaches the 'dispatch' stage, after reading the values via data forwarding/ after register renaming, IQ entries are initialized and the LSQ entry initialization and LSQ enqueue operation is executed in the LOAD/STORE case inside the dispatch section.

Post that, the the values are transfered from the dispatch stage to the LSQ stage. In the LSQ stage, the validity of the memory addess has been checked as the 'conditon 1' and 'ROB entry index' is compared with the 'LSQ entry index' to execute memory operations in the MAU stage.



