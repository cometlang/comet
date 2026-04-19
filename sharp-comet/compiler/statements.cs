using System.Linq.Expressions;
using sharpcomet.lexer;
using sharpcomet.vmlib;
using System;

namespace sharpcomet.compiler;

public partial class Parser
{
    private void ForStatement()
    {
        CurrentFunction.BeginScope();
        Consume(TokenType.LeftParen, "Expected '(' after 'for'.");
        if (Check(TokenType.SemiColon))
        {
            // No initializer.
        }
        else if (Match(TokenType.Var))
        {
            VarDeclaration();
        }
        else
        {
            ExpressionStatement();
        }
        Consume(TokenType.SemiColon, "Expected ';' after loop intializer.");
        CurrentLoop = new LoopCompiler(CurrentFunction.CurrentOffset, CurrentFunction.ScopeDepth, CurrentLoop);

        if (!Match(TokenType.SemiColon))
        {
            Expression();
            Consume(TokenType.SemiColon, "Expected ';' after loop condition.");

            // Jump out of the loop if the condition is false.
            CurrentLoop.ExitAddress = CurrentFunction.EmitJump(Op.JumpIfFalse);
            CurrentFunction.EmitBytes(Op.Pop); // Condition.
        }
        if (!Match(TokenType.RightParen))
        {
            int bodyJump = CurrentFunction.EmitJump(Op.Jump);

            int incrementStart = CurrentFunction.CurrentOffset;
            Expression();
            CurrentFunction.EmitBytes(Op.Pop);
            Consume(TokenType.RightParen, "Expected ')' after for clauses.");

            if (!CurrentFunction.EmitLoop(CurrentLoop.StartAddress))
            {
                Error("Loop body too large.");
            }
            CurrentLoop.StartAddress = incrementStart;
            CurrentFunction.PatchJump(bodyJump);
        }

        Statement();

        CurrentFunction.EmitLoop(CurrentLoop.StartAddress);

        if (CurrentLoop.BreakJump != null)
        {
            CurrentFunction.PatchJump(CurrentLoop.BreakJump.Value);
        }
        if (CurrentLoop.ExitAddress != null)
        {
            CurrentFunction.PatchJump(CurrentLoop.ExitAddress.Value);
            CurrentFunction.EmitBytes(Op.Pop);
        }

        CurrentFunction.EndScope();
        CurrentLoop = CurrentLoop.Enclosing;
    }

    private void IfStatement()
    {
        Consume(TokenType.LeftParen, "Expected '(' after 'if'.");
        Expression();
        Consume(TokenType.RightParen, "Expected ')' after condition.");

        int thenJump = CurrentFunction.EmitJump(Op.JumpIfFalse);
        CurrentFunction.EmitBytes((byte)Op.Pop);
        Statement();

        int elseJump = CurrentFunction.EmitJump(Op.Jump);
        if (!CurrentFunction.PatchJump(thenJump))
        {
            Error("Too much code to jump over!");
        }
        CurrentFunction.EmitBytes((byte)Op.Pop);

        Match(TokenType.EndOfLine);
        if (Match(TokenType.Else))
            Statement();

        if (!CurrentFunction.PatchJump(elseJump))
        {
            Error("Too much code to jump over!");
        }
    }

    private void Block()
    {
        Match(TokenType.EndOfLine);
        while (!Check(TokenType.RightBrace) && !Check(TokenType.EndOfFile))
        {
            Declaration();
        }
        Match(TokenType.EndOfLine);
        Consume(TokenType.RightBrace, "Expected a '}' after a block.");
    }

    private void ExpressionStatement()
    {
        Expression();
        if (!Check(TokenType.EndOfFile) && !Check(TokenType.RightBrace))
        {
            Consume(TokenType.EndOfLine, "Only one statement per line allowed.");
        }
        CurrentFunction.EmitBytes((byte)Op.Pop);
    }

    private void ThrowStatement()
    {
        Expression();
        if (!Check(TokenType.EndOfFile))
            Consume(TokenType.EndOfLine, "Only one statement per line allowed.");
        CurrentFunction.EmitBytes((byte)Op.Throw);
    }

