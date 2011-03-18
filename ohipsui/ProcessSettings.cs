using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Win32;

namespace ohipsui
{
    class ProcessSettings
    {
        public string Name;
        public Boolean NullPrealloc;
        public Boolean GenericPrealloc;
        public int MaxMem;
        public int MinNopSledLength;

        private const string szRegSettingsRoot = "Software\\OpenHIPS\\HeapLocker\\Applications";
        private const string szNullPrealloc = "NullPagePreallocate";
        private const string szGenericPrealloc = "GenericPreAllocate";
        private const string szMaxMem = "PrivateUsageMax";
        private const string szMinNopSledLength = "NOPSledLengthMin";

        /// <summary>
        /// Read registry to find out which processes have configurations
        /// </summary>
        /// <returns></returns>
        public static string[] GetConfiguredProcesses()
        {
            string[] procs = null;

            RegistryKey key = null;
            try
            {
                key = Registry.LocalMachine.OpenSubKey(szRegSettingsRoot, false);
                if (key != null)
                {
                    procs = key.GetSubKeyNames();
                }
            }
            finally
            {
                if (key != null)
                {
                    key.Close();
                }
            }

            return procs;
        }

        public ProcessSettings(string name)
        {
            this.Name = name;

            RegistryKey settingsKey = null;
            RegistryKey procKey = null;
            try
            {
                settingsKey = Registry.LocalMachine.OpenSubKey(szRegSettingsRoot, false);
                if (settingsKey == null)
                {
                    return;
                }
                
                procKey = settingsKey.OpenSubKey(name);
                if (procKey == null)
                {
                    return;
                }

                //
                // Set values
                //

                // Set GenericPrealloc
                int iGenericPrealloc = (int)procKey.GetValue(szGenericPrealloc, 0);
                if (iGenericPrealloc == 1)
                {
                    GenericPrealloc = true;
                }

                // Set NullPrealloc
                int iNullPrealloc = (int)procKey.GetValue(szNullPrealloc, 0);
                if (iNullPrealloc == 1)
                {
                    NullPrealloc = true;
                }

                MinNopSledLength = (int)procKey.GetValue(szMinNopSledLength, 0);
                MaxMem = (int)procKey.GetValue(szMaxMem, 0);            
            }
            finally
            {
                if (settingsKey != null)
                {
                    settingsKey.Close();
                }
                if (procKey != null)
                {
                    procKey.Close();
                }
            }
        }
    }
}
