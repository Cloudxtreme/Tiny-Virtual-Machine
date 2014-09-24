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

	vm.SetWord(512, 1000);
	vm.SetWord(612, 1004);
	vm.SetWord(2, 1008);

	vm.LoadProgram(g->GetBytecode(), g->GetBytecodeSize());

	vm.Run();

	return 0;	
}
