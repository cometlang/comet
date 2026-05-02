namespace sharpcomet.compiler.tests;
using NUnit.Framework;
using sharpcomet.lexer;
using sharpcomet.vmlib;

public class StatementTests
{
    [TestCase("import 'thing' as thing")]
    public void ParseImportStatement(string statement)
    {
        // arrange
        var source = new SourceFile("test", statement);

        // act
        var result = Compiler.Compile(source);

        // assert
        Assert.That(result, Is.InstanceOf<CometFunction>());
    }
}