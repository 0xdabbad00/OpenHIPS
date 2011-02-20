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
 
#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <Windows.h>

#define ERROR_LEVEL 1
#define INFO_LEVEL 2

void PrintMessage(char *file, int lineno, int errorLvl, char *fmt, ...);

#ifdef _DEBUG
#	define PrintError(fmt, ...)		PrintMessage(__FILE__, __LINE__, ERROR_LEVEL, fmt, __VA_ARGS__)
#	define PrintInfo(fmt, ...)		PrintMessage(__FILE__, __LINE__, INFO_LEVEL , fmt, __VA_ARGS__)
#else
#	define PrintError(fmt, ...)		PrintMessage(__FILE__, __LINE__, ERROR_LEVEL, fmt, __VA_ARGS__)
#	define PrintInfo(...) ((void)0)
#endif

#endif