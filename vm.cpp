#include "vm.hpp"

BytecodeException::BytecodeException(unsigned int code)
{
	this->code = code;
}

VirtualMachine::VirtualMachine()
{
	memory = std::unique_ptr<char[]>(new char[MEMORY_SIZE]);
	registers = std::unique_ptr<int[]>(new int[NUM_REGISTERS]);

	for(int i=0; i<MEMORY_SIZE; i++)
		memory[i] = 0;

	for(int i=0; i<NUM_REGISTERS; i++)
		registers[i] = 0;

	pc = 0;
	sp = 0;

	cmp_flag = CMP_NONE;

	halt_signal = 0;
}
bool VirtualMachine::LoadProgram(char* data, unsigned int length, std::map<unsigned int, char>* constants)
{
	if(length > MEMORY_SIZE)
		return false;

	for(unsigned int i=0; i<length; i++)
	{
		memory[i] = data[i];
	}

	std::map<unsigned int, char>::iterator constants_it;
	for(constants_it = constants->begin(); constants_it != constants->end(); constants_it++)
	{
		try
		{
			SetByte(constants_it->second, constants_it->first);
		}
		catch(BytecodeException exception)
		{
			halt_signal = exception.code;
		}
	}

	return true;
}
void VirtualMachine::Run()
{
	while(halt_signal == 0)
	{
		Cycle();
	}
	#ifdef DEBUG
		switch(halt_signal)
		{
			case HS_OUT_BOUNDS:
				std::cout << "TERMINATED: ADDRESS OUT OF BOUNDS" << std::endl;
				break;
			case HS_UNKNOWN_COMMAND:
				std::cout << "TERMINATED: UNKNOWN COMMAND" << std::endl;
				break;
			case HS_INVALID_ARGUMENT:
				std::cout << "TERMINATED: INVALID ARGUMENT" << std::endl;
				break;
			case HS_USER:
				std::cout << "Program terminated naturally." << std::endl;
				break;
		}
	#endif
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
			if(pc + 6 >= MEMORY_SIZE)
			{
				halt_signal = HS_OUT_BOUNDS;
				break;
			}

			int register_index;
			char mode;
			unsigned int address;

			try
			{
				register_index = GetByte(argument_address);
				mode = GetByte(argument_address+1);

				if(mode == 0)
					address = GetWord(argument_address+2);
				else
					address = GetByte(argument_address+2);
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
					if(mode == 0)
					{
						value = GetWord(address);
					}
					else
					{
						unsigned int value_address = registers[(char)address];
						value = GetWord(value_address);
					}
				}
				catch(BytecodeException exception)
				{
					halt_signal = exception.code;
					break;
				}

				registers[register_index] = value;
				
				#ifdef DEBUG
					std::cout << "Loaded value " << value << " from " << address << " to R" << (int)register_index << std::endl;
				#endif
			}
			else //if INS_STORE
			{
				value = registers[register_index];

				try
				{
					if(mode == 0)
					{
						SetWord(value, address);
					}
					else
					{
						unsigned int value_address = registers[(char)address];
						SetWord(value, value_address);
					}
				}
				catch(BytecodeException exception)
				{
					halt_signal = exception.code;
					break;
				}

				#ifdef DEBUG
					std::cout << "Stored value " << value << " to " << address << " from R" << (int)register_index << std::endl;
				#endif
			}

			//1 byte for the type, 1 for register, 1 for mode, 4 for address
			pc += 7;
			
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

				#ifdef DEBUG
					std::cout << "Performed ADD of R" << (int)register_2 << " and R" << (int)register_3 << ", result stored in R" << (int)register_1 << std::endl;
				#endif
			}
			else
			{
				registers[register_1] = registers[register_2] - registers[register_3];

				#ifdef DEBUG
					std::cout << "Performed SUB of R" << (int)register_2 << " and R" << (int)register_3 << ", result stored in R" << (int)register_1 << std::endl;
				#endif
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

				#ifdef DEBUG
					std::cout << "Performed LEFT SHIFT of R" << (int)register_src << " for " << (int)shift_amount << ", result stored in R" << (int)register_dest << std::endl;
				#endif
			}
			else
			{
				registers[register_dest] = registers[register_src] >> shift_amount;

				#ifdef DEBUG
					std::cout << "Performed RIGHT SHIFT of R" << (int)register_src << " for " << (int)shift_amount << ", result stored in R" << (int)register_dest << std::endl;
				#endif
			}

			pc += 4;

			break;
		}	
		case INS_JMP:
		{
			if(pc + 5 >= MEMORY_SIZE)
			{
				halt_signal = HS_OUT_BOUNDS;
				break;
			}

			unsigned int address;
			char condition;

			try
			{
				address = GetWord(argument_address);		
				condition = GetByte(argument_address+4);		
			}
			catch(BytecodeException exception)
			{
				halt_signal = exception.code;
				break;
			}		

			bool condition_fulfilled = false;
			bool invalid_condition = false;	

			switch(condition)
			{
				case COND_NONE:
					condition_fulfilled = true;
					break;
				case COND_LT:
					if(cmp_flag == CMP_LT)
						condition_fulfilled = true;
					break;
				case COND_GT:
					if(cmp_flag == CMP_GT)
						condition_fulfilled = true;
					break;
				case COND_LE:
					if(cmp_flag == CMP_LT || cmp_flag == CMP_EQ)
						condition_fulfilled = true;
					break;
				case COND_GE:
					if(cmp_flag == CMP_GT || cmp_flag == CMP_EQ)
						condition_fulfilled = true;
					break;
				case COND_EQ:
					if(cmp_flag == CMP_EQ)
						condition_fulfilled = true;
					break;
				default:
					invalid_condition = true;
					break;
			}

			if(invalid_condition)
			{
				halt_signal = HS_INVALID_ARGUMENT;
				break;
			}

			if(condition_fulfilled)
				pc = address;
			else
				pc += 6;

			break;
		}
		case INS_CMP:
		{
			if(pc + 2 >= MEMORY_SIZE)
			{
				halt_signal = HS_OUT_BOUNDS;
				break;
			}

			char register_1, register_2;

			try
			{
				register_1 = GetByte(argument_address);			
				register_2 = GetByte(argument_address+1);							
			}
			catch(BytecodeException exception)
			{
				halt_signal = exception.code;
				break;
			}

			if(register_1 < 0 || register_1 >= NUM_REGISTERS
				|| register_2 < 0 || register_2 >= NUM_REGISTERS)
			{
				halt_signal = HS_OUT_BOUNDS;
				break;
			}

			unsigned int value_1, value_2;

			value_1 = registers[register_1];
			value_2 = registers[register_2];

			if(value_1 < value_2)
				cmp_flag = CMP_LT;
			else if(value_1 > value_2)
				cmp_flag = CMP_GT;
			else
				cmp_flag = CMP_EQ;

			#ifdef DEBUG
				std::cout << "Compared values in R" << (int)register_1 << " and R" << (int)register_2 << ", flag: " << (int)cmp_flag << std::endl;
			#endif

			pc += 3;

			break;
		}
		case INS_HALT:
		{
			halt_signal = HS_USER;

			#ifdef DEBUG
				std::cout << "HALT" << std::endl;
			#endif

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
char* VirtualMachine::TrackByte(unsigned int address)
{
	return (char*)(memory.get() + address);
}
int* VirtualMachine::TrackWord(unsigned int address)
{
	char* memory_ptr = memory.get() + address;
	return ((int*)memory_ptr);
}