using System.Collections.Generic;
using sharpcomet.lexer;

namespace sharpcomet.compiler;

public partial class Parser
{

    private void ParseVariable(bool canAssign)
    {

    }

    private void ParseString(bool canAssign)
    {

    }

    private Dictionary<TokenType, ParseRule> _parseRules;

    private void InitialiseParseRules()
    {
        _parseRules = new Dictionary<TokenType, ParseRule>()
        {
            // Literals
            {TokenType.Identifier, new ParseRule(ParseVariable, null, Precedence.None) },
        // [TOKEN_STRING]           = {string,       NULL,      PREC_NONE},
        // [TOKEN_NUMBER]           = {number,       NULL,      PREC_NONE},
        // [TOKEN_FILE_NAME]        = {replacement,  NULL,      PREC_NONE},
        };
    }

    private ParseRule GetRule(TokenType tokenType)
    {
        if (_parseRules.ContainsKey(tokenType))
        {
            return _parseRules[tokenType];
        }
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
            infixRule(canAssign);
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