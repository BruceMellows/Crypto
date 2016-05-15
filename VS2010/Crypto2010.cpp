// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

// Crypto2010.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>

extern void RunTests();

int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		RunTests();
	}
	catch(const char* message)
	{
		std::cout << message << std::endl;
	}
	catch(const wchar_t* message)
	{
		std::wcout << message << std::endl;
	}
	return 0;
}

