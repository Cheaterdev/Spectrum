// Tools > Regenerate Prism code (also on the Prism toolbar). Compiled into
// PrismLanguageClient.dll by gen_vs_extension.py, which also writes this
// package's registration into prism.pkgdef -- there is no RegPkg step.
using System;
using System.Collections.Generic;
using System.ComponentModel.Design;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using EnvDTE;
using EnvDTE80;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Task = System.Threading.Tasks.Task;

namespace Spectrum.Prism
{
    [Guid(PackageGuid)]
    public sealed class PrismPackage : AsyncPackage
    {
        // Must match guidPrismPackage / guidPrismCmdSet in PrismCommands.vsct.
        public const string PackageGuid = "40863436-2fa7-4ea3-9809-55aa6ce8b482";
        static readonly Guid CommandSet = new Guid("c5413801-5494-47f5-8ba9-1fc5983af154");
        const int RegenerateId = 0x0100;

        static readonly Guid PaneGuid = new Guid("3924caf2-9eb2-40b4-80c0-c9e50ee5d2f3");

        // Generator output folders, relative to the repo root: what the summary diffs.
        static readonly string[] OutputDirs =
        {
            @"sources\HAL\autogen", @"sources\RenderSystem\FrameGraph\autogen", @"workdir\shaders\autogen",
        };
        static readonly string[] OutputFiles = { @"workdir\shaders\enums.h" };

        bool running;

        protected override async Task InitializeAsync(CancellationToken token, IProgress<ServiceProgressData> progress)
        {
            await JoinableTaskFactory.SwitchToMainThreadAsync(token);
            var commands = (OleMenuCommandService)await GetServiceAsync(typeof(IMenuCommandService));
            var command = new OleMenuCommand((s, e) => JoinableTaskFactory.RunAsync(RegenerateAsync).FileAndForget("prism/regenerate"),
                                             new CommandID(CommandSet, RegenerateId));
            command.BeforeQueryStatus += (s, e) => command.Enabled = !running;
            commands.AddCommand(command);
        }

        async Task RegenerateAsync()
        {
            await JoinableTaskFactory.SwitchToMainThreadAsync();
            var dte = (DTE2)await GetServiceAsync(typeof(DTE));
            IVsOutputWindowPane pane = GetPane();
            pane.Clear();
            pane.Activate();

            // The generator reads .prism files from disk.
            foreach (Document doc in dte.Documents)
                if (!doc.Saved && doc.FullName.EndsWith(".prism", StringComparison.OrdinalIgnoreCase))
                    doc.Save();

            string repo = FindRepo(dte);
            if (repo == null)
            {
                pane.OutputStringThreadSafe("Could not find sources\\Prism\\defs above the solution or the active document.\n");
                ShowMessage("Could not find the Spectrum checkout (sources\\Prism\\defs) for this solution.", OLEMSGICON.OLEMSGICON_WARNING);
                return;
            }

            string sigDir = Path.Combine(repo, "sources", "Prism");
            string exe = PickGenerator(repo);
            pane.OutputStringThreadSafe($"Generator: {exe}\nWorking directory: {sigDir}\n\n");
            SetStatus("Regenerating Prism code...");

            Dictionary<string, string> before, after;
            int exitCode;
            string output;
            running = true;
            try
            {
                before = Snapshot(repo);
                (exitCode, output) = await Task.Run(() => Run(exe, "", sigDir));
                after = Snapshot(repo);
            }
            catch (Exception e)
            {
                await JoinableTaskFactory.SwitchToMainThreadAsync();
                pane.OutputStringThreadSafe($"Could not run the generator: {e.Message}\n");
                SetStatus("Prism generation could not start.");
                return;
            }
            finally
            {
                running = false;
            }

            await JoinableTaskFactory.SwitchToMainThreadAsync();
            pane.OutputStringThreadSafe(output);

            if (exitCode != 0)
            {
                int errors = output.Split('\n').Count(l => l.Contains(": error:"));
                string what = errors > 0 ? $"{errors} error(s) in .prism files" : $"exit code {exitCode}";
                SetStatus($"Prism generation failed: {what}; nothing was written.");
                ShowMessage($"Prism generation failed ({what}); nothing was written.\n\nDetails are in the Output window, \"Prism\" pane.",
                            OLEMSGICON.OLEMSGICON_CRITICAL);
                return;
            }

            var added = after.Keys.Where(k => !before.ContainsKey(k)).OrderBy(k => k).ToList();
            var removed = before.Keys.Where(k => !after.ContainsKey(k)).OrderBy(k => k).ToList();
            var modified = after.Keys.Where(k => before.TryGetValue(k, out var b) && b != after[k]).OrderBy(k => k).ToList();

            var summary = new StringBuilder();
            summary.Append($"\nPrism regenerated: {modified.Count} modified, {added.Count} added, {removed.Count} removed.\n");
            foreach (var f in added) summary.Append("  + " + f + "\n");
            foreach (var f in removed) summary.Append("  - " + f + "\n");
            foreach (var f in modified) summary.Append("  M " + f + "\n");
            pane.OutputStringThreadSafe(summary.ToString());
            SetStatus($"Prism regenerated: {modified.Count} modified, {added.Count} added, {removed.Count} removed.");

            // A new or deleted generated file is invisible to the build until
            // the Sharpmake projects are regenerated.
            if (added.Count + removed.Count > 0)
            {
                string bat = Path.Combine(repo, "generate_project.bat");
                int answer = ShowMessage($"The generated file set changed ({added.Count} added, {removed.Count} removed).\n\n" +
                                         "Run generate_project.bat now? Visual Studio will then offer to reload the projects.",
                                         OLEMSGICON.OLEMSGICON_QUERY, OLEMSGBUTTON.OLEMSGBUTTON_YESNO);
                if (answer == 6 /* IDYES */ && File.Exists(bat))
                {
                    pane.OutputStringThreadSafe("\nRunning generate_project.bat...\n");
                    var (code, log) = await Task.Run(() => Run(Environment.ExpandEnvironmentVariables("%ComSpec%"), $"/c \"{bat}\"", repo));
                    await JoinableTaskFactory.SwitchToMainThreadAsync();
                    pane.OutputStringThreadSafe(log + $"\ngenerate_project.bat exited with {code}.\n");
                }
            }
        }

