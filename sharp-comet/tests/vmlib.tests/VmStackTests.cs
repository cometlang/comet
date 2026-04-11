using sharpcomet.vmlib;

namespace vmlib.tests
{
    public class VmStackTests
    {
        [Test]
        public void CanPushAndPop()
        {
            // arrange
            var stack = new VmStack<int>();

            // act
            stack.Push(1);
            var result = stack.Pop();

            // assert
            Assert.That(result, Is.EqualTo(1));
        }

        [Test]
        public void CanPushAndPopMultipleValues()
        {
            // arrange
            var stack = new VmStack<int>();

            // act
            for (int i = 1; i <= 20; i++)
            {
                stack.Push(i);
            }

            // assert
            for (int i = 20; i > 0; i--)
            {
                Assert.That(stack.Pop(), Is.EqualTo(i));
            }
        }

        [Test]
        public void CanPopMany()
        {
            // arrange
            var stack = new VmStack<int>();
            for (int i = 1; i <= 10; i++)
            {
                stack.Push(i);
            }

            // act
            stack.PopMany(5);

            //assert
            Assert.That(stack.Peek(), Is.EqualTo(5));
        }

        [Test]
        public void CanPeekAndPopTheSameValue()
        {
            // arrange
            var stack = new VmStack<int>();
            stack.Push(1);

            // act
            var peekedValue = stack.Peek();
            var result = stack.Pop();

            // assert
            Assert.That(peekedValue, Is.EqualTo(1));
            Assert.That(result, Is.EqualTo(1));
        }

        [Test]
        public void CanGetTopValues()
        {
            // arrange
            var stack = new VmStack<int>();
            for (int i = 1; i <= 10; i++)
            {
                stack.Push(i);
            }

            // act
            var result = stack.GetTop(5);

            //assert
            Assert.That(result, Is.EqualTo([6, 7, 8, 9, 10]));
        }
    }
}
