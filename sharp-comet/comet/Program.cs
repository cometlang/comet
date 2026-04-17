using System;
using sharpcomet.lexer;
using sharpcomet.compiler;
using sharpcomet.vmlib;
using sharpcomet.vm;
using System.Linq;
using vmlib;

class Program
{
    public static void Main(string[] args)
    {
        if (args.Length == 0)
        {
            Console.Error.WriteLine("No file to interpret");
            Environment.Exit(1);
        }
        var compileOnly = false;
        if (args.First() == "--compile-only" ||  args.First() == "-c")
        {
            compileOnly = true;
        }
        var sourceFile = SourceFile.Create(args.Last());
        var compilationResult = Compiler.Compile(sourceFile);
        if (compileOnly)
        {
            if (compilationResult is not CometFunction)
            {
                Environment.ExitCode = (int)InterpretResult.CompilationError;
            }
        }
        else if (compilationResult is CometFunction function)
        {
            Initialisation.InitStdLib(); // pretty sure I can get away with doing this after compilation
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
