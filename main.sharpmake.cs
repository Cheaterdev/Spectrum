using Sharpmake;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text.Json;
using System.Text.Json.Nodes;

namespace Spectrum
{
    [Fragment, Flags]
    public enum Mode
    {
        Debug = 1,
        Profile = 2,
        Retail = 4
    }

    [Fragment, Flags]
    public enum Backend
    {
        D3D12  = 1,
        Vulkan = 2
    }

    public class CustomTarget : ITarget
    {
        public Platform Platform;
        public DevEnv DevEnv;
        public Optimization Optimization;
        public Mode Mode;
        public Backend Backend;
    }

    // The engine's layer stack, declared once. The Windows projects take their dependencies from it and
    // AndroidMetadata exports it to projects/android/targets.json for build/platforms/android/build.py, so a layer or a
    // dependency added here shows up in both builds.
    public static class EngineLayers
    {
        public class Layer
        {
            public string Name;      // key in targets.json
            public Type Project;
            public string Dir;       // under sources/, also where its Defines.h (the forced include) lives
            public string[] Deps;    // names of the layers this one depends on
        }

        public static readonly Layer[] All =
        {
            new Layer { Name = "modules",      Project = typeof(Modules),      Dir = "Modules",      Deps = new string[0] },
            new Layer { Name = "core",         Project = typeof(Core),         Dir = "Core",         Deps = new[] { "modules" } },
            new Layer { Name = "hal",          Project = typeof(HAL),          Dir = "HAL",          Deps = new[] { "core" } },
            new Layer { Name = "rendersystem", Project = typeof(RenderSystem), Dir = "RenderSystem", Deps = new[] { "hal" } },
            new Layer { Name = "prism",        Project = typeof(Prism),        Dir = "Prism",        Deps = new[] { "core" } },
            new Layer { Name = "spectrum",     Project = typeof(Spectrum),     Dir = "Spectrum",     Deps = new[] { "rendersystem" } },
            new Layer { Name = "test",         Project = typeof(Test),         Dir = "Test",         Deps = new[] { "rendersystem" } },
            new Layer { Name = "tools",        Project = typeof(Tools),        Dir = "Tools",        Deps = new[] { "rendersystem" } },
        };

        public static void AddDependencies(Project project, Project.Configuration conf, ITarget target)
        {
            var layer = All.First(l => l.Project == project.GetType());
            foreach (var dep in layer.Deps)
                conf.AddPublicDependency(target, All.First(l => l.Name == dep).Project);
        }
    }

    // What the Android build (build/platforms/android/build.py, plain Python + Ninja) needs from this description, written
    // to projects/android/ on every generate: the layer stack and a vcpkg manifest for arm64-android.
    public static class AndroidMetadata
    {
        // The ports the arm64-android build uses: a subset of the root vcpkg.json (everything else there is
        // Windows, DirectX, NVIDIA or a host tool). Versions and the baseline come from the root manifest.
        static readonly string[] VcpkgPorts =
        {
            "zlib", "magic-enum", "crossguid", "crc32c", "cereal", "bshoshany-thread-pool",
            "vulkan-headers", "vulkan-memory-allocator", "meshoptimizer", "skribidi"
        };

        // Ports only the Android build links directly (Windows has WIC/DirectXTex for these jobs).
        // libpng: PNG encode/decode for the golden-image tests (sources/Platform/Android/png_bridge.cpp).
        static readonly string[] AndroidOnlyPorts = { "libpng" };

        static readonly JsonSerializerOptions Indented = new JsonSerializerOptions
        {
            WriteIndented = true,
            // keeps "version>=" readable instead of "version>="
            Encoder = System.Text.Encodings.Web.JavaScriptEncoder.UnsafeRelaxedJsonEscaping
        };

