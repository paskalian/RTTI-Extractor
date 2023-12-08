#include <Windows.h>
#include <iostream>
#include <typeinfo>
#include <vector>
#include <algorithm>

// Uncomment if you wish to test it directly
//#include "example.hpp"

// Source for GetRuntimeClassname function
// https://github.com/ReClassNET/ReClass.NET/blob/b89bbfd97bc431d12dc2b8897478c98e09797f09/ReClass.NET/Memory/RemoteProcess.cs#L367-L482

#ifdef _WIN64
std::vector<std::string> GetRuntimeClassname(void* pClassInstance)
{
	void** pVT = *(void***)pClassInstance;

	std::vector<std::string> ClassNames;

	if (pVT != (void*)0xCCCCCCCCCCCCCCCC)
	{
		void* pData = *(pVT - 1);

		uint32_t ClassHierarchyDescOff = *(uint32_t*)((uintptr_t)pData + 0x10);

		if (ClassHierarchyDescOff)
		{
			uint32_t ModBaseOff = *(uint32_t*)((uintptr_t)pData + 0x14);

			uintptr_t ModBase = (uintptr_t)pData - ModBaseOff;
			uintptr_t ClassHierarchyDesc = ModBase + ClassHierarchyDescOff;
			uint32_t ClassCount = *(uint32_t*)(ClassHierarchyDesc + 0x8);
			uint32_t BaseClassArrayOff = *(uint32_t*)(ClassHierarchyDesc + 0xC);

			if (BaseClassArrayOff)
			{
				uint32_t* BaseClassArray = (uint32_t*)(ModBase + BaseClassArrayOff);

				if (ClassCount > 0 && ClassCount < 25)
				{
					for (unsigned int i = 0; i < ClassCount; i++)
					{
						uint32_t BaseClassDescOff = BaseClassArray[i];

						if (BaseClassDescOff)
						{
							uint32_t TypeDescriptorOff = *(uint32_t*)(ModBase + BaseClassDescOff);
							if (TypeDescriptorOff)
							{
								uintptr_t TypeInfo = ModBase + TypeDescriptorOff;
								std::string TypeName = (const char*)TypeInfo + 0x14;

								if (TypeName.find_last_of("@@") != std::string::npos)
									TypeName.resize(TypeName.length() - 2);

								ClassNames.push_back(TypeName);
							}
						}
					}
				}


			}
		}
	}

	return ClassNames;
}
#else
std::vector<std::string> GetRuntimeClassname(void* pClassInstance)
{
	void** pVT = *(void***)pClassInstance;

	std::vector<std::string> ClassNames;

	if (pVT != (void*)0xCCCCCCCC)
	{
		void* pData = *(pVT - 1);

		uint32_t ClassHierarchyDesc = *(uint32_t*)((uintptr_t)pData + 0x10);
		uint32_t ClassCount = *(uint32_t*)(ClassHierarchyDesc + 0x8);

		if (ClassCount > 0 && ClassCount < 25)
		{
			uint32_t** BaseClassArray = *(uint32_t***)(ClassHierarchyDesc + 0xC);

			for (unsigned int i = 0; i < ClassCount; i++)
			{
				uint32_t TypeInfo = BaseClassArray[i][0];

				std::string TypeName = (const char*)TypeInfo + 0x0C;

				if (TypeName.find_last_of("@@") != std::string::npos)
					TypeName.resize(TypeName.length() - 2);

				ClassNames.push_back(TypeName);
			}
		}
	}

	return ClassNames;
}
#endif

void PrintClassnames(std::vector<std::string>& Classnames)
{
	if (Classnames.empty())
		printf("No class with rtti found.\n");
	else
	{
		std::reverse(Classnames.begin(), Classnames.end());

		for (unsigned int i = 0; i < Classnames.size(); i++)
		{
			printf("%s\n", Classnames[i].c_str());
			if (i + 1 < Classnames.size())
			{
				for (unsigned int i2 = 0; i2 < (i + 1) * 2; i2++)
				{
					putchar(' ');
				}
				printf("-> ");
			}
		}
	}
}

void MainThread(HMODULE hModule)
{
	AllocConsole();

	FILE* EmptyFile;
	freopen_s(&EmptyFile, "CONIN$", "r", stdin);
	freopen_s(&EmptyFile, "CONOUT$", "w", stdout);
	freopen_s(&EmptyFile, "CONOUT$", "w", stderr);


	while (true)
	{
		char ClassAddressString[_MAX_PATH]{};
		printf("Enter a class address (must have vtable): ");
		std::cin >> ClassAddressString;
		printf("\n");

		void* ClassAddress = (void*)strtoll(ClassAddressString, NULL, 0);

		std::vector<std::string> Classnames = GetRuntimeClassname(ClassAddress);

		PrintClassnames(Classnames);

		bool Redo = true;
		printf("\nPress [R] to extract a new address.\nPress [F] to finish.\n");

		while (true)
		{
			if (GetAsyncKeyState(0x52) & 1)
			{
				system("cls");
				break;
			}

			if (GetAsyncKeyState(0x46) & 1)
			{
				Redo = false;
				break;
			}
		}

		if (!Redo)
			break;
	}

	ExitProcess(0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)MainThread, hModule, NULL, NULL);

		break;
	}
	}
	return TRUE;
}