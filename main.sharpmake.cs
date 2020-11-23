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
    [Sharpmake.Generate]
    public class Common : Project
    {
        public Common()
        {
            SourceFilesExtensions.Add(".sig");
            SourceFilesExtensions.Add(".hlsl");

            RootPath = @"[project.SharpmakeCsPath]\projects\[project.Name]";

            AddTargets(new Target(
                Platform.win64,
                DevEnv.vs2019,
                Optimization.Debug | Optimization.Release,
                OutputType.Lib,
                Blob.NoBlob,
                BuildSystem.MSBuild
            ));

        }

        [Configure()]
        public virtual void ConfigureAll(Configuration conf, Target target)
        {
            conf.ProjectFileName = "[project.Name].[target.DevEnv].[target.Framework]";
            conf.ProjectPath = @"[project.RootPath]";

            conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.Latest);		
            conf.Options.Add(Options.Vc.Compiler.Exceptions.Enable);		
            conf.Options.Add(Options.Vc.Compiler.RTTI.Enable);		
            conf.Options.Add(Options.Vc.Linker.SubSystem.Application);		
            conf.Options.Add(Options.Vc.General.WindowsTargetPlatformVersion.Latest);		
            conf.Options.Add(Options.Vc.Compiler.FunctionLevelLinking.Disable);		
            conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDLL);		
            conf.Options.Add(Options.Vc.Compiler.Optimization.Disable);		
            conf.Options.Add(Options.Vc.Compiler.Inline.OnlyInline);		
            conf.Options.Add(Options.Vc.General.Vcpkg.True);		
          
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
        }
        
        [Configure(BuildSystem.FastBuild)]
        public void ConfigureFastBuild(Configuration conf, Target target)
        {
            conf.IsFastBuild = true;
            conf.FastBuildBlobbed = target.Blob == Blob.FastBuildUnitys;
        }
    }

     [Sharpmake.Generate]
    public class Library : Common
    {
        public Library()
        {
   
        }

        public override void  ConfigureAll(Configuration conf, Target target)
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
    public class Core : Library
    {
        public Core()
        { 
            SourceRootPath = @"[project.SharpmakeCsPath]\sources\Core";
            AssemblyName = "Core";
        }

        public override void ConfigureAll(Configuration conf, Target target) 
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

        public override void ConfigureAll(Configuration conf, Target target) 
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

        public override void ConfigureAll(Configuration conf, Target target) 
        {
            base.ConfigureAll(conf, target);

            conf.PrecompHeader = "pch.h";
            conf.PrecompSource = "pch.cpp";    


            conf.LibraryFiles.Add("dxgi.lib");
            conf.LibraryFiles.Add("d3d12.lib");
            conf.LibraryFiles.Add("dxguid.lib");    
            conf.LibraryFiles.Add("dxcompiler.lib");    

            conf.TargetCopyFiles.Add(@"[project.SourceRootPath]\dxcompiler.dll");

            conf.AddPrivateDependency<FileSystem>(target);     
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

        public override void ConfigureAll(Configuration conf, Target target) 
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

        public override void ConfigureAll(Configuration conf, Target target) 
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

  public override void ConfigureAll(Configuration conf, Target target) 
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

  public override void ConfigureAll(Configuration conf, Target target) 
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
        {
            AddTargets(new Target(
                Platform.win64,
                DevEnv.vs2019,
                Optimization.Debug | Optimization.Release,
                OutputType.Lib,
                Blob.NoBlob,
                BuildSystem.MSBuild
            ));
        }

        [Configure()]
        public void ConfigureAll(Configuration conf, Target target)
        {
            conf.SolutionFileName = "Spectrum";
            conf.SolutionPath = @"[solution.SharpmakeCsPath]\projects\";

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
