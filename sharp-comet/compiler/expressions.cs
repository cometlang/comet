using sharpcomet.lexer;
using sharpcomet.stdlib;
using sharpcomet.vmlib;
using System;
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

    private void Literal(bool canAssign)
    {
        switch (Previous.TokenType)
        {
            case TokenType.False:
            {
                CurrentFunction.EmitBytes((byte)Op.False);
                break;
            }
            case TokenType.True:
            {
                CurrentFunction.EmitBytes((byte)Op.True);
                break;
            }
            case TokenType.Nil:
            {
                CurrentFunction.EmitBytes((byte)Op.Nil);
                break;
            }
        }
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

    private void PropertyAssignOp(Op opCode, byte name)
    {
        CurrentFunction.EmitBytes((byte)Op.DuplicateStackTop, (byte)Op.GetProperty, name);
        Expression();
        CurrentFunction.EmitBytes((byte)opCode, (byte)Op.SetProperty, name);
    }

    private void Dot(bool canAssign)
    {
        Consume(TokenType.Identifier, "Expected an identifier after '.'");
        byte name = IdentifierConstant(Previous);

        if (canAssign)
        {
            if (Match(TokenType.Equal))
            {
                Expression();
                CurrentFunction.EmitBytes((byte)Op.SetProperty, name);
                return;
            }
            else if (Match(TokenType.PlusEqual))
            {
                PropertyAssignOp(Op.Add, name);
                return;
            }
            else if (Match(TokenType.MinusEqual))
            {
                PropertyAssignOp(Op.Subtract, name);
                return;
            }
            else if (Match(TokenType.StarEqual))
            {
                PropertyAssignOp(Op.Multiply, name);
                return;
            }
            else if (Match(TokenType.SlashEqual))
            {
                PropertyAssignOp(Op.Divide, name);
                return;
            }
        }

        if (Match(TokenType.LeftParen))
        {
            byte argCount = ArgumentList(TokenType.RightParen);
            CurrentFunction.EmitBytes((byte)Op.Invoke, name, argCount);
        }
        else
        {
            CurrentFunction.EmitBytes((byte)Op.GetProperty, name);
        }
    }

    private void Unary(bool canAssign)
    {
        TokenType operatorType = Previous.TokenType;

        // Compile the operand.
        ParsePrecedence(Precedence.Unary);

        // Emit the operator instruction.
        switch (operatorType)
        {
            case TokenType.Bang:
                CurrentFunction.EmitBytes(Op.Not);
                break;
            case TokenType.Minus:
                CurrentFunction.EmitBytes(Op.Negate);
                break;
            case TokenType.BitwiseNegate:
                CurrentFunction.EmitBytes(Op.Negate);
                break;
            case TokenType.Star:
                CurrentFunction.EmitBytes(Op.Splat);
                break;
            default:
                return; // Unreachable.
        }
    }

    private void Binary(bool canAssign)
    {
        // Remember the operator.
        TokenType operatorType = Previous.TokenType;

        // Compile the right operand.
        ParseRule rule = _parseRules[operatorType];
        ParsePrecedence(rule.Precedence + 1);

        // Emit the operator instruction.
        switch (operatorType)
        {
            case TokenType.BangEqual:
                CurrentFunction.EmitBytes(Op.Equal, Op.Not);
                break;
            case TokenType.EqualEqual:
                CurrentFunction.EmitBytes(Op.Equal);
                break;
            case TokenType.GreaterThan:
                CurrentFunction.EmitBytes(Op.GreaterThan);
                break;
            case TokenType.GreaterEqual:
                CurrentFunction.EmitBytes(Op.GreaterEqual);
                break;
            case TokenType.LessThan:
                CurrentFunction.EmitBytes(Op.LessThan);
                break;
            case TokenType.LessEqual:
                CurrentFunction.EmitBytes(Op.LessEqual);
                break;
            case TokenType.Plus:
                CurrentFunction.EmitBytes(Op.Add);
                break;
            case TokenType.Minus:
                CurrentFunction.EmitBytes(Op.Subtract);
                break;
            case TokenType.Star:
                CurrentFunction.EmitBytes(Op.Multiply);
                break;
            case TokenType.Slash:
                CurrentFunction.EmitBytes(Op.Divide);
                break;
            case TokenType.Is:
                CurrentFunction.EmitBytes(Op.Is);
                break;
            case TokenType.Percent:
                CurrentFunction.EmitBytes(Op.Modulo);
                break;
            case TokenType.BitwiseAnd:
                CurrentFunction.EmitBytes(Op.BitwiseAnd);
                break;
            case TokenType.VBar:
                CurrentFunction.EmitBytes(Op.BitwiseOr);
                break;
            case TokenType.BitwiseXor:
                CurrentFunction.EmitBytes(Op.BitwiseXor);
                break;
            case TokenType.BitShiftRight:
                CurrentFunction.EmitBytes(Op.BitShiftRight);
                break;
            case TokenType.BitShiftLeft:
                CurrentFunction.EmitBytes(Op.BitShiftLeft);
                break;
            default:
                return; // Unreachable.
        }
    }

    private void Ternary(bool canAssign)
    {
        Match(TokenType.EndOfLine);
        int elseJump = CurrentFunction.EmitJump(Op.JumpIfFalse);
        CurrentFunction.EmitBytes(Op.Pop);
        Expression();
        int endJump = CurrentFunction.EmitJump(Op.Jump);
        Consume(TokenType.Colon, "Expected ':' in a ternary operation.");
        Match(TokenType.EndOfLine);
        CurrentFunction.PatchJump(elseJump);
        CurrentFunction.EmitBytes(Op.Pop);
        Expression();
        CurrentFunction.PatchJump(endJump);
    }

    private void Attribute_(bool canAssign)
    {
        byte attributeCount = 0;
        do {
            Consume(TokenType.Identifier, "Expected an identifier.");
            NamedVariable(Previous, canAssign);
            if (Match(TokenType.Dot) && Match(TokenType.Identifier)) {
                byte name = IdentifierConstant(Previous);
                CurrentFunction.EmitBytes((byte)Op.GetProperty, name);
            }
            Consume(TokenType.LeftParen, "Expected '(' after an attribute.");
            Call(canAssign);
            Match(TokenType.EndOfLine);
            attributeCount++;
        } while (Match(TokenType.AtSymbol));
        if (Match(TokenType.Function))
        {
            FunctionDeclaration(FunctionType.Function, attributeCount);
        }
        else if (Match(TokenType.Class))
        {
            ClassDeclaration(attributeCount);
        }
        else if (CurrentClass != null &&
            (Check(TokenType.Identifier) || Check(TokenType.Static)))
        {
            Method(attributeCount);
        }
        else
        {
            ErrorAtCurrent("Epected a function or class after an attribute.");
        }
    }

    private void Lambda(bool canAssign)
    {
        CurrentFunction = new FunctionCompiler(CurrentFunction, FunctionType.Lambda);
        CurrentFunction.BeginScope();

        if (!Check(TokenType.LambdaArgsClose))
        {
            do
            {
                CurrentFunction.Function.Arity++;
                if (CurrentFunction.Function.Arity > 255)
                {
                    ErrorAtCurrent("Cannot have more than 255 parameters.");
                }

                byte paramConstant = ParseVariable("Expected a parameter name.");
                CurrentFunction.DefineVariable(paramConstant);
            } while (Match(TokenType.Comma));
        }
        Consume(TokenType.LambdaArgsClose, "Expected '|)' after lambda parameters.");

        // The body.
        Match(TokenType.EndOfLine);
        Consume(TokenType.LeftBrace, "Expected '{' before lambda body.");
        Block();

        CurrentFunction.EndScope();
        // Create the function object.
        CurrentFunction.EndCompiler(true);
    }

    private void Self(bool canAssign)
    {
        if (CurrentClass == null)
        {
            Error("Cannot use 'self' outside of a class.");
        }
        else
        {
            Variable(false); // It's never possible to re-assign self.
        }
    }

    private void PushSuperClass()
    {
        NamedVariable(SyntheticToken("super"), false);
    }

    private void Super(bool canAssign)
    {
        if (CurrentClass == null)
        {
            Error("Cannot use 'super' outside of a class.");
        }

        Consume(TokenType.Dot, "Expected '.' after 'super'.");
        Consume(TokenType.Identifier, "Expected superclass method name.");
        byte name = IdentifierConstant(Previous);

        // Push the receiver.
        NamedVariable(SyntheticToken("self"), false);

        if (Match(TokenType.LeftParen))
        {
            PushSuperClass();
            CurrentFunction.EmitBytes((byte)Op.GetSuper, name);
            byte argCount = ArgumentList(TokenType.RightParen);
            CurrentFunction.EmitBytes((byte)Op.Call, argCount);
        }
        else
        {
            PushSuperClass();
            CurrentFunction.EmitBytes((byte)Op.GetSuper, name);
        }
    }

    private void LiteralHash(bool canAssign)
    {
        NamedVariable(SyntheticToken("Hash"), canAssign);
        CurrentFunction.EmitBytes(Op.Call, 0, Op.DuplicateStackTop);
        Match(TokenType.EndOfLine);

        if (!Check(TokenType.RightBrace))
        {
            do
            {
                Match(TokenType.EndOfLine);
                if (Check(TokenType.RightBrace)) // hanging comma
                    break;
                CurrentFunction.EmitBytes(Op.DuplicateStackTop);
                Expression();
                Consume(TokenType.Colon, "':' expected between key and value of a literal hash");
                Match(TokenType.EndOfLine);
                Expression();
                Token addToken = SyntheticToken("add");
                byte name = IdentifierConstant(addToken);
                CurrentFunction.EmitBytes((byte)Op.Invoke, name, 2, (byte)Op.Pop);
            } while (Match(TokenType.Comma));
        }

        CurrentFunction.EmitBytes(Op.Pop);
        Match(TokenType.EndOfLine);
        Consume(TokenType.RightBrace, "Expected '}' for a literal hash declaration");
    }

    private void LiteralList(bool canAssign)
    {
        NamedVariable(SyntheticToken("List"), canAssign);
        CurrentFunction.EmitBytes(Op.Call, 0); // Create a list using the default constructor
        CurrentFunction.EmitBytes(Op.DuplicateStackTop); // Duplicate the list instance, so the pop leaves it for the return value of assignment
        byte argCount = ArgumentList(TokenType.RightSquareBracket);
        if (argCount > 0)
        {
            Token addToken = SyntheticToken("add");
            byte name = IdentifierConstant(addToken);
            CurrentFunction.EmitBytes((byte)Op.Invoke, name, argCount);
        }
        CurrentFunction.EmitBytes(Op.Pop);
    }

    public void EmitSubscriptAssignOpInstructions(Op opCode)
    {
        CurrentFunction.EmitBytes((byte)Op.DuplicateStackTopTwice, (byte)Op.Index, 1);
        Expression();
        CurrentFunction.EmitBytes((byte)opCode, (byte)Op.IndexAssign, 2);
    }

    private void Subscript(bool canAssign)
    {
        byte argCount = ArgumentList(TokenType.RightSquareBracket);
        if (argCount != 1)
        {
            Error("Subscript requires exactly one argument");
        }
        if (canAssign)
        {
            if (Match(TokenType.Equal))
            {
                Expression();
                CurrentFunction.EmitBytes((byte)Op.IndexAssign, (byte)(argCount + 1));
            }
            else if (Match(TokenType.PlusEqual))
            {
                EmitSubscriptAssignOpInstructions(Op.Add);
            }
            else if (Match(TokenType.MinusEqual))
            {
                EmitSubscriptAssignOpInstructions(Op.Subtract);
            }
            else if (Match(TokenType.StarEqual))
            {
                EmitSubscriptAssignOpInstructions(Op.Multiply);
            }
            else if (Match(TokenType.SlashEqual))
            {
                EmitSubscriptAssignOpInstructions(Op.Divide);
            }
            else
            {
                CurrentFunction.EmitBytes((byte)Op.Index, argCount);
            }
        }
        else
        {
            CurrentFunction.EmitBytes((byte)Op.Index, argCount);
        }
    }

    private void Or_(bool canAssign)
    {
        int elseJump = CurrentFunction.EmitJump(Op.JumpIfFalse);
        int endJump = CurrentFunction.EmitJump(Op.Jump);

        CurrentFunction.PatchJump(elseJump);
        CurrentFunction.EmitBytes(Op.Pop);

        ParsePrecedence(Precedence.Or);
        CurrentFunction.PatchJump(endJump);
    }

    private void And_(bool canAssign)
    {
        int endJump = CurrentFunction.EmitJump(Op.JumpIfFalse);

        CurrentFunction.EmitBytes(Op.Pop);
        ParsePrecedence(Precedence.And);

        CurrentFunction.PatchJump(endJump);
    }

    private Dictionary<TokenType, ParseRule> _parseRules;

    [MemberNotNull(nameof(_parseRules))]
    private void InitialiseParseRules()
    {
        _parseRules = new Dictionary<TokenType, ParseRule>()
        {
            // Single-character tokens.
            {TokenType.LeftParen,          new ParseRule(Grouping,      Call,      Precedence.Call) },
            {TokenType.LeftBrace,          new ParseRule(LiteralHash,   null,      Precedence.None)},
            {TokenType.LeftSquareBracket,  new ParseRule(LiteralList,   Subscript, Precedence.Call)},
            {TokenType.Dot,                new ParseRule(null,          Dot,       Precedence.Call) },
            // Math operations
            {TokenType.Minus,              new ParseRule(Unary,         Binary,    Precedence.Term)},
            {TokenType.Plus,               new ParseRule(null,          Binary,    Precedence.Term)},
            {TokenType.Slash,              new ParseRule(null,          Binary,    Precedence.Factor)},
            {TokenType.Star,               new ParseRule(Unary,         Binary,    Precedence.Factor)},
            {TokenType.Percent,            new ParseRule(null,          Binary,    Precedence.Factor)},
            //bitwise operations
            {TokenType.VBar,               new ParseRule(null,          Binary,    Precedence.BitwiseOr)},
            {TokenType.BitwiseAnd,         new ParseRule(null,          Binary,    Precedence.BitwiseAnd)},
            {TokenType.BitwiseXor,         new ParseRule(null,          Binary,    Precedence.Xor)},
            {TokenType.BitwiseNegate,      new ParseRule(Unary,         null,      Precedence.Unary)},
            {TokenType.BitShiftLeft,       new ParseRule(null,          Binary,    Precedence.BitShift)},
            {TokenType.BitShiftRight,      new ParseRule(null,          Binary,    Precedence.BitShift)},
            //
            {TokenType.QuestionMark,       new ParseRule(null,          Ternary,   Precedence.Ternary)},
            {TokenType.AtSymbol,           new ParseRule(Attribute_,    null,      Precedence.None)},
            {TokenType.LambdaArgsOpen,     new ParseRule(Lambda,        null,      Precedence.None)},
            {TokenType.Is,                 new ParseRule(null,          Binary,    Precedence.Is)},
            //equality
            {TokenType.Bang,               new ParseRule(Unary,         null,      Precedence.Unary)},
            {TokenType.BangEqual,          new ParseRule(null,          Binary,    Precedence.Equality)},
            {TokenType.EqualEqual,         new ParseRule(null,          Binary,    Precedence.Equality)},
            {TokenType.GreaterThan,        new ParseRule(null,          Binary,    Precedence.Comparison)},
            {TokenType.GreaterEqual,       new ParseRule(null,          Binary,    Precedence.Comparison)},
            {TokenType.LessThan,           new ParseRule(null,          Binary,    Precedence.Comparison)},
            {TokenType.LessEqual,          new ParseRule(null,          Binary,    Precedence.Comparison)},
            // logical operations
            {TokenType.LogicalOr,          new ParseRule(null,          Or_,       Precedence.Or)},
            {TokenType.LogicalAnd,         new ParseRule(null,          And_,      Precedence.And)},
            // object identifiers
            {TokenType.Self,               new ParseRule(Self,          null,      Precedence.None)},
            {TokenType.Super,              new ParseRule(Super,         null,      Precedence.None)},
            // Literals
            {TokenType.Identifier,         new ParseRule(Variable,      null,      Precedence.None) },
            {TokenType.Var,                new ParseRule(ParseVariable, null,      Precedence.None) },
            {TokenType.String,             new ParseRule(ParseString,   null,      Precedence.None) },
            {TokenType.Number,             new ParseRule(ParseNumber,   null,      Precedence.None) },
            {TokenType.Filename,           new ParseRule(Replacement,   null,      Precedence.None) },
            {TokenType.False,              new ParseRule(Literal,       null,      Precedence.None) },
            {TokenType.True,               new ParseRule(Literal,       null,      Precedence.None) },
            {TokenType.Nil,                new ParseRule(Literal,       null,      Precedence.None) },
            // Empty Rules
            {TokenType.As,                 new ParseRule() },
            {TokenType.EndOfLine,          new ParseRule() },
            {TokenType.Comma,              new ParseRule() },
            {TokenType.RightParen,         new ParseRule() },
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