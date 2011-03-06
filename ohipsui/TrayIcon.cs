using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace ohipsui
{
    public partial class TrayIcon : Form
    {
        private System.ComponentModel.IContainer components = null;
        private System.Windows.Forms.NotifyIcon TrayNotifyIcon;
        

        public TrayIcon()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(TrayIcon));
            this.TrayNotifyIcon = new System.Windows.Forms.NotifyIcon(this.components);

            this.SuspendLayout();

            //this.AlarmNotifyIcon.ContextMenuStrip = this.TrayContextMenuStrip;
            this.TrayNotifyIcon.Icon = ((System.Drawing.Icon)(resources.GetObject("taskbar.ico")));
            this.TrayNotifyIcon.Text = "OpenHIPS";
            this.TrayNotifyIcon.Visible = true;

            this.WindowState = FormWindowState.Minimized;
            Visible       = false; // Hide form window.
            ShowInTaskbar = false; // Remove from taskbar
            this.ResumeLayout(false);
            this.PerformLayout();
        }

        protected override void Dispose(bool disposing)
        {
            // Clean up any components being used.
            if (disposing && components != null)
            {
                components.Dispose();
            }

            base.Dispose(disposing);
        }

    }
}
