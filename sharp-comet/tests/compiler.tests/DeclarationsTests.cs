using sharpcomet.lexer;
using sharpcomet.vmlib;

namespace sharpcomet.compiler.tests;
public class DeclarationsTests
{
    [Test]
    public void FunctionDeclarationTest()
    {
        // arrange
        var source = new SourceFile("test", "function test_func() {}");

        // act
        var result = Compiler.Compile(source);

        // assert
        Assert.That(result, Is.InstanceOf<CometFunction>());
    }
}
