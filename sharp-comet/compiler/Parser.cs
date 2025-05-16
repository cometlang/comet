using System;
using sharpcomet.lexer;

namespace sharpcomet.compiler;

public class Parser
{
    private bool _hadError;
    private bool _panicMode;
    private readonly Scanner _scanner;

// typedef struct
// {
//     ClassCompiler *currentClass;
//     LoopCompiler *currentLoop;
//     VALUE currentModule;
//     Compiler *currentFunction;
//     VM *compilation_thread;
// } Parser;
    public Parser(Scanner scanner)
    {
        _scanner = scanner;
    }

    public Token? Current { get; private set; }
    public Token? Previous { get; private set; }

    public void Advance()
    {
        Previous = Current;
        while (true)
        {
            Current = _scanner.ScanToken();
            if (Current.TokenType != TokenType.Error)
                break;

            ErrorAtCurrent(Current.Representation);
        }
    }

    private void ErrorAt(Token token, string message)
    {
        if (_panicMode)
            return;

        _panicMode = true;
        Console.Error.Write("[{0}:{1}]", _scanner.Filename, token.LineNumer);

        if (token.TokenType == TokenType.EndOfFile)
        {
            Console.Error.WriteLine(" at end");
        }
        else if (token.TokenType == TokenType.Error)
        {
            // Nothing
        }
        else
        {
            Console.Error.Write(" at '{0}'", token.Representation);
        }
        Console.Error.WriteLine(": {0}", message);
        _hadError = true;
    }

    private void ErrorAtCurrent(string message)
    {
        ErrorAt(Current, message);
    }

    private void Error(string message)
    {
        ErrorAt(Previous, message);
    }
}

