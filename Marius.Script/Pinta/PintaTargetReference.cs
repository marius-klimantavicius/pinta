using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Pinta
{
    public abstract class PintaTargetReference : IDisposable
    {
        public abstract void Prepare(PintaGenerateCode generate, bool loadHint);
        public abstract void Load(PintaGenerateCode generate);
        public abstract void Store(PintaGenerateCode generate);
        public abstract void Dispose();

        public virtual void Cleanup(PintaGenerateCode generate)
        {
        }
    }
}
