using sharpcomet.stdlib;

namespace vmlib
{
    public class CometClass : CometObject
    {
        private CometClass? _super;

        public CometClass(CometClass? super = null)
        {
            _super = super;
        }

        public CometObject? Super => _super;

        public Dictionary<CometObject, CometObject> Methods { get; } = new();
        public Dictionary<CometObject, CometObject> StaticMethods { get; } = new();
    }
}