        public static void Write([CallerFilePath] string callerPath = "")
        {
            string root = Path.GetDirectoryName(callerPath);
            string outDir = Path.Combine(root, "projects", "android");
            Directory.CreateDirectory(outDir);

            var layers = new JsonObject();
            foreach (var l in EngineLayers.All)
                layers[l.Name] = new JsonObject
                {
                    ["root"] = l.Dir,
                    ["forced"] = l.Dir + "/Defines.h",
                    ["deps"] = new JsonArray(l.Deps.Select(d => (JsonNode)JsonValue.Create(d)).ToArray()),
                };
            WriteIfChanged(Path.Combine(outDir, "targets.json"), new JsonObject { ["layers"] = layers });

            // Manifest: the Android subset of the root one.
            var rootManifest = JsonNode.Parse(File.ReadAllText(Path.Combine(root, "vcpkg.json")));
            string NameOf(JsonNode n) => n is JsonValue ? n.GetValue<string>() : n["name"].GetValue<string>();
            var deps = new JsonArray();
            foreach (var dep in rootManifest["dependencies"].AsArray().Where(d => VcpkgPorts.Contains(NameOf(d))))
                deps.Add(dep.DeepClone());
            foreach (var port in AndroidOnlyPorts)
                deps.Add(JsonValue.Create(port));
            var manifest = new JsonObject
            {
                ["name"] = "spectrum-android",
                ["version-string"] = rootManifest["version-string"].DeepClone(),
                ["dependencies"] = deps,
                ["builtin-baseline"] = rootManifest["builtin-baseline"].DeepClone(),
            };
            var overrides = new JsonArray();
            if (rootManifest["overrides"] != null)
                foreach (var o in rootManifest["overrides"].AsArray().Where(o => VcpkgPorts.Contains(o["name"].GetValue<string>())))
                    overrides.Add(o.DeepClone());
            if (overrides.Count > 0)
                manifest["overrides"] = overrides;
            WriteIfChanged(Path.Combine(outDir, "vcpkg.json"), manifest);

            // Configuration: the root one without the artifact registry (an Android build has no use for it),
            // with the overlay path made relative to this directory.
            var config = JsonNode.Parse(File.ReadAllText(Path.Combine(root, "vcpkg-configuration.json"))).AsObject();
            config.Remove("registries");
            string overlay = Path.GetRelativePath(outDir, Path.Combine(root, "custom-overlay")).Replace('\\', '/');
            config["overlay-ports"] = new JsonArray(JsonValue.Create(overlay));
            WriteIfChanged(Path.Combine(outDir, "vcpkg-configuration.json"), config);
        }

        // Only touches a file whose content changed, so tools that watch timestamps are not disturbed.
        static void WriteIfChanged(string path, JsonNode json)
        {
            string text = json.ToJsonString(Indented).Replace("\r\n", "\n") + "\n";
            if (!File.Exists(path) || File.ReadAllText(path) != text)
                File.WriteAllText(path, text);
        }
    }

    // Subdirectory (relative to the exe) holding the NVIDIA Streamline runtime.
    public static class Streamline
    {
        public const string Dir = "Streamline";
    }

    public static class Vcpkg
    {
        public const string Triplet = "x64-windows";
        public const string InstalledRoot = @"[project.SharpmakeCsPath]\vcpkg_installed\" + Triplet;
        public const string Bin = InstalledRoot + @"\" + Triplet + @"\bin";
        public const string DebugBin = InstalledRoot + @"\" + Triplet + @"\debug\bin";
        public const string Lib = InstalledRoot + @"\" + Triplet + @"\lib";
        public const string DebugLib = InstalledRoot + @"\" + Triplet + @"\debug\lib";
        public const string Include = InstalledRoot + @"\" + Triplet + @"\include";
    }


    [Sharpmake.Generate]
    public class Common : Project
    {
        public Common() : base(typeof(CustomTarget))
        {
            
           
            SourceFilesExtensions.Add(".prism");
            SourceFilesExtensions.Add(".hlsl");
			SourceFilesExtensions.Add(".ixx");
	        SourceFilesExtensions.Add(".g4");
	        SourceFilesExtensions.Add(".jinja");

			SourceFilesCompileExtensions.Add(".ixx");

            RootPath = @"[project.SharpmakeCsPath]\projects\[project.Name]";

            AddTargets(new CustomTarget
            {
                Platform = Platform.win64,
                DevEnv = DevEnv.vs2026,
                Optimization = Optimization.Release,
                Mode = Mode.Debug | Mode.Profile | Mode.Retail,
                Backend = Backend.D3D12 | Backend.Vulkan
            });

            CustomProperties.Add("VcpkgEnabled", "true");
            CustomProperties.Add("VcpkgEnableManifest", "true");
			CustomProperties.Add("VcpkgTriplet", Vcpkg.Triplet);
			CustomProperties.Add("VcpkgConfiguration", "Release");
            CustomProperties.Add("VcpkgApplocalDeps", "true");
            CustomProperties.Add("ScanSourceForModuleDependencies", "true");
            // Lets MSBuild run multiple graph-respecting cl.exe invocations concurrently.
            CustomProperties.Add("UseMultiToolTask", "true");
            CustomProperties.Add("EnforceProcessCountAcrossBuilds", "true");
         //   BlobWorkFileCount = 8;
        //    GeneratableBlobCount  = 8;
        }

