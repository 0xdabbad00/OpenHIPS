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
        private System.Windows.Forms.ContextMenuStrip TrayContextMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem SettingsToolStripMenuItem;


        public TrayIcon()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(TrayIcon));
            this.TrayNotifyIcon = new System.Windows.Forms.NotifyIcon(this.components);
            this.TrayContextMenuStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.TrayContextMenuStrip.SuspendLayout();
            this.SettingsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.SuspendLayout();

            // 
            // Set up tray icon
            //
            this.TrayNotifyIcon.Icon = ((System.Drawing.Icon)(resources.GetObject("taskbar.ico")));
            this.TrayNotifyIcon.Text = "OpenHIPS";
            this.TrayNotifyIcon.Visible = true;
            // Attach menu strip
            this.TrayNotifyIcon.ContextMenuStrip = this.TrayContextMenuStrip;
            this.TrayNotifyIcon.MouseClick += new System.Windows.Forms.MouseEventHandler(this.TrayNotifyIcon_MouseClick);
            this.TrayNotifyIcon.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.TrayNotifyIcon_MouseDoubleClick);

            // 
            // TrayContextMenuStrip
            // 
            this.TrayContextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.SettingsToolStripMenuItem
            });
            this.TrayContextMenuStrip.Name = "TrayContextMenuStrip";
            this.TrayContextMenuStrip.Size = new System.Drawing.Size(115, 70);

            // 
            // SettingsToolStripMenuItem
            // 
            this.SettingsToolStripMenuItem.Name = "SettingsToolStripMenuItem";
            this.SettingsToolStripMenuItem.Size = new System.Drawing.Size(32, 19);
            this.SettingsToolStripMenuItem.Text = "Settings...";
            this.SettingsToolStripMenuItem.Click += new System.EventHandler(this.SettingsToolStripMenuItem_Click);
            // TODO Set image for Settings

            // Create settings form
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(290, 180);
            this.ControlBox = false;
            // TODO this.Controls.Add(this.SoundBrowseButton);
            this.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "SettingsForm";
            this.Text = "OpenHIPS Settings";
            // TODO this.Load += new System.EventHandler(this.AlarmSettingsForm_Load);

            // Make sure nothing shows except the tray icon initially
            this.HideSettingsForm();
            this.TrayContextMenuStrip.ResumeLayout(false);
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

        private void HideSettingsForm()
        {
            this.WindowState = FormWindowState.Minimized;
            Visible = false; // Hide form window.
            ShowInTaskbar = false; // Remove from taskbar
            //base.Hide();
        }

        private void ShowSettingsForm()
        {
            this.WindowState = FormWindowState.Normal;
            Visible = true;
            ShowInTaskbar = true;
            base.Show();
        }
        
        /// <summary>
        /// Tray icon right click opens menu strip
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void TrayNotifyIcon_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                TrayContextMenuStrip.Show();
            }
        }

        /// <summary>
        ///  Tray icon double click opens settings window
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void TrayNotifyIcon_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            ShowSettingsForm();
        }

        private void SettingsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TrayContextMenuStrip.Hide();
            ShowSettingsForm();
        }
    }
}
