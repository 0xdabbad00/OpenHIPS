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
 * @file		monitor\main.cpp
 * @summary		Monitors OpenHips components to ensure it is not removed accidentally or maliciously.
 */

// TODO MUST Do something for LoadAppInit_DLLs and RequireSignedAppInit_DLLs 

#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include "common.h"
#include <string.h>

///////////////////////////////////////////////////////////////////////////////
// Local prototypes
BOOL CreateConfig();
DWORD ReadCommand();
BOOL Uninstall();
BOOL EnsureInstalled();
BOOL EnsureAppInitIsSet(HKEY hKey);
BOOL AddDllToAppInitValue(HKEY hKey, char *pOrigValue, DWORD cbOrigValue);
BOOL GetDllShortPath(PCHAR &pShortPathBuf, DWORD *cbShortPathBuf);
bool Is64BitWindows();

///////////////////////////////////////////////////////////////////////////////
// Globals
#ifdef PORTABLE_32_BIT
BOOL bIam32Bit = TRUE;
char szFirstStageDll[] = "ohipsfs32.dll";
#else
BOOL bIam32Bit = FALSE;
char szFirstStageDll[] = "ohipsfs64.dll";
#endif

BOOL bSystemIs32Bit = !Is64BitWindows();

char szConfigFile[] = "ohips_config.txt";
char szConfigTextContinue[] = "Monitor: Monitor\r\n";
char szConfigTextUninstall[] = "Monitor: Uninstall\r\n";
char szAppInitKey[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows";
char szAppInitValue[] = "AppInit_DLLs";
const DWORD SLEEP_TIME = 5*1000; // 5 seconds

const enum COMMAND {COMMAND_SET, COMMAND_REMOVE, COMMAND_ERROR};

///////////////////////////////////////////////////////////////////////////////
// Functions

/******************************************************************************
 * Main
 ******************************************************************************/
int main(int argc, char **argv) 
{
	PrintInfo("Running OpenHips Monitor, compiled on: %s", __TIMESTAMP__);

	// TODO MUST instead of polling, set up monitors
	while(true)
	{
		PrintInfo("Start of loop");  // TODO REMOVE
		DWORD command = ReadCommand();
		if (command == COMMAND_SET)
		{
			PrintInfo("Set");
			EnsureInstalled();
		} else if (command == COMMAND_REMOVE)
		{
			PrintInfo("Uninstall");
			Uninstall();
		}
		else
		{
			PrintInfo("Error command");
			CreateConfig();
			EnsureInstalled();
		}
		
		PrintInfo("Sleeping for %d seconds", SLEEP_TIME/1000.0);
		Sleep(SLEEP_TIME);
	}
	
	return 0;
}

/******************************************************************************
 * @return TRUE if we created the config file correctly
 ******************************************************************************/
BOOL CreateConfig()
{
	HANDLE hFile = INVALID_HANDLE_VALUE;

	PrintInfo("Creating new config file"); // TODO REMOVE
	__try
	{
		hFile = CreateFile(
			szConfigFile,			//__in      LPCTSTR lpFileName,
			GENERIC_WRITE,			//__in      DWORD dwDesiredAccess,
			FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, //__in      DWORD dwShareMode,
			NULL,					//__in_opt  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
			CREATE_ALWAYS,			//__in      DWORD dwCreationDisposition,
			FILE_ATTRIBUTE_NORMAL,	//__in      DWORD dwFlagsAndAttributes,
			NULL);					//__in_opt  HANDLE hTemplateFile
		if (hFile == INVALID_HANDLE_VALUE)
		{
			PrintError("Unable to open config file %s for writing", szConfigFile);
			return FALSE;
		}

		BOOL bWriteStatus = FALSE;
		DWORD numBytesWritten = 0;

		bWriteStatus = WriteFile(
			hFile,							//__in         HANDLE hFile,
			szConfigTextContinue,			//__in         LPCVOID lpBuffer,
			(DWORD)strlen(szConfigTextContinue),	//__in         DWORD nNumberOfBytesToWrite,
			&numBytesWritten,				//__out_opt    LPDWORD lpNumberOfBytesWritten,
			NULL);							//__inout_opt  LPOVERLAPPED lpOverlapped
		if (!bWriteStatus || numBytesWritten <= 0)
		{
			PrintError("Unable to write to config file %s", szConfigFile);
			return FALSE;
		}

		// File written to successfully
		return TRUE;
	}
	__finally
	{
		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
		}
	}

	return FALSE;
}


