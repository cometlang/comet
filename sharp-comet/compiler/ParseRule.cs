using System;

namespace sharpcomet.compiler;

public class ParseRule
{
    public ParseRule() { }

    public ParseRule(Action<bool>? prefix, Action<bool>? infix, Precedence precedence)
    {
        Prefix = prefix;
        Infix = infix;
        Precedence = precedence;
    }

    public Action<bool>? Prefix { get; set; }
    public Action<bool>? Infix { get; set; }
    public Precedence Precedence { get; set; } = Precedence.None;

}
