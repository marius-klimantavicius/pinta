using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Parser.Ast;

namespace Marius.Pinta.Script.Optimization
{
    public class PintaCodeOptimizer
    {
        public IEnumerable<Statement> Optimize(IEnumerable<Statement> body)
        {
            var matchRewriter = new PintaCodeMatchRewriter();
            body = matchRewriter.Rewrite(body);

            return body;
        }
    }
}
