using System;
using System.IO;
using sharpcomet.lexer;
using sharpcomet.compiler;

class Program
{
    public static void Main(string[] args)
    {
        foreach (var arg in args)
        {
            var sourceFile = SourceFile.Create(arg);
            Compiler.Compile(sourceFile);
        }
    }
}
