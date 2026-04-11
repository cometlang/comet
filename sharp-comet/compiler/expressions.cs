using sharpcomet.lexer;
using sharpcomet.stdlib;
using sharpcomet.vmlib;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using vmlib;

namespace sharpcomet.compiler;

public partial class Parser
{

    private const int MAX_ARG_COUNT = 255;

    private void ParseVariable(bool canAssign)
    {
        int variable = CurrentFunction.ResolveLocal(Current.Representation);
    }

    private void ParseString(bool canAssign)
    {
        CurrentFunction.EmitConstant(Globals.InternString(Previous.Representation));
    }

    private void ParseNumber(bool canAssign)
    {
        if (!double.TryParse(Previous.Representation, out var value))
        {
            ErrorAt(Current, "Unable to parse number value");
        }
        else
        {
            CurrentFunction.EmitConstant(Memory.AllocateObject<Number>(value));
        }
    }

    private void Replacement(bool canAssign)
    {
        CurrentFunction.EmitConstant(Globals.InternString(_scanner.Filename));
    }

    private void Grouping(bool canAssign)
    {
        Expression();
        Consume(TokenType.RightParen, "Expected ')' after expression");
    }

    private byte ArgumentList(TokenType closingToken)
    {
        byte argCount = 0;
        if (!Check(closingToken))
        {
            do
            {
                if (Match(TokenType.EndOfLine) && Check(closingToken))
                    break;

                Expression();
                argCount++;
                if (argCount == MAX_ARG_COUNT)
                {
                    Error($"Cannot have more than {MAX_ARG_COUNT} arguments");
                }
            } while (Match(TokenType.Comma));
        }

        Match(TokenType.EndOfLine);

        Consume(closingToken, $"Expected '{closingToken.GetRepresentation()}' after arguments");

        return argCount;
    }

    private void Call(bool canAssign)
    {
        byte argCount = ArgumentList(TokenType.RightParen);
        CurrentFunction.EmitBytes((byte)Op.Call,  argCount);
    }

    private Dictionary<TokenType, ParseRule> _parseRules;

    [MemberNotNull(nameof(_parseRules))]
    private void InitialiseParseRules()
    {
        _parseRules = new Dictionary<TokenType, ParseRule>()
        {
            // Single-character tokens.
            {TokenType.LeftParen,  new ParseRule(Grouping,      Call, Precedence.Call) },
            {TokenType.RightParen, new ParseRule(null,          null, Precedence.None) },
            // Literals
            {TokenType.Identifier, new ParseRule(Variable,      null, Precedence.None) },
            {TokenType.Var,        new ParseRule(ParseVariable, null, Precedence.None) },
            {TokenType.String,     new ParseRule(ParseString,   null, Precedence.None) },
            {TokenType.Number,     new ParseRule(ParseNumber,   null, Precedence.None) },
            {TokenType.EndOfLine,  new ParseRule(null,          null, Precedence.None) },
            {TokenType.EndOfFile,  new ParseRule(null,          null, Precedence.None) },
            {TokenType.Filename,   new ParseRule(Replacement,   null, Precedence.None) },
        };
    }

    private ParseRule GetRule(TokenType tokenType)
    {
        if (_parseRules.ContainsKey(tokenType))
        {
            return _parseRules[tokenType];
        }
        ErrorAtCurrent("No rule to parse token");
        return new ParseRule();
    }

    private void ParsePrecedence(Precedence precedence)
    {
        Match(TokenType.EndOfLine);
        Advance();
        var prefixRule = GetRule(Previous.TokenType).Prefix;
        if (prefixRule == null)
        {
            Error("Expect expression");
            return;
        }

        bool canAssign = precedence <= Precedence.Assignment;
        prefixRule(canAssign);

        while (precedence <= GetRule(Current.TokenType).Precedence)
        {
            Advance();
            var infixRule = GetRule(Previous.TokenType).Infix;
            // TODO do I need to error if the infix rule is null?
            if (infixRule != null)
            {
                infixRule(canAssign);
            }
        }

        if (canAssign && Match(TokenType.Equal))
        {
            Error("Invalid assignment target");
        }
    }

    private void Expression()
    {
        ParsePrecedence(Precedence.Assignment);
    }
}