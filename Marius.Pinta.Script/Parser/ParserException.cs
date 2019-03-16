﻿using System;

namespace Marius.Pinta.Script.Parser
{
    public class ParserException : Exception
    {
        public int Column;
        public string Description;
        public int Index;
        public int LineNumber;
        public new string Source;

        public ParserException(string message) : base(message)
        {
        }
    }
}