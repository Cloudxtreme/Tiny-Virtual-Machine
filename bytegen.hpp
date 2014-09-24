#ifndef BYTEGEN_HPP
#define BYTEGEN_HPP
#include <memory>
#include <map>
#include <iostream>

//#define DEBUG

#define EXCEPTION_UNKNOWN_COMMAND 1
#define EXCEPTION_INVALID_ARGUMENTS 2

struct SourceException
{
	unsigned int code;
	std::string data;

	SourceException(unsigned int code, std::string data);
	SourceException(unsigned int code, unsigned int data);

	std::string Get();
};

class BytecodeGenerator
{
private:
	std::unique_ptr<char[]> bytecode;
	unsigned int bytecode_size;

	std::map<unsigned int, char> constants;
public:
	BytecodeGenerator();

	char* GetBytecode();
	unsigned int GetBytecodeSize();
	std::map<unsigned int, char>*  GetConstants();
};

#endif