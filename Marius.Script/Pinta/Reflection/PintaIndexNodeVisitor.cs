using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaIndexNodeVisitor : PintaNodeVisitor
    {
        public override void Visit(PintaProgramBuilder program)
        {
            var index = 0U;
            foreach (var item in program.Strings)
            {
                item.Value.Data.Index = index;
                index++;
            }

            index = 0U;
            foreach (var item in program.Blobs)
            {
                item.Value.Data.Index = index;
                index++;
            }

            index = 0U;
            foreach (var item in program.Globals)
            {
                item.Value.Data.Index = index;
                index++;
            }

            for (var i = 0; i < program.Functions.Count; i++)
            {
                var current = program.Functions[i];
                current.Data.Index = unchecked((uint)i);

                for (var m = 0; m < current.Variables.Count; m++)
                {
                    var variable = current.Variables[m];
                    variable.Data.Index = unchecked((uint)m);
                }

                for (var m = 0; m < current.Parameters.Count; m++)
                {
                    var parameter = current.Parameters[m];
                    parameter.Data.Index = unchecked((uint)m);
                }
            }
        }
    }
}
