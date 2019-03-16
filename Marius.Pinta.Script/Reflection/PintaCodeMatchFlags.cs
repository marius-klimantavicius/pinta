using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection
{
    public enum PintaCodeMatchFlags
    {
        // 0x?0 - 0x?F
        ArgumentCount1 = 0x01,
        ArgumentCount2 = 0x02,
        ArgumentCount3 = 0x03,
        ArgumentCount4 = 0x04,
        
        // 0x0? - 0xF?
        Simple = 0x00,
        PrimaryAccountNumber = 0x10,
    }
}