/******************************************************************************
 * Reads the config file to determine if we should ensure we are installed 
 * or uninstall.
 * @return The results of reading the config file
 ******************************************************************************/
DWORD ReadCommand()
{
	// TODO MUST Redo this whole function so it monitors a file and parses it more cleanly, 
	// probably should watch a reg value or something eventualy

	HANDLE hFile = INVALID_HANDLE_VALUE;

	__try
	{
		hFile = CreateFile(
			szConfigFile,			//__in      LPCTSTR lpFileName,
			GENERIC_READ,			//__in      DWORD dwDesiredAccess,
			FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, //__in      DWORD dwShareMode,
			NULL,					//__in_opt  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
			OPEN_EXISTING,			//__in      DWORD dwCreationDisposition,
			FILE_ATTRIBUTE_NORMAL,	//__in      DWORD dwFlagsAndAttributes,
			NULL);					//__in_opt  HANDLE hTemplateFile
		if (hFile == INVALID_HANDLE_VALUE)
		{
			PrintError("Unable to open config file %s", szConfigFile);
			return COMMAND_ERROR;
		}

		char szBuffer[80] = {0};
		DWORD numBytesRead = 0;
		BOOL bReadStatus = FALSE;

		bReadStatus = ReadFile(
			hFile,				//__in         HANDLE hFile,
			szBuffer,			//__out        LPVOID lpBuffer,
			sizeof(szBuffer),	//__in         DWORD nNumberOfBytesToRead,
			&numBytesRead,		//__out_opt    LPDWORD lpNumberOfBytesRead,
			NULL); //__inout_opt  LPOVERLAPPED lpOverlapped
		if (!bReadStatus || numBytesRead <= 0)
		{
			PrintError("Unable to read config file %s (read %d bytes)", szConfigFile, numBytesRead);
			return COMMAND_ERROR;
		}

		if (0 == strcmp(szBuffer, szConfigTextContinue))
		{
			return COMMAND_SET;
		} else if (0 == strcmp(szBuffer, szConfigTextUninstall))
		{
			return COMMAND_REMOVE;
		} else
		{
			PrintError("Unable to parse config file %s (read \'%s\')", szConfigFile, szBuffer);
			return COMMAND_ERROR;
		}
	}
	__finally
	{
		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
		}
	}

	return COMMAND_ERROR;
}

/******************************************************************************
 * @return TRUE if we uninstall correctly
 ******************************************************************************/
