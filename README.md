Tiny-Virtual-Machine
====================

A small virtual machine that runs custom bytecode.

Still being heavily worked on. For fun.

##How it works:
- reads the source file from STDIN, parses it and converts it into bytecode
- bytecode (a set of instructions) is then loaded into the virtual machine's memory
- virtual machine attempts to execute instructions in a manner real chips do

If something bad happens, like trying to read from a memory address that is out of bounds, or writing to a register that doesn't exist, a halt signal is generated and the virtual machine stops bytecode execution.

##How to use:
For testing purposes, the program is set to receive source code via standard input. This will be changed later to allow for flexibility.

For example, if your source code is located in source.txt, you would compile it into bytecode and execute like so (Unix Terminal):
```
bin/main.out < bin/source.txt
```

Current features:
- source file parsing and conversion into bytecode
- bytecode execution
- LOAD, STORE, ADD, SUB, SHR, SHL and HALT instructions
- flow control instruction with conditionals (JMP, CMP)
- pointers
- labels instead of raw addresses (in source code)
- error handling during both execution and parsing (needs more work)

To do:
- find a way to handle endianess
- better debugging options
- procedures
- embedding in C++ programs

##Log
###v0.03
- added support for loading data from address stored in a register
- JMP now works with conditions which are set via CMP instruction
- Labels! Raw address data no longer needed for JMPs

###v0.02
- added register shifting instructions SHR and SHL
- added flow control instruction JMP
- added condition inspection via CMP

###v0.01
- hosted on github
- didn't exactly keep a log before this

