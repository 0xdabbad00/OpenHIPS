using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Diagnostics;
using System.Reflection;
using System.IO;
using System.Runtime.InteropServices;

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
                    Debug.WriteLine("Running in thread");
                }
                Thread.Sleep(5 * 1000);
            }
        }

        public void EnsureAppInitSet()
        {
            // The purpose of this function is just to set our reg value and ensure other apps
            // don't accidentally erase our value if they are poorly written and just set appinit instead
            // of appending to it
            Debug.WriteLine("Ensuring app init is set");
            Microsoft.Win32.RegistryKey key = null;
            try
            {
                key = Microsoft.Win32.Registry.LocalMachine.OpenSubKey(szAppInitKey, true);
                String value = key.GetValue(szAppInitValue).ToString();
                Debug.WriteLine(String.Format("Value:{0}", value));

                if (value.ToLower().Contains(GetDllPath().ToLower()))
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
                value += GetDllPath();
                Debug.WriteLine(String.Format("New value:{0}", value));

                // Set registry value
                key.SetValue(szAppInitValue, value);
            }
            finally
            {
                if (key != null)
                {
                    key.Close();
                }
            }

        }

        public static String GetDllPath()
        {
            // Get the full path to this file
            String filepath = Assembly.GetExecutingAssembly().Location;
            // Get the name of the dll
            String dllpath = Path.GetDirectoryName(filepath) + "\\" + szDllNamePrefix + "32" + ".dll";

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
                String dllPath = GetDllPath();
                key = Microsoft.Win32.Registry.LocalMachine.OpenSubKey(szAppInitKey, true);
                String value = key.GetValue(szAppInitValue).ToString();
                Debug.WriteLine(String.Format("Read value:{0}", value));
                if (value.Length == 0 || !value.ToLower().Contains(dllPath.ToLower()))
                {
                    // Dll path isn't there
                    return;
                }

                Debug.WriteLine(String.Format("Finding start"));

                int dllPathStart = value.ToLower().IndexOf(dllPath.ToLower());
                int dllPathEnd = dllPathStart + dllPath.Length+1;
                if (dllPathEnd > value.Length)
                {
                    dllPathEnd = value.Length;
                }
                Debug.WriteLine(String.Format("Indexes set to {0} and {1}", dllPathStart, dllPathEnd));

                // Remove separator
                if (dllPathStart - 1 >= 0 && isSeparator(value[dllPathStart-1]))
                {
                    dllPathStart--;
                }

                if (dllPathEnd + 1 < value.Length && isSeparator(value[dllPathEnd+1]))
                {
                    dllPathEnd++;
                }
                Debug.WriteLine(String.Format("Corrected indexes set to {0} and {1}", dllPathStart, dllPathEnd));
            
                String prefix = value.Substring(0, dllPathStart);
                String suffix = value.Substring(dllPathEnd);

                Debug.WriteLine(String.Format("Have strings \"{0}\" and \"{1}\"", prefix, suffix));
                if (prefix.Length != 0 && suffix.Length != 0)
                {
                    // Add separator
                    prefix += " ";
                }

                String newValue = prefix + suffix;
                Debug.WriteLine(String.Format("New value:{0}", newValue));
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