BOOL Uninstall()
{
	HKEY hKey = NULL;
	DWORD cbBuf = 0;
	LPBYTE pBuf = NULL;
	DWORD dwStatus = 0;

	PCHAR szNewValue = NULL;
	DWORD cbNewValue = 0;

	// TODO Should need to spend some time recording what some values were, so if we uninstall, 
	// we can set all values back properly (necessary for non-AppInit values)

	__try
	{
		// TODO SHOULD do something for both 32-bit and 64-bit key here
		dwStatus = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,	//__in        HKEY hKey,
			szAppInitKey,		//__in_opt    LPCTSTR lpSubKey,
			0,					//__reserved  DWORD ulOptions,
			KEY_ALL_ACCESS,		//__in        REGSAM samDesired,
			&hKey);				//__out       PHKEY phkResult
		if(ERROR_SUCCESS != dwStatus)
		{
			PrintError("Unable to open registry key %s for reason %d", szAppInitKey, dwStatus);
			return FALSE;
		}

		//
		// Read AppInit_DLL value so we can remove ourselves from it
		//
	
		// Get size of reg value	
		dwStatus = RegQueryValueEx(
			hKey,			//__in         HKEY hKey,
			szAppInitValue,	//__in_opt     LPCTSTR lpValueName,
			NULL,			//__reserved   LPDWORD lpReserved,
			NULL,			//__out_opt    LPDWORD lpType,
			NULL,			//__out_opt    LPBYTE lpData,
			&cbBuf			//__inout_opt  LPDWORD lpcbData
			);
		if (ERROR_SUCCESS != dwStatus)
		{
			PrintError("Unable to read registry value %s\\%s for reason %d", szAppInitKey, szAppInitValue, dwStatus);
			return FALSE;
		}

		// Alloc mem for the reg value
		pBuf = (LPBYTE)LocalAlloc(LMEM_ZEROINIT, cbBuf+1);
		if (pBuf == NULL)
		{
			PrintError("Unable to alloc %d bytes to store value in %s\\%s", cbBuf, szAppInitKey, szAppInitValue);
			return FALSE;
		}

		// Get the reg value
		if (ERROR_SUCCESS != RegQueryValueEx(
			hKey,			//__in         HKEY hKey,
			szAppInitValue,	//__in_opt     LPCTSTR lpValueName,
			NULL,			//__reserved   LPDWORD lpReserved,
			NULL,			//__out_opt    LPDWORD lpType,
			pBuf,			//__out_opt    LPBYTE lpData,
			&cbBuf			//__inout_opt  LPDWORD lpcbData
			))
		{
			PrintError("Unable to read registry value %s\\%s", szAppInitKey, szAppInitValue);
			return FALSE;
		}

		// Find our DLL in the AppInit value
		PCHAR pDllStart = strstr((PCHAR)pBuf, szFirstStageDll);
		if (pDllStart == NULL)
		{
			PrintInfo("Dll not found in AppInit_Dll, so no need to uninstall");
			return TRUE;
		}

		PCHAR pDllPathStart = pDllStart;
		for(pDllPathStart = pDllStart; pDllPathStart > (PCHAR)pBuf && *pDllPathStart != ':'; pDllPathStart--);
		// We now have the location of the ":" in the path, so our path should be something like C:\blah\blah.dll
		// So go back one more to point to the drive letter
		pDllPathStart--;

		// Sanity check we didn't go back too far
		if (pDllPathStart < (PCHAR)pBuf)
		{
			PrintError("Problem finding DLL path in reg value");
			return FALSE;
		}

		// We should have now found the start of our DLL, but if possible also remove the space or comma between us and any other DLL's
		if (pDllPathStart-1 > (PCHAR)pBuf)
		{
			pDllPathStart--;
		}

		pDllPathStart[0] = '\0';

		// Find end
		PCHAR pDllEnd = pDllStart+strlen(szFirstStageDll);
		// We may have a comma or period after our DLL before the next DLL, so move our DLL end offset
		if (pDllEnd+1 < (PCHAR)pBuf+cbBuf)
		{
			pDllEnd++;
		}

		// Alloc mem for new value and build it
		// It will be smaller than the current value, but we'll just get a buffer of the same size
		cbNewValue = cbBuf;

		szNewValue = (PCHAR)LocalAlloc(LMEM_ZEROINIT, cbNewValue+1);
		if (szNewValue == NULL)
		{
			PrintError("Unable to alloc mem");
			return FALSE;
		}

		// Concat everything before our DLL with everything after
		DWORD dwNewValueLength = 0;
		dwNewValueLength = sprintf_s(szNewValue, cbNewValue, "%s%s", pBuf, pDllEnd);
		if (dwNewValueLength == -1)
		{
			PrintError("Unable to create string for new reg value");
			return FALSE;
		}
		
		// Set registry value
		if (ERROR_SUCCESS != RegSetValueEx(
			hKey,				//__in        HKEY hKey,
			szAppInitValue,		//__in_opt    LPCTSTR lpValueName,
			0,					//__reserved  DWORD Reserved,
			REG_SZ,				//__in        DWORD dwType,
			(PBYTE)szNewValue,			//__in_opt    const BYTE *lpData,
			dwNewValueLength	//__in        DWORD cbData
			))
		{
			PrintError("Unable to set reg value");
			return FALSE;
		}
	
		return TRUE;
	}
	__finally
	{
		if (hKey != NULL)
		{
			RegCloseKey(hKey);
		}
		if (pBuf != NULL)
		{
			LocalFree(pBuf);
		}
		if (szNewValue != NULL)
		{
			LocalFree(szNewValue);
		}
	}

	return FALSE;
}


/******************************************************************************
 * @return TRUE if the reg values are set correctly
 ******************************************************************************/
