namespace sharpcomet.vmlib;

public class VmStack<T>
{
    private const double REDUCTION_FACTOR = 0.75;
    private const int BASE_SIZE = 10;

    private T[] _array;
    private int _index;

    public VmStack()
    {
        _array = new T[BASE_SIZE];
    }

    public T Pop()
    {
        T result = _array[--_index];
        _array[_index] = default;
        ResizeIfRequired();
        return result;
    }

    public void PopMany(int count)
    {
        for (int i = 0; i < count; i++)
        {
            _array[--_index] = default;
        }
        ResizeIfRequired();
    }

    public void Push(T item)
    {
        ResizeIfRequired();
        _array[_index++] = item;
    }

    public T Peek(int offset = 0)
    {
        return _array[_index - offset - 1];
    }

    public T[] GetTop(int count)
    {
        T[] result = new T[count];
        for (int i = 0; i < count; i++)
        {
            result[i] = _array[_index - count + i];
        }
        return result;
    }

    private void ResizeIfRequired()
    {
        if (_array.Length == _index)
        {
            Array.Resize(ref _array, _array.Length * 2);
        }
        else if (_index > BASE_SIZE && _index < (_array.Length * REDUCTION_FACTOR))
        {
            Array.Resize(ref _array, (int)(_array.Length * REDUCTION_FACTOR));
        }
    }
}