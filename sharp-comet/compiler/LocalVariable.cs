namespace sharpcomet.compiler;


public class LocalVariable
{
    public string Name { get; }
    public int Depth { get; set; }

    public bool IsCaptured { get; set; }

    public LocalVariable(string name, int scope)
    {
        Name = name;
        Depth = scope;
    }
}