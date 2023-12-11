Project-3

Group members:
Karthick Gunasekar
Mukunthan Sriram Balaji - B00946990
Shruti Dhande

Contributions:

Mukunthan Sriram Balaji:
I've implemented the Load/Store queue, Reorder buffer and Memory Access Unit in this project. The LSQ implementation was carried out through different stages of the out-of-order APEX simulator like dispatch, LSQ and MAU. LSQ and LSQEntry structs were initiated in the 'apex_cpu.h' file. A separate stage called LSQ has been implemented in 'apex_cpu.c' file to implement the LSQ stage logic.

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


