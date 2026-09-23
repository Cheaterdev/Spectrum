REM Regenerates the ANTLR parser from sources/Prism/Prism.g4 (only needed after a grammar change).
REM This does NOT regenerate code from .prism files: that is prismc.exe, or Tools > Regenerate Prism code in VS.
SET CLASSPATH=%CD%\sources\Prism\antlr-4.11.1-complete.jar;%CLASSPATH%
java org.antlr.v4.Tool sources/Prism/Prism.g4 -o sources/Prism/.antlr/ -listener -visitor
