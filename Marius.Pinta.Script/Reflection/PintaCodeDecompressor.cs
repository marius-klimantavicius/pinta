using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection
{
    public class PintaCodeDecompressor
    {
        public const int MaxStack = 8; // this must match PINTA_DECOMPRESS_MAX_STACK

        private class State
        {
            public int Position;
            public int Length;
        }

        private byte[] _data;
        private int _length;

        private Stack<State> _stack = new Stack<State>();
        private State _state;

        private int _maxStack;

        public PintaCodeDecompressor()
        {
        }

        public void Reset(byte[] data, int length, int start)
        {
            _data = data;
            _length = length;

            _state = new State() { Position = start, Length = -1 };
            _stack = new Stack<State>();

            // We need to limit our max stack this way because
            // 1. Native decompressor uses stack for initial state (effectively pushing initial state to stack)
            // 2. When we are compressing then we are starting decompression at expected run, while native decompressor 
            //    will have to push a new state to move to this position
            _maxStack = MaxStack - 2; 
        }

        public void Reset(byte[] data, int length, int start, int fragmentLength)
        {
            _data = data;
            _length = length;

            _state = new State() { Position = start, Length = fragmentLength };
            _stack = new Stack<State>();

            // We need to limit our max stack this way because
            // 1. Native decompressor uses stack for initial state (effectively pushing initial state to stack)
            _maxStack = MaxStack - 1;
        }

        public bool Decompress(out byte value)
        {
            value = 0;
            while (true)
            {
                while (_state.Length == 0)
                {
                    if (_stack.Count == 0)
                        return false;
                    _state = _stack.Pop();
                }

                if (_length <= _state.Position)
                    return false;

                var current = _data[_state.Position++];
                if (current == 0xFF)
                {
                    current = _data[_state.Position++];

                    if (_state.Length > 0)
                        _state.Length--;

                    value = (byte)(current | 0x80);
                    return true;
                }
                else if ((current & 0x80) == 0x80)
                {
                    if (_stack.Count >= _maxStack)
                        return false;

                    var length = (current & 0x7F) + 3;
                    var offset = _data[_state.Position++];

                    if (_state.Length > 0 && length > _state.Length)
                        length = _state.Length;

                    if (_state.Length > 0)
                        _state.Length -= length;

                    var state = new State()
                    {
                        Position = _state.Position - offset - 2,
                        Length = length
                    };

                    _stack.Push(_state);
                    _state = state;
                }
                else
                {
                    if (_state.Length > 0)
                        _state.Length--;

                    value = current;
                    return true;
                }
            }
        }
    }
}
