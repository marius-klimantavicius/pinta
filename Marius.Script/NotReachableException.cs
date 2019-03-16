using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script
{
    [Serializable]
    public class NotReachableException: Exception
    {
        public NotReachableException() : this("This code path should not be reached") { }
        public NotReachableException(string message) : base(message) { }
        public NotReachableException(string message, Exception inner) : base(message, inner) { }
        protected NotReachableException(
          System.Runtime.Serialization.SerializationInfo info,
          System.Runtime.Serialization.StreamingContext context)
            : base(info, context) { }
    }
}