BOOL EnsureInstalled()
{
	HKEY hKey = NULL;

	__try
	{
		// TODO SHOULD set both 32-bit and 64-bit key here
		DWORD dwStatus = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,	//__in        HKEY hKey,
			szAppInitKey,		//__in_opt    LPCTSTR lpSubKey,
			0,					//__reserved  DWORD ulOptions,
			KEY_ALL_ACCESS,		//__in        REGSAM samDesired,
			&hKey);				//__out       PHKEY phkResult

		if(ERROR_SUCCESS != dwStatus)
		{
			PrintError("Unable to open registry key %s for reason %d", szAppInitKey, dwStatus);
			return FALSE;
		}

		EnsureAppInitIsSet(hKey);
	}
	__finally
	{
		if (hKey != NULL)
		{
			RegCloseKey(hKey);
		}
	}

	return TRUE;
}


/******************************************************************************
 * @param hKey	Handle to key containing AppInit_Dlls value
 * @return TRUE if AppInit_Dlls value is set correctly, or was able to be set.
 ******************************************************************************/
BOOL EnsureAppInitIsSet(HKEY hKey)
{
	DWORD cbBuf = 0;
	LPBYTE pBuf = NULL;

	__try
	{
		// Get size of reg value	
		DWORD dwStatus = 0;
		dwStatus = RegQueryValueEx(
			hKey,			//__in         HKEY hKey,
			szAppInitValue,	//__in_opt     LPCTSTR lpValueName,
			NULL,			//__reserved   LPDWORD lpReserved,
			NULL,			//__out_opt    LPDWORD lpType,
			NULL,			//__out_opt    LPBYTE lpData,
			&cbBuf			//__inout_opt  LPDWORD lpcbData
			);
		if (ERROR_SUCCESS != dwStatus)
		{
			PrintError("Unable to read registry value %s\\%s for reason %d", szAppInitKey, szAppInitValue, dwStatus);
			return FALSE;
		}

		// Alloc mem for the reg value
		pBuf = (LPBYTE)LocalAlloc(LMEM_ZEROINIT, cbBuf+1);
		if (pBuf == NULL)
		{
			PrintError("Unable to alloc %d bytes to store value in %s\\%s", cbBuf, szAppInitKey, szAppInitValue);
			return FALSE;
		}

		// Get the reg value
		if (ERROR_SUCCESS != RegQueryValueEx(
			hKey,			//__in         HKEY hKey,
			szAppInitValue,	//__in_opt     LPCTSTR lpValueName,
			NULL,			//__reserved   LPDWORD lpReserved,
			NULL,			//__out_opt    LPDWORD lpType,
			pBuf,			//__out_opt    LPBYTE lpData,
			&cbBuf			//__inout_opt  LPDWORD lpcbData
			))
		{
			PrintError("Unable to read registry value %s\\%s", szAppInitKey, szAppInitValue);
			return FALSE;
		}

		// TODO SHOULD actually check for the full path to DLL
		// Check if the reg value has our string
		if (NULL != strstr((char *)pBuf, szFirstStageDll))
		{
			// Reg value already has our DLL in it
			return TRUE;
		}
		else
		{
			// Need to set the reg value
			return AddDllToAppInitValue(hKey, (char *)pBuf, cbBuf);
		}
	}
	__finally
	{
		if (pBuf != NULL)
		{
			LocalFree(pBuf);
		}
	}
	return FALSE;
}




/******************************************************************************
 * @return TRUE if DLL could be properly set
 ******************************************************************************/
BOOL AddDllToAppInitValue(HKEY hKey, char *pOrigValue, DWORD cbOrigValue)
{
	DWORD cbShortPathBuf = 0;
	PCHAR pShortPathBuf = NULL;

	PCHAR szNewValue = NULL;
	DWORD cbNewValue = 0;

	PrintInfo("Adding DLL to AppInit_Dll");

	__try
	{
		// Append our DLL path to the end of the reg value
		if (!GetDllShortPath(pShortPathBuf, &cbShortPathBuf))
		{
			PrintError("Unable to get DLL path");
			return FALSE;
		}

		PrintInfo("Dll path is: %s", pShortPathBuf); // TODO REMOVE

		// Determine lenght of new value and alloc mem
		// String will be: orig_value + ' ' + dll_path
		cbNewValue = cbOrigValue + 1 + (DWORD)strlen(pShortPathBuf) + 1;  

		szNewValue = (PCHAR)LocalAlloc(LMEM_ZEROINIT, cbNewValue);
		if (szNewValue == NULL)
		{
			PrintError("Unable to alloc mem for new value");
			return FALSE;
		}

		// Create new value
		DWORD dwNewValueLength = 0;
		dwNewValueLength = sprintf_s(szNewValue, cbNewValue, "%s %s", pOrigValue, pShortPathBuf);
		if (dwNewValueLength == -1)
		{
			PrintError("Unable to create string for new reg value");
			return FALSE;
		}

		PrintInfo("New reg value: %s", szNewValue);

		// Set registry value
		if (ERROR_SUCCESS != RegSetValueEx(
			hKey,				//__in        HKEY hKey,
			szAppInitValue,		//__in_opt    LPCTSTR lpValueName,
			0,					//__reserved  DWORD Reserved,
			REG_SZ,				//__in        DWORD dwType,
			(PBYTE)szNewValue,			//__in_opt    const BYTE *lpData,
			dwNewValueLength	//__in        DWORD cbData
			))
		{
			PrintError("Unable to set reg value");
			return FALSE;
		}

		// It worked!
		return TRUE;
	
	}
	__finally
	{
		if (pShortPathBuf != NULL)
		{
			LocalFree(pShortPathBuf);
		}
		if (szNewValue != NULL)
		{
			LocalFree(szNewValue);
		}
	}
	return FALSE;
}


