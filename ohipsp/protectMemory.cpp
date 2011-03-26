#include "common.h"
#include "protector.h"

///////////////////////////////////////////////////////////////////////////////
// Globals
BYTE abNOPs[] = {
	0x27, // daa
	0x2F, // das
	0x37, // aaa
	0x3F, // aas
	0x40, // inc eax
	0x41, // inc ecx
	0x42, // inc edx
	0x43, // inc ebx
	0x45, // inc ebp
	0x46, // inc esi
	0x47, // inc edi
	0x48, // dec eax
	0x49, // dec ecx
	0x4A, // dec edx
	0x4B, // dec ebx
	0x4D, // dec ebp
	0x4E, // dec esi
	0x4F, // dec edi
	0x90, // nop
	0x91, // xchg    eax, ecx
	0x92, // xchg    eax, edx
	0x93, // xchg    eax, ebx
	0x95, // xchg    eax, ebp
	0x96, // xchg    eax, esi
	0x97, // xchg    eax, edi
	0x98, // cwde
	0x99, // cdq
	0x9E, // sahf
	0x9F, // lahf
	0xD6, // setalc
	0xF5, // cmc
	0xF8, // clc
	0xF9, // stc
	0xFC, // cld
	0xFD, // std
};

///////////////////////////////////////////////////////////////////////////////
// Local prototypes
SIZE_T PreallocateAddress(DWORD dwAddress, int iMode);


///////////////////////////////////////////////////////////////////////////////
// Functions

/******************************************************************************
 * Fills a memory page with calls to our shell-code
 ******************************************************************************/
SIZE_T PreallocateAddress(DWORD dwAddress, int iMode)
{
	MEMORY_BASIC_INFORMATION sMBI = {0};
	LPVOID lpvPage;
	PBYTE pbAddress;
	unsigned int uiIter;
	DWORD dwOldProtect;
	SIZE_T stReturn;

	__try
	{
		stReturn = 0;
		pbAddress = (PBYTE)dwAddress;
		lpvPage = VirtualAlloc((LPVOID)pbAddress, sizeof(abHeapLockerShellcode), MEM_COMMIT | MEM_RESERVE, ADDRESS_MODE_NOACCESS == iMode ? PAGE_NOACCESS : PAGE_EXECUTE_READWRITE);
		if (NULL == lpvPage)
		{
			PrintError(_TEXT("VirtualAlloc failed"));
			return stReturn;
		}
		if (FUNCTION_FAILED(VirtualQuery(lpvPage, &sMBI, sizeof(sMBI))))
		{
			PrintError(_TEXT("VirtualQuery failed"));
			return stReturn;
		}
		
		PrintInfo(_TEXT("Exploit address = %08x mode = %d page address = %08x memory size = %ld"), pbAddress, iMode, lpvPage, sMBI.RegionSize);
		
		stReturn = sMBI.RegionSize;
		if (ADDRESS_MODE_NOACCESS != iMode)
		{
			srand((unsigned int)time(0));
			// Fill region with nop-sled
			for (uiIter = 0; uiIter < sMBI.RegionSize; uiIter++)
			{
				*((PBYTE)lpvPage + uiIter) = abNOPs[rand() % countof(abNOPs)];
			}
			// TODO MUST Why is this calling rand()?
			pbAddress = (PBYTE)lpvPage + sMBI.RegionSize - 7 - rand() % 0x10;
			*pbAddress = 0xB8; // mov eax
			*(unsigned int *)(pbAddress + 1) = (unsigned int)ShellCodeToEntryPoint();
			*(pbAddress + 5) = 0xFF; // call
			*(pbAddress + 6) = 0xD0; // eax
			if (FUNCTION_FAILED(VirtualProtect(lpvPage, sMBI.RegionSize, PAGE_EXECUTE, &dwOldProtect)))
			{
				PrintError(_TEXT("VirtualProtect"));
				return stReturn;
			}
		}
	}
	__except(1)
	{
		PrintError("Exception");
		return stReturn;
	}

	return stReturn;
}


