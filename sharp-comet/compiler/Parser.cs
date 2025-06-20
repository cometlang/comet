using System;
using sharpcomet.lexer;
using sharpcomet.stdlib;

namespace sharpcomet.compiler;

public partial class Parser
{
    private bool _hadError;
    private bool _panicMode;
    private readonly Scanner _scanner;


    // typedef struct
    // {
    //     ClassCompiler *currentClass;
    //     LoopCompiler *currentLoop;
    //     VALUE currentModule;
    //     VM *compilation_thread;
    // } Parser;
    public Parser(Scanner scanner)
    {
        _scanner = scanner;
        InitialiseParseRules();
    }

    private FunctionCompiler? CurrentFunction { get; set; }
    private ClassCompiler? CurrentClass { get; set; }
    private LoopCompiler? CurrentLoop { get; set; }

    private Token? Current { get; set; }
    private Token? Previous { get; set; }

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

    private bool Match(TokenType tokenType)
    {
        if (!Check(tokenType))
            return false;

        Advance();
        return true;
    }

    private bool Check(TokenType tokenType)
    {
        return Current?.TokenType == tokenType;
    }

    private void Consume(TokenType tokenType, string message)
    {
        if (Current?.TokenType == tokenType)
        {
            Advance();
        }
        else
        {
            ErrorAtCurrent(message);
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

    private void Synchronize()
    {
        _panicMode = false;

        while (Current?.TokenType != TokenType.EndOfFile)
        {
            if (Previous?.TokenType == TokenType.EndOfLine)
                return;

            switch (Current?.TokenType)
            {
                case TokenType.Class:
                case TokenType.Function:
                case TokenType.Var:
                case TokenType.Operator:
                case TokenType.For:
                case TokenType.ForEach:
                case TokenType.If:
                case TokenType.While:
                case TokenType.Throw:
                case TokenType.Return:
                case TokenType.Static:
                case TokenType.Try:
                case TokenType.Import:
                case TokenType.Enum:
                    return;

                default:
                    // Do nothing.
                    break;
            }

            Advance();
        }
    }

    public CometObject Parse()
    {
        var functionCompiler = new FunctionCompiler(null, FunctionType.Script);

        Advance();
        while (!Match(TokenType.EndOfFile))
        {
            Declaration();
        }

        if (_hadError)
        {
            return Nil.Instance;
        }

        return null;
        //     Compiler compiler;
        //     initCompiler(&compiler, TYPE_SCRIPT, &parser);

        //     // chicken-egg situation.
        //     advance(&parser);
        //     while (!match(&parser, TOKEN_EOF))
        //     {
        //         declaration(&parser);
        //     }
        //     ObjFunction *function = endCompiler(&parser);
        //     module_set_main(function->module, function);
        //     pop(thread);

        //     return parser.hadError ? NIL_VAL : function->module;

    }

}

