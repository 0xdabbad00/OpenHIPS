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
 * @file		firststage\dllmain.cpp
 * @summary		Based heavily on Didier Steven's LoadDllViaAppInit project 
 *		(http://blog.didierstevens.com/2009/12/23/loaddllviaappinit/).
 *		This DLL will be loaded via the AppInit registry value into every process.
 *		The purpose of this DLL is then to load another DLL based on the process name.
 */

#include "common.h"
#include <stdio.h>
#include <share.h>

///////////////////////////////////////////////////////////////////////////////
// Local prototypes
BOOL LoadDlls(HMODULE hModule);
BOOL LoadDllsSpecifiedInConfig(PCHAR szConfigPath, PCHAR szProcessName, PCHAR szProjectPath);

///////////////////////////////////////////////////////////////////////////////
// Globals
#ifdef PORTABLE_32_BIT
char szConfigFile[] = "dllLoad32.cfg";
#else
char szConfigFile[] = "dllLoad64.cfg";
#endif

///////////////////////////////////////////////////////////////////////////////
// Functions
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  dwReason,
                       LPVOID lpReserved
					 )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		PrintInfo("Running OpenHips First Stage (ohipsfs), compiled on: %s", __TIMESTAMP__);
		PrintInfo("Loaded into a process %d", GetCurrentProcessId());

		LoadDlls(hModule);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		PrintInfo("Unloading ohipsfs from process %d", GetCurrentProcessId());
	}

	return TRUE;
}


/******************************************************************************
 * Read in a config file and determine what DLL's (if any) to load into this process
 ******************************************************************************/
BOOL LoadDlls(HMODULE hModule)
{
	const DWORD dwDllPath = MAX_PATH;
	PCHAR szDllPath = NULL;
	
	const DWORD dwConfigPath = MAX_PATH;
	PCHAR szConfigPath = NULL;

	// TODO MAYBE find the size of the path, instead of using MAX_PATH
	const DWORD dwProcessPath = MAX_PATH;
	PCHAR szProcessPath = NULL;

	__try
	{
		__try {
			//
			// Get this DLL path so we can find the config file (which will be in the same directory)
			//

			// Alloc mem
			szDllPath = (PCHAR)LocalAlloc(LMEM_ZEROINIT, dwDllPath+1);
			if (szDllPath == NULL)
			{
				PrintError("Could not alloc mem");
				return FALSE;
			}

			// Get process path
			DWORD dwDllLength = GetModuleFileName(hModule, szDllPath, dwDllPath);
			if (0 == dwDllLength
				|| dwDllLength == dwDllPath
				|| GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				PrintError("Unable to determine process path, retrieved %s", szDllPath);
				return FALSE;
			}

			PCHAR szDllName = strrchr(szDllPath, '\\');
			if (szDllName == NULL)
			{
				PrintError("Unable to determine dll name for path %s", szProcessPath);
				return FALSE;
			}

			// Set the char after the '\\' to zero, giving us the path without the filename and with
			// a null-terminator
			szDllName[1] = '\0';
			//PrintInfo("Path DLL is running from: %s", szDllPath);

			//
			// Get the config path
			//

			// Alloc mem
			szConfigPath = (PCHAR)LocalAlloc(LMEM_ZEROINIT, dwConfigPath+1);
			if (szConfigPath == NULL)
			{
				PrintError("Could not alloc mem");
				return FALSE;
			}
			int errno = _snprintf_s(szConfigPath, dwConfigPath, _TRUNCATE, "%s%s", szDllPath, szConfigFile);
			if (errno == -1)
			{
				PrintError("Unable to set config path");
				return FALSE;
			}
			//PrintInfo("Config file: %s", szConfigPath);

			//
			// Get this processes name
			//

			// Alloc mem
			szProcessPath = (PCHAR)LocalAlloc(LMEM_ZEROINIT, dwProcessPath+1);
			if (szProcessPath == NULL)
			{
				PrintError("Could not alloc mem");
				return FALSE;
			}

			// Get process path
			DWORD dwPathLength = GetModuleFileName(NULL, szProcessPath, dwProcessPath);
			if (0 == dwPathLength
				|| dwPathLength == dwProcessPath
				|| GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				PrintError("Unable to determine process path, retrieved %s", szProcessPath);
				return FALSE;
			}

			PrintInfo("Process path: %s", szProcessPath);

			PCHAR szProcessName = strrchr(szProcessPath, '\\');
			if (szProcessName == NULL)
			{
				PrintError("Unable to determine process name for path %s", szProcessPath);
				return FALSE;
			}

			// Skip '\\'
			szProcessName++;

			// Read the file and see if this is in it, if so, load the desired DLL
			LoadDllsSpecifiedInConfig(szConfigPath, szProcessName, szDllPath);
		}
		__except(1)
		{
			PrintError("Exception");
		}
	}
	__finally
	{
		if (szDllPath != NULL)
		{
			LocalFree(szDllPath);
		}
		if (szConfigPath != NULL)
		{
			LocalFree(szConfigPath);
		}
		if (szProcessPath != NULL)
		{
			LocalFree(szProcessPath);
		}
	}

	return TRUE;
}