/******************************************************************************
 * Reads from the registry to determine if memory for the given process should
 * be protected, and if so protects it.
 ******************************************************************************/
void ProtectAddresses(HKEY hKeyApplication)
{
	HKEY hKey;
	DWORD dwValue;
	DWORD dwValueSize;
	DWORD dwType;
	DWORD dwIndex;
	TCHAR szValueName[MAX_REGISTRY_VALUE_NAME];
	DWORD dwValueNameSize;
	SIZE_T stMemory;
	int iIter;

	stMemory = 0;
	if (NULL != hKeyApplication && IS_SUCCESS(RegOpenKeyEx(hKeyApplication, REGISTRY_ADDRESSES, 0L, KEY_READ, &hKey)))
	{
		dwIndex = 0;
		dwValueNameSize = countof(szValueName);
		dwValueSize = sizeof(dwValue);
		while (ERROR_NO_MORE_ITEMS != RegEnumValue(hKey, dwIndex++, szValueName, &dwValueNameSize, NULL, &dwType, (LPBYTE) &dwValue, &dwValueSize))
		{
			if (REG_DWORD == dwType)
			{
				if (sHeapLockerSettings.dwVerbose > 0)
				{
					PrintInfo(_TEXT("Pre-allocating page for address 0x%08x (%s)"), dwValue, szValueName);
				}
				if (isdigit(szValueName[0]))
				{
					stMemory += PreallocateAddress(dwValue, szValueName[0] - '0');
				}
				else
				{
					stMemory += PreallocateAddress(dwValue, ADDRESS_MODE_NOACCESS);
				}
			}
			dwValueNameSize = countof(szValueName);
			dwValueSize = sizeof(dwValue);
		}
		RegCloseKey(hKey);
	}
	if (sHeapLockerSettings.dwGenericPreAllocate != 0xFFFFFFFF)
	{
		for (iIter = 1; iIter < 128; iIter++)
		{
			stMemory += PreallocateAddress(iIter * 0x1000000 + iIter * 0x10000 + iIter * 0x100 + iIter, sHeapLockerSettings.dwGenericPreAllocate);
		}
	}
	if (sHeapLockerSettings.dwVerbose > 0)
	{
		PrintInfo(_TEXT("Memory used for pre-allocated pages = %ld KB"), stMemory / 1024);
	}
}


/******************************************************************************
 * Allocate page 0 (NULL), to protect against possible problems
 ******************************************************************************/
void PreallocatePage0(void)
{
	// Based on http://www.ivanlef0u.tuxfamily.org/?p=355

	HMODULE hNTDLL;
	NTALLOCATEVIRTUALMEMORY NtAllocateVirtualMemory;
	SIZE_T stRegionSize;
	PVOID pvBaseAddress;
	DWORD dwResult;

	hNTDLL = LoadLibrary(_TEXT("ntdll.dll"));
	if (NULL == hNTDLL)
	{
		PrintError(_TEXT("Unable to load ntdll.dll"));
		return;
	}
	else
	{
		NtAllocateVirtualMemory = (NTALLOCATEVIRTUALMEMORY) GetProcAddress(hNTDLL, _TEXT("NtAllocateVirtualMemory"));
		if (NULL == NtAllocateVirtualMemory)
		{
			PrintError(_TEXT("GetProcAddress failed"));
			return;
		}
		else
		{
			pvBaseAddress = (PVOID) 0x1;
			stRegionSize = 0x1000;
			dwResult = NtAllocateVirtualMemory(GetCurrentProcess(), &pvBaseAddress, 0L, &stRegionSize, MEM_COMMIT | MEM_RESERVE, PAGE_NOACCESS);

			if (0 == dwResult)
			{
				PrintInfo(_TEXT("NULL page address = %08x memory size = %ld"), pvBaseAddress, stRegionSize);
			}
			else
			{
				PrintInfo(_TEXT("NtAllocateVirtualMemory failed, return code = %ld"), dwResult);
			}
			
		}
	}
}