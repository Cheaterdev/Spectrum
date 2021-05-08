// Copyright (c) 2017 Ubisoft Entertainment
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

using Sharpmake;
using System;

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
	  // DevEnv and Platform are mandatory on all targets so we define them.
    public Platform Platform;    public DevEnv DevEnv;


    // Also put debug/release configurations since this is common.
    public Optimization Optimization;
    public Mode Mode;
}


    [Sharpmake.Generate]
    public class Common : Project
    {
        public Common(): base(typeof(CustomTarget))
        {
            SourceFilesExtensions.Add(".sig");
            SourceFilesExtensions.Add(".hlsl");

            RootPath = @"[project.SharpmakeCsPath]\projects\[project.Name]";

            AddTargets(new CustomTarget{
                Platform = Platform.win64,
               DevEnv =  DevEnv.vs2019,
              Optimization =   Optimization.Release,
				Mode = Mode.Dev | Mode.Profile | Mode.Retail
            });

        }

        [Configure]
        public virtual void ConfigureAll(Configuration conf, CustomTarget target)
        {
            conf.ProjectFileName = "[project.Name]";
            conf.ProjectPath = @"[project.RootPath]";

            conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.Latest);		
            conf.Options.Add(Options.Vc.Compiler.Exceptions.Enable);		
            conf.Options.Add(Options.Vc.Compiler.RTTI.Enable);		
            conf.Options.Add(Options.Vc.Linker.SubSystem.Application);		
            conf.Options.Add(Options.Vc.General.WindowsTargetPlatformVersion.Latest);		
            conf.Options.Add(Options.Vc.Compiler.FunctionLevelLinking.Disable);		
            conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDLL);		
            conf.Options.Add(Options.Vc.Compiler.Inline.OnlyInline);		     
            conf.Options.Add(Options.Vc.General.WarningLevel.Level3);		 // hate warnings, love errors

            conf.AdditionalCompilerOptions.Add("/bigobj");

            conf.Defines.Add("_MBCS");
            conf.Defines.Add("_SCL_SECURE_NO_WARNINGS");
            conf.Defines.Add("_CRT_SECURE_NO_WARNINGS");
            conf.Defines.Add("BOOST_NO_USER_CONFIG");
            conf.Defines.Add("_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING");
            conf.Defines.Add("BOOST_ENDIAN_DEPRECATED_NAMES");
          
            conf.Defines.Add("BOOST_ALL_NO_LIB");
            conf.Defines.Add("NOMINMAX");    


			if (target.Mode == Mode.Dev)
			{
				conf.Options.Add(Options.Vc.Compiler.Optimization.Disable);		
				conf.Options.Add(Options.Vc.Compiler.Inline.Disable);	
                conf.Defines.Remove("NDEBUG");	
                conf.Defines.Add("DEV");
				conf.Defines.Add("PROFILING");
			}else{
				conf.Options.Add(Options.Vc.Linker.LinkTimeCodeGeneration.UseLinkTimeCodeGeneration);				
				conf.Options.Add(Options.Vc.Compiler.Optimization.FullOptimization);		
				conf.Defines.Add("RETAIL");	
			}
    
			if (target.Mode == Mode.Profile)
			{
				conf.Defines.Add("PROFILING");	
			}
			
			
			
			 conf.ReferencesByNuGetPackage.Add(
                "Microsoft.Direct3D.D3D12", "1.4.9"
            );
			
			
        }
        
     
    }

     [Sharpmake.Generate]
    public class Library : Common
    {
        public Library()
        {
   
        }

        public override void  ConfigureAll(Configuration conf, CustomTarget target)
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
    public class ZipLib : Library
    {
        public ZipLib()
        { 
            SourceRootPath = @"[project.SharpmakeCsPath]\sources\3rdparty\ZipLib";
            AssemblyName = "ZipLib";
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

            conf.PrecompHeader = "pch.h";
            conf.PrecompSource = "pch.cpp"; 

            conf.LibraryFiles.Add("Dbghelp.lib");
            conf.LibraryFiles.Add("version.lib");

            conf.AddPrivateDependency<ZipLib>(target);     
        }
    }

    [Sharpmake.Generate]
    public class FileSystem : Library
    {
        public FileSystem()
        { 
            SourceRootPath = @"[project.SharpmakeCsPath]\sources\FileSystem";
            AssemblyName = "FileSystem";
        }

        public override void ConfigureAll(Configuration conf, CustomTarget target) 
        {
            base.ConfigureAll(conf, target);

            conf.PrecompHeader = "pch.h";
            conf.PrecompSource = "pch.cpp";    

            conf.AddPrivateDependency<Core>(target);     
        }
    }

    [Sharpmake.Generate]
    public class DirectXFramework : Library
    {
        public DirectXFramework()
        { 
            SourceRootPath = @"[project.SharpmakeCsPath]\sources\DirectXFramework";
            AssemblyName = "DirectXFramework";
        }

        public override void ConfigureAll(Configuration conf, CustomTarget target) 
        {
            base.ConfigureAll(conf, target);

            conf.PrecompHeader = "pch.h";
            conf.PrecompSource = "pch.cpp";    


            conf.LibraryFiles.Add("dxgi.lib");
            conf.LibraryFiles.Add("d3d12.lib");
            conf.LibraryFiles.Add("dxguid.lib");    
            conf.LibraryFiles.Add("dxcompiler.lib");    

            conf.TargetCopyFiles.Add(@"[project.SourceRootPath]\dxcompiler.dll");
            conf.TargetCopyFiles.Add(@"[project.SourceRootPath]\dxil.dll");
            
         
            conf.AddPrivateDependency<FileSystem>(target);     
            conf.AddPublicDependency<Aftermath>(target);     
         
        }
    }

    [Sharpmake.Generate]
    public class FlowGraph : Library
    {
        public FlowGraph()
        { 
            SourceRootPath = @"[project.SharpmakeCsPath]\sources\FlowGraph";
            AssemblyName = "FlowGraph";
        }

        public override void ConfigureAll(Configuration conf, CustomTarget target) 
        {
            base.ConfigureAll(conf, target);

            conf.PrecompHeader = "pch.h";
            conf.PrecompSource = "pch.cpp";    

            conf.AddPrivateDependency<DirectXFramework>(target);     
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

            conf.PrecompHeader = "pch.h";
            conf.PrecompSource = "pch.cpp";    

            conf.AddPrivateDependency<DirectXFramework>(target);    
            conf.AddPrivateDependency<FlowGraph>(target);               
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
            conf.Options.Remove(Options.Vc.Compiler.CppLanguageStandard.Latest);	
            conf.Options.Remove(Options.Vc.Compiler.CppLanguageStandard.Latest);	
            conf.Options.Remove(Options.Vc.Linker.SubSystem.Application);		

            conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP17);	
            conf.Options.Add(Options.Vc.General.CharacterSet.Unicode);	  

                
            conf.Options.Remove(Options.Vc.General.WarningLevel.Level3);		 // hate warnings, love errors    
            conf.Options.Add(Options.Vc.General.WarningLevel.Level0);		 // hate warnings, love errors


  conf.VcxprojUserFile = new Project.Configuration.VcxprojUserFileSettings ();
            conf.VcxprojUserFile.LocalDebuggerWorkingDirectory= @"[project.SharpmakeCsPath]\sources\SIGParser";

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

            conf.PrecompHeader = "pch.h";
            conf.PrecompSource = "pch.cpp";    
            
            conf.VcxprojUserFile = new Project.Configuration.VcxprojUserFileSettings ();
            conf.VcxprojUserFile.LocalDebuggerWorkingDirectory = @"[project.SharpmakeCsPath]\workdir";
            
            conf.AddPrivateDependency<DirectXFramework>(target);    
            conf.AddPrivateDependency<RenderSystem>(target);             
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
            AddTargets(new CustomTarget{
               Platform =  Platform.win64,
               DevEnv =  DevEnv.vs2019,
              Optimization =   Optimization.Release,       
			Mode = 	Mode.Dev | Mode.Profile | Mode.Retail
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
