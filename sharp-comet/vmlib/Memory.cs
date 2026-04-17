using sharpcomet.stdlib;
using System.Diagnostics.CodeAnalysis;

namespace vmlib
{
    public class Memory
    {
        private static List<CometObject> _objects = new List<CometObject>();

        public static T AllocateObject<[DynamicallyAccessedMembers(DynamicallyAccessedMemberTypes.PublicConstructors)] T>(
            params object?[]? parameters
        ) where T : CometObject
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
