using sharpcomet.lexer;

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
    public void CanScanTokens(string content, TokenType expected)
    {
        // arrange
        var scanner = new Scanner(content);

        // act
        var result = scanner.ScanToken();

        // assert
        Assert.That(result.TokenType, Is.EqualTo(expected));
        Assert.That(result.Representation, Is.EqualTo(content));
    }
}