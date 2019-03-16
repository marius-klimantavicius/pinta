using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script
{
    [Serializable]
    public class ScriptSyntaxException : Exception
    {
        public ScriptSyntaxException() { }
        public ScriptSyntaxException(string message) : base(message) { }
        public ScriptSyntaxException(string message, Exception inner) : base(message, inner) { }
        protected ScriptSyntaxException(
          System.Runtime.Serialization.SerializationInfo info,
          System.Runtime.Serialization.StreamingContext context)
            : base(info, context) { }
    }
}
