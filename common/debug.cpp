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

#include "debug.h"
#include <stdarg.h>
#include <varargs.h>
#include <stdio.h>
#include <tchar.h>

void PrintMessage(char *file, int lineno, int errorLvl, char *fmt, ...)
{
	char str[256];
	char output[256];

	LPVOID lpMsgBuf = NULL;
	DWORD dwError = GetLastError();

	va_list args;
    va_start(args, fmt);
	vsprintf_s(str, sizeof(str), fmt, args);
	va_end(args);
	// TODO MUST, On error, need to call GetLastError and print error message
	_sntprintf_s(output, sizeof(output), _TRUNCATE, _TEXT("%s[%s:%d] %s"), 
		(errorLvl == ERROR_LEVEL) ? _TEXT("ERROR ") : _TEXT(""), 
		file, lineno, str);

#ifdef _DEBUG
	OutputDebugString(output);
#endif

	if (errorLvl == ERROR_LEVEL)
	{
		// TODO MUST print the error to a log file
		//printf("%s\n", output);

		// Decode error code
		FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

		_sntprintf_s(output, sizeof(output), _TRUNCATE, _TEXT("Last Error (%d): %s"), dwError, lpMsgBuf);
#ifdef _DEBUG
		OutputDebugString(output);
#endif
		// TODO MUST print the error to a log file
		// printf("%s\n", output);
	}

	// Clean-up
	if (lpMsgBuf != NULL)
	{
		LocalFree(lpMsgBuf);
	}
}