/******************************************************************************
 * @param pShortPathBuf		Set to short path to DLL
 * @param cbShortPathBuf	Set to length of pShortPathBuf
 * @return TRUE if DLL path was found and set in the value
 ******************************************************************************/
BOOL GetDllShortPath(PCHAR &pShortPathBuf, DWORD *cbShortPathBuf)
{
	DWORD cbFullPathBuf = MAX_PATH;
	PCHAR pFullPathBuf = NULL;
	*cbShortPathBuf = MAX_PATH+(DWORD)strlen(szFirstStageDll);
	pShortPathBuf = NULL;
	BOOL bRet = FALSE;

	__try
	{
		// Alloc mem for our path
		pFullPathBuf = (PCHAR)LocalAlloc(LMEM_ZEROINIT, cbFullPathBuf+1);
		if (pFullPathBuf == NULL)
		{
			PrintError("Unable to alloc mem for path");
			return FALSE;
		}

		if (0 == GetModuleFileName(NULL, pFullPathBuf, cbFullPathBuf))
		{
			PrintError("Unable to get module path");
			return FALSE;
		}

		PrintInfo("Exe path: %s", pFullPathBuf);
		char *pLastPathDelimiter = strrchr(pFullPathBuf, '\\');
		if (NULL == pLastPathDelimiter)
		{
			PrintError("Unable to find path delimiter");
			return FALSE;
		}

		// Ignore file name
		*(pLastPathDelimiter+1) = '\0';

		// Alloc mem for short path
		pShortPathBuf = (PCHAR)LocalAlloc(LMEM_ZEROINIT, *cbShortPathBuf+1);
		if (pShortPathBuf == NULL)
		{
			PrintError("Unable to alloc mem for short path");
			return FALSE;
		}
		// Get short path
		if (0 == GetShortPathName(pFullPathBuf, pShortPathBuf, *cbShortPathBuf))
		{
			PrintError("Unable to determine short path");
			return FALSE;
		}

		// Concat DLL name to shortened path
		if (0 != strncat_s(pShortPathBuf, *cbShortPathBuf, szFirstStageDll, strlen(szFirstStageDll)))
		{
			PrintError("Unable concat DLL name to path");
			return FALSE;
		}

		PrintInfo("Dll path: %s", pShortPathBuf);

		bRet = TRUE;
	}
	__finally
	{
		if (pFullPathBuf != NULL)
		{
			LocalFree(pFullPathBuf);
		}
		if (!bRet)
		{
			if (pShortPathBuf != NULL)
			{
				LocalFree(pShortPathBuf);
			}
			cbShortPathBuf = 0;
		}
	}
	return bRet;
}

/******************************************************************************
 * @return TRUE if system is 64-bit
 ******************************************************************************/
bool Is64BitWindows()
{
#if defined(PORTABLE_64_BIT)
    return true;  // 64-bit programs run only on Win64
#elif defined(PORTABLE_32_BIT)
    // 32-bit programs run on both 32-bit and 64-bit Windows
    // so must sniff
    BOOL f64 = false;
    return (bool)(IsWow64Process(GetCurrentProcess(), &f64) && f64);
#else
    return false; // Win64 does not support Win16
#endif
}

