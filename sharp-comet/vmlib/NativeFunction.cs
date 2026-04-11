using sharpcomet.stdlib;

namespace vmlib
{
    public class NativeFunction : CometObject
    {
        public delegate CometObject NativeFn(params CometObject[] args);

        private NativeFn _function;

        public NativeFunction(NativeFn func)
        {
            _function = func;
        }

        public CometObject Call(params CometObject[] args)
        {
            return _function(args);
        }
    }
}
