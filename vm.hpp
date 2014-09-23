#ifndef VM_H
#define VM_H
#include <iostream>
#include <memory>

#define MEMORY_SIZE 2048

#define NUM_REGISTERS 8

#define INS_HALT 127
#define INS_LOAD 1
#define INS_STORE 2
#define INS_ADD 3
#define INS_SUB 4

#define HS_USER 1
#define HS_OUT_BOUNDS 2
#define HS_UNKNOWN_COMMAND 3

struct BytecodeException
{
	unsigned int code;

	BytecodeException(unsigned int code);
};

class VirtualMachine
{
private:
	std::unique_ptr<char[]> memory;
	std::unique_ptr<int[]> registers;

	unsigned int pc;
	unsigned int sp;

	unsigned int halt_signal;	
public:
	VirtualMachine();

	bool LoadProgram(char* data, unsigned int length);
	void Run();

	void Cycle();

	void ParseInstruction(char type, unsigned int argument_address);

	char GetByte(unsigned int address);
	int GetWord(unsigned int address);

	void SetByte(char value, unsigned int address);
	void SetWord(int value, unsigned int address);

	bool AddressInBounds(unsigned int address);
	bool AddressRangeInBounds(unsigned int address, unsigned int length);
};

#endif