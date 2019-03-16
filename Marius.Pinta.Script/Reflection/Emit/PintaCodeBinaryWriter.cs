using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection.Emit
{
    public class PintaCodeBinaryWriter : IDisposable
    {
        private readonly bool _emitBigEndian;
        private MemoryStream _stream;

        public uint Length
        {
            get { return unchecked((uint)_stream.Length); }
        }

        public uint Offset
        {
            get { return unchecked((uint)_stream.Position); }
            set { _stream.Position = (long)value; }
        }

        public byte[] Data
        {
            get { return _stream.ToArray(); }
        }

        public PintaCodeBinaryWriter(bool emitBigEndian)
        {
            _emitBigEndian = emitBigEndian;

            _stream = new MemoryStream(256);
        }

        public byte[] GetBuffer()
        {
            return _stream.GetBuffer();
        }

        public void Clear()
        {
            _stream.Position = 0;
            _stream.SetLength(0);
        }

        public void WriteUInt(uint value)
        {
            var buffer = new byte[4];
            if (_emitBigEndian)
            {
                buffer[3] = (byte)(value & 0xFF);
                buffer[2] = (byte)((value >> 8) & 0xFF);
                buffer[1] = (byte)((value >> 16) & 0xFF);
                buffer[0] = (byte)((value >> 24) & 0xFF);
            }
            else
            {
                buffer[0] = (byte)(value & 0xFF);
                buffer[1] = (byte)((value >> 8) & 0xFF);
                buffer[2] = (byte)((value >> 16) & 0xFF);
                buffer[3] = (byte)((value >> 24) & 0xFF);
            }

            _stream.Write(buffer, 0, buffer.Length);
        }

        public void WriteUleb128(uint value)
        {
            var buffer = new byte[5];
            var index = 0;
            var current = 0U;

            do
            {
                current = value & 0x7F;
                value = value >> 7;

                if (value != 0)
                    current = current | 0x80;

                buffer[index++] = (byte)current;

            } while (value != 0);

            _stream.Write(buffer, 0, index);
        }

        public void WriteUleb128p1(uint value)
        {
            WriteUleb128(value + 1);
        }

        public void WriteSleb128(int value)
        {
            var ziggy = (value << 1) ^ (value >> 31);
            WriteUleb128(unchecked((uint)ziggy));
        }

        public void Write(string value)
        {
            var stream = new MemoryStream(value.Length);

            for (var i = 0; i < value.Length; i++)
            {
                var current = value[i];

                if (current <= 0x007F && current != 0x0000)
                {
                    stream.WriteByte((byte)current);
                }
                else if (current <= 0x07FF)
                {
                    stream.WriteByte((byte)(((current >> 6) & 0x1F) | 0xC0));
                    stream.WriteByte((byte)((current & 0x3F) | 0x80));
                }
                else
                {
                    stream.WriteByte((byte)(((current >> 12) & 0x0F) | 0xE0));
                    stream.WriteByte((byte)(((current >> 6) & 0x3F) | 0x80));
                    stream.WriteByte((byte)((current & 0x3F) | 0x80));
                }
            }

            var data = stream.GetBuffer();
            _stream.Write(data, 0, (int)stream.Length);
            _stream.WriteByte(0);
        }

        public void WriteByte(byte value)
        {
            _stream.WriteByte(value);
        }

        public void Write(PintaCodeBinaryWriter other)
        {
            var data = other.Data;
            _stream.Write(data, 0, data.Length);
        }

        public void Write(byte[] data)
        {
            _stream.Write(data, 0, data.Length);
        }

        public void Write(byte[] data, int offset, int count)
        {
            _stream.Write(data, offset, count);
        }

        public static int GetLengthUInt(uint value)
        {
            return 4;
        }

        public static int GetLengthUleb128(uint value)
        {
            if (value < 0x80)
                return 1;

            if (value < 0x4000)
                return 2;

            if (value < 0x200000)
                return 3;

            if (value < 0x10000000)
                return 4;

            return 5;
        }

        public static int GetLengthUleb128p1(uint value)
        {
            return GetLengthUleb128(value + 1);
        }

        public static int GetLengthSleb128(int value)
        {
            var ziggy = (value << 1) ^ (value >> 31);
            return GetLengthUleb128(unchecked((uint)ziggy));
        }

        public void CopyTo(Stream output)
        {
            _stream.Position = 0;
            _stream.CopyTo(output);
        }

        public void Dispose()
        {
            if (_stream != null)
                _stream.Dispose();
        }
    }
}
