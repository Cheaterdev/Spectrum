using Sharpmake;
using System;
using System.Collections.Generic;

namespace Spectrum
{
    [Fragment, Flags]
    public enum Mode
    {
        Dev = 1,
        Profile = 2,
        Retail = 4
    }

    public class CustomTarget : ITarget
    {

        public Platform Platform; 
        public DevEnv DevEnv;
        public Optimization Optimization;
        public Mode Mode;
    }


    [Sharpmake.Generate]
    public class Common : Project
    {
        public Common() : base(typeof(CustomTarget))
        {
            SourceFilesExtensions.Add(".sig");
            SourceFilesExtensions.Add(".hlsl");
			SourceFilesExtensions.Add(".ixx");
	        SourceFilesExtensions.Add(".g4");

			SourceFilesCompileExtensions.Add(".ixx");

            RootPath = @"[project.SharpmakeCsPath]\projects\[project.Name]";

            AddTargets(new CustomTarget
            {
                Platform = Platform.win64,
                DevEnv = DevEnv.vs2022,
                Optimization = Optimization.Release,
                Mode = Mode.Dev | Mode.Profile | Mode.Retail
            });

            CustomProperties.Add("VcpkgEnabled", "true");
            CustomProperties.Add("VcpkgEnableManifest", "true");
			CustomProperties.Add("VcpkgTriplet", "x64-windows");
			CustomProperties.Add("VcpkgConfiguration", "Release");

         //   BlobWorkFileCount = 8;
        //    GeneratableBlobCount  = 8;
        }

        [Configure]
        public virtual void ConfigureAll(Configuration conf, CustomTarget target)
        {
            conf.ProjectFileName = "[project.Name]";
            conf.ProjectPath = @"[project.RootPath]";

            conf.IncludePaths.Add(SourceRootPath);
		//	conf.ExportAdditionalLibrariesEvenForStaticLib = true;
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

            conf.Defines.Add("_MBCS");
			conf.Defines.Add("BOOST_NO_USER_CONFIG");
			conf.Defines.Add("BOOST_ENDIAN_DEPRECATED_NAMES");
			conf.Defines.Add("BOOST_ALL_NO_LIB");
			conf.Defines.Add("BOOST_NO_CXX11_NOEXCEPT");
			
			conf.Defines.Add("_SCL_SECURE_NO_WARNINGS");
            conf.Defines.Add("_CRT_SECURE_NO_WARNINGS");
            conf.Defines.Add("NOMINMAX");
            conf.Defines.Add("_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING");
       
			conf.Defines.Add("WIN32_LEAN_AND_MEAN");
            conf.Defines.Add("SPECTRUM_ENABLE_EXEPTIONS");
            conf.Defines.Add("CEREAL_THREAD_SAFE");
            conf.Defines.Add("USE_PIX");
         
            conf.Options.Add(new Sharpmake.Options.Vc.Compiler.DisableSpecificWarnings("4005", "5104", "5105", "5106", "4494")); //module reference issues

            if (target.Mode == Mode.Dev)
            {
                conf.Options.Add(Options.Vc.Compiler.Optimization.Disable);
                conf.Options.Add(Options.Vc.Compiler.Inline.Disable);
                conf.Defines.Add("DEV");
                conf.Defines.Add("PROFILING");
            }
            else
            {
                conf.Options.Add(Options.Vc.Linker.LinkTimeCodeGeneration.UseLinkTimeCodeGeneration);
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
    }



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
            conf.LibraryFiles.Add(@"[project.SourceRootPath]\lib\x64\GFSDK_Aftermath_Lib.x64.lib");
            conf.TargetCopyFiles.Add(@"[project.SourceRootPath]\lib\x64\GFSDK_Aftermath_Lib.x64.dll");
            conf.IncludePaths.Add(@"[project.SourceRootPath]/include");
        }
    }
	
	
[Sharpmake.Generate]
    public class Modules : Library
    {
        public Modules()
        {
            SourceRootPath = @"[project.SharpmakeCsPath]\sources\Modules";
            AssemblyName = "Modules";
			
			CustomProperties.Add("ScanSourceForModuleDependencies", "true");
        }


