SET CLASSPATH=%CD%\sources\SIGParser\antlr-4.9.2-complete.jar;%CLASSPATH%
java org.antlr.v4.Tool sources/SIGParser/SIG.g4 -o sources/SIGParser/.antlr/ -listener -visitor