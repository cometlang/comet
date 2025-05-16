namespace sharpcomet.lexer;

public class Source
{
    public Source(string filename, string content)
    {
        Filename = filename;
        Content = content;
    }

    public string Filename { get; }
    public string Content { get; }
}