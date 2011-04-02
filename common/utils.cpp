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

#include "common.h"

/******************************************************************************
 * Get the path to a config file in the current working directory.
 *
 * @param szFileName
 *		Name of the file, without the path
 * @param hModule
 *		Module handle so we can get the path of the DLL
 * @param szPathBuffer
 *		Buffer to fill with the path, recommended this be of size MAX_PATH.
 *		Caller is responsible for both alloc'ing and free'ing this buffer.
 * @paran dwPathBuffer
 *		Size of the buffer
 * @return Length of the full path copied into the buffer once found, or 0 on
 *		any errors, including the buffer being too small.
 ******************************************************************************/
DWORD GetConfigFilePath(PCHAR szFileName, HMODULE hModule, PCHAR szPathBuffer, DWORD dwPathBuffer)
{
	const DWORD dwDllPath = MAX_PATH;
	PCHAR szDllPath = NULL;

	DWORD dwDllLength;
	PCHAR szDllName;
	int length;

	__try
	{
		//
		// Get this DLL path so we can find the config file (which will be in the same directory)
		//

		// Alloc mem
		szDllPath = (PCHAR)LocalAlloc(LMEM_ZEROINIT, dwDllPath+1);
		if (szDllPath == NULL)
		{
			PrintError("Could not alloc mem");
			return 0;
		}

		// Get process path
		dwDllLength = GetModuleFileName(hModule, szDllPath, dwDllPath);
		if (0 == dwDllLength
			|| dwDllLength == dwDllPath
			|| GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			PrintError("Unable to determine process path, retrieved %s", szDllPath);
			return 0;
		}

		szDllName = strrchr(szDllPath, '\\');
		if (szDllName == NULL)
		{
			PrintError("Unable to determine dll name for path %s", szDllPath);
			return 0;
		}

		// Set the char after the '\\' to zero, giving us the path without the filename and with
		// a null-terminator
		szDllName[1] = '\0';
		//PrintInfo("Path DLL is running from: %s", szDllPath);

		//
		// Get the config path
		//
		length = _snprintf_s(szPathBuffer, dwPathBuffer, _TRUNCATE, "%s%s", szDllPath, szFileName);
		if (length == -1)
		{
			PrintError("Unable to set config path");
			return 0;
		}

		return length;

	}
	__finally
	{
		if (szDllPath != NULL)
		{
			LocalFree(szDllPath);
		}
	}
}