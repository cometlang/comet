using sharpcomet.stdlib;

namespace stdlib
{
    public static class Functions
    {
        public static CometObject Print(params CometObject[] args)
        {
            var output = string.Join(" ", args.Select(obj => obj.ToString()));
            Console.WriteLine(output);
            return Nil.Instance;
        }
    }
}
