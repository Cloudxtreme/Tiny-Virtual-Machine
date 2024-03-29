#include "bytegen.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>
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
	std::map<std::string, unsigned int> labels;
	std::map<unsigned int, std::string> jumps;
	
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
		else if (tokens[0].compare("SHL") == 0)
		{
			instruction_type = INS_SHL;
		}
		else if (tokens[0].compare("SHR") == 0)
		{
			instruction_type = INS_SHR;
		}
		else if (tokens[0].compare("JMP") == 0)
		{
			instruction_type = INS_JMP;
		}
		else if (tokens[0].compare("CMP") == 0)
		{
			instruction_type = INS_CMP;
		}
		else if (tokens[0].compare("HALT") == 0)
		{
			instruction_type = INS_HALT;
		}
		else if (tokens[0].compare("DB") == 0)
		{
			if(tokens.size() != 2 + 1)
				throw SourceException(EXCEPTION_INVALID_ARGUMENTS, line_number);

			unsigned int address = atoi(tokens[1].c_str());
			unsigned int value = atoi(tokens[2].c_str());

			constants[address] = value;

			std::cout << (unsigned int)value << std::endl;

			continue;
		}
		else if (tokens[0].compare("DW") == 0)
		{
			if(tokens.size() != 2 + 1)
				throw SourceException(EXCEPTION_INVALID_ARGUMENTS, line_number);

			unsigned int address = atoi(tokens[1].c_str());
			unsigned int value = atoi(tokens[2].c_str());

			constants[address] = value;
			constants[address+1] = value >> 8;
			constants[address+2] = value >> 16;
			constants[address+3] = value >> 24;

			continue;
		}
		else
		{
			//if the parsed token is a label
			if(tokens[0].length() > 0 && tokens[0].at(tokens[0].length()-1) == ':')
			{
				//the size of bytes array is actually the address of the next instruction
				labels[tokens[0].substr(0, tokens[0].length()-1)] = bytes.size();				
				continue;
			}
			else
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
				char mode = 0;
				unsigned int address = 0; //token[2] tostring

				register_index = atoi(tokens[1].c_str());						

				if(tokens[2].length() > 2 && tokens[2].at(0) == '(' && tokens[2].at(tokens[2].length()-1) == ')')
				{
					address = atoi(tokens[2].substr(1, tokens[2].length()-2).c_str());					
					mode = 1;
				}
				else
				{
					address = atoi(tokens[2].c_str());
					mode = 0;
				}

				bytes.push_back(register_index);	

				bytes.push_back(mode);			
				
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
			case INS_SHL: case INS_SHR:
			{
				if(tokens.size() != 3 + 1)
					throw SourceException(EXCEPTION_INVALID_ARGUMENTS, line_number);						

				char register_src = atoi(tokens[1].c_str());
				char shift_amount = atoi(tokens[2].c_str());
				char register_dest = atoi(tokens[3].c_str());
				
				bytes.push_back(register_src);	
				bytes.push_back(shift_amount);	
				bytes.push_back(register_dest);	
				
				break;
			}
			case INS_CMP:
			{
				if(tokens.size() != 2 + 1)
					throw SourceException(EXCEPTION_INVALID_ARGUMENTS, line_number);						

				char register_1 = atoi(tokens[1].c_str());
				char register_2 = atoi(tokens[2].c_str());				
				
				bytes.push_back(register_1);	
				bytes.push_back(register_2);
				
				break;
			}
			case INS_JMP: 
			{
				if(tokens.size() != 1 + 1 && tokens.size() != 2 + 1)
					throw SourceException(EXCEPTION_INVALID_ARGUMENTS, line_number);

				jumps[bytes.size()-1] = tokens[1];	

				unsigned int address = atoi(tokens[1].c_str());				
				
				bytes.push_back(address); 
				bytes.push_back(address >> 8); 
				bytes.push_back(address >> 16);
				bytes.push_back(address >> 24);	

				char condition = COND_NONE;

				//if we have a condition specified
				if(tokens.size() == 2 + 1)
				{
					std::string cond = tokens[2];

					if(tokens[2].compare("EQ") == 0)
						condition = COND_EQ;
					if(tokens[2].compare("LT") == 0)
						condition = COND_LT;
					if(tokens[2].compare("GT") == 0)
						condition = COND_GT;
					if(tokens[2].compare("LE") == 0)
						condition = COND_LE;
					if(tokens[2].compare("GE") == 0)
						condition = COND_GE;
				}	

				bytes.push_back(condition);		
				
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

	std::map<unsigned int, std::string>::iterator it;
	for(it = jumps.begin(); it != jumps.end(); it++)
	{
		std::map<std::string, unsigned int>::iterator check;
		check = labels.find(it->second);
		if(check != labels.end())
		{
			bytes[it->first+1] = check->second;
			bytes[it->first+2] = check->second >> 8;
			bytes[it->first+3] = check->second >> 16;
			bytes[it->first+4] = check->second >> 24;
		}
		else
		{
			std::cout << "No such label: " << it->second << std::endl;
		}
	}


	bytecode = std::unique_ptr<char[]>(new char[bytes.size()]);
	for(int i=0; i<bytes.size(); i++)
	{
		bytecode[i] = bytes[i];		
	}	

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
std::map<unsigned int, char>*  BytecodeGenerator::GetConstants()
{
	return &constants;
}