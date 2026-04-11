using sharpcomet.stdlib;

namespace vmlib
{
    public class Memory
    {
        private static List<CometObject> _objects = new List<CometObject>();

        public static T AllocateObject<T>(params object?[]? parameters) where T : CometObject
        {
            var obj = (T?) Activator.CreateInstance(typeof(T), parameters);
            if (obj == null)
            {
                throw new OutOfMemoryException($"Couldn't instantiate object of type {typeof(T).Name}");
            }
            _objects.Add(obj);
            return obj;
        }
    }
}
