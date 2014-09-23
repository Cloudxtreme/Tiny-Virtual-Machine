Tiny-Virtual-Machine
====================

A small virtual machine that runs custom bytecode.

Still being heavily worked on. For fun.

How it works:
1. reads the source file, parses it and converts it into bytecode
2. bytecode (a set of instructions) is then loaded into the virtual machine's memory
3. virtual machine attempts to execute commands in a manner real chips do

If something bad happens, like trying to read from a memory address that is out of bounds, or writing to a register that doesn't exist, a halt signal is generated and the virtual machine stops bytecode execution.

Current features:
- LOAD, STORE, ADD, SUB and HALT commands
- source file parsing and conversion into bytecode
- error handling during both execution and parsing (needs more work)

To do:
- find a way to handle endianess
- more commands
- pointers
- procedures
- embedding in C++ programs
