using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Diagnostics;
using System.Reflection;
using System.IO;
using System.Runtime.InteropServices;
using Microsoft.Win32;

// TODO MUST Set both 32 and 64 reg key, need to handle 64-bit redirection, as we are only writing to
//  HKLM\SOFTWARE\wow6432node\...

namespace ohipssvc
{
    public class OhipsMonitor
    {
        private static String szAppInitKey = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows";
        private static String szAppInitValue = "AppInit_DLLs";
        private static String szDllNamePrefix = "ohipsfs";
        public OhipsMonitor() { }

        // Need to call GetShortPathName for setting AppInit value
        [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
        public static extern int GetShortPathName(
                 [MarshalAs(UnmanagedType.LPTStr)]
                   string path,
                 [MarshalAs(UnmanagedType.LPTStr)]
                   StringBuilder shortPath,
                 int shortPathLength
                 );

        // This method that will be called when the thread is started
        public void OhipsMonitorThread()
        {
            while (true)
            {
                if (WindowsService.running)
                {
                    EnsureAppInitSet();
                }
                // Sleep a minute
                Thread.Sleep(60 * 1000);
            }
        }

        public void EnsureAppInitSet()
        {
            // The purpose of this function is just to set our reg value and ensure other apps
            // don't accidentally erase our value if they just set appinit instead
            // of appending to it
            Debug.WriteLine("OHIPS: Ensuring app init is set");

            RegistryKey key = null;
            RegistryKey localMachineX64View = null;
            try
            {
                // Set 32-bit AppInit_Dll value
                key = Registry.LocalMachine.OpenSubKey(szAppInitKey, true);
                EnsureAppInitSet(key, szDllNamePrefix + "32" + ".dll");
                key.Close();
                key = null;

                // Set 64-bit AppInit_Dll value
                // TODO MUST Uncomment so we'll install on 64-bit builds
                /*
                localMachineX64View = RegistryKey.OpenBaseKey(
                    RegistryHive.LocalMachine, 
                    RegistryView.Registry64);
                key = localMachineX64View.OpenSubKey(szAppInitKey, true);
                EnsureAppInitSet(key, szDllNamePrefix + "64" + ".dll");
                key.Close();
                key = null;
                 */
            }
            finally
            {
                if (key != null)
                {
                    key.Close();
                }
                if (localMachineX64View != null)
                {
                    localMachineX64View.Close();
                }
            }
        }

        public void EnsureAppInitSet(Microsoft.Win32.RegistryKey key, String szDllName)
        {
            key = Microsoft.Win32.Registry.LocalMachine.OpenSubKey(szAppInitKey, true);

            // Set the relevant values
            // TODO MAYBE record what these were set to and set them back if we uninstall?
            String LoadAppInit_DLLs = "LoadAppInit_DLLs";
            if (key.GetValue(LoadAppInit_DLLs, null) != null)
            {
                key.SetValue(LoadAppInit_DLLs, 1);
            }

            String RequireSignedAppInit_DLLs = "RequireSignedAppInit_DLLs";
            if (key.GetValue(RequireSignedAppInit_DLLs, null) != null)
            {
                key.SetValue(RequireSignedAppInit_DLLs, 1);
            }

            // Set the AppInit_Dlls value to include our DLL
            String value = key.GetValue(szAppInitValue).ToString();

            if (value.ToLower().Contains(GetDllPath(szDllName).ToLower()))
            {
                // Value already contains our DLL, so return
                return;
            }

            // Sanity check so I don't accidentally repeatedly add to this buffer
            if (value.Length > 1024)
            {
                // TODO Record an error
                return;
            }

            // Append a space if something is already there
            if (value.Length != 0)
            {
                value += " ";
            }

            // Append our DLL path
            value += GetDllPath(szDllName);

            // Set registry value
            key.SetValue(szAppInitValue, value);
        }

        public static String GetDllPath(String szDllName)
        {
            // Get the full path to this file
            String filepath = Assembly.GetExecutingAssembly().Location;
            // Get the name of the dll
            String dllpath = Path.GetDirectoryName(filepath) + "\\" + szDllName;

            // Get the short path
            StringBuilder shortPath = new StringBuilder(255);
            if (0 == GetShortPathName(dllpath, shortPath, shortPath.Capacity))
            {
                // TODO throw error
            }
            return shortPath.ToString();
        }


        public static void Uninstall()
        {
            Debug.WriteLine("Uninstalling");
            Microsoft.Win32.RegistryKey key = null;
            try
            {
                // TODO MUST uninstall from both 32-bit and 64-bit versions
                // TODO MUST use a regex instead of this junk
                String dllPath = GetDllPath(szDllNamePrefix + "32" + ".dll");
                key = Microsoft.Win32.Registry.LocalMachine.OpenSubKey(szAppInitKey, true);
                String value = key.GetValue(szAppInitValue).ToString();
                if (value.Length == 0 || !value.ToLower().Contains(dllPath.ToLower()))
                {
                    // Dll path isn't there
                    return;
                }

                int dllPathStart = value.ToLower().IndexOf(dllPath.ToLower());
                int dllPathEnd = dllPathStart + dllPath.Length+1;
                if (dllPathEnd > value.Length)
                {
                    dllPathEnd = value.Length;
                }
                
                // Remove separator
                if (dllPathStart - 1 >= 0 && isSeparator(value[dllPathStart-1]))
                {
                    dllPathStart--;
                }

                if (dllPathEnd + 1 < value.Length && isSeparator(value[dllPathEnd+1]))
                {
                    dllPathEnd++;
                }
                
                String prefix = value.Substring(0, dllPathStart);
                String suffix = value.Substring(dllPathEnd);

                if (prefix.Length != 0 && suffix.Length != 0)
                {
                    // Add separator
                    prefix += " ";
                }

                String newValue = prefix + suffix;
                
                // Set registry value
                key.SetValue(szAppInitValue, newValue);
            }
            finally
            {
                if (key != null)
                {
                    key.Close();
                }
            }
        }

        private static bool isSeparator(char c)
        {
            if (c == ' ' || c == ',')
            {
                return true;
            }
            return false;
        }
    };

}
