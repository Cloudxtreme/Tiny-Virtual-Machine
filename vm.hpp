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
#define INS_SHL 5
#define INS_SHR 6
#define INS_JMP 7
#define INS_CMP 8

#define HS_USER 1
#define HS_OUT_BOUNDS 2
#define HS_UNKNOWN_COMMAND 3
#define HS_INVALID_ARGUMENT 4

#define CMP_NONE 0
#define CMP_LT 1
#define CMP_EQ 2
#define CMP_GT 3

#define COND_NONE 0
#define COND_LT 1
#define COND_EQ 2
#define COND_GT 3
#define COND_LE 4
#define COND_GE 5

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

	unsigned char cmp_flag;

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