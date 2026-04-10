using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using sharpcomet.lexer;
using sharpcomet.stdlib;
using sharpcomet.vmlib;

namespace sharpcomet.compiler;

public partial class Parser
{

    private void ParseVariable(bool canAssign)
    {
        int variable = CurrentFunction.ResolveLocal(Current.Representation);
    }

    private void ParseString(bool canAssign)
    {
        CurrentFunction.EmitConstant(Strings.InternString(Previous.Representation));
    }

    private void ParseNumber(bool canAssign)
    {
        if (!double.TryParse(Previous.Representation, out var value))
        {
            ErrorAt(Current, "Unable to parse number value");
        }
        else
        {
            CurrentFunction.EmitConstant(new Number(value));
        }
    }

    private Dictionary<TokenType, ParseRule> _parseRules;

    [MemberNotNull(nameof(_parseRules))]
    private void InitialiseParseRules()
    {
        _parseRules = new Dictionary<TokenType, ParseRule>()
        {
            // Literals
            {TokenType.Var,        new ParseRule(ParseVariable, null, Precedence.None) },
            {TokenType.String,     new ParseRule(ParseString,   null, Precedence.None) },
            {TokenType.Number,     new ParseRule(ParseNumber,   null, Precedence.None) },
            {TokenType.EndOfLine,  new ParseRule(null,          null, Precedence.None) },
            {TokenType.EndOfFile,  new ParseRule(null,          null, Precedence.None) },
        // [TOKEN_FILE_NAME]        = {replacement,  NULL,      PREC_NONE},
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