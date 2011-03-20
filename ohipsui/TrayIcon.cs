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
        private string szVersion = "0.0.0.1";

        private System.ComponentModel.IContainer components = null;
        private System.Windows.Forms.NotifyIcon TrayNotifyIcon;
        private System.Windows.Forms.ContextMenuStrip TrayContextMenuStrip;
        private System.Windows.Forms.Button BtnClose;
        private System.Windows.Forms.Button BtnSaveClose;
        private System.Windows.Forms.Label LblProcessSelection;
        private System.Windows.Forms.ListBox ListBoxProcessSelector;
        private System.Windows.Forms.Label LblMemMax;
        private System.Windows.Forms.TextBox TextBoxMemMax;
        private System.Windows.Forms.Label LblNopSledMin;
        private System.Windows.Forms.TextBox TextBoxNopSledMin;
        private System.Windows.Forms.CheckBox ChkBoxNullPrealloc;
        private System.Windows.Forms.CheckBox ChkBoxGenericPrealloc;
        private System.Windows.Forms.ToolStripMenuItem SettingsToolStripMenuItem;


        public TrayIcon()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(TrayIcon));
            this.TrayNotifyIcon = new System.Windows.Forms.NotifyIcon(this.components);
            
            // Context menu strip
            this.TrayContextMenuStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.SettingsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();

            this.TrayContextMenuStrip.SuspendLayout();
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
                this.SettingsToolStripMenuItem,
            });
            this.TrayContextMenuStrip.Name = "TrayContextMenuStrip";
            this.TrayContextMenuStrip.Size = new System.Drawing.Size(115, 70);

            // 
            // SettingsToolStripMenuItem
            // 
            this.SettingsToolStripMenuItem.Name = "SettingsToolStripMenuItem";
            this.SettingsToolStripMenuItem.Text = "Settings...";
            this.SettingsToolStripMenuItem.Click += new System.EventHandler(this.SettingsToolStripMenuItem_Click);
            // TODO Set image for Settings

            InitializeComponent();

            // Make sure nothing shows except the tray icon initially
            this.TrayContextMenuStrip.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();
            this.HideSettingsForm();
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
            this.ShowInTaskbar = false; // Remove from taskbar
            base.Hide();
        }

        private void ShowSettingsForm()
        {
            SetFormValues();
            this.WindowState = FormWindowState.Normal;
            this.ShowInTaskbar = true;
            this.Show();
            this.ResumeLayout(true);
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

        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(TrayIcon));
            this.SuspendLayout();

            int formWidth = 500;
            int formHeight = 300;
            int margin = 15;

            // 
            // BtnClose
            //
            this.BtnClose = new System.Windows.Forms.Button();
            this.BtnClose.Text = "Close";
            this.BtnClose.Width = 150;
            this.BtnClose.Location = new System.Drawing.Point(formWidth - this.BtnClose.Size.Width - margin, formHeight - this.BtnClose.Size.Height * 2 - margin);
            this.BtnClose.TabIndex = 0;
            this.BtnClose.Click += new System.EventHandler(this.CloseBtn_Click);

            // 
            // BtnSaveClose
            //
            this.BtnSaveClose = new System.Windows.Forms.Button();
            this.BtnSaveClose.Text = "Save";
            this.BtnSaveClose.Width = 150;
            this.BtnSaveClose.Location = new System.Drawing.Point(this.BtnClose.Location.X, this.BtnClose.Location.Y - this.BtnSaveClose.Height);
            this.BtnSaveClose.TabIndex = 0;
            this.BtnSaveClose.Click += new System.EventHandler(this.SaveCloseBtn_Click);

            //
            // Process selector
            //
            this.LblProcessSelection = new System.Windows.Forms.Label();
            this.LblProcessSelection.Text = "Process name";
            this.LblProcessSelection.Location = new System.Drawing.Point(margin, margin);
            this.LblProcessSelection.TextAlign = ContentAlignment.BottomLeft;
            
            this.ListBoxProcessSelector = new System.Windows.Forms.ListBox();
            this.ListBoxProcessSelector.Text = "Process names";
            this.ListBoxProcessSelector.Location = new System.Drawing.Point(margin, this.LblProcessSelection.Location.Y + this.LblProcessSelection.Size.Height);
            this.ListBoxProcessSelector.ScrollAlwaysVisible = true;
            this.ListBoxProcessSelector.Height = 200;
            this.ListBoxProcessSelector.SelectedIndexChanged += new System.EventHandler(this.ProcessSelector_SelectedIndexChanged); 
            
            //
            // Mem max
            //
            int col2 = this.ListBoxProcessSelector.Location.X + this.ListBoxProcessSelector.Width + margin;
            this.LblMemMax = new System.Windows.Forms.Label();
            this.LblMemMax.Text = "Memory max (MB)";
            this.LblMemMax.Location = new System.Drawing.Point(col2, margin);
            this.LblMemMax.TextAlign = ContentAlignment.BottomLeft;

            this.TextBoxMemMax = new System.Windows.Forms.TextBox();
            this.TextBoxMemMax.Location = new System.Drawing.Point(col2, LblMemMax.Location.Y + LblMemMax.Height);
            this.TextBoxMemMax.Enabled = false;

            // 
            // Nop sled min
            //
            this.LblNopSledMin = new System.Windows.Forms.Label();
            this.LblNopSledMin.Text = "Nop sled minimum length";
            this.LblNopSledMin.Location = new System.Drawing.Point(col2, TextBoxMemMax.Location.Y + TextBoxMemMax.Height);
            this.LblNopSledMin.TextAlign = ContentAlignment.BottomLeft;
            this.LblNopSledMin.Width = 200;

            this.TextBoxNopSledMin = new System.Windows.Forms.TextBox();
            this.TextBoxNopSledMin.Location = new System.Drawing.Point(col2, LblNopSledMin.Location.Y + LblNopSledMin.Height);
            this.TextBoxNopSledMin.Enabled = false;

            // 
            // Null pre-alloc
            //
            this.ChkBoxNullPrealloc = new System.Windows.Forms.CheckBox();
            this.ChkBoxNullPrealloc.Location = new System.Drawing.Point(col2, TextBoxNopSledMin.Location.Y + TextBoxNopSledMin.Height);
            this.ChkBoxNullPrealloc.Text = "Pre-alloc null address";
            this.ChkBoxNullPrealloc.Width = 200;
            this.ChkBoxNullPrealloc.Enabled = false;

            // 
            // Generic pre-alloc
            //
            this.ChkBoxGenericPrealloc = new System.Windows.Forms.CheckBox();
            this.ChkBoxGenericPrealloc.Location = new System.Drawing.Point(col2, ChkBoxNullPrealloc.Location.Y + ChkBoxNullPrealloc.Height);
            this.ChkBoxGenericPrealloc.Text = "Pre-alloc generic addresses";
            this.ChkBoxGenericPrealloc.Width = 200;
            this.ChkBoxGenericPrealloc.Enabled = false;

            // 
            // TrayIcon Settings Form
            // 
            this.ClientSize = new System.Drawing.Size(formWidth, formHeight);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("icon.ico")));
            this.ControlBox = false;
            this.Controls.Add(this.BtnClose);
            //TODO this.Controls.Add(this.BtnSaveClose);
            this.Controls.Add(this.LblProcessSelection);
            this.Controls.Add(this.ListBoxProcessSelector);
            this.Controls.Add(this.LblMemMax);
            this.Controls.Add(this.TextBoxMemMax);
            this.Controls.Add(this.LblNopSledMin);
            this.Controls.Add(this.TextBoxNopSledMin);
            this.Controls.Add(this.ChkBoxNullPrealloc);
            this.Controls.Add(this.ChkBoxGenericPrealloc);

            this.MinimumSize = new System.Drawing.Size(formWidth, formHeight);
            this.MaximumSize = new System.Drawing.Size(formWidth, formHeight);
            this.Name = "TrayIcon";
            this.Text = "OpenHIPS v"+szVersion+" Settings";
            this.WindowState = System.Windows.Forms.FormWindowState.Minimized;
            this.ResumeLayout(false);

        }

        private void CloseBtn_Click(object sender, EventArgs e)
        {
            //Application.Exit();
            HideSettingsForm();
        }

        private void SaveCloseBtn_Click(object sender, EventArgs e)
        {
            // TODO Save
        }

        private void ProcessSelector_SelectedIndexChanged(object sender, EventArgs e)
        {
            SetProcessValues();
        }

        private void SetFormValues()
        {
            // Find all the processes that have configurations
            string[] procs = ProcessSettings.GetConfiguredProcesses();
            if (procs == null)
            {
                return;
            }

            // Clear the current list
            ListBoxProcessSelector.Items.Clear();

            // Add the procs to the list
            ListBoxProcessSelector.BeginUpdate();
            // Add items to the ListBox
            foreach (string proc in procs)
            {
                ListBoxProcessSelector.Items.Add(proc);
            }
            // Allow the ListBox to repaint and display the new items.
            ListBoxProcessSelector.EndUpdate();
            // Select the first process
            ListBoxProcessSelector.SetSelected(0, true);

            // Set the other values based on the process selected
            SetProcessValues();
        }

        private void SetProcessValues()
        {
            String proc = ListBoxProcessSelector.SelectedItem.ToString();
            ProcessSettings currentSelection = new ProcessSettings(proc);
            if (currentSelection == null)
            {
                // TODO
                return;
            }

            this.TextBoxMemMax.Text = currentSelection.MaxMem.ToString();
            this.TextBoxNopSledMin.Text = currentSelection.MinNopSledLength.ToString();
            this.ChkBoxNullPrealloc.Checked = currentSelection.NullPrealloc;
            this.ChkBoxGenericPrealloc.Checked = currentSelection.GenericPrealloc;
        }
    }
}