        [Configure]
        public virtual void ConfigureAll(Configuration conf, CustomTarget target)
        {
             conf.Output = Configuration.OutputType.Utility;


            conf.ProjectFileName = "[project.Name]";
            conf.ProjectPath = @"[project.RootPath]";

            conf.IncludePaths.Add(SourceRootPath);

            // Platform-specific code sits in folders named after the platform (HAL/API/Vulkan/Android, ...). A Windows
            // project builds none of the other platforms' folders; the Android build (build.py) does the inverse.
            foreach (string folder in BuildPlatforms.NonWindowsFolders)
                conf.SourceFilesBuildExcludeRegex.Add(@".*\\" + folder + @"\\.*");

            // Core/Defines.h includes "Defines.Platform.h"; the include path decides whose (Core/Android has the other).
            conf.IncludePaths.Add(@"[project.SharpmakeCsPath]\sources\Core\Windows");

            // Repository sources root — allows cross-project Defines.h chains
            // (e.g. HAL/Defines.h can #include "Core/Defines.h").
            conf.IncludePaths.Add(@"[project.SharpmakeCsPath]\sources");

            // Each subproject has a Defines.h at its root that chains upward
            // through the dependency graph.  We use the full SourceRootPath so
            // the forced include is always resolved correctly regardless of
            // include-path ordering — especially for files in subdirectories
            // (e.g. HAL/D3D12/*.cpp which couldn't find a plain "Defines.h").
            conf.ForcedIncludes.Add(@"[project.SourceRootPath]\Defines.h");

			//conf.ExportAdditionalLibrariesEvenForStaticLib = true;
			conf.PrecompSourceExcludeExtension.Add(".ixx");

            conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.Latest);
            conf.Options.Add(Options.Vc.Compiler.Exceptions.Enable);
            conf.Options.Add(Options.Vc.Compiler.RTTI.Enable);
            conf.Options.Add(Options.Vc.Linker.SubSystem.Windows);
            conf.Options.Add(Options.Vc.General.WindowsTargetPlatformVersion.Latest);
            conf.Options.Add(Options.Vc.Compiler.FunctionLevelLinking.Disable);
            conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDLL);
            conf.Options.Add(Options.Vc.Compiler.Inline.OnlyInline);
            conf.Options.Add(Options.Vc.General.WarningLevel.Level3);		 // hate warnings, love errors

            conf.AdditionalCompilerOptions.Add("/bigobj");
         //   conf.AdditionalCompilerOptions.Add("/dxifcInlineFunctions-");

            conf.Defines.Add("_MBCS");
			conf.Defines.Add("BOOST_NO_USER_CONFIG");
			conf.Defines.Add("BOOST_ENDIAN_DEPRECATED_NAMES");
			conf.Defines.Add("BOOST_ALL_NO_LIB");
			conf.Defines.Add("BOOST_NO_CXX11_NOEXCEPT");
			
			conf.Defines.Add("_SCL_SECURE_NO_WARNINGS");
            conf.Defines.Add("_CRT_SECURE_NO_WARNINGS");
            conf.Defines.Add("NOMINMAX");
            conf.Defines.Add("_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING");
            conf.Defines.Add("_SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING");
       
            conf.Defines.Add("SPECTRUM_ENABLE_EXCEPTIONS");  // fixed: was "EXEPTIONS"
            // CEREAL_THREAD_SAFE removed: it makes cereal include <mutex> which in MSVC 14.51+
            // transitively pulls <stop_token> into _cereal.h's header unit IFC.
            // That causes C1116 when any TU imports both cereal and a threading header unit.
            // cereal's internal polymorphic registry (what this flag protects) is populated
            // during single-threaded static init, so removing it is safe.
            conf.Defines.Add("USE_PIX");
            conf.Defines.Add("WIN32_LEAN_AND_MEAN");
         
            conf.Options.Add(new Sharpmake.Options.Vc.Compiler.DisableSpecificWarnings("4005", "5104", "5105", "5106", "4494")); //module reference issues

            if (target.Mode == Mode.Debug)
            {
                conf.Options.Add(Options.Vc.Compiler.Optimization.Disable);
                conf.Options.Add(Options.Vc.Compiler.Inline.Disable);
                conf.Defines.Add("DEV");
                conf.Defines.Add("PROFILING");
            }
            else
            {
                conf.Options.Add(Options.Vc.Compiler.Optimization.FullOptimization);
                conf.Defines.Add("RETAIL");
                conf.Defines.Remove("NDEBUG");
            }

            if (target.Mode == Mode.Profile)
            {
                conf.Defines.Add("PROFILING");
            }


        }

    }

    [Sharpmake.Generate]
    public class Library : Common
    {
        public Library()
        {

        }

        public override void ConfigureAll(Configuration conf, CustomTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.Output = Configuration.OutputType.Lib;
        }
    }



    [Sharpmake.Generate]
    public class Application : Common
    {
        public Application()
        {
            RootPath = @"[project.SharpmakeCsPath]\projects\[project.Name]";
        }

        public override void ConfigureAll(Configuration conf, CustomTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.Output = Configuration.OutputType.Exe;
            conf.TargetPath = @"[project.SharpmakeCsPath]\bin\" + target.Mode.ToString();

            // Both backends share TargetPath, so distinct names keep one backend's
            // build from being launched as the other's (VS's up-to-date check sees
            // a newer exe from the other backend and skips relinking).
            if (target.Backend == Backend.Vulkan)
                conf.TargetFileName = GetType().Name.ToLower() + "_vulkan";
        }

    }
