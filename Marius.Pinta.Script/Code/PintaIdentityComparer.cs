using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace Marius.Pinta.Script.Code
{
    public class PintaIdentityComparer<T> : IEqualityComparer<T>
        where T: class
    {
        public static readonly PintaIdentityComparer<T> Instance = new PintaIdentityComparer<T>();

        public bool Equals(T x, T y)
        {
            return object.ReferenceEquals(x, y);
        }

        public int GetHashCode(T obj)
        {
            return RuntimeHelpers.GetHashCode(obj);
        }
    }
}
