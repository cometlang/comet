using System.IO;

namespace sharpcomet.lexer;

public class SourceFile
{
    public SourceFile(string filename, string content)
    {
        Filename = filename;
        Content = content;
    }

    public string Filename { get; }
    public string Content { get; }

    public static SourceFile Create(string path)
    {
        var fullpath = Path.GetFullPath(path);
        var content = File.ReadAllText(fullpath);
        return new SourceFile(fullpath, content);
    }
}