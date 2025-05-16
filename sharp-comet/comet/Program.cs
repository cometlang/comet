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
            var content = File.ReadAllText(arg);
            var scanner = new Scanner(new Source(arg, content));
            var parser = new Parser(scanner);

            while (parser.Current?.TokenType != TokenType.EndOfFile)
            {
                parser.Advance();
            }
        }
    }
}
