#include "common.h"
#include "protector.h"

///////////////////////////////////////////////////////////////////////////////
// Globals

///////////////////////////////////////////////////////////////////////////////
// Local prototypes


///////////////////////////////////////////////////////////////////////////////
// Functions

/******************************************************************************
 * Read application specific registry settings
 ******************************************************************************/
void ReadHeapLockerSettingsFromRegistryApplication(HKEY hKeyApplication)
{
	DWORD dwType;
	DWORD dwValue;
	DWORD dwValueSize;
	BYTE abValue[1025];
	int iIter;

	// Read "PrivateUsageMax" for the max amount of mem the app can use
	dwValueSize = sizeof(dwValue);
	if (IS_SUCCESS(RegQueryValueEx(hKeyApplication, REGISTRY_PRIVATE_USAGE_MAX, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
	{
		if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwPrivateUsageMax < ORIGIN_REGISTRY_APPLICATION)
		{
			sHeapLockerSettings.iOrigin_dwPrivateUsageMax = ORIGIN_REGISTRY_APPLICATION;
			sHeapLockerSettings.dwPrivateUsageMax = dwValue;
		}
	}

	// Read "NOPSledLengthMin"
	dwValueSize = sizeof(dwValue);
	if (IS_SUCCESS(RegQueryValueEx(hKeyApplication, REGISTRY_NOP_SLED_LENGTH_MAX, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
	{
		if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwNOPSledLengthMin < ORIGIN_REGISTRY_APPLICATION)
		{
			sHeapLockerSettings.iOrigin_dwNOPSledLengthMin = ORIGIN_REGISTRY_APPLICATION;
			sHeapLockerSettings.dwNOPSledLengthMin = dwValue;
		}
	}

	// Read "GenericPreAllocate"
	dwValueSize = sizeof(dwValue);
	if (IS_SUCCESS(RegQueryValueEx(hKeyApplication, REGISTRY_GENERIC_PRE_ALLOCATE, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
	{
		if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwGenericPreAllocate < ORIGIN_REGISTRY_APPLICATION)
		{
			sHeapLockerSettings.iOrigin_dwGenericPreAllocate = ORIGIN_REGISTRY_APPLICATION;
			sHeapLockerSettings.dwGenericPreAllocate = dwValue;
		}
	}

	// Read Verbose
	dwValueSize = sizeof(dwValue);
	if (IS_SUCCESS(RegQueryValueEx(hKeyApplication, REGISTRY_VERBOSE, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
	{
		if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwVerbose < ORIGIN_REGISTRY_APPLICATION)
		{
			sHeapLockerSettings.iOrigin_dwVerbose = ORIGIN_REGISTRY_APPLICATION;
			sHeapLockerSettings.dwVerbose = dwValue;
		}
	}

	dwValueSize = sizeof(dwValue);
	if (IS_SUCCESS(RegQueryValueEx(hKeyApplication, REGISTRY_SEARCH_MODE, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
	{
		if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_iSearchMode < ORIGIN_REGISTRY_APPLICATION)
		{
			sHeapLockerSettings.iOrigin_iSearchMode = ORIGIN_REGISTRY_APPLICATION;
			sHeapLockerSettings.iSearchMode = dwValue;
		}
	}

	dwValueSize = sizeof(abValue);
	if (IS_SUCCESS(RegQueryValueEx(hKeyApplication, REGISTRY_SEARCH_STRING, 0, &dwType, abValue, &dwValueSize)))
	{
		if (REG_BINARY == dwType && sHeapLockerSettings.iOrigin_iSearchLen < ORIGIN_REGISTRY_APPLICATION)
		{
			sHeapLockerSettings.iOrigin_iSearchLen = ORIGIN_REGISTRY_APPLICATION;
			sHeapLockerSettings.iSearchLen = dwValueSize;
			for (iIter = 0; iIter < sHeapLockerSettings.iSearchLen; iIter++)
				sHeapLockerSettings.abSearch[iIter] = abValue[iIter];
		}
	}

	dwValueSize = sizeof(dwValue);
	if (IS_SUCCESS(RegQueryValueEx(hKeyApplication, REGISTRY_NULL_PAGE_PRE_ALLOCATE, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
	{
		if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwPreallocatePage0 < ORIGIN_REGISTRY_APPLICATION)
		{
			sHeapLockerSettings.iOrigin_dwPreallocatePage0 = ORIGIN_REGISTRY_APPLICATION;
			sHeapLockerSettings.dwPreallocatePage0 = dwValue;
		}
	}

	dwValueSize = sizeof(dwValue);
	if (IS_SUCCESS(RegQueryValueEx(hKeyApplication, REGISTRY_FORCE_TERMINATION, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
	{
		if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwForceTermination < ORIGIN_REGISTRY_APPLICATION)
		{
			sHeapLockerSettings.iOrigin_dwForceTermination = ORIGIN_REGISTRY_APPLICATION;
			sHeapLockerSettings.dwForceTermination = dwValue;
		}
	}

	dwValueSize = sizeof(dwValue);
	if (IS_SUCCESS(RegQueryValueEx(hKeyApplication, REGISTRY_RESUME_MONITORING, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
	{
		if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwResumeMonitoring < ORIGIN_REGISTRY_APPLICATION)
		{
			sHeapLockerSettings.iOrigin_dwResumeMonitoring = ORIGIN_REGISTRY_APPLICATION;
			sHeapLockerSettings.dwResumeMonitoring = dwValue;
		}
	}
}


/******************************************************************************
 * Read generic HeapLocker settings from the registry
 ******************************************************************************/
void ReadHeapLockerSettingsFromRegistry(void)
{
	HKEY hKey;
	DWORD dwType;
	DWORD dwValue;
	DWORD dwValueSize;
	BYTE abValue[1025];
	int iIter;

	if (IS_SUCCESS(RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_PATH, 0L, KEY_READ, &hKey)))
	{
		dwValueSize = sizeof(dwValue);
		if (IS_SUCCESS(RegQueryValueEx(hKey, REGISTRY_PRIVATE_USAGE_MAX, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
		{
			if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwPrivateUsageMax < ORIGIN_REGISTRY_GENERIC)
			{
				sHeapLockerSettings.iOrigin_dwPrivateUsageMax = ORIGIN_REGISTRY_GENERIC;
				sHeapLockerSettings.dwPrivateUsageMax = dwValue;
			}
		}

		dwValueSize = sizeof(dwValue);
		if (IS_SUCCESS(RegQueryValueEx(hKey, REGISTRY_NOP_SLED_LENGTH_MAX, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
		{
			if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwNOPSledLengthMin < ORIGIN_REGISTRY_GENERIC)
			{
				sHeapLockerSettings.iOrigin_dwNOPSledLengthMin = ORIGIN_REGISTRY_GENERIC;
				sHeapLockerSettings.dwNOPSledLengthMin = dwValue;
			}
		}

		dwValueSize = sizeof(dwValue);
		if (IS_SUCCESS(RegQueryValueEx(hKey, REGISTRY_GENERIC_PRE_ALLOCATE, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
		{
			if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwGenericPreAllocate < ORIGIN_REGISTRY_GENERIC)
			{
				sHeapLockerSettings.iOrigin_dwGenericPreAllocate = ORIGIN_REGISTRY_GENERIC;
				sHeapLockerSettings.dwGenericPreAllocate = dwValue;
			}
		}

		dwValueSize = sizeof(dwValue);
		if (IS_SUCCESS(RegQueryValueEx(hKey, REGISTRY_VERBOSE, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
		{
			if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwVerbose < ORIGIN_REGISTRY_GENERIC)
			{
				sHeapLockerSettings.iOrigin_dwVerbose = ORIGIN_REGISTRY_GENERIC;
				sHeapLockerSettings.dwVerbose = dwValue;
			}
		}

		dwValueSize = sizeof(dwValue);
		if (IS_SUCCESS(RegQueryValueEx(hKey, REGISTRY_SEARCH_MODE, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
		{
			if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_iSearchMode < ORIGIN_REGISTRY_GENERIC)
			{
				sHeapLockerSettings.iOrigin_iSearchMode = ORIGIN_REGISTRY_GENERIC;
				sHeapLockerSettings.iSearchMode = dwValue;
			}
		}

		dwValueSize = sizeof(abValue);
		if (IS_SUCCESS(RegQueryValueEx(hKey, REGISTRY_SEARCH_STRING, 0, &dwType, abValue, &dwValueSize)))
		{
			if (REG_BINARY == dwType && sHeapLockerSettings.iOrigin_iSearchLen < ORIGIN_REGISTRY_GENERIC)
			{
				sHeapLockerSettings.iOrigin_iSearchLen = ORIGIN_REGISTRY_GENERIC;
				sHeapLockerSettings.iSearchLen = dwValueSize;
				for (iIter = 0; iIter < sHeapLockerSettings.iSearchLen; iIter++)
				{
					sHeapLockerSettings.abSearch[iIter] = abValue[iIter];
				}
			}
		}

		dwValueSize = sizeof(dwValue);
		if (IS_SUCCESS(RegQueryValueEx(hKey, REGISTRY_NULL_PAGE_PRE_ALLOCATE, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
		{
			if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwPreallocatePage0 < ORIGIN_REGISTRY_GENERIC)
			{
				sHeapLockerSettings.iOrigin_dwPreallocatePage0 = ORIGIN_REGISTRY_GENERIC;
				sHeapLockerSettings.dwPreallocatePage0 = dwValue;
			}
		}

		dwValueSize = sizeof(dwValue);
		if (IS_SUCCESS(RegQueryValueEx(hKey, REGISTRY_FORCE_TERMINATION, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
		{
			if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwForceTermination < ORIGIN_REGISTRY_GENERIC)
			{
				sHeapLockerSettings.iOrigin_dwForceTermination = ORIGIN_REGISTRY_GENERIC;
				sHeapLockerSettings.dwForceTermination = dwValue;
			}
		}

		dwValueSize = sizeof(dwValue);
		if (IS_SUCCESS(RegQueryValueEx(hKey, REGISTRY_RESUME_MONITORING, 0, &dwType, (LPBYTE) &dwValue, &dwValueSize)))
		{
			if (REG_DWORD == dwType && sHeapLockerSettings.iOrigin_dwResumeMonitoring < ORIGIN_REGISTRY_GENERIC)
			{
				sHeapLockerSettings.iOrigin_dwResumeMonitoring = ORIGIN_REGISTRY_GENERIC;
				sHeapLockerSettings.dwResumeMonitoring = dwValue;
			}
		}

		RegCloseKey(hKey);
	}
}

/******************************************************************************
 * Set default HeapLocker settings
 ******************************************************************************/
void SetHeapLockerSettingsDefaults(void)
{
	if (sHeapLockerSettings.iOrigin_dwPrivateUsageMax < ORIGIN_DEFAULT)
	{
		sHeapLockerSettings.iOrigin_dwPrivateUsageMax = ORIGIN_DEFAULT;
		sHeapLockerSettings.dwPrivateUsageMax = 0xFFFFFFFF;
	}

	if (sHeapLockerSettings.iOrigin_dwNOPSledLengthMin < ORIGIN_DEFAULT)
	{
		sHeapLockerSettings.iOrigin_dwNOPSledLengthMin = ORIGIN_DEFAULT;
		sHeapLockerSettings.dwNOPSledLengthMin = 0;
	}

	if (sHeapLockerSettings.iOrigin_dwGenericPreAllocate < ORIGIN_DEFAULT)
	{
		sHeapLockerSettings.iOrigin_dwGenericPreAllocate = ORIGIN_DEFAULT;
		sHeapLockerSettings.dwGenericPreAllocate = 0xFFFFFFFF;
	}

	if (sHeapLockerSettings.iOrigin_dwVerbose < ORIGIN_DEFAULT)
	{
		sHeapLockerSettings.iOrigin_dwVerbose = ORIGIN_DEFAULT;
		sHeapLockerSettings.dwVerbose = 0;
	}

	if (sHeapLockerSettings.iOrigin_iSearchMode < ORIGIN_DEFAULT)
	{
		sHeapLockerSettings.iOrigin_iSearchMode = ORIGIN_DEFAULT;
		sHeapLockerSettings.iSearchMode = 0;
	}

	if (sHeapLockerSettings.iOrigin_iSearchLen < ORIGIN_DEFAULT)
	{
		sHeapLockerSettings.iOrigin_iSearchLen = ORIGIN_DEFAULT;
		sHeapLockerSettings.iSearchLen = 0;
	}

	if (sHeapLockerSettings.iOrigin_dwPreallocatePage0 < ORIGIN_DEFAULT)
	{
		sHeapLockerSettings.iOrigin_dwPreallocatePage0 = ORIGIN_DEFAULT;
		sHeapLockerSettings.dwPreallocatePage0 = 0;
	}

	if (sHeapLockerSettings.iOrigin_dwForceTermination < ORIGIN_DEFAULT)
	{
		sHeapLockerSettings.iOrigin_dwForceTermination = ORIGIN_DEFAULT;
		sHeapLockerSettings.dwForceTermination = 0;
	}

	if (sHeapLockerSettings.iOrigin_dwResumeMonitoring < ORIGIN_DEFAULT)
	{
		sHeapLockerSettings.iOrigin_dwResumeMonitoring = ORIGIN_DEFAULT;
		sHeapLockerSettings.dwResumeMonitoring = 0;
	}
}


/******************************************************************************
 * Open application registry key
 ******************************************************************************/
HKEY GetApplicationRegKey(void)
{
	LPTSTR pszEXE;
	HKEY hKey;
	HKEY hKeyApplication;
	DWORD dwIndex;
	TCHAR szKeyName[MAX_REGISTRY_KEY_NAME];
	DWORD dwKeyNameSize;

	__try
	{
		pszEXE = GetExecutableName();
		if (NULL == pszEXE)
			return NULL;
		PrintInfo(_TEXT("Application name: %s"), pszEXE);
		if (IS_SUCCESS(RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_PATH_APPLICATIONS, 0L, KEY_READ, &hKey)))
		{
			// TODO SHOULD instead of enumerating the subkeys, just open the key, or return failure if it does not exist
			dwIndex = 0;
			dwKeyNameSize = countof(szKeyName);
			while (ERROR_NO_MORE_ITEMS != RegEnumKeyEx(hKey, dwIndex++, szKeyName, &dwKeyNameSize, NULL, NULL, NULL, NULL))
			{
				if (!_tcsicmp(pszEXE, szKeyName))
				{
					if (IS_SUCCESS(RegOpenKeyEx(hKey, szKeyName, 0L, KEY_READ, &hKeyApplication)))
					{
						PrintInfo(_TEXT("Found registry settings for application %s"), szKeyName);
						RegCloseKey(hKey);
						return hKeyApplication;
					}
				}
				dwKeyNameSize = countof(szKeyName);
			}
			RegCloseKey(hKey);
		}
	}
	__except(1)
	{
		PrintError("Exception");
		return NULL;
	}

	return NULL;
}