/******************************************************************************
 * Read the given config file looking for lines with the given process name
 * and load the specified DLLs
 ******************************************************************************/
BOOL LoadDllsSpecifiedInConfig(PCHAR szConfigPath, PCHAR szProcessName, PCHAR szProjectPath)
{
	// TODO SHOULD use CreateFile and ReadFile

	FILE *fConfig = NULL;
	const DWORD dwLine = 255;
	CHAR szLine[dwLine];

	PCHAR szDllToLoadPath = NULL;
	DWORD dwDllToLoadPath = MAX_PATH;

	PCHAR nextToken;

	__try
	{
		fConfig = _fsopen(szConfigPath, "rt", _SH_DENYNO);
		if (fConfig == NULL)
		{
			PrintError("Could not open config file %s", szConfigPath);
			return FALSE;
		}

		while( !feof(fConfig) )
		{
			if (NULL != fgets(szLine, dwLine, fConfig))
			{
				if (szLine == NULL || strlen(szLine) == 0 || szLine[0] == '#')
				{
					// Ignore comment lines
					continue;
				}

				PCHAR szProcessNameToCheckFor = strtok_s(szLine, "\t", &nextToken);
				PCHAR szDllToLoadName = strtok_s(NULL, "\t\r\n", &nextToken);
				if (szProcessNameToCheckFor == NULL || szDllToLoadName == NULL)
				{
					PrintInfo("Line parsed to null values: %s", szLine);
					continue;
				}

				// Check if this line from the config file is describing this process
				if (_stricmp(szProcessName, szProcessNameToCheckFor) == 0)
				{
					// Alloc mem
					szDllToLoadPath = (PCHAR)LocalAlloc(LMEM_ZEROINIT, dwDllToLoadPath+1);
					if (szDllToLoadPath == NULL)
					{
						PrintError("Could not alloc mem");
						return FALSE;
					}
					int errno = _snprintf_s(szDllToLoadPath, dwDllToLoadPath, _TRUNCATE, "%s%s", szProjectPath, szDllToLoadName);
					if (errno == -1)
					{
						PrintError("Unable to set DLL path");
						return FALSE;
					}
					PrintInfo("Loading DLL: %s", szDllToLoadPath);

					// Load it!
					if (NULL == LoadLibrary(szDllToLoadPath))
					{
						PrintError("LoadLibrary failed for %s", szDllToLoadPath);
					}

					if (szDllToLoadPath != NULL)
					{
						LocalFree(szDllToLoadPath);
						szDllToLoadPath = NULL;
					}
				}
			}
		}
	}
	__finally
	{
		if (fConfig != NULL)
		{
			fclose(fConfig);
		}
		if (szDllToLoadPath != NULL)
		{
			LocalFree(szDllToLoadPath);
		}
	}

	return TRUE;
}
