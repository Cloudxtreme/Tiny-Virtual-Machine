#include "vm.hpp"

BytecodeException::BytecodeException(unsigned int code)
{
	this->code = code;
}

VirtualMachine::VirtualMachine()
{
	memory = std::unique_ptr<char[]>(new char[MEMORY_SIZE]);
	registers = std::unique_ptr<int[]>(new int[NUM_REGISTERS]);

	pc = 0;
	sp = 0;

	halt_signal = 0;
}
bool VirtualMachine::LoadProgram(char* data, unsigned int length)
{
	if(length > MEMORY_SIZE)
		return false;

	for(unsigned int i=0; i<length; i++)
	{
		memory[i] = data[i];
	}

	return true;
}
void VirtualMachine::Run()
{
	while(halt_signal == 0)
	{
		Cycle();
	}
	switch(halt_signal)
	{
		case HS_OUT_BOUNDS:
			std::cout << "TERMINATED: ADDRESS OUT OF BOUNDS" << std::endl;
			break;
		case HS_UNKNOWN_COMMAND:
			std::cout << "TERMINATED: UNKNOWN COMMAND" << std::endl;
			break;
		case HS_USER:
			std::cout << "Program terminated naturally." << std::endl;
			break;
	}
}
void VirtualMachine::Cycle()
{
	if(pc >= MEMORY_SIZE)
	{
		//error, pc outside memory bounds		
		halt_signal = HS_OUT_BOUNDS;
		return;
	}
	//get next instruction
	char* memory_ptr = memory.get() + pc;

	//the first byte of the instruction is the instruction type
	char instruction_type = GetByte(pc);

	ParseInstruction(instruction_type, pc+1);	
}
void VirtualMachine::ParseInstruction(char type, unsigned int argument_address)
{
	switch(type)
	{
		case INS_LOAD: case INS_STORE:
		{			
			if(pc + 5 >= MEMORY_SIZE)
			{
				halt_signal = HS_OUT_BOUNDS;
				break;
			}

			int register_index;
			unsigned int address;

			try
			{
				register_index = GetByte(argument_address);
				address = GetWord(argument_address+1);
			}
			catch(BytecodeException exception)
			{
				halt_signal = exception.code;
				break;
			}

			if(register_index < 0 || register_index >= NUM_REGISTERS || address < 0 || address >= MEMORY_SIZE)
			{
				halt_signal = HS_OUT_BOUNDS;
				break;
			}	

			int value;		

			if(type == INS_LOAD)
			{
				try
				{
					value = GetWord(address);
				}
				catch(BytecodeException exception)
				{
					halt_signal = exception.code;
					break;
				}

				registers[register_index] = value;

				std::cout << "LOAD " << register_index << ", " << address << std::endl;
				std::cout << "Loaded value " << value << std::endl;
			}
			else //if INS_STORE
			{
				value = registers[register_index];

				try
				{
					SetWord(value, address);
				}
				catch(BytecodeException exception)
				{
					halt_signal = exception.code;
					break;
				}

				std::cout << "STORE " << register_index << ", " << address << std::endl;
				std::cout << "Stored value " << value << std::endl;
			}

			//1 byte for the type, 1 for register, 4 for address
			pc += 6;
			
			break;
		}	
		case INS_ADD: case INS_SUB:
		{
			if(pc + 3 >= MEMORY_SIZE)
			{
				halt_signal = HS_OUT_BOUNDS;
				break;
			}

			char register_1;
			char register_2;
			char register_3;

			try
			{
				register_1 = GetByte(argument_address);			
				register_2 = GetByte(argument_address+1);
				register_3 = GetByte(argument_address+2);
			}
			catch(BytecodeException exception)
			{
				halt_signal = exception.code;
				break;
			}

			if(register_1 < 0 || register_1 >= NUM_REGISTERS
				|| register_2 < 0 || register_2 >= NUM_REGISTERS
				|| register_3 < 0 || register_3 >= NUM_REGISTERS)
			{
				halt_signal = HS_OUT_BOUNDS;
				break;
			}

			if(type == INS_ADD)
			{
				registers[register_1] = registers[register_2] + registers[register_3];
				//std::cout << "ADD R" << register_1 << ", R" << register_2, << ", R" << register_3 << std::endl;
			}
			else
			{
				registers[register_1] = registers[register_2] - registers[register_3];
				//std::cout << "SUB R" << register_1 << ", R" << register_2, << ", R" << register_3 << std::endl;
			}

			pc += 4;

			break;
		}	
		case INS_SHL: case INS_SHR:
		{
			if(pc + 3 >= MEMORY_SIZE)
			{
				halt_signal = HS_OUT_BOUNDS;
				break;
			}

			char register_src;
			char register_dest;
			char shift_amount;

			try
			{
				register_src = GetByte(argument_address);			
				shift_amount = GetByte(argument_address+1);
				register_dest = GetByte(argument_address+2);				
			}
			catch(BytecodeException exception)
			{
				halt_signal = exception.code;
				break;
			}

			if(register_src < 0 || register_src >= NUM_REGISTERS
				|| register_dest < 0 || register_dest >= NUM_REGISTERS)
			{
				halt_signal = HS_OUT_BOUNDS;
				break;
			}

			if(type == INS_SHL)
			{
				registers[register_dest] = registers[register_src] << shift_amount;
				//std::cout << "ADD R" << register_1 << ", R" << register_2, << ", R" << register_3 << std::endl;
			}
			else
			{
				registers[register_dest] = registers[register_src] >> shift_amount;
				//std::cout << "SUB R" << register_1 << ", R" << register_2, << ", R" << register_3 << std::endl;
			}

			pc += 4;

			break;
		}	
		case INS_JMP:
		{
			if(pc + 1 >= MEMORY_SIZE)
			{
				halt_signal = HS_OUT_BOUNDS;
				break;
			}

			unsigned int address;

			try
			{
				address = GetWord(argument_address);				
			}
			catch(BytecodeException exception)
			{
				halt_signal = exception.code;
				break;
			}				

			pc = address;

			break;
		}
		case INS_HALT:
		{
			halt_signal = HS_USER;

			std::cout << "HALT" << std::endl;
			break;
		}
		default:
		{
			halt_signal = HS_UNKNOWN_COMMAND;
			break;
		}
	}
}
char VirtualMachine::GetByte(unsigned int address)
{
	if(!AddressInBounds(address))
		throw BytecodeException(HS_OUT_BOUNDS);

	char* memory_ptr = memory.get() + address;

	return *memory_ptr;
}
int VirtualMachine::GetWord(unsigned int address)
{
	if(!AddressRangeInBounds(address, 4))
		throw BytecodeException(HS_OUT_BOUNDS);

	char* memory_ptr = memory.get() + address;

	return *((int*)memory_ptr);
}
void VirtualMachine::SetByte(char value, unsigned int address)
{
	if(!AddressInBounds(address))
		throw BytecodeException(HS_OUT_BOUNDS);

	char* memory_ptr = memory.get() + address;
	*memory_ptr = value;
}
void VirtualMachine::SetWord(int value, unsigned int address)
{
	if(!AddressRangeInBounds(address, 4))
		throw BytecodeException(HS_OUT_BOUNDS);

	char* memory_ptr = memory.get() + address;
	*((int*)memory_ptr) = value;
}
bool VirtualMachine::AddressInBounds(unsigned int address)
{
	if(address >= 0 && address < MEMORY_SIZE)
		return true;
	return false;
}
bool VirtualMachine::AddressRangeInBounds(unsigned int address, unsigned int length)
{
	if(address >= 0 && address + length < MEMORY_SIZE)
		return true;
	return false;
}