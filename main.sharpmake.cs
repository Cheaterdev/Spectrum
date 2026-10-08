using Sharpmake;
using System;
using System.Collections.Generic;
using System.Linq;

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
        // Left at 0 for Windows targets: Sharpmake omits zero-valued fragments
        // from target/config names, so existing Windows config names don't change.
        public Android.AndroidBuildTargets AndroidBuildTargets;
    }

    public static class AndroidSetup
    {
        // Resolves SDK/NDK/JDK without requiring environment variables (a freshly
        // set variable isn't seen by already-running shells): explicit env var
        // wins, then the default Android Studio / cmdline-tools location.
        public static bool Configure()
        {
            string sdk = FirstExisting(
                Environment.GetEnvironmentVariable("ANDROID_HOME"),
                Environment.GetEnvironmentVariable("ANDROID_SDK_ROOT"),
                System.IO.Path.Combine(Environment.GetEnvironmentVariable("LOCALAPPDATA") ?? "", "Android", "Sdk"));

            string ndk = FirstExisting(
                Environment.GetEnvironmentVariable("ANDROID_NDK_HOME"),
                Environment.GetEnvironmentVariable("ANDROID_NDK_ROOT"),
                LatestSubdir(sdk == null ? null : System.IO.Path.Combine(sdk, "ndk"), "", MaxAgdeNdkMajor));

            string java = FirstExisting(
                Environment.GetEnvironmentVariable("JAVA_HOME"),
                LatestSubdir(@"C:\Program Files\Java", "jdk"),
                @"C:\Program Files\Android\Android Studio\jbr");

            if (sdk == null || ndk == null || java == null)
            {
                Console.WriteLine("WARNING: Android SDK/NDK/JDK not found (sdk={0}, ndk={1} [AGDE needs r<={2}], jdk={3}); " +
                    "skipping the Android solution. Set ANDROID_HOME / ANDROID_NDK_HOME / JAVA_HOME to override.",
                    sdk ?? "-", ndk ?? "-", MaxAgdeNdkMajor, java ?? "-");
                return false;
            }

            Android.GlobalSettings.AndroidHome = sdk;
            Android.GlobalSettings.NdkRoot = ndk;
            Android.GlobalSettings.JavaHome = java;
            return true;
        }

        static string FirstExisting(params string[] paths)
        {
            foreach (var p in paths)
                if (!string.IsNullOrEmpty(p) && System.IO.Directory.Exists(p))
                    return p;
            return null;
        }

        // AGDE's MSBuild integration only ships toolset files up to NDK r29
        // (Google.Android.Cpp.Toolset.r*.props); newer NDKs fail with
        // "NDK major version N is unknown or not supported".
        const int MaxAgdeNdkMajor = 29;

        static string LatestSubdir(string root, string prefix = "", int maxMajor = int.MaxValue)
        {
            if (string.IsNullOrEmpty(root) || !System.IO.Directory.Exists(root))
                return null;
            string best = null;
            foreach (var d in System.IO.Directory.GetDirectories(root))
            {
                string name = System.IO.Path.GetFileName(d);
                if (!name.StartsWith(prefix, StringComparison.OrdinalIgnoreCase))
                    continue;
                var digits = new string(name.Substring(prefix.Length).TrimStart('-').TakeWhile(char.IsDigit).ToArray());
                if (maxMajor != int.MaxValue && (digits.Length == 0 || int.Parse(digits) > maxMajor))
                    continue;
                if (best == null || string.Compare(d, best, StringComparison.OrdinalIgnoreCase) > 0)
                    best = d;
            }
            return best;
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

            AddTargets(AndroidTarget());

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

        // UI-only Android (arm64, Vulkan) for now; 3D passes are compiled out.
        public static CustomTarget AndroidTarget()
        {
            return new CustomTarget
            {
                Platform = Platform.agde,
                DevEnv = DevEnv.vs2026,
                Optimization = Optimization.Release,
                Mode = Mode.Debug | Mode.Retail,
                Backend = Backend.Vulkan,
                AndroidBuildTargets = Android.AndroidBuildTargets.arm64_v8a
            };
        }

        [Configure]
        public virtual void ConfigureAll(Configuration conf, CustomTarget target)
        {
             conf.Output = Configuration.OutputType.Utility;


            conf.ProjectFileName = "[project.Name]";
            conf.ProjectPath = @"[project.RootPath]";

            if (target.Platform == Platform.agde)
            {
                // A separate vcxproj: AGDE projects can't share a file with the win64 ones.
                conf.ProjectFileName = "[project.Name]_Android";
                conf.ProjectName = "[project.Name]_Android";
                conf.IntermediatePath = @"[project.SharpmakeCsPath]\projects\obj\android\[target.Mode]\[project.Name]";
                conf.TargetLibraryPath = conf.IntermediatePath;
                ConfigureAndroid(conf, target);
                return;
            }

            conf.IncludePaths.Add(SourceRootPath);
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

        void ConfigureAndroid(Configuration conf, CustomTarget target)
        {
            conf.IncludePaths.Add(SourceRootPath);
            conf.IncludePaths.Add(@"[project.SharpmakeCsPath]\sources");
            conf.ForcedIncludes.Add(@"[project.SourceRootPath]\Defines.h");
            conf.PrecompSourceExcludeExtension.Add(".ixx");

            conf.Options.Add(new Options.Agde.General.AndroidGradleBuildDir("$(SolutionDir)"));
            conf.Options.Add(Options.Android.General.AndroidAPILevel.Android29);
            conf.Options.Add(Options.Agde.General.UseOfStl.LibCpp_Shared);
            conf.Options.Add(Options.Agde.Linker.BuildId.Md5);
            conf.Options.Add(Options.Agde.Compiler.ExceptionHandling.Enable);
            conf.Options.Add(Options.Agde.Compiler.CppLanguageStandard.Cpp20);

            // The project-level Vcpkg properties are x64-windows; the msbuild integration
            // would feed Windows include/lib paths into the NDK build.
            conf.CustomProperties["VcpkgEnabled"] = "false";
            conf.CustomProperties["VcpkgEnableManifest"] = "false";
            conf.CustomProperties["VcpkgApplocalDeps"] = "false";
            // AGDE has no C++23 enum value; modules are built from .ixx by the
            // compiler's own scanning, which needs a >= C++20 standard.
            conf.AdditionalCompilerOptions.Add("-std=c++2b", "-frtti");

            conf.Defines.Add("SPECTRUM_PLATFORM_ANDROID");
            conf.Defines.Add("SPECTRUM_ENABLE_EXCEPTIONS");
            conf.Defines.Add("HAL_BACKEND_VULKAN");

            if (target.Mode == Mode.Debug)
            {
                conf.Defines.Add("DEV");
                conf.Defines.Add("PROFILING");
            }
            else
            {
                conf.Defines.Add("RETAIL");
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

            if (target.Platform == Platform.agde)
            {
                // Windows/DirectX/NVIDIA wrappers can't exist on Android. The vcpkg-backed
                // ones are off until their arm64-android builds are set up.
                foreach (var w in new[] { "windows", "WRL", "ppl", "d3d12", "dxc", "streamline", "nrd", "vulkan",
                                          "assimp", "antlr4", "jinja2cpp", "meshoptimizer", "skribidi", "zlib" })
                    conf.SourceFilesBuildExcludeRegex.Add(@".*\\Modules\\" + w + @"\\.*");
                return;
            }

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
			conf.AddPublicDependency<Modules>(target);	
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

            conf.AddPublicDependency<Core>(target);
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
            conf.AddPublicDependency<HAL>(target);

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

            conf.AddPublicDependency<Core>(target);	
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

            conf.AddPublicDependency<RenderSystem>(target);
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

            conf.AddPublicDependency<RenderSystem>(target);
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
            conf.AddProject<Resources>(target);
        }

        [Sharpmake.Main]
        public static void SharpmakeMain(Sharpmake.Arguments arguments)
        {
            bool android = AndroidSetup.Configure();
            FastBuildSettings.FastBuildMakeCommand = @"[project.SharpmakeCsPath]\tools\FastBuild\FBuild.exe";
            arguments.Generate<SpectrumSolution>();
            if (android)
                arguments.Generate<SpectrumAndroidSolution>();
        }
    }

    // Separate from SpectrumSolution: the generator emits an Android platform entry
    // for every solution target, so mixing would force an Android architecture
    // onto the Windows targets (and rename their configs).
    [Sharpmake.Generate]
    public class SpectrumAndroidSolution : Solution
    {
        public SpectrumAndroidSolution()
        : base(typeof(CustomTarget))
        {
            AddTargets(Common.AndroidTarget());
        }

        [Configure()]
        public void ConfigureAll(Configuration conf, CustomTarget target)
        {
            conf.SolutionFileName = "SpectrumAndroid";
            conf.SolutionPath = @"[solution.SharpmakeCsPath]\projects\";
            conf.Name = target.Mode + "-Android";

            conf.AddProject<Core>(target);   // pulls in Modules; grows as more layers port
        }
    }
}
