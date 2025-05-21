namespace sharpcomet.compiler;


public class LocalVariable
{
    public string Name { get; }
    public int Depth { get; set; }
    public LocalVariable(string name)
    {
        Name = name;
        Depth = 0;
    }
}