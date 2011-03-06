using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace ohipsui
{
    public partial class TrayIcon : Form
    {
        private System.ComponentModel.IContainer components = null;
        private System.Windows.Forms.NotifyIcon AlarmNotifyIcon;
        

        public TrayIcon()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(TrayIcon));
            this.AlarmNotifyIcon = new System.Windows.Forms.NotifyIcon(this.components);

            this.SuspendLayout();

            this.AlarmNotifyIcon.BalloonTipIcon = System.Windows.Forms.ToolTipIcon.Info;
            this.AlarmNotifyIcon.BalloonTipText = "This is your requested alarm!";
            this.AlarmNotifyIcon.BalloonTipTitle = "Alarm";
            //this.AlarmNotifyIcon.ContextMenuStrip = this.TrayContextMenuStrip;
            this.AlarmNotifyIcon.Icon = ((System.Drawing.Icon)(resources.GetObject("taskbar.ico")));
            this.AlarmNotifyIcon.Text = "Alarm Clock";
            this.AlarmNotifyIcon.Visible = true;

            //this.AlarmNotifyIcon.BalloonTipClosed += new System.EventHandler(this.AlarmNotifyIcon_BalloonTipClicked);
            //this.AlarmNotifyIcon.BalloonTipClicked += new System.EventHandler(this.AlarmNotifyIcon_BalloonTipClicked);
            //this.AlarmNotifyIcon.MouseClick += new System.Windows.Forms.MouseEventHandler(this.AlarmNotifyIcon_MouseClick);
            //this.AlarmNotifyIcon.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.AlarmNotifyIcon_MouseDoubleClick);

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
