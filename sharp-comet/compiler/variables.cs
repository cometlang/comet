using sharpcomet.lexer;
using sharpcomet.vmlib;

namespace sharpcomet.compiler;

public partial class Parser
{
    private void NamedVariable(Token token, bool canAssign)
    {
        Op getOp;
        Op setOp;
        int arg = CurrentFunction.ResolveLocal(token.Representation);
        if (arg != FunctionCompiler.UNRESOLVED_VARIABLE_INDEX)
        {
            getOp = Op.GetLocal;
            setOp = Op.SetLocal;
        }
        //else if ((arg = resolveUpvalue(parser->currentFunction, parser, &name)) != UNRESOLVED_VARIABLE_INDEX)
        //{
        //    getOp = OP_GET_UPVALUE;
        //    setOp = OP_SET_UPVALUE;
        //}
        else
        {
            arg = IdentifierConstant(token);
            getOp = Op.GetGlobal;
            setOp = Op.SetGlobal;
        }

        if (canAssign)
        {
            if (Match(TokenType.Equal))
            {
                Expression();
                CurrentFunction.EmitBytes((byte)setOp, (byte)arg);
            }
        }

        CurrentFunction.EmitBytes((byte)getOp, (byte)arg);
    }

    private void Variable(bool canAssign)
    {
        NamedVariable(Previous, canAssign);
    }
}
