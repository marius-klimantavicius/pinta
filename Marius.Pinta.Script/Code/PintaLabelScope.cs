using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Reflection;

namespace Marius.Pinta.Script.Code
{
    public class PintaLabelScope
    {
        private class Label
        {
            public Label Parent;

            public HashSet<string> Names;
            public PintaCodeLabel BreakLabel;
            public PintaCodeLabel ContinueLabel;

            public Label(Label parent, HashSet<string> names, PintaCodeLabel breakLabel, PintaCodeLabel continueLabel)
            {
                Parent = parent;

                Names = names;
                BreakLabel = breakLabel;
                ContinueLabel = continueLabel;
            }
        }

        private class Pop : IDisposable
        {
            private PintaLabelScope _scope;

            public Pop(PintaLabelScope scope)
            {
                _scope = scope;
            }

            public void Dispose()
            {
                _scope._current = _scope._current.Parent;
            }
        }

        private Pop _pop;
        private Label _current;

        public PintaLabelScope()
        {
            _pop = new Pop(this);
        }

        public IDisposable Push(string labelSet, PintaCodeLabel breakLabel, PintaCodeLabel continueLabel)
        {
            var names = default(HashSet<string>);
            if (!string.IsNullOrEmpty(labelSet))
                names = new HashSet<string>(labelSet.Split(':'), StringComparer.Ordinal);

            _current = new Label(_current, names, breakLabel, continueLabel);
            return _pop;
        }

        public PintaCodeLabel GetBreak()
        {
            var current = _current;
            while (current != null)
            {
                if (current.BreakLabel != null)
                    return current.BreakLabel;
                current = current.Parent;
            }

            return null;
        }

        public PintaCodeLabel GetBreak(string name)
        {
            var current = _current;
            while (current != null)
            {
                if (current.Names != null && current.Names.Contains(name))
                    return current.BreakLabel;

                current = current.Parent;
            }

            return null;
        }

        public PintaCodeLabel GetContinue()
        {
            var current = _current;
            while (current != null)
            {
                if (current.ContinueLabel != null)
                    return current.ContinueLabel;
                current = current.Parent;
            }

            return null;
        }

        public PintaCodeLabel GetContinue(string name)
        {
            var current = _current;
            while (current != null)
            {
                if (current.Names != null && current.Names.Contains(name))
                    return current.ContinueLabel;

                current = current.Parent;
            }

            return null;
        }
    }
}
