using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Diagnostics;


namespace ohipssvc
{
    public class OhipsMonitor
    {
        public OhipsMonitor() { }

        // This method that will be called when the thread is started
        public void OhipsMonitorThread()
        {
            while (true)
            {
                if (WindowsService.running)
                {
                    Debug.WriteLine("Running in thread");
                }
                Thread.Sleep(5 * 1000);
            }
        }
    };

}
