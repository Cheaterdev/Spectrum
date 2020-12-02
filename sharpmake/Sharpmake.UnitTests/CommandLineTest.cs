﻿// Copyright (c) 2017 Ubisoft Entertainment
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
using NUnit.Framework;

namespace Sharpmake.UnitTests
{
    public class CommandLineTest
    {
        [Test, Sequential]
        public void ParsesOneParameter([Values("/toto",
                                               "  \t   \t/\t   \ttoto\t  ",
                                               "/toto()")] string commandLine)
        {
            CommandLine.Parameter[] p = CommandLine.Parse(commandLine);
            Assert.That(p, Has.Length.EqualTo(1));
            Assert.That(p[0].Name, Is.EqualTo("toto"));
            Assert.That(p[0].Args, Is.Empty);
        }

        [Test]
        public void ParsesOneParameterWithVoidArgument()
        {
            CommandLine.Parameter[] p = CommandLine.Parse("/toto\t   \t\t(  \t\t ) \t\t \t");
            Assert.That(p, Has.Length.EqualTo(1));
            Assert.That(p[0].Name, Is.EqualTo("toto"));
            Assert.That(p[0].Args, Is.Empty);
            Assert.That(p[0].ArgsCount, Is.EqualTo(0));
        }

        [Test, Sequential]
        public void ParsesTwoParameters([Values("/toto /tata", " /  toto   /  tata  ", " /  toto (    )  /  tata (   )  ")] string commandLine)
        {
            CommandLine.Parameter[] p = CommandLine.Parse(commandLine);
            Assert.That(p, Has.Length.EqualTo(2));

            Assert.That(p[0].Name, Is.EqualTo("toto"));
            Assert.That(p[0].Args, Is.Empty);
            Assert.That(p[0].ArgsCount, Is.EqualTo(0));

            Assert.That(p[1].Name, Is.EqualTo("tata"));
            Assert.That(p[1].Args, Is.Empty);
            Assert.That(p[1].ArgsCount, Is.EqualTo(0));
        }

        [Test]
        public void ParsesOneParameterWithArgument()
        {
            CommandLine.Parameter[] p = CommandLine.Parse("/toto(1)");

            Assert.That(p, Has.Length.EqualTo(1));

            Assert.That(p[0].Name, Is.EqualTo("toto"));
            Assert.That(p[0].Args, Is.EqualTo("1"));
            Assert.That(p[0].ArgsCount, Is.EqualTo(1));
        }

        [Test]
        public void ParsesOneParameterWithComplexeArgument()
        {
            CommandLine.Parameter[] p = CommandLine.Parse("/toto(project.ToString())");

            Assert.That(p, Has.Length.EqualTo(1));

            Assert.That(p[0].Name, Is.EqualTo("toto"));
            Assert.That(p[0].Args, Is.EqualTo("project.ToString()"));
            Assert.That(p[0].ArgsCount, Is.EqualTo(1));
        }

        [Test]
        public void ParsesTwoParametersWithComplexeArguments()
        {
            CommandLine.Parameter[] p = CommandLine.Parse("/toto(project.ToString())/tata(project2.ToString())");

            Assert.That(p, Has.Length.EqualTo(2));

            Assert.That(p[0].Name, Is.EqualTo("toto"));
            Assert.That(p[0].Args, Is.EqualTo("project.ToString()"));
            Assert.That(p[0].ArgsCount, Is.EqualTo(1));

            Assert.That(p[1].Name, Is.EqualTo("tata"));
            Assert.That(p[1].Args, Is.EqualTo("project2.ToString()"));
            Assert.That(p[1].ArgsCount, Is.EqualTo(1));
        }

        public class Foo
        {
            public string ReceivedString = "";
            public int ReceivedInt;

            [CommandLine.Option("Test")]
            public void CommandLineTest(int p0)
            {
                ReceivedInt = p0;
            }

            [CommandLine.Option("Test")]
            public void CommandLineTest(string p0)
            {
                ReceivedString = p0;
            }
        }

        [Test]
        public void CanCallMethodsWithOptionAttribute()
        {
            var foo = new Foo();

            Assert.That(foo.ReceivedInt, Is.EqualTo(0));
            Assert.That(foo.ReceivedString, Is.Empty);

            CommandLine.ExecuteOnObject(foo, "/Test(\"123123\")");
            CommandLine.ExecuteOnObject(foo, "/Test(123123)");

            Assert.That(foo.ReceivedInt, Is.EqualTo(123123));
            Assert.That(foo.ReceivedString, Is.EqualTo("123123"));
        }
    }
}

