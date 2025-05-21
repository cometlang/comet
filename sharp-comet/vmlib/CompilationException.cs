using System;

namespace sharpcomet.vmlib;

public class CompilationException : Exception
{

    public CompilationException(string message) : base(message) { }

}