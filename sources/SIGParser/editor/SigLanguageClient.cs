// VS side of the SIG language server: registers a content type for .sig files
// and starts `sigparser.exe --lsp` for them. Compiled by gen_vs_extension.py.
using System;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.VisualStudio.LanguageServer.Client;
using Microsoft.VisualStudio.Threading;
using Microsoft.VisualStudio.Utilities;

namespace Spectrum.Sig
{
    public static class SigContentDefinition
    {
        // Based on the remote-content type so the TextMate grammar shipped in
        // the same VSIX keeps colouring these files.
        [Export]
        [Name("sig")]
        [BaseDefinition(CodeRemoteContentDefinition.CodeRemoteContentTypeName)]
        internal static ContentTypeDefinition SigContentType = null;

        [Export]
        [FileExtension(".sig")]
        [ContentType("sig")]
        internal static FileExtensionToContentTypeDefinition SigFileExtension = null;
    }

    [ContentType("sig")]
    [Export(typeof(ILanguageClient))]
    public class SigLanguageClient : ILanguageClient
    {
        public string Name => "SIG Language Server";
        public IEnumerable<string> ConfigurationSections => null;
        public object InitializationOptions => null;
        public IEnumerable<string> FilesToWatch => null;
        public bool ShowNotificationOnInitializeFailed => true;

        public event AsyncEventHandler<EventArgs> StartAsync;
        public event AsyncEventHandler<EventArgs> StopAsync;

        // SIG_LSP_SERVER overrides the bundled server, e.g. to point at a
        // freshly built bin/profile/sigparser.exe without reinstalling.
        static string ServerPath()
        {
            string overridePath = Environment.GetEnvironmentVariable("SIG_LSP_SERVER");
            if (!string.IsNullOrEmpty(overridePath) && File.Exists(overridePath))
                return overridePath;

            string dir = Path.GetDirectoryName(typeof(SigLanguageClient).Assembly.Location);
            return Path.Combine(dir, "server", "sigparser.exe");
        }

        public Task<Connection> ActivateAsync(CancellationToken token)
        {
            string exe = ServerPath();
            var info = new ProcessStartInfo
            {
                FileName = exe,
                Arguments = "--lsp",
                WorkingDirectory = Path.GetDirectoryName(exe),
                RedirectStandardInput = true,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                UseShellExecute = false,
                CreateNoWindow = true,
            };

            var process = new Process { StartInfo = info };
            if (!process.Start())
                return Task.FromResult<Connection>(null);

            // Drain stderr so a chatty server can never block on a full pipe.
            process.ErrorDataReceived += (s, e) => { if (e.Data != null) Debug.WriteLine("[sig-lsp] " + e.Data); };
            process.BeginErrorReadLine();

            return Task.FromResult(new Connection(process.StandardOutput.BaseStream, process.StandardInput.BaseStream));
        }

        public async Task OnLoadedAsync()
        {
            if (StartAsync != null)
                await StartAsync.InvokeAsync(this, EventArgs.Empty);
        }

        public Task OnServerInitializedAsync() => Task.CompletedTask;

        public Task<InitializationFailureContext> OnServerInitializeFailedAsync(ILanguageClientInitializationInfo initializationState)
        {
            return Task.FromResult(new InitializationFailureContext
            {
                FailureMessage = "SIG language server failed to start: " + initializationState.StatusMessage,
            });
        }
    }
}