    private void RethrowStatement()
    {
        Expression();
        if (!Check(TokenType.EndOfFile))
            Consume(TokenType.EndOfLine, "Only one statement per line allowed.");
        CurrentFunction.EmitBytes((byte)Op.Rethrow);
    }

    private void SyntheticMethodCall(string methodName)
    {
        Token methodNameToken = SyntheticToken(methodName);
        byte constant = IdentifierConstant(methodNameToken);
        CurrentFunction.EmitBytes((byte)Op.Invoke, constant, 0); // zero arguments
    }

    private void ForEachStatement()
    {
        CurrentFunction.BeginScope();
        Consume(TokenType.LeftParen, "Expected '(' after 'foreach'.");
        Consume(TokenType.Var, "Expected 'var' to declare foreach loop variable.");
        Token loopVarName = Current;
        byte loopVar = ParseVariable("Expected a variable name in the foreach loop.");
        CurrentFunction.EmitBytes((byte)Op.Nil);
        CurrentFunction.DefineVariable(loopVar);

        Consume(TokenType.In, "Expected 'in' keyword in foreach loop.");
        Expression();
        SyntheticMethodCall("iterator");
        Consume(TokenType.RightParen, "Expected ')' after 'foreach' condition.");

        byte iterVar = CurrentFunction.AddLocal(string.Empty);
        CurrentFunction.EmitBytes((byte)Op.SetLocal, iterVar);
        CurrentFunction.MarkInitialized();

        CurrentLoop = new LoopCompiler(CurrentFunction.CurrentOffset, CurrentFunction.ScopeDepth, CurrentLoop);

        CurrentFunction.EmitBytes((byte)Op.GetLocal, iterVar);
        SyntheticMethodCall("has_next?");
        CurrentLoop.ExitAddress = CurrentFunction.EmitJump(Op.JumpIfFalse);
        CurrentFunction.EmitBytes((byte)Op.Pop);

        CurrentFunction.EmitBytes((byte)Op.GetLocal, iterVar);
        SyntheticMethodCall("get_next");
        int variable = CurrentFunction.ResolveLocal(loopVarName.Representation);
        CurrentFunction.EmitBytes((byte)Op.SetLocal, (byte)variable, (byte)Op.Pop);

        Statement();

        if (!CurrentFunction.EmitLoop(CurrentLoop.StartAddress))
        {
            Error("Loop body too large.");
        }
        CurrentFunction.PatchJump(CurrentLoop.ExitAddress.Value);
        if (CurrentLoop.BreakJump != null)
        {
            CurrentFunction.PatchJump(CurrentLoop.BreakJump.Value);
        }
        CurrentFunction.EndScope();
        CurrentFunction.EmitBytes((byte)Op.Pop); // This feels weird, like I shouldn't need to do it.
        CurrentLoop = CurrentLoop.Enclosing;
    }

    private void ReturnStatement()
    {
        if (CurrentFunction.IsScript())
        {
            Error("Cannot return from top-level code.");
        }
        if (Match(TokenType.EndOfLine))
        {
            CurrentFunction.EmitReturn();
        }
        else
        {
            if (CurrentFunction.IsInitializer())
            {
                Error("Cannot return from an initializer.");
            }
            Expression();
            if (!Check(TokenType.RightBrace))
            {
                Consume(TokenType.EndOfLine, "Only one statement per line allowed.");
            }
            CurrentFunction.EmitBytes((byte)Op.Return);
        }
    }