        // The checkout the solution (or, failing that, the active document) is in.
        static string FindRepo(DTE2 dte)
        {
            var starts = new List<string>();
            if (!string.IsNullOrEmpty(dte.Solution?.FullName))
                starts.Add(Path.GetDirectoryName(dte.Solution.FullName));
            if (dte.ActiveDocument != null)
                starts.Add(Path.GetDirectoryName(dte.ActiveDocument.FullName));

            foreach (string start in starts)
                for (var dir = new DirectoryInfo(start); dir != null; dir = dir.Parent)
                    if (Directory.Exists(Path.Combine(dir.FullName, "sources", "Prism", "defs")))
                        return dir.FullName;
            return null;
        }

        // The repo's own Profile build when it is newer than the copy bundled
        // with the extension: generator changes then apply without reinstalling.
        static string PickGenerator(string repo)
        {
            string bundled = Path.Combine(Path.GetDirectoryName(typeof(PrismPackage).Assembly.Location), "server", "prismc.exe");
            string local = Path.Combine(repo, "bin", "profile", "prismc.exe");
            if (!File.Exists(local)) return bundled;
            if (!File.Exists(bundled)) return local;
            return File.GetLastWriteTimeUtc(local) > File.GetLastWriteTimeUtc(bundled) ? local : bundled;
        }

        // Size + write time per file. The generator only rewrites files whose
        // content changed, so this is enough to tell modified from untouched.
        static Dictionary<string, string> Snapshot(string repo)
        {
            var result = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
            void add(string path)
            {
                var info = new FileInfo(path);
                result[path.Substring(repo.Length + 1)] = info.Length + "/" + info.LastWriteTimeUtc.Ticks;
            }
            foreach (string dir in OutputDirs)
            {
                string full = Path.Combine(repo, dir);
                if (Directory.Exists(full))
                    foreach (string f in Directory.EnumerateFiles(full, "*", SearchOption.AllDirectories))
                        add(f);
            }
            foreach (string f in OutputFiles)
                if (File.Exists(Path.Combine(repo, f)))
                    add(Path.Combine(repo, f));
            return result;
        }

        static (int, string) Run(string exe, string args, string cwd)
        {
            var info = new ProcessStartInfo(exe, args)
            {
                WorkingDirectory = cwd,
                UseShellExecute = false,
                CreateNoWindow = true,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
            };
            var output = new StringBuilder();
            using (var process = new System.Diagnostics.Process { StartInfo = info })
            {
                process.OutputDataReceived += (s, e) => { if (e.Data != null) lock (output) output.AppendLine(e.Data); };
                process.ErrorDataReceived += (s, e) => { if (e.Data != null) lock (output) output.AppendLine(e.Data); };
                process.Start();
                process.BeginOutputReadLine();
                process.BeginErrorReadLine();
                process.WaitForExit();
                return (process.ExitCode, output.ToString());
            }
        }

        IVsOutputWindowPane GetPane()
        {
            ThreadHelper.ThrowIfNotOnUIThread();
            var window = (IVsOutputWindow)GetService(typeof(SVsOutputWindow));
            Guid guid = PaneGuid;
            if (window.GetPane(ref guid, out IVsOutputWindowPane pane) != 0 || pane == null)
            {
                window.CreatePane(ref guid, "Prism", 1, 1);
                window.GetPane(ref guid, out pane);
            }
            return pane;
        }

        void SetStatus(string text)
        {
            ThreadHelper.ThrowIfNotOnUIThread();
            (GetService(typeof(SVsStatusbar)) as IVsStatusbar)?.SetText(text);
        }

        int ShowMessage(string text, OLEMSGICON icon, OLEMSGBUTTON buttons = OLEMSGBUTTON.OLEMSGBUTTON_OK)
        {
            return VsShellUtilities.ShowMessageBox(this, text, "Prism", icon, buttons, OLEMSGDEFBUTTON.OLEMSGDEFBUTTON_FIRST);
        }
    }
}
