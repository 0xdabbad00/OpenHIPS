using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using System.ServiceProcess;
using System.Threading;
using System.IO;
using Microsoft.Win32;
using System.Reflection;

namespace ohipssvc
{
    [System.ComponentModel.DesignerCategory("")]
    class WindowsService : ServiceBase
    {
        public static Boolean running = true;
        public static OhipsMonitor ohipsMonitor = null;

        public WindowsService()
        {
            Debug.WriteLine("Running WindowsService class");

            this.ServiceName = "OpenHIPS";
            this.EventLog.Log = "Application";

            // These Flags set whether or not to handle that specific
            //  type of event. Set to true if you need it, false otherwise.
            this.CanHandlePowerEvent = false;
            this.CanHandleSessionChangeEvent = true;
            this.CanPauseAndContinue = true;
            this.CanShutdown = true;
            this.CanStop = true;
        }

        static void Main()
        {
            Debug.WriteLine("Running WindowsService.Main");
            Debug.WriteLine(Directory.GetCurrentDirectory());

            RunUI();

            if (ohipsMonitor == null)
            {
                // Start our monitor thread
                ohipsMonitor = new OhipsMonitor();
                Thread oThread = new Thread(new ThreadStart(ohipsMonitor.OhipsMonitorThread));

                // Start the thread
                oThread.Start();

                // Spin for a while waiting for the started thread to become
                // alive:
                while (!oThread.IsAlive) ;
            }

            // Start service
            ServiceBase.Run(new WindowsService());
        }

        protected override void OnSessionChange(
                  SessionChangeDescription changeDescription)
        {
            Debug.WriteLine("On session change event"); // TODO REMOVE
            switch (changeDescription.Reason)
            {
                case SessionChangeReason.SessionLogon:
                    RunUI();
                    break;
                default:
                    break;
            }


            base.OnSessionChange(changeDescription);
        }


        /// <summary>
        /// Runs the UI (User Interface)
        /// </summary>
        private static void RunUI()
        {
            // TODO Need to have this run in the context of each user's desktop
            /*
            String filepath = Assembly.GetExecutingAssembly().Location;
            // Get the name of the dll
            String uipath = Path.GetDirectoryName(filepath) + "\\" + szUiProcess;

            if (uiProcess == null || uiProcess.HasExited)
            {
                uiProcess = new Process();
                uiProcess.StartInfo.FileName = uipath;
                uiProcess.Start();
            }
            */
        }

        /// <summary>
        /// Dispose of objects that need it here.
        /// </summary>
        /// <param name="disposing">Whether
        ///    or not disposing is going on.</param>
        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
        }

        /// <summary>
        /// OnStop(): Put your stop code here
        /// - Stop threads, set final data, etc.
        /// </summary>
        protected override void OnStop()
        {
            System.Diagnostics.Debug.WriteLine("OpenHIPS Stop");
            running = false;
            OhipsMonitor.Uninstall();
            KillUI();
            base.OnStop();
        }

        private void KillUI()
        {
            Process[] Processes = Process.GetProcesses();
            foreach (Process p in Processes)
            {
                // TODO Not exactly the cleanest way of doing this
                if (p.ProcessName.ToLower().Equals("ohipsui"))
                {
                    p.Kill();
                }
            }
            
        }

        /// <summary>
        /// OnPause: Put your pause code here
        /// - Pause working threads, etc.
        /// </summary>
        protected override void OnPause()
        {
            System.Diagnostics.Debug.WriteLine("OpenHIPS Pause");
            running = false;
            base.OnPause();
        }

        /// <summary>
        /// OnContinue(): Put your continue code here
        /// - Un-pause working threads, etc.
        /// </summary>
        protected override void OnContinue()
        {
            System.Diagnostics.Debug.WriteLine("OpenHIPS Continue");
            running = true;
            base.OnContinue();
        }

        /// <summary>
        /// OnShutdown(): Called when the System is shutting down
        /// - Put code here when you need special handling
        ///   of code that deals with a system shutdown, such
        ///   as saving special data before shutdown.
        /// </summary>
        protected override void OnShutdown()
        {
            System.Diagnostics.Debug.WriteLine("OpenHIPS Shutdown");
            running = false;
            base.OnShutdown();
        }

        /// <summary>
        /// OnCustomCommand(): If you need to send a command to your
        ///   service without the need for Remoting or Sockets, use
        ///   this method to do custom methods.
        /// </summary>
        /// <param name="command">Arbitrary Integer between 128 & 256</param>
        protected override void OnCustomCommand(int command)
        {
            //  A custom command can be sent to a service by using this method:
            //#  int command = 128; //Some Arbitrary number between 128 & 256
            //#  ServiceController sc = new ServiceController("NameOfService");
            //#  sc.ExecuteCommand(command);
            System.Diagnostics.Debug.WriteLine("OpenHIPS Custom Command");

            base.OnCustomCommand(command);
        }

    }
}
