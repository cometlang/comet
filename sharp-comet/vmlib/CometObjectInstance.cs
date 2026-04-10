using sharpcomet.stdlib;

namespace vmlib
{
    public class CometObjectInstance : CometObject
    {
        private CometClass _klass;
        private Dictionary<CometObject, CometObject> _fields;

        public CometObjectInstance(CometClass klass)
        {
            _klass = klass;
            _fields = new();
        }
    }
}
