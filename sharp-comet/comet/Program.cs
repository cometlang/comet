using System;
using sharpcomet.lexer;
using sharpcomet.compiler;
using sharpcomet.vmlib;
using sharpcomet.vm;
using System.Linq;

class Program
{
    public static void Main(string[] args)
    {
        if (args.Length == 0)
        {
            Console.Error.WriteLine("No file to interpret");
            Environment.Exit(1);
        }
        var sourceFile = SourceFile.Create(args.First());
        var compilationResult = Compiler.Compile(sourceFile);
        if (compilationResult is CometFunction function)
        {
            var vm = new VirtualMachine(function);
            var result = vm.Run();
            Environment.ExitCode = (int)result;
        }
        else
        {
            Environment.ExitCode = (int)InterpretResult.CompilationError;
        }
    }
}
