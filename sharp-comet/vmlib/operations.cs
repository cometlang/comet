namespace sharpcomet.vmlib;

public enum Op : byte
{
    Nil,
    True,
    False,
    DefineGlobal,
    GetGlobal,
    SetGlobal,
    Pop,
    CloseUpValue,
    Closure,
    Constant,
    GetLocal,
    SetLocal,
    Return,
    Call,
    GetProperty,
    SetProperty,
    Invoke,
    Add,
    Subtract,
    Multiply,
    Divide,
    DuplicateStackTop,
};