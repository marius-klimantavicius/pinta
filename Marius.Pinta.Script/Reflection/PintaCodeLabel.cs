using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection
{
    public class PintaCodeLabel
    {
        public uint Id { get; private set; }
        public bool IsMarked { get; set; }

        public int ActualOffset { get; set; }
        public int MinimumOffset { get; set; }
        public int MaximumOffset { get; set; }

        public PintaCodeLabel(uint id)
        {
            Id = id;
        }

        public override string ToString()
        {
            return "@" + Id;
        }
    }
}