    private void WhileStatement()
    {
        CurrentLoop = new LoopCompiler(CurrentFunction.CurrentOffset, CurrentFunction.ScopeDepth, CurrentLoop);
        Consume(TokenType.LeftParen, "Expected '(' after 'while'.");
        Expression();
        Consume(TokenType.RightParen, "Expected ')' after condition.");

        CurrentLoop.ExitAddress = CurrentFunction.EmitJump(Op.JumpIfFalse);

        CurrentFunction.EmitBytes((byte)Op.Pop);
        Statement();
        if (!CurrentFunction.EmitLoop(CurrentLoop.StartAddress))
        {
            Error("Loop body too large.");
        }

        CurrentFunction.PatchJump(CurrentLoop.ExitAddress.Value);
        if (CurrentLoop.BreakJump.HasValue)
        {
            CurrentFunction.PatchJump(CurrentLoop.BreakJump.Value);
        }
        CurrentFunction.EmitBytes((byte)Op.Pop);
        CurrentLoop = CurrentLoop.Enclosing;
    }

    private void TryStatement()
    {
        throw new NotImplementedException();
    }

    private void ImportStatement()
    {
        byte? importParamCount = 0;
        if (Match(TokenType.LeftBrace))
        {
            byte[] moduleIdentifierConstants = new byte[255];
            if (Match(TokenType.Star))
            {
                importParamCount = null;
            }
            else
            {
                do
                {
                    if (importParamCount == 255)
                    {
                        ErrorAtCurrent("Cannot have more than 255 import parameters.");
                    }
                    Match(TokenType.EndOfLine);
                    Consume(TokenType.Identifier, "Expected a module import identifier.");
                    byte paramConstant = IdentifierConstant(Previous);
                    moduleIdentifierConstants[importParamCount.Value] = paramConstant;
                    importParamCount++;

                } while (Match(TokenType.Comma));
            }
            Consume(TokenType.RightBrace, "Expected a '}' after import parameters.");
            Match(TokenType.EndOfLine);
            Consume(TokenType.From, "Expected 'from' after import parameters.");
            Expression();
            // Imports are a function that returns nil, so pop that off the stack, too
            CurrentFunction.EmitBytes((byte)Op.Import, (byte)Op.Pop);
            CurrentFunction.EmitBytes((byte)Op.ImportParams, importParamCount ?? 0);
            if (importParamCount != null)
            {
                CurrentFunction.EmitBytes(moduleIdentifierConstants);
            }
        }
        else
        {
            Expression();
            // Imports are a function that returns nil, so pop that off the stack, too
            CurrentFunction.EmitBytes((byte)Op.Import, (byte)Op.Pop);
            Consume(TokenType.As, "Expected 'as' after the module to import.");
            byte global = ParseVariable("Expected a variable name for the imported module.");
            CurrentFunction.DefineVariable(global);
        }
    }

    private void NextStatement()
    {
        if (CurrentLoop == null) {
            Error("Can't use 'next' outside of a loop.");
            return;
        }

        // Discard any locals created inside the loop.
        CurrentFunction.DiscardCurrentScope(CurrentLoop.LoopScopeDepth);

        // Jump to top of current innermost loop.
        CurrentFunction.EmitLoop(CurrentLoop.StartAddress);
    }

    private void BreakStatement()
    {
        throw new NotImplementedException();
    }

    private void Statement()
    {
        Match(TokenType.EndOfLine);
        if (Match(TokenType.For))
        {
            ForStatement();
        }
        else if (Match(TokenType.ForEach))
        {
            ForEachStatement();
        }
        else if (Match(TokenType.If))
        {
            IfStatement();
        }
        else if (Match(TokenType.Return))
        {
            ReturnStatement();
        }
        else if (Match(TokenType.While))
        {
            WhileStatement();
        }
        else if (Match(TokenType.Try))
        {
            TryStatement();
        }
        else if (Match(TokenType.Rethrow))
        {
            RethrowStatement();
        }
        else if (Match(TokenType.Throw))
        {
            ThrowStatement();
        }
        else if (Match(TokenType.LeftBrace))
        {
            CurrentFunction.BeginScope();
            Block();
            CurrentFunction.EndScope();
        }
        else if (Match(TokenType.Import))
        {
            ImportStatement();
        }
        else if (Match(TokenType.Next))
        {
            NextStatement();
        }
        else if (Match(TokenType.Break))
        {
            BreakStatement();
        }
        else
        {
            ExpressionStatement();
        }
    }
}