using System;
using System.IO;
using sharpcomet.lexer;

class Program
{
    public static void Main(string[] args)
    {
        Console.WriteLine("Inside Main!");
        foreach (var arg in args)
        {
            var content = File.ReadAllText(arg);
            var scanner = new Scanner(content);
            Token token = scanner.ScanToken();
            while (token.TokenType != TokenType.EndOfFile)
            {
                token = scanner.ScanToken();
            }
        }
    }
}
