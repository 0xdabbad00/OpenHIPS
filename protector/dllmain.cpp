/**
 * This file is part of OpenHIPS.
 *
 * OpenHIPS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenHIPS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenHIPS.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file		protector\dllmain.cpp
 * @summary		Based heavily on Didier Steven's HeapLocker project
 *		(http://blog.didierstevens.com/programs/heaplocker/)
 *		Based on HeapLocker 0.0.0.3 from 2010/12/15
 */

#include "common.h"
#include "protector.h"

#pragma comment(lib, "psapi.lib")

///////////////////////////////////////////////////////////////////////////////
// Globals

HEAPLOCKER_SETTINGS sHeapLockerSettings;





static TCHAR szModuleName[MAX_PATH];
static TCHAR szDump[256];

///////////////////////////////////////////////////////////////////////////////
// Functions

/******************************************************************************
 * @return An empty string if pszString is NULL
 ******************************************************************************/
LPTSTR NULL2EmptyString(LPTSTR pszString)
{
	return pszString == NULL ? "" : pszString;
}


/******************************************************************************
 * @return An empty string if pszString is NULL
 ******************************************************************************/
LPTSTR GetExecutableName(void)
{
	LPTSTR pszEXE;

	__try
	{
		if (!GetModuleFileName(NULL, szModuleName, MAX_PATH))
		{
			PrintError(_TEXT("GetModuleFileName"));
			return NULL;
		}
		pszEXE = _tcsrchr(szModuleName, '\\');
		if (NULL == pszEXE)
		{
			return NULL;
		}
		return ++pszEXE;
	}
	__except(1)
	{
		PrintError("Exception");
		return NULL;
	}
}


/******************************************************************************
 * @return Returns a pointer to a global var containing a hexdump of the given values
 ******************************************************************************/
LPTSTR HexDump(PBYTE pbFound, int iSize)
{
	int iIter;

	__try
	{
		szDump[iSize] = '\0';
		for (iIter = 0; iIter < iSize && iIter < countof(szDump); iIter++)
		{
			szDump[iIter] = isprint(*(pbFound + iIter*2)) ? *(pbFound + iIter*2) : '.';
		}
		PrintInfo(_TEXT(" %08X: %s"), pbFound, szDump);
		return szDump;
	}
	__except(1)
	{
		PrintError("Exception");
	}

	return NULL;
}





/******************************************************************************
 * 
 ******************************************************************************/
DWORD WINAPI HeapLocker(LPVOID lpvArgument)
{
	HKEY hKeyApplication;

	Sleep(100); // Sleep some time to wait for advapi32.dll to load completely (should this DLL be loaded via appinit_dll)

	PrintInfo("Running HeapLocker");

	hKeyApplication = GetApplicationRegKey();

	if (NULL != hKeyApplication)
	{
		ReadHeapLockerSettingsFromRegistryApplication(hKeyApplication);
	}

	ReadHeapLockerSettingsFromRegistry();

	SetHeapLockerSettingsDefaults();

	PrintInfo(_TEXT("Maximum value for PrivateUsage = %ld MB (%d)"), sHeapLockerSettings.dwPrivateUsageMax, sHeapLockerSettings.iOrigin_dwPrivateUsageMax);
	PrintInfo(_TEXT("Minimum value for NOP-sled length = %ld (%d)"), sHeapLockerSettings.dwNOPSledLengthMin, sHeapLockerSettings.iOrigin_dwNOPSledLengthMin);
	PrintInfo(_TEXT("Pre-allocation of generic addresses = %ld (%d)"), sHeapLockerSettings.dwGenericPreAllocate, sHeapLockerSettings.iOrigin_dwGenericPreAllocate);
	PrintInfo(_TEXT("Search mode = %d (%d) length = %d (%d)"), sHeapLockerSettings.iSearchMode, sHeapLockerSettings.iOrigin_iSearchMode, sHeapLockerSettings.iSearchLen, sHeapLockerSettings.iOrigin_iSearchLen);
	PrintInfo(_TEXT("Pre-allocation of NULL page = %ld (%d)"), sHeapLockerSettings.dwPreallocatePage0, sHeapLockerSettings.iOrigin_dwPreallocatePage0);
	PrintInfo(_TEXT("Verbosity = %ld (%d)"), sHeapLockerSettings.dwVerbose, sHeapLockerSettings.iOrigin_dwVerbose);
	PrintInfo(_TEXT("Force process termination = %ld (%d)"), sHeapLockerSettings.dwForceTermination, sHeapLockerSettings.iOrigin_dwForceTermination);
	PrintInfo(_TEXT("Resume monitoring = %ld (%d)"), sHeapLockerSettings.dwResumeMonitoring, sHeapLockerSettings.iOrigin_dwResumeMonitoring);

	ProtectAddresses(hKeyApplication);
	if (NULL != hKeyApplication)
	{
		RegCloseKey(hKeyApplication);
	}

	if (sHeapLockerSettings.dwPreallocatePage0 > 0)
	{
		PreallocatePage0();
	}

	if (sHeapLockerSettings.dwNOPSledLengthMin > 0)
	{
		CreateThread(NULL, 0, MonitorNewPagesForNOPSleds, NULL, 0, NULL);
	}

	if (0xFFFFFFFF != sHeapLockerSettings.dwPrivateUsageMax)
	{
		CreateThread(NULL, 0, MonitorPrivateUsage, NULL, 0, NULL);
	}

	if (sHeapLockerSettings.iSearchLen > 0)
	{
		CreateThread(NULL, 0, MonitorNewPagesToSearchThem, NULL, 0, NULL);
	}

	return 0;
}


/******************************************************************************
 * 
 ******************************************************************************/
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  dwReason,
                       LPVOID lpReserved
					 )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		PrintInfo("Running OpenHips Protector (ohipsp), compiled on: %s", __TIMESTAMP__);
		PrintInfo("Loaded into a process %d", GetCurrentProcessId());

		CreateThread(NULL, 0, HeapLocker, NULL, 0, NULL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		PrintInfo("Unloading ohipsp from process %d", GetCurrentProcessId());
	}

	return TRUE;
}