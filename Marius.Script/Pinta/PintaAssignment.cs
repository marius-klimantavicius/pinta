using Marius.Script.Pinta.Reflection;
using Marius.Script.Tree;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Pinta
{
    public class PintaAssignment : IDisposable
    {
        private PintaGenerateCode _generate;
        private ScriptExpression _target;

        private PintaTargetReference _reference;

        private class PrepareVisitor : ScriptVisitor
        {
            public PintaTargetReference Reference { get; private set; }

            public override void Visit(ScriptArrayAccessExpression expression)
            {
                Reference = new PintaArrayReference(expression);
            }

            public override void Visit(ScriptNameExpression expression)
            {
                Reference = new PintaNameReference(expression);
            }
        }

        public PintaAssignment(PintaGenerateCode generate, ScriptExpression target)
        {
            _generate = generate;
            _target = target;
        }

        public void Prepare(bool expectLoad = false)
        {
            var prepare = new PrepareVisitor();
            _target.Accept(prepare);
            _reference = prepare.Reference;
            if (_reference == null)
                throw new ScriptSyntaxException("Cannot assign");

            _reference.Prepare(_generate, expectLoad);
        }

        public void Load()
        {
            _reference.Load(_generate);
        }

        public void Assign()
        {
            _reference.Store(_generate);
        }

        public void Cleanup()
        {
            _reference.Cleanup(_generate);
        }

        public void Dispose()
        {
            if (_reference != null)
                _reference.Dispose();
        }
    }
}
