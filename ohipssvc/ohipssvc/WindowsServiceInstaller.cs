using System;
using System.ComponentModel;
using System.Configuration.Install;
using System.ServiceProcess;

namespace ohipssvc
{
    [RunInstaller(true)]
    public class WindowsServiceInstaller : Installer
    {
        /// <summary>
        /// Public Constructor for WindowsServiceInstaller.
        /// - Put all of your Initialization code here.
        /// </summary>

        public WindowsServiceInstaller()
        {
            ServiceProcessInstaller serviceProcessInstaller =
                               new ServiceProcessInstaller();
            ServiceInstaller serviceInstaller = new ServiceInstaller();

            //# Service Account Information
            serviceProcessInstaller.Account = ServiceAccount.LocalSystem;
            serviceProcessInstaller.Username = null;
            serviceProcessInstaller.Password = null;

            //# Service Information

            serviceInstaller.DisplayName = "OpenHIPS";
            serviceInstaller.Description = "Protects against intrusions";
            serviceInstaller.StartType = ServiceStartMode.Automatic;

            //# This must be identical to the WindowsService.ServiceBase name

            //# set in the constructor of WindowsService.cs

            serviceInstaller.ServiceName = "OpenHIPS";

            this.Installers.Add(serviceProcessInstaller);
            this.Installers.Add(serviceInstaller);

            this.AfterInstall += new InstallEventHandler(AfterInstallEventHandler);
        }

        private void AfterInstallEventHandler(object sender, InstallEventArgs e)
        {
            System.ServiceProcess.ServiceController myController =
                new System.ServiceProcess.ServiceController("OpenHIPS");
            myController.Start();
        }

    }
}