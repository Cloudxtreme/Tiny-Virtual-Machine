#include "vm.hpp"
#include "bytegen.hpp"

int main(int argc, char** args)
{	
	std::unique_ptr<BytecodeGenerator> g;

	try
	{
		g = std::unique_ptr<BytecodeGenerator>(new BytecodeGenerator());
	}
	catch(SourceException e)
	{
		std::cout << e.Get() << std::endl;
		return 1;
	}

	VirtualMachine vm;	

	vm.LoadProgram(g->GetBytecode(), g->GetBytecodeSize(), g->GetConstants());

	char* result = vm.TrackByte(2000);

	vm.Run();

	std::cout << "The value of tracked variable is: " << (int)*result << std::endl;

	return 0;	
}
