using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection
{
    public class PintaCodeCompressor
    {
        private const int MaxOffset = 255;
        private const int MaxLength = 126 + 3;

        private PintaCodeDecompressor _decompressor;
        private MemoryStream _output;
        private SortedSet<int>[] _hash;

        private int _total;

        public MemoryStream Output
        {
            get { return _output; }
        }

        private int Position { get { return (int)_output.Position; } }

        public PintaCodeCompressor()
        {
            _decompressor = new PintaCodeDecompressor();
            _output = new MemoryStream(256);
            _total = 0;

            _hash = new SortedSet<int>[256];
            for (var i = 0; i < _hash.Length; i++)
            {
                _hash[i] = new SortedSet<int>();
            }
        }

        public int Compress(byte[] data, int offset, int count)
        {
            var result = Position;
            _total += count;

            var length = offset + count;
            for (var i = offset; i < length; )
            {
                if (!Compress(data, length, ref i))
                {
                    var current = data[i++];

                    _hash[current].Add(Position);
                    if ((current & 0x80) == 0x80)
                    {
                        _output.WriteByte(0xFF);
                        _output.WriteByte((byte)(0x7F & current));
                    }
                    else
                    {
                        _output.WriteByte(current);
                    }
                }
            }
            return result;
        }

        private bool Compress(byte[] data, int dataLength, ref int index)
        {
            if (index + 2 >= dataLength)
                return false;

            var set = default(SortedSet<int>);
            var current = data[index];
            set = _hash[current];

            var maxLength = 0;
            var maxOffset = 0;

            var pivot = Position - MaxOffset;
            while (set.Count > 0 && set.Min < pivot)
                set.Remove(set.Min);

            var buffer = _output.GetBuffer();
            var bufferLength = (int)_output.Length;

            foreach (var windowStart in set)
            {
                var offset = Position - windowStart;
                if (offset > MaxOffset)
                    continue;

                _decompressor.Reset(buffer, bufferLength, windowStart);

                var length = 0;
                var code = default(byte);

                while (_decompressor.Decompress(out code) && (index + length) < dataLength && code == data[index + length] && length < MaxLength)
                    length++;

                if (length > 2 && length > maxLength)
                {
                    maxLength = length;
                    maxOffset = offset;
                }

                if (length == MaxLength || (index + length) >= dataLength)
                    break;
            }

            if (maxLength == 0)
                return false;

            set.Add(Position);

            _output.WriteByte((byte)(0x80 | (maxLength - 3)));
            _output.WriteByte((byte)(maxOffset));

            index += maxLength;

            return true;
        }
    }
}
