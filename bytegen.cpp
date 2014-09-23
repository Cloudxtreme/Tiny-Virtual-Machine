#include "bytegen.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <regex>
#include "vm.hpp"

SourceException::SourceException(unsigned int code, std::string data)
{
	this->code = code;
	this->data = data;
}
SourceException::SourceException(unsigned int code, unsigned int data)
{
	this->code = code;
	this->data = std::to_string(data);
}
std::string SourceException::Get()
{
	std::ostringstream stream;

	std::string code_desc;
	switch(code)
	{
		case EXCEPTION_UNKNOWN_COMMAND:
			code_desc = "Unknown command "; break;
		case EXCEPTION_INVALID_ARGUMENTS:
			code_desc = "Invalid arguments, line "; break;
		default:
			code_desc = ""; break;
	}

    stream << "Source error: " << code_desc << data;

    return stream.str();
}

BytecodeGenerator::BytecodeGenerator()
{
	std::vector<char> bytes;	
 
	std::string line;
	unsigned int line_number = 1;
	while(std::getline(std::cin, line))
	{
		std::string temp;
		std::vector<std::string> tokens;

		std::istringstream stream(line, std::istringstream::in);

		while(stream >> temp)
		{
			tokens.push_back(temp);	
		}

		if(tokens.size() == 0)
			continue;

		char instruction_type = 0;

		if(tokens[0].compare("LOAD") == 0)
		{				
			instruction_type = INS_LOAD;		
		}
		else if (tokens[0].compare("STORE") == 0)
		{
			instruction_type = INS_STORE;
		}
		else if (tokens[0].compare("ADD") == 0)
		{
			instruction_type = INS_ADD;
		}
		else if (tokens[0].compare("SUB") == 0)
		{
			instruction_type = INS_SUB;
		}
		else if (tokens[0].compare("HALT") == 0)
		{
			instruction_type = INS_HALT;
		}
		else
		{
			throw SourceException(EXCEPTION_UNKNOWN_COMMAND, tokens[0]);
		}

		bytes.push_back(instruction_type);

		//arguments
		switch(instruction_type)
		{
			case INS_LOAD: case INS_STORE:
			{
				if(tokens.size() != 2 + 1)
					throw SourceException(EXCEPTION_INVALID_ARGUMENTS, line_number);

				char register_index = 0; //token[1] tostring
				unsigned int address = 0; //token[2] tostring

				register_index = atoi(tokens[1].c_str());
				address = atoi(tokens[2].c_str());

				bytes.push_back(register_index);			
				
				bytes.push_back(address); 
				bytes.push_back(address >> 8); 
				bytes.push_back(address >> 16);
				bytes.push_back(address >> 24);
				break;
			}
			case INS_ADD: case INS_SUB:
			{
				if(tokens.size() != 3 + 1)
					throw SourceException(EXCEPTION_INVALID_ARGUMENTS, line_number);

				char register_index[3] = {0}; //token[1,2,3] tostring				

				for(int i=0; i<3; i++)
				{
					register_index[i] = atoi(tokens[i+1].c_str());
				
					bytes.push_back(register_index[i]);	
				}
				break;
			}		
			case INS_HALT:
			{
				if(tokens.size() != 1)
					throw SourceException(EXCEPTION_INVALID_ARGUMENTS, line_number);
				break;
			}	
			default:
			{
				break;
			}
		}	
		line_number++;	
	}

	bytecode = std::unique_ptr<char[]>(new char[bytes.size()]);
	for(int i=0; i<bytes.size(); i++)
	{
		bytecode[i] = bytes[i];

		#ifdef DEBUG
			std::cout << (int)bytes[i] << "|"; 
		#endif
	}
	#ifdef DEBUG
		std::cout << std::endl;
	#endif

	bytecode_size = bytes.size();
}
char* BytecodeGenerator::GetBytecode()
{
	return bytecode.get();
}
unsigned int BytecodeGenerator::GetBytecodeSize()
{
	return bytecode_size;
}