        public override void ConfigureAll(Configuration conf, CustomTarget target)
        {
            base.ConfigureAll(conf, target);
            conf.IncludePaths.Add(@"[project.SourceRootPath]/include");

			conf.ExportAdditionalLibrariesEvenForStaticLib = false;

			
			{ // AgilitySDK
				conf.IncludePaths.Add(@"[project.SharpmakeCsPath]\AgilitySDK\build\native\include", 66);
				conf.TargetCopyFilesToSubDirectory.Add(new KeyValuePair<string, string>(@"[project.SharpmakeCsPath]\AgilitySDK\build\native\bin\x64\D3D12Core.dll", "D3D12"));

				if(target.Mode != Mode.Retail)
				{
					conf.TargetCopyFilesToSubDirectory.Add(new KeyValuePair<string, string>(@"[project.SharpmakeCsPath]\AgilitySDK\build\native\bin\x64\D3D12SDKLayers.dll", "D3D12"));
				}
			}


            { // PIX
				conf.IncludePaths.Add(@"[project.SharpmakeCsPath]\PIX\Include\WinPixEventRuntime", 66);
                conf.TargetCopyFiles.Add(@"[project.SharpmakeCsPath]\PIX\bin\x64\WinPixEventRuntime.dll");
			
			    conf.LibraryFiles.Add("WinPixEventRuntime.lib");
                conf.LibraryPaths.Add(@"[project.SharpmakeCsPath]\PIX\bin\x64", 66);
			}



            // fix: dstorage vcpkg issue -> copy manually
            conf.TargetCopyFiles.Add(@"[project.SharpmakeCsPath]\vcpkg_installed\x64-windows\x64-windows\bin\dstoragecore.dll");
           conf.TargetCopyFiles.Add(@"[project.SharpmakeCsPath]\vcpkg_installed\x64-windows\x64-windows\bin\dxcompiler.dll");
         conf.TargetCopyFiles.Add(@"[project.SharpmakeCsPath]\vcpkg_installed\x64-windows\x64-windows\bin\dxil.dll");
         
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

            conf.LibraryFiles.Add("dxgi.lib");
            conf.LibraryFiles.Add("d3d12.lib");
            conf.LibraryFiles.Add("dxguid.lib");

            conf.AddPublicDependency<Core>(target);	
            conf.AddPrivateDependency<Aftermath>(target);
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

        // conf.IsBlobbed = true;
        
            conf.AddPublicDependency<HAL>(target);
        }
    }

    [Sharpmake.Generate]
    public class SIGParser : Application
    {
        public SIGParser()
        {
            SourceRootPath = @"[project.SharpmakeCsPath]\sources\SIGParser";
            AssemblyName = "SIGParser";
        }

        public override void ConfigureAll(Configuration conf, CustomTarget target)
        {
            base.ConfigureAll(conf, target);
            conf.Options.Remove(Options.Vc.Linker.SubSystem.Windows);
            conf.Options.Add(Options.Vc.General.CharacterSet.Unicode);
            conf.Options.Remove(Options.Vc.General.WarningLevel.Level3);		 // hate warnings, love errors    
            conf.Options.Add(Options.Vc.General.WarningLevel.Level0);        // hate warnings, love errors


            conf.VcxprojUserFile = new Project.Configuration.VcxprojUserFileSettings();
            conf.VcxprojUserFile.LocalDebuggerWorkingDirectory = @"[project.SharpmakeCsPath]\sources\SIGParser";
            conf.Defines.Add("ANTLR4CPP_STATIC");
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

            conf.LibraryFiles.Add("Onecore.lib");

            conf.VcxprojUserFile = new Project.Configuration.VcxprojUserFileSettings();
            conf.VcxprojUserFile.LocalDebuggerWorkingDirectory = @"[project.SharpmakeCsPath]\workdir";

            conf.AddPublicDependency<HAL>(target);
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
                DevEnv = DevEnv.vs2022,
                Optimization =  Optimization.Release,
                Mode = Mode.Dev | Mode.Profile | Mode.Retail
            });
        }

        [Configure()]
        public void ConfigureAll(Configuration conf, CustomTarget target)
        {
            conf.SolutionFileName = "Spectrum";
            conf.SolutionPath = @"[solution.SharpmakeCsPath]\projects\";
            string platformName = string.Empty;
/*
			switch (target.Optimization)
            {
                case Optimization.Debug: platformName += "Debug "; break;
                case Optimization.Release: platformName += "Release "; break;
                default:
                    throw new NotImplementedException();
            }
			*/
			
            switch (target.Mode)
            {
                case Mode.Dev: platformName += "Dev"; break;
                case Mode.Retail: platformName += "Retail"; break;
                case Mode.Profile: platformName += "Profile"; break;
                default:
                    throw new NotImplementedException();
            }
            conf.Name = platformName;

            conf.AddProject<Spectrum>(target);
            conf.AddProject<SIGParser>(target);
            conf.AddProject<Resources>(target);
        }

        [Sharpmake.Main]
        public static void SharpmakeMain(Sharpmake.Arguments arguments)
        {
            FastBuildSettings.FastBuildMakeCommand = @"[project.SharpmakeCsPath]\tools\FastBuild\FBuild.exe";
            arguments.Generate<SpectrumSolution>();
        }
    }
}
