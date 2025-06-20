using sharpcomet.stdlib;
using sharpcomet.lexer;

namespace sharpcomet.compiler;

public class Compiler
{
    public static CometObject Compile(SourceFile sourceFile)
    {
        var scanner = new Scanner(sourceFile);
        var parser = new Parser(scanner);
        return parser.Parse();
    }

}