/*
    [Sharpmake.Generate]
    public class Aftermath : Library
    {
        public Aftermath()
        {
            SourceRootPath = @"[project.SharpmakeCsPath]\sources\3rdparty\Aftermath";
            AssemblyName = "Aftermath";
        }


        public override void ConfigureAll(Configuration conf, CustomTarget target)
        {
            base.ConfigureAll(conf, target);
            // Aftermath is third-party NVIDIA code with no custom Defines.h.
            // Replace the base forced include with the bottom of the chain.
            conf.ForcedIncludes.Remove(@"[project.SourceRootPath]\Defines.h");
            conf.ForcedIncludes.Add(@"[project.SharpmakeCsPath]\sources\Modules\Defines.h");
            conf.LibraryFiles.Add(@"[project.SourceRootPath]\lib\x64\GFSDK_Aftermath_Lib.x64.lib");
            conf.TargetCopyFiles.Add(@"[project.SourceRootPath]\lib\x64\GFSDK_Aftermath_Lib.x64.dll");
            conf.IncludePaths.Add(@"[project.SourceRootPath]/include");
        }
    }
	*/
	
[Sharpmake.Generate]
    public class Modules : Library
    {
        public Modules()
        {
            SourceRootPath = @"[project.SharpmakeCsPath]\sources\Modules";
            AssemblyName = "Modules";
        }


        public override void ConfigureAll(Configuration conf, CustomTarget target)
        {
            base.ConfigureAll(conf, target);
            conf.IncludePaths.Add(@"[project.SourceRootPath]/include");

			conf.ExportAdditionalLibrariesEvenForStaticLib = false;

            { // PIX
			//	conf.IncludePaths.Add(@"[project.SharpmakeCsPath]\PIX\Include\WinPixEventRuntime", 66);
            //    conf.TargetCopyFiles.Add(@"[project.SharpmakeCsPath]\PIX\bin\x64\WinPixEventRuntime.dll");

	//		    conf.LibraryFiles.Add("WinPixEventRuntime.lib");
           //     conf.LibraryPaths.Add(@"[project.SharpmakeCsPath]\PIX\bin\x64", 66);
			}

            { // NVIDIA Streamline (DLSS-SR / DLSS-RR)
                // Manual-hooking mode: nothing is linked against Streamline (see
                // the sl.interposer.lib note below). Every plugin/NGX DLL is
                // LoadLibrary'd at runtime, so VcpkgApplocalDeps can't see them
                // and they're copied explicitly.
                string slBin = (target.Mode == Mode.Debug) ? Vcpkg.DebugBin : Vcpkg.Bin;

                var slDlls = new List<string> {
                    "sl.interposer.dll",   // loader / sl* entry points
                    "sl.common.dll",       // shared plugin infrastructure
                    "sl.dlss.dll",         // Super Resolution
                    "nvngx_dlss.dll",      //   ...its NGX model
                    "sl.dlss_d.dll",       // Ray Reconstruction
                    "nvngx_dlssd.dll",     //   ...its NGX model
                };
                if (target.Mode == Mode.Debug)
                {
                    slDlls.Add("sl.imgui.dll");
                    slDlls.Add("WinPixEventRuntime.dll");
                }

                foreach (var dll in slDlls)
                    conf.TargetCopyFilesToSubDirectory.Add(
                        new KeyValuePair<string, string>(slBin + @"\" + dll, Streamline.Dir));

                // sl.interposer.lib is not linked on purpose: HAL LoadLibrary's
                // it from Streamline.Dir and resolves sl* entry points via
                // GetProcAddress. vcpkg's auto-link does still pick it up (and
                // applocal-deps copies sl.interposer.dll next to the exe), so
                // any API it also exports must come from an explicitly listed
                // lib -- see vulkan-1.lib in HAL.
            }

            { // NVIDIA NRD
                // nrd.lib itself is auto-linked by the vcpkg manifest
                // integration (its name matches the port name, which also
                // gets its directory added to the library search path
                // automatically), but NRD.lib calls
                // ShaderMake::FindPermutationInBlob() without bundling it --
                // that symbol lives in a second static lib the nrd port
                // also installs (ShaderMakeBlob.lib) that doesn't match any
                // port name, so neither the reference nor its directory get
                // added automatically.
                conf.LibraryPaths.Add((target.Mode == Mode.Debug) ? Vcpkg.DebugLib : Vcpkg.Lib);
                conf.LibraryFiles.Add("ShaderMakeBlob.lib");
            }

            // runtime-loaded DLLs not detected by VcpkgApplocalDeps
            conf.TargetCopyFiles.Add(Vcpkg.Bin + @"\dstoragecore.dll");
            conf.TargetCopyFiles.Add(Vcpkg.Bin + @"\dxcompiler.dll");
            conf.TargetCopyFiles.Add(Vcpkg.Bin + @"\dxil.dll");
            conf.TargetCopyFilesToSubDirectory.Add(new KeyValuePair<string, string>(Vcpkg.DebugBin + @"\D3D12Core.dll", "D3D12"));
            conf.TargetCopyFilesToSubDirectory.Add(new KeyValuePair<string, string>(Vcpkg.DebugBin + @"\d3d12SDKLayers.dll", "D3D12"));

            // Per-backend module wrappers: compile only the active backend's wrapper.
            if (target.Backend == Backend.D3D12)
                conf.SourceFilesBuildExcludeRegex.Add(@".*\\Modules\\vulkan\\.*");
            else // Vulkan
                conf.SourceFilesBuildExcludeRegex.Add(@".*\\Modules\\d3d12\\.*");

            conf.Options.Add(new Sharpmake.Options.Vc.Compiler.DisableSpecificWarnings("5260")); // adding inline to header units

        }
    }
	
	
    [Sharpmake.Generate]
    public class Core : Library
    {
        public Core()
        {
            SourceRootPath = @"[project.SharpmakeCsPath]\sources\Core";
            AssemblyName = "Core";
            
        }

        public override void ConfigureAll(Configuration conf, CustomTarget target)
        {
            base.ConfigureAll(conf, target);
            EngineLayers.AddDependencies(this, conf, target);
            conf.Defines.Add("LEAK_TEST_ENABLE");
            
        }
    }


    [Sharpmake.Generate]
    public class HAL : Library
    {
        public HAL()
        {
            SourceRootPath = @"[project.SharpmakeCsPath]\sources\HAL";
            AssemblyName = "HAL";
        }

        public override void ConfigureAll(Configuration conf, CustomTarget target)
        {
            base.ConfigureAll(conf, target);

            if (target.Backend == Backend.D3D12)
            {
                // D3D12 backend: include D3D12/ and DXGI/ folders, exclude Vulkan/
                conf.LibraryFiles.Add("dxgi.lib");
                conf.LibraryFiles.Add("d3d12.lib");
                conf.LibraryFiles.Add("dxguid.lib");
                conf.LibraryFiles.Add("volatileaccessu.lib");

                conf.SourceFilesBuildExcludeRegex.Add(@".*\\HAL\\API\\Vulkan\\.*");

                conf.Defines.Add("HAL_BACKEND_D3D12");
            }
            else // Vulkan
            {
                // Vulkan backend: include Vulkan/ folder, exclude D3D12/ and DXGI/
                conf.SourceFilesBuildExcludeRegex.Add(@".*\\HAL\\API\\D3D12\\.*");
                conf.SourceFilesBuildExcludeRegex.Add(@".*\\HAL\\DXGI\\.*");

                conf.Defines.Add("HAL_BACKEND_VULKAN");

                // Must be listed explicitly: vcpkg's auto-link appends every
                // installed .lib after the project's own, alphabetically, and
                // sl.interposer.lib (streamline port) also exports the vk*
                // entry points -- left to auto-link, the linker resolves them
                // from it, all Vulkan calls go through Streamline's proxy, and
                // it errors "call slInit before ... Vulkan API". Explicit libs
                // come first, like d3d12.lib/dxgi.lib above.
                conf.LibraryFiles.Add("vulkan-1.lib");
            }

            // DXC folder is always compiled — DXC emits SPIR-V for Vulkan too.
            // (No exclusion needed for DXC.)

            EngineLayers.AddDependencies(this, conf, target);
        }
    }

    [Sharpmake.Generate]
    public class RenderSystem : Library
    {
        public RenderSystem()
        {
            SourceRootPath = @"[project.SharpmakeCsPath]\sources\RenderSystem";
            AssemblyName = "RenderSystem";
        }

        public override void ConfigureAll(Configuration conf, CustomTarget target)
        {
            base.ConfigureAll(conf, target);
            EngineLayers.AddDependencies(this, conf, target);

            // Sharpmake Defines don't propagate through AddPublicDependency, so
            // mirror HAL's own HAL_BACKEND_* defines here for RenderSystem's
            // #ifdef HAL_BACKEND_D3D12 guards (GraphicsSystem.ixx,
            // universal_material.cpp, UpscalingDLSS.cpp).
            if (target.Backend == Backend.D3D12)
                conf.Defines.Add("HAL_BACKEND_D3D12");
            else
                conf.Defines.Add("HAL_BACKEND_VULKAN");
        }
    }

    [Sharpmake.Generate]
    public class Prism : Application
    {
        public Prism()
        {
            SourceRootPath = @"[project.SharpmakeCsPath]\sources\Prism";
            AssemblyName = "Prism";
        }

        public override void ConfigureAll(Configuration conf, CustomTarget target)
        {
            base.ConfigureAll(conf, target);
            conf.Options.Remove(Options.Vc.Linker.SubSystem.Windows);
            conf.Options.Add(Options.Vc.General.CharacterSet.Unicode);
            conf.Options.Remove(Options.Vc.General.WarningLevel.Level3);		 // hate warnings, love errors
            conf.Options.Add(Options.Vc.General.WarningLevel.Level0);        // hate warnings, love errors
          //  conf.Options.Remove(Options.Vc.Compiler.CppLanguageStandard.Latest);
          //  conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP14);

            // "Prism compiler"; the editor extension and docs refer to prismc.exe.
            conf.TargetFileName = "prismc";

            conf.VcxprojUserFile = new Project.Configuration.VcxprojUserFileSettings();
            conf.VcxprojUserFile.LocalDebuggerWorkingDirectory = @"[project.SharpmakeCsPath]\sources\Prism";

            EngineLayers.AddDependencies(this, conf, target);
        }
    }

    [Sharpmake.Generate]
    public class Spectrum : Application
    {
        public Spectrum()
        {
            SourceRootPath = @"[project.SharpmakeCsPath]\sources\Spectrum";
            AssemblyName = "Spectrum";
        }

        public override void ConfigureAll(Configuration conf, CustomTarget target)
        {
            base.ConfigureAll(conf, target);

            //conf.IsBlobbed = true;

            // Same mirroring as RenderSystem: main.cpp's #ifndef HAL_BACKEND_VULKAN
            // guards (the 3D scene drawer) need the backend define too.
            if (target.Backend == Backend.D3D12)
                conf.Defines.Add("HAL_BACKEND_D3D12");
            else
                conf.Defines.Add("HAL_BACKEND_VULKAN");

            conf.LibraryFiles.Add("Onecore.lib");

            conf.VcxprojUserFile = new Project.Configuration.VcxprojUserFileSettings();
            conf.VcxprojUserFile.LocalDebuggerWorkingDirectory = @"[project.SharpmakeCsPath]\workdir";

            // Vendored NVIDIA NRD shader source (workdir/shaders/nrd/3rdparty/,
            // gitignored -- proprietary, see .gitignore's comment there) is
            // NOT checked in, and the nrd vcpkg port only installs headers +
            // libs, not its Shaders/ folder -- so a fresh clone/machine has
            // nothing there until synced. Runs every build (idempotent
            // copy-only, never deletes) so a freshly-restored/rebuilt nrd
            // vcpkg package gets synced automatically with zero extra manual
            // steps; see sync_nrd_shaders.bat for the actual copy + the
            // corresponding custom-overlay/nrd/portfile.cmake install step
            // this depends on.
            conf.EventPostBuild.Add(@"call ""[project.SharpmakeCsPath]\sync_nrd_shaders.bat""");

            EngineLayers.AddDependencies(this, conf, target);
        }
    }

    [Sharpmake.Generate]
    public class Resources : Common
    {
        public Resources()
        {
            SourceRootPath = @"[project.SharpmakeCsPath]\workdir";
            AssemblyName = "Resources";
        }

        public override void ConfigureAll(Configuration conf, CustomTarget target)
        {
            base.ConfigureAll(conf, target);
            // workdir has no Defines.h; redirect to the top of the chain.
            conf.ForcedIncludes.Remove(@"[project.SourceRootPath]\Defines.h");
            conf.ForcedIncludes.Add(@"[project.SharpmakeCsPath]\sources\Spectrum\Defines.h");
        }
    }

    [Sharpmake.Generate]
    public class Test : Application
    {
        public Test()
        {
            SourceRootPath = @"[project.SharpmakeCsPath]\sources\Test";
            AssemblyName = "SpectrumTest";
        }

        public override void ConfigureAll(Configuration conf, CustomTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.Options.Remove(Options.Vc.Linker.SubSystem.Windows);

            conf.VcxprojUserFile = new Project.Configuration.VcxprojUserFileSettings();
            conf.VcxprojUserFile.LocalDebuggerWorkingDirectory = @"[project.SharpmakeCsPath]\workdir";

            EngineLayers.AddDependencies(this, conf, target);
        }
    }

    // Offline command-line tooling on top of the whole engine (shader precompile
    // for other targets, more commands later). Run from workdir/.
    [Sharpmake.Generate]
    public class Tools : Application
    {
        public Tools()
        {
            SourceRootPath = @"[project.SharpmakeCsPath]\sources\Tools";
            AssemblyName = "SpectrumTools";
        }

        public override void ConfigureAll(Configuration conf, CustomTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.Options.Remove(Options.Vc.Linker.SubSystem.Windows);

            conf.TargetFileName = target.Backend == Backend.Vulkan ? "spectrum_tool_vulkan" : "spectrum_tool";

            conf.VcxprojUserFile = new Project.Configuration.VcxprojUserFileSettings();
            conf.VcxprojUserFile.LocalDebuggerWorkingDirectory = @"[project.SharpmakeCsPath]\workdir";

            EngineLayers.AddDependencies(this, conf, target);
        }
    }

    [Sharpmake.Generate]
    public class SpectrumSolution : Solution
    {
        public SpectrumSolution()
        : base(typeof(CustomTarget))
        {
            AddTargets(new CustomTarget
            {
                Platform = Platform.win64,
                DevEnv = DevEnv.vs2026,
                Optimization =  Optimization.Release,
                Mode = Mode.Debug | Mode.Profile | Mode.Retail,
                Backend = Backend.D3D12 | Backend.Vulkan
            });
        }

        [Configure()]
        public void ConfigureAll(Configuration conf, CustomTarget target)
        {
            conf.SolutionFileName = "Spectrum";
            conf.SolutionPath = @"[solution.SharpmakeCsPath]\projects\";
            string platformName = string.Empty;

            switch (target.Mode)
            {
                case Mode.Debug:   platformName += "Debug";   break;
                case Mode.Retail:  platformName += "Retail";  break;
                case Mode.Profile: platformName += "Profile"; break;
                default:
                    throw new NotImplementedException();
            }

            switch (target.Backend)
            {
                case Backend.D3D12:  platformName += "-D3D12";  break;
                case Backend.Vulkan: platformName += "-Vulkan"; break;
                default:
                    throw new NotImplementedException();
            }

            conf.Name = platformName;

            conf.AddProject<Spectrum>(target);
            conf.AddProject<Prism>(target);
            conf.AddProject<Test>(target);
            conf.AddProject<Tools>(target);
            conf.AddProject<Resources>(target);
        }

        [Sharpmake.Main]
        public static void SharpmakeMain(Sharpmake.Arguments arguments)
        {
            AndroidMetadata.Write();
            FastBuildSettings.FastBuildMakeCommand = @"[project.SharpmakeCsPath]\tools\FastBuild\FBuild.exe";
            arguments.Generate<SpectrumSolution>();
            arguments.Generate<SpectrumAndroidSolution>();
        }
    }

    // Where a platform's source code and its build tooling live. Platform code (entry point, stand-ins for
    // missing APIs, packaging files) is under sources/Platform/<Name>/, like sources/HAL/API/<Backend>/ is;
    // the scripts that build and run it are build/platforms/<name>/, started through build/driver.py.
    public static class BuildPlatforms
    {
        // Folder names (anywhere under sources/) that hold one platform's code; the Windows projects skip these.
        public static readonly string[] NonWindowsFolders = { "Android" };

        public const string AndroidSources = @"[project.SharpmakeCsPath]\sources\Platform\Android";
        public const string AndroidScripts = @"[project.SharpmakeCsPath]\build\platforms\android";
        public const string Tooling = @"[project.SharpmakeCsPath]\build";
        public const string Driver = @"[project.SharpmakeCsPath]\build\driver.py";
    }

    // The Android build cannot be an MSBuild/AGDE one: AGDE has no C++ module or header-unit support.
    // build/platforms/android/build.py does the compiling (header units + clang-scan-deps + Ninja); this project is
    // only the Visual Studio front end for it: F7 builds and packages the APK, F5 installs, launches
    // and streams the log. Compiler errors land in the Error List because Clang prints file:line:col.
    [Sharpmake.Generate]
    public class AndroidApp : Project
    {
        public AndroidApp() : base(typeof(CustomTarget))
        {
            Name = "SpectrumAndroid";
            RootPath = @"[project.SharpmakeCsPath]\projects\android";
            SourceRootPath = BuildPlatforms.AndroidSources;        // entry point, shims, Java/manifest
            AdditionalSourceRootPaths.Add(BuildPlatforms.Tooling); // build/driver.py, clang_modules.py, platforms/android/*.py
            SourceFilesExtensions.Add(".py");
            SourceFilesExtensions.Add(".java");
            SourceFilesExtensions.Add(".xml");
            SourceFilesExtensions.Add(".ixx");
            // The files are listed for browsing only; cl.exe must not try to compile Android sources.
            SourceFilesCompileExtensions.Clear();

            AddTargets(new CustomTarget
            {
                Platform = Platform.win64,
                DevEnv = DevEnv.vs2026,
                Optimization = Optimization.Release,
                Mode = Mode.Debug | Mode.Profile | Mode.Retail,
                Backend = Backend.Vulkan
            });
        }

        // The pythonw.exe of the first python on PATH that is not the WindowsApps alias (an app-execution
        // alias, not a program the debugger can launch). A launcher shim in between (the "py install
        // manager" bin dir) would be the debuggee instead of the interpreter, so the real executable is
        // asked for: the debugger has to be attached to the process that calls OutputDebugString.
        static string FindPythonw()
        {
            foreach (var dir in (Environment.GetEnvironmentVariable("PATH") ?? "").Split(System.IO.Path.PathSeparator))
            {
                if (dir.Length == 0 || dir.IndexOf("WindowsApps", StringComparison.OrdinalIgnoreCase) >= 0)
                    continue;
                string python = System.IO.Path.Combine(dir.Trim('"'), "python.exe");
                if (!System.IO.File.Exists(python))
                    continue;
                try
                {
                    var psi = new System.Diagnostics.ProcessStartInfo(python, "-c \"import sys; print(sys.executable)\"")
                    {
                        RedirectStandardOutput = true,
                        UseShellExecute = false,
                        CreateNoWindow = true
                    };
                    using (var p = System.Diagnostics.Process.Start(psi))
                    {
                        string real = p.StandardOutput.ReadToEnd().Trim();
                        p.WaitForExit();
                        string candidate = System.IO.Path.Combine(System.IO.Path.GetDirectoryName(real) ?? "", "pythonw.exe");
                        if (System.IO.File.Exists(candidate))
                            return candidate;
                    }
                }
                catch (Exception)
                {
                }
            }
            Console.WriteLine("WARNING: no pythonw.exe on PATH; F5 in SpectrumAndroid will not start. Put Python on PATH and regenerate.");
            return "pythonw.exe";
        }

        [Configure]
        public void ConfigureAll(Configuration conf, CustomTarget target)
        {
            // build.py's --mode values: debug | profile | retail
            string mode = target.Mode.ToString().ToLowerInvariant();
            string driver = @"python """ + BuildPlatforms.Driver + @""" android ";

            conf.ProjectFileName = "[project.Name]";
            conf.ProjectPath = @"[project.RootPath]";
            conf.Output = Configuration.OutputType.None;   // None + CustomBuildSettings is what makes a Makefile (NMake) project

            // The commands are written into the vcxproj as-is, so & has to be XML-escaped.
            // prepare.py: arm64 vcpkg ports and the precompiled shaders/textures/assets (needs the Tools project built,
            // which the dependency below takes care of); each step skips itself while its inputs are unchanged.
            string build = driver + "prepare --mode " + mode + " &amp;&amp; " +
                           driver + "build --mode " + mode + " --target app &amp;&amp; " +
                           driver + "package --mode " + mode;

            conf.CustomBuildSettings = new Configuration.NMakeBuildSettings
            {
                BuildCommand = build,
                RebuildCommand = @"rmdir /s /q ""[project.RootPath]\" + mode + @""" 2&gt;nul &amp; " + build,
                CleanCommand = @"rmdir /s /q ""[project.RootPath]\" + mode + @"""",
                OutputFile = @"[project.RootPath]\" + mode + @"\spectrum.apk",
            };
            // spectrum_tool (Windows, Vulkan, same mode) precompiles what the device cannot: it is built
            // first, with the engine layers it uses, like any other project of the Windows solution.
            conf.AddPublicDependency<Tools>(target);

            // F5 debugs pythonw.exe running run.py: it has no console of its own, and run.py sends the device
            // log to OutputDebugString, so it shows up in the Output window's Debug pane. The debugger launcher
            // does not search PATH (the build commands do, under cmd), so the full path is needed. The platform
            // script is started directly, not through the driver: the debuggee has to be the process that logs.
            conf.VcxprojUserFile = new Project.Configuration.VcxprojUserFileSettings();
            conf.VcxprojUserFile.LocalDebuggerWorkingDirectory = @"[project.SharpmakeCsPath]";
            conf.VcxprojUserFile.LocalDebuggerCommand = FindPythonw();
            conf.VcxprojUserFile.LocalDebuggerCommandArguments = @"""" + BuildPlatforms.AndroidScripts + @"\run.py"" --mode " + mode;
        }
    }

    // A separate solution: Android has its own build tool, so it must not be mixed into the
    // Windows one (which would also rename those configurations).
    [Sharpmake.Generate]
    public class SpectrumAndroidSolution : Solution
    {
        public SpectrumAndroidSolution()
        : base(typeof(CustomTarget))
        {
            AddTargets(new CustomTarget
            {
                Platform = Platform.win64,
                DevEnv = DevEnv.vs2026,
                Optimization = Optimization.Release,
                Mode = Mode.Debug | Mode.Profile | Mode.Retail,
                Backend = Backend.Vulkan
            });
        }

        [Configure()]
        public void ConfigureAll(Configuration conf, CustomTarget target)
        {
            conf.SolutionFileName = "SpectrumAndroid";
            conf.SolutionPath = @"[solution.SharpmakeCsPath]\projects\";
            conf.Name = target.Mode + "-Android";

            conf.AddProject<AndroidApp>(target);

            // The same vcxproj files the Windows solution uses, grouped in a folder for editing and navigation.
            // They are built here too: AndroidApp depends on Tools (the precompile tool), which pulls in the layers
            // below it, so a first F7 compiles them for Windows (MSBuild keeps them up to date afterwards).
            // IntelliSense sees them as a Windows build, so SPECTRUM_PLATFORM_ANDROID branches look disabled.
            conf.AddProject<Modules>(target, false, "Engine");
            conf.AddProject<Core>(target, false, "Engine");
            conf.AddProject<HAL>(target, false, "Engine");
            conf.AddProject<RenderSystem>(target, false, "Engine");
            conf.AddProject<Tools>(target, false, "Engine");
        }
    }
}
