using sharpcomet.vmlib;
using stdlib;

namespace vmlib
{
    public static class Initialisation
    {
        public static void InitStdLib()
        {
            InitFunctions();
        }

        private static void InitFunctions()
        {
            NativeFunction.NativeFn print = Functions.Print;
            Globals.AddGlobal(Globals.InternString("print"), Memory.AllocateObject<NativeFunction>(print));
        }
    }
}
