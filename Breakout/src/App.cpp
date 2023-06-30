
#ifdef KARGONO_EXPORTS
#define KARGONO_API __declspec(dllexport)
#else
#define KARGONO_API __declspec(dllimport)
#endif

namespace Kargono 
{
	KARGONO_API void Print();
}


int main() 
{
	Kargono::Print();
}