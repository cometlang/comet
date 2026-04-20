using sharpcomet.lexer;
using sharpcomet.vmlib;
using System;

namespace sharpcomet.compiler;

public partial class Parser
{
    private void EmitLoop()
    {
        if (!CurrentFunction.EmitLoop(CurrentLoop!.StartAddress))
        {
            Error("Loop body too large.");
        }
    }

    private void PatchJump(int jump)
    {
        if (!CurrentFunction.PatchJump(jump))
        {
            Error("Too much code to jump over!");
        }
    }

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

            EmitLoop();

            CurrentLoop.StartAddress = incrementStart;
            PatchJump(bodyJump);
        }

        Statement();

        EmitLoop();

        if (CurrentLoop.BreakJump != null)
        {

            PatchJump(CurrentLoop.BreakJump.Value);
        }
        if (CurrentLoop.ExitAddress != null)
        {
            PatchJump(CurrentLoop.ExitAddress.Value);
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
        CurrentFunction.EmitBytes(Op.Pop);
        Statement();

        int elseJump = CurrentFunction.EmitJump(Op.Jump);
        PatchJump(thenJump);
        CurrentFunction.EmitBytes(Op.Pop);

        Match(TokenType.EndOfLine);
        if (Match(TokenType.Else))
            Statement();

        PatchJump(elseJump);
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
        CurrentFunction.EmitBytes(Op.Pop);
    }

    private void ThrowStatement()
    {
        Expression();
        if (!Check(TokenType.EndOfFile))
            Consume(TokenType.EndOfLine, "Only one statement per line allowed.");
        CurrentFunction.EmitBytes(Op.Throw);
    }

    private void RethrowStatement()
    {
        Expression();
        if (!Check(TokenType.EndOfFile))
            Consume(TokenType.EndOfLine, "Only one statement per line allowed.");
        CurrentFunction.EmitBytes(Op.Rethrow);
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
        CurrentFunction.EmitBytes(Op.Nil);
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
        CurrentFunction.EmitBytes(Op.Pop);

        CurrentFunction.EmitBytes((byte)Op.GetLocal, iterVar);
        SyntheticMethodCall("get_next");
        int variable = CurrentFunction.ResolveLocal(loopVarName.Representation);
        CurrentFunction.EmitBytes((byte)Op.SetLocal, (byte)variable, (byte)Op.Pop);

        Statement();

        EmitLoop();
        PatchJump(CurrentLoop.ExitAddress.Value);
        if (CurrentLoop.BreakJump != null)
        {
            PatchJump(CurrentLoop.BreakJump.Value);
        }
        CurrentFunction.EndScope();
        CurrentFunction.EmitBytes(Op.Pop); // This feels weird, like I shouldn't need to do it.
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
            CurrentFunction.EmitBytes(Op.Return);
        }
    }

    private void WhileStatement()
    {
        CurrentLoop = new LoopCompiler(CurrentFunction.CurrentOffset, CurrentFunction.ScopeDepth, CurrentLoop);
        Consume(TokenType.LeftParen, "Expected '(' after 'while'.");
        Expression();
        Consume(TokenType.RightParen, "Expected ')' after condition.");

        CurrentLoop.ExitAddress = CurrentFunction.EmitJump(Op.JumpIfFalse);

        CurrentFunction.EmitBytes(Op.Pop);
        Statement();
        EmitLoop();

        PatchJump(CurrentLoop.ExitAddress.Value);
        if (CurrentLoop.BreakJump.HasValue)
        {
            PatchJump(CurrentLoop.BreakJump.Value);
        }
        CurrentFunction.EmitBytes(Op.Pop);
        CurrentLoop = CurrentLoop.Enclosing;
    }

    private void TryStatement()
    {
        CurrentFunction.EmitBytes(Op.PushExceptionHandler);
        int exceptionType = CurrentFunction.CurrentOffset;
        CurrentFunction.EmitBytes(0xFF);
        int handlerAddress = CurrentFunction.CurrentOffset;
        CurrentFunction.EmitBytes(0XFF, 0XFF);
        int finallyAddress = CurrentFunction.CurrentOffset;
        CurrentFunction.EmitBytes(0XFF, 0XFF);

        Statement();

        CurrentFunction.EmitBytes(Op.PopExceptionHandler);
        int successJump = CurrentFunction.EmitJump(Op.Jump);
        Match(TokenType.EndOfLine);
        bool tryBlockCompleted = false;

        if (Match(TokenType.Catch))
        {
            tryBlockCompleted = true;
            CurrentFunction.BeginScope();
            Consume(TokenType.LeftParen, "Expected '(' after catch.");
            Consume(TokenType.Identifier, "Expected a type name to catch.");
            byte name = IdentifierConstant(Previous);
            CurrentFunction.SetCodeOffset(exceptionType, name);
            CurrentFunction.PatchAddress(handlerAddress);
            if (Match(TokenType.As))
            {
                Consume(TokenType.Identifier, "Expected an identifier for the exception instance.");
                byte exVar = CurrentFunction.AddLocal(Previous.Representation);
                CurrentFunction.MarkInitialized();
                CurrentFunction.EmitBytes((byte)Op.SetLocal, exVar);
            }
            Consume(TokenType.RightParen, "Expected ')' after a catch statement.");
            CurrentFunction.EmitBytes(Op.PopExceptionHandler);
            Statement();
            Match(TokenType.EndOfLine);
            CurrentFunction.EndScope();
        }
        PatchJump(successJump);

        if (Match(TokenType.Finally))
        {
            tryBlockCompleted = true;
            // If we arrive here from either the try or handler blocks, then we don't
            // want to continue propagating the exception
            CurrentFunction.EmitBytes(Op.False);

            CurrentFunction.PatchAddress(finallyAddress);
            Statement();

            int continueExecution = CurrentFunction.EmitJump(Op.JumpIfFalse);
            CurrentFunction.EmitBytes(Op.Pop); // Pop the bool off the stack
            CurrentFunction.EmitBytes(Op.PropagateException);
            PatchJump(continueExecution);
            CurrentFunction.EmitBytes(Op.Pop);
        }

        if (tryBlockCompleted == false)
        {
            ErrorAtCurrent("A try statement requires a catch, finally or both");
        }
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
            CurrentFunction.EmitBytes(Op.Import, Op.Pop);
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
            CurrentFunction.EmitBytes(Op.Import, Op.Pop);
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
        EmitLoop();
    }

    private void BreakStatement()
    {
        if (CurrentLoop == null)
        {
            Error("Can't use 'break' outside of a loop.");
            return;
        }

        if (CurrentLoop.BreakJump != null)
        {
            Error("Only one break statement per loop is supported.");
            return;
        }

        // Discard any locals created inside the loop.
        CurrentFunction.DiscardCurrentScope(CurrentLoop.LoopScopeDepth);

        CurrentLoop.BreakJump = CurrentFunction.EmitJump(Op.Jump);
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