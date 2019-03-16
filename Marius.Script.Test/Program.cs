using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Dynamic;
using System.IO;
using System.Linq;
using System.Linq.Expressions;
using System.Runtime.CompilerServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using Marius.Script.Parsing;
using Marius.Script.Pinta;
using Marius.Script.Tree;

namespace Marius.Script.Test
{
    public static class Program
    {
        public static void Main(string[] args)
        {
            PintaTestFileGeneratorV1.ArraySum("array-sum.pint");
            PintaTestFileGeneratorV1.LoadConstants("load-const.pint");
            PintaTestFileGeneratorV1.Substring("simple-substring.pint");
            PintaTestFileGeneratorV1.SimpleSpecialFunction("simple-internal-function.pint");
            PintaTestFileGeneratorV1.SimpleScript("simple-script.pint");
            PintaTestFileGeneratorV1.SimpleForLoopScript("simple-for-loop.pint");
            PintaTestFileGeneratorV1.OperationScripts("operations-script.pint");
            PintaTestFileGeneratorV1.ExitOnGlobalReturn("exit-on-global-return.pint");
            PintaTestFileGeneratorV1.UnicodeString("unicode-string.pint");
            PintaTestFileGeneratorV1.UnicodeOutline("unicode-outline.pint");
            PintaTestFileGeneratorV1.SubBufferOutline("sub-buffer.pint");
            PintaTestFileGeneratorV1.StringOperationsWithGlobalVariables("string-operations-with-global-variables.pint");
            PintaTestFileGeneratorV2.SubtractTwoStrings("subtract-two-strings.pint");

            PintaTestFileGeneratorV2.ArraySum("array-sum-v2.pint");
            PintaTestFileGeneratorV2.LoadConstants("load-const-v2.pint");
            PintaTestFileGeneratorV2.Substring("simple-substring-v2.pint");
            PintaTestFileGeneratorV2.SimpleSpecialFunction("simple-internal-function-v2.pint");
            PintaTestFileGeneratorV2.SimpleScript("simple-script-v2.pint");
            PintaTestFileGeneratorV2.ScriptClosure("script-closure-v2.pint");
            PintaTestFileGeneratorV2.GlobalFunctionObjects("global-function-v2.pint");
            PintaTestFileGeneratorV2.SimpleForLoopScript("simple-for-loop-v2.pint");
            PintaTestFileGeneratorV2.OperationScripts("operations-script-v2.pint");
            PintaTestFileGeneratorV2.ExitOnGlobalReturn("exit-on-global-return-v2.pint");
            PintaTestFileGeneratorV2.UnicodeString("unicode-string-v2.pint");
            PintaTestFileGeneratorV2.UnicodeOutline("unicode-outline-v2.pint");
            PintaTestFileGeneratorV2.SubBufferOutline("sub-buffer-v2.pint");
            PintaTestFileGeneratorV2.StringOperationsWithGlobalVariables("string-operations-with-global-variables-v2.pint");
            PintaTestFileGeneratorV2.NewFunction("new-function-v2.pint");
            PintaTestFileGeneratorV2.GlobalsAccessibleViaProperties("globals-properties-v2.pint");
            PintaTestFileGeneratorV2.Require("require-v2.pint");
            PintaTestFileGeneratorV2.RequireReturnNewExports("require-exports-v2.pint");
            PintaTestFileGeneratorV2.ConstructTest("construct-v2.pint");
            PintaTestFileGeneratorV2.NativeFunctionCallingManagedFunction("native-call-managed-v2.pint");
            PintaTestFileGeneratorV2.CreateObjectUsingObjectLiteral("object-literal-v2.pint");
            PintaTestFileGeneratorV2.PatternSimple("pattern-simple-v2.pint");
            PintaTestFileGeneratorV2.PatternSimple("pattern-pan-v2.pint");
            PintaTestFileGeneratorV2.SubtractTwoStrings("subtract-two-strings-v2.pint");

            PintaSampleFileGenerator.Sample();
        }
    }
}
