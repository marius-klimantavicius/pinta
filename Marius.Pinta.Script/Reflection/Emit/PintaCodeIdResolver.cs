using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection.Emit
{
    public class PintaCodeIdResolver
    {
        public void Resolve(SortedDictionary<string, PintaCodeString> strings)
        {
            var index = 0U;
            foreach (var item in strings)
                item.Value.Id = index++;
        }

        public void Resolve(SortedDictionary<byte[], PintaCodeBinary> binary)
        {
            var index = 0U;
            foreach (var item in binary)
                item.Value.Id = index++;
        }

        public void Resolve(SortedDictionary<string, PintaCodeGlobal> globals)
        {
            var index = 0U;
            foreach (var item in globals)
                item.Value.Id = index++;
        }

        public void Resolve(List<PintaCodeFunction> functions)
        {
            var functionIndex = 0U;
            foreach (var item in functions)
            {
                item.Id = functionIndex++;

                var index = 0U;
                foreach (var local in item.Locals)
                    local.Id = index++;

                index = 0U;
                foreach (var parameter in item.Parameters)
                    parameter.Id = index++;
            }
        }
    }
}
