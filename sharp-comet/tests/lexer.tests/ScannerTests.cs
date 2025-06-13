namespace sharpcomet.lexer.tests;

public class ScannerTests
{

    [TestCase("\n", TokenType.EndOfLine)]
    [TestCase("", TokenType.EndOfFile)]
    [TestCase("(", TokenType.LeftParen)]
    [TestCase(")", TokenType.RightParen)]
    [TestCase("{", TokenType.LeftBrace)]
    [TestCase("}", TokenType.RightBrace)]
    [TestCase("[", TokenType.LeftSquareBracket)]
    [TestCase("]", TokenType.RightSquareBracket)]
    [TestCase(",", TokenType.Comma)]
    [TestCase(".", TokenType.Dot)]
    [TestCase("-", TokenType.Minus)]
    [TestCase("+", TokenType.Plus)]
    [TestCase(";", TokenType.SemiColon)]
    [TestCase("/", TokenType.Slash)]
    [TestCase("*", TokenType.Star)]
    [TestCase(":", TokenType.Colon)]
    [TestCase("|", TokenType.VBar)]
    [TestCase("%", TokenType.Percent)]
    [TestCase("?", TokenType.QuestionMark)]
    [TestCase("@", TokenType.AtSymbol)]

    [TestCase("!", TokenType.Bang)]
    [TestCase("!=", TokenType.BangEqual)]
    [TestCase("=", TokenType.Equal)]
    [TestCase("==", TokenType.EqualEqual)]
    [TestCase(">", TokenType.GreaterThan)]
    [TestCase(">=", TokenType.GreaterEqual)]
    [TestCase("<", TokenType.LessThan)]
    [TestCase("<=", TokenType.LessEqual)]
    [TestCase("+=", TokenType.PlusEqual)]
    [TestCase("-=", TokenType.MinusEqual)]
    [TestCase("/=", TokenType.SlashEqual)]
    [TestCase("*=", TokenType.StarEqual)]
    [TestCase("%=", TokenType.PercentEqual)]
    [TestCase("||", TokenType.LogicalOr)]
    [TestCase("&&", TokenType.LogicalAnd)]
    [TestCase("&", TokenType.BitwiseAnd)]
    [TestCase("^", TokenType.BitwiseXor)]
    [TestCase("~", TokenType.BitwiseNegate)]
    [TestCase("<<", TokenType.BitShiftLeft)]
    [TestCase(">>", TokenType.BitShiftRight)]
    [TestCase("(|", TokenType.LambdaArgsOpen)]
    [TestCase("|)", TokenType.LambdaArgsClose)]

    [TestCase("some_name", TokenType.Identifier)]
    [TestCase("1234", TokenType.Number)]
    [TestCase("1234.4321", TokenType.Number)]
    [TestCase("0xabcd1234", TokenType.Number)]
    [TestCase("1_234", TokenType.Number)]
    [TestCase("__FILE__", TokenType.Filename)]

    // Keywords.
    [TestCase("as", TokenType.As)]
    [TestCase("break", TokenType.Break)]
    [TestCase("class", TokenType.Class)]
    [TestCase("else", TokenType.Else)]
    [TestCase("enum", TokenType.Enum)]
    [TestCase("false", TokenType.False)]
    [TestCase("for", TokenType.For)]
    [TestCase("foreach", TokenType.ForEach)]
    [TestCase("from", TokenType.From)]
    [TestCase("function", TokenType.Function)]
    [TestCase("if", TokenType.If)]
    [TestCase("import", TokenType.Import)]
    [TestCase("in", TokenType.In)]
    [TestCase("is", TokenType.Is)]
    [TestCase("next", TokenType.Next)]
    [TestCase("nil", TokenType.Nil)]
    [TestCase("operator", TokenType.Operator)]
    [TestCase("rethrow", TokenType.Rethrow)]
    [TestCase("return", TokenType.Return)]
    [TestCase("self", TokenType.Self)]
    [TestCase("super", TokenType.Super)]
    [TestCase("true", TokenType.True)]
    [TestCase("var", TokenType.Var)]
    [TestCase("while", TokenType.While)]
    [TestCase("try", TokenType.Try)]
    [TestCase("catch", TokenType.Catch)]
    [TestCase("throw", TokenType.Throw)]
    [TestCase("final", TokenType.Final)]
    [TestCase("finally", TokenType.Finally)]

    [TestCase("private", TokenType.Private)]
    [TestCase("protected", TokenType.Protected)]
    [TestCase("public", TokenType.Public)]
    [TestCase("static", TokenType.Static)]
    public void CanScanTokens(string content, TokenType expected)
    {
        // arrange
        var scanner = new Scanner(new SourceFile("test", content));

        // act
        var result = scanner.ScanToken();

        // assert
        Assert.That(result.TokenType, Is.EqualTo(expected));
        Assert.That(result.Representation, Is.EqualTo(content));
    }

    [TestCase("'this is a string'", "this is a string", TokenType.String)]
    [TestCase("\"this is a string\"", "this is a string", TokenType.String)]
    [TestCase(@"'this is a string
with newlines, etc'", @"this is a string
with newlines, etc", TokenType.String)]
    public void CanScanStrings(string content, string expectedRepresentation, TokenType expected)
    {
        // arrange
        var scanner = new Scanner(new SourceFile("test", content));

        // act
        var result = scanner.ScanToken();

        // assert
        Assert.That(result.TokenType, Is.EqualTo(expected));
        Assert.That(result.Representation, Is.EqualTo(expectedRepresentation));
    }
}