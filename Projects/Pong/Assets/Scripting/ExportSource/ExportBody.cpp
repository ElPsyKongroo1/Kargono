#include "ExportHeader.h"
#include <iostream>
namespace Kargono
{
static std::function<void()> testFuncPtr {};
void testFunction()
{
testFuncPtr();
}
void AddVoidNone(const std::string& funcName, std::function<void()> funcPtr)
{
if (funcName == "testFunction") { testFuncPtr = funcPtr; return; }
}
void KG_FUNC_753079771072191180()
{
}

void KG_FUNC_7448269885476125662()
{
	std::cout << "Hello world! HAHAHAHAA\n";
	testFunction();
}

}
