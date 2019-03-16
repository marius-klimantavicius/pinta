using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using Marius.Script.Pinta;
using Marius.Script.Pinta.Reflection;

namespace Marius.Script.Test
{
    public static class PintaTestFileGeneratorV1
    {
        private const bool _emitBigEndian = false;
        private static readonly object _defaultInternalFunctions;

        static PintaTestFileGeneratorV1()
        {
            _defaultInternalFunctions = new { assertEqual = 0 };
        }

        public static void ArraySum(string filename)
        {
            var program = new PintaProgramBuilder(_emitBigEndian);

            var main = program.DefineFunction("Main");
            program.StartFunction = main;

            var glo = program.GetGlobal("glo");
            var gg = program.GetGlobal("gg");

            var arr = main.DefineVariable("arr");
            var index = main.DefineVariable("index");
            var sum = main.DefineVariable("sum");

            main.Emit(PintaCode.LoadInteger, 5);
            main.Emit(PintaCode.NewArray);
            main.Emit(PintaCode.StoreLocal, arr);

            main.Emit(PintaCode.LoadIntegerZero);
            main.Emit(PintaCode.StoreLocal, index);

            main.Emit(PintaCode.LoadDecimalZero);
            main.Emit(PintaCode.StoreLocal, sum);

            for (int i = 0; i < 5; i++)
            {
                main.Emit(PintaCode.LoadInteger, (i + 1) * 5);
                main.Emit(PintaCode.ConvertDecimal);
                main.Emit(PintaCode.LoadInteger, i);
                main.Emit(PintaCode.LoadLocal, arr);
                main.Emit(PintaCode.StoreItem);
            }

            var breakLabel = main.DefineLabel();
            var continueLabel = main.DefineLabel();
            var startLabel = main.DefineLabel();

            main.Emit(PintaCode.Jump, continueLabel);

            main.Emit(startLabel);

            main.Emit(PintaCode.LoadLocal, index);
            main.Emit(PintaCode.LoadLocal, arr);
            main.Emit(PintaCode.LoadItem);

            main.Emit(PintaCode.LoadLocal, sum);
            main.Emit(PintaCode.Add);
            main.Emit(PintaCode.StoreLocal, sum);

            main.Emit(PintaCode.LoadIntegerOne);
            main.Emit(PintaCode.LoadLocal, index);
            main.Emit(PintaCode.Add);
            main.Emit(PintaCode.StoreLocal, index);

            main.Emit(continueLabel);

            main.Emit(PintaCode.LoadLocal, index);
            main.Emit(PintaCode.LoadLocal, arr);
            main.Emit(PintaCode.GetLength);

            main.Emit(PintaCode.CompareLessThan);
            main.Emit(PintaCode.JumpNotZero, startLabel);

            main.Emit(breakLabel);

            main.Emit(PintaCode.LoadLocal, sum);
            main.Emit(PintaCode.StoreGlobal, glo);

            main.Emit(PintaCode.LoadLocal, index);
            main.Emit(PintaCode.StoreGlobal, gg);

            main.Emit(PintaCode.Exit);

            SaveTestFile(program, filename);
        }

        public static void LoadConstants(string filename)
        {
            var program = new PintaProgramBuilder(_emitBigEndian);

            var main = program.DefineFunction("main");
            program.StartFunction = main;

            var gnull = program.GetGlobal("gnull");
            var gione = program.GetGlobal("gione");
            var gizero = program.GetGlobal("gizero");
            var gimpi = program.GetGlobal("gimpi");
            var gdone = program.GetGlobal("gdone");
            var gdzero = program.GetGlobal("gdzero");
            var gstring1 = program.GetGlobal("gstring1");
            var gstringLong = program.GetGlobal("gstringLong");

            main.Emit(PintaCode.LoadNull);
            main.Emit(PintaCode.StoreGlobal, gnull);

            main.Emit(PintaCode.LoadIntegerOne);
            main.Emit(PintaCode.StoreGlobal, gione);

            main.Emit(PintaCode.LoadIntegerZero);
            main.Emit(PintaCode.StoreGlobal, gizero);

            main.Emit(PintaCode.LoadInteger, -31415);
            main.Emit(PintaCode.StoreGlobal, gimpi);

            main.Emit(PintaCode.LoadDecimalOne);
            main.Emit(PintaCode.StoreGlobal, gdone);

            main.Emit(PintaCode.LoadDecimalZero);
            main.Emit(PintaCode.StoreGlobal, gdzero);

            main.Emit(PintaCode.LoadString, "1");
            main.Emit(PintaCode.StoreGlobal, gstring1);

            main.Emit(PintaCode.LoadString, "A very very very long string, that is extremely long. Well I am lying a bit. Well... A lot. This is not that long, but still good enough. I guess... I might need like a kilobyte string or even longer.\nMaybe I should add special function to read file? What I am talking (writing) about? There are no special functions yet ;))");
            main.Emit(PintaCode.StoreGlobal, gstringLong);

            main.Emit(PintaCode.Exit);

            SaveTestFile(program, filename);
        }

        public static void Substring(string filename)
        {
            var program = new PintaProgramBuilder(_emitBigEndian);
            var main = program.DefineFunction("main");

            program.StartFunction = main;

            var empty = program.GetGlobal("empty");
            var whole = program.GetGlobal("whole");
            var fr = program.GetGlobal("fr");

            main.Emit(PintaCode.LoadInteger, 1);
            main.Emit(PintaCode.LoadIntegerZero);
            main.Emit(PintaCode.LoadString, "qwertyuiop");
            main.Emit(PintaCode.Substring);
            main.Emit(PintaCode.StoreGlobal, empty);

            main.Emit(PintaCode.LoadIntegerZero);
            main.Emit(PintaCode.LoadInteger, 10);
            main.Emit(PintaCode.LoadString, "qwertyuiop");
            main.Emit(PintaCode.Substring);
            main.Emit(PintaCode.StoreGlobal, whole);

            main.Emit(PintaCode.LoadInteger, 3);
            main.Emit(PintaCode.LoadInteger, 3);
            main.Emit(PintaCode.LoadString, "qwertyuiop");
            main.Emit(PintaCode.Substring);
            main.Emit(PintaCode.StoreGlobal, fr);

            main.Emit(PintaCode.Exit);

            SaveTestFile(program, filename);
        }

        public static void SimpleSpecialFunction(string filename)
        {
            var program = new PintaProgramBuilder(_emitBigEndian);
            var main = program.DefineFunction("main");

            program.StartFunction = main;

            main.Emit(PintaCode.LoadInteger, 6);
            main.EmitCallInternal(0, 1);

            main.Emit(PintaCode.LoadString, "Well, hello");
            main.Emit(PintaCode.LoadString, "Hey there");
            main.EmitCallInternal(1, 2);

            main.Emit(PintaCode.LoadString, "314.15");
            main.Emit(PintaCode.ConvertDecimal);
            main.EmitCallInternal(2, 1);

            main.Emit(PintaCode.Exit);

            SaveTestFile(program, filename);
        }

        public static void SimpleScript(string filename)
        {
            SaveScriptTestFile(@"
var greeting = 'Hello, ' + name + '!';
assertEqual('Hello, World!', greeting, '\'Hello, World!\' == greeting');
", filename, _defaultInternalFunctions);
        }

        public static void SimpleForLoopScript(string filename)
        {
            SaveScriptTestFile(@"
var data = new Array(10);
data[0] = 1;
data[1] = 1;

for (var i = 2; i < 10; i++) {
    data[i] = data[i - 2] + data[i - 1];
}

assertEqual(data[9], 55, 'data[9] == 55');

", filename, _defaultInternalFunctions);
        }

        public static void OperationScripts(string filename)
        {
            SaveScriptTestFile(@"
var a = 5;
var b = 3.5;
var c = 'And the result is: ';
var nullValue;

var sum = a + b;
var cc = concat(c, sum);
var sub = cc.substring(5, 2);
var mul = [a, b, c, sum, cc, sub];

assertEqual(5                       , mul[0], 'mul[0] == 5');
assertEqual(3.5                     , mul[1], 'mul[1] == 3.5');
assertEqual('And the result is: '   , mul[2], 'mul[2] == \'And the result is: \'');
assertEqual(8.5                     , mul[3], 'mul[3] == 8.5');
assertEqual('And the result is: 8.5', mul[4], 'mul[4] == \'And the result is: 8.5\'');

assertEqual(17.5, a * b, '17.5 == a * b');

assertEqual(3.5, '12.9' - '9.4', '\'12.9\' - \'9.4\' == 3.5');

var d;

assertEqual('he', d || sub, 'd || sub == \'he\'');
assertEqual(mul, d || null || mul, 'd || null || mul == mul');

a = cc.substring(19, cc.length - 19);
assertEqual('8.5', a, '\'8.5\' == a');

if (a && b && c)
    assertEqual(b, a = b, 'a = b = b');
else
    assertEqual(0, 1, 'a && b && c failed');

var arr = new Array(20);
arr[0] = 1;
arr[1] = a ? 1 : -1;

for (var i = 2; i < arr.length; i++) {
    arr[i] = arr[i - 1] + arr[i - 2];
}

assertEqual(89, arr[10], 'arr[10] == 89');
assertEqual(3.5, 7.0 / 2.0, '7 / 2 = 3.5');
assertEqual('He is the one', 'He ' + 'is ' + 'the ' + 'one', '\'He is the one\' = \'He \'+\'is \'+\'the \'+\'one\'');

assertEqual('ęėį', substring(nullValue || 'ąčęėįšųū', 2, 3), 'substring(null || \'eei\', 2, 3)');

arr = new Array(10);
i = 0;
arr[i++] = 'H';
arr[i++] = 'e';
arr[i++] = 'l';
arr[i++] = 'l';
arr[i++] = 'o';
arr[i++] = ',';
arr[i++] = ' ';
arr[i++] = 'Worl';
arr[i++] = 'd';
arr[i++] = '!';

sum = null;

for (var i = 0; i < arr.length; i++) {
    sum = sum.concat(arr[i]);
}

assertEqual('Hello, World!', sum, '\'Hello, World!\' = concat(arr)');
assertEqual('H', sum[0], 'H == sum[0]');

assertEqual(1, 'a' < 'ab', 'a < ab');
assertEqual(1, 'a'[0] < 'ab', 'a[0] < ab');
assertEqual(1, 'b' > 'ab', 'b > ab');
assertEqual(1, 'b'[0] > 'ab', 'b[0] > ab');
assertEqual(1, 'b'[0] > '', 'b[0] > []');
assertEqual(1, 'bb'[0] < 'cccc'[0], 'bb[0] < cccc[0]');
assertEqual(1, 'b'[0] == 'cbcc'[1], 'b[0] == cbcc[1]');

var hexValue = hex('C0FFEE');
assertEqual(0xC0, hexValue[0], 'C0 == hex[0]');
assertEqual(0xFF, hexValue[1], 'FF == hex[1]');
assertEqual(0xEE, hexValue[2], 'EE == hex[2]');
hexValue[2] = 0x55;
assertEqual(0x55, hexValue[2], '55 == hex[2]');
assertEqual(3, hexValue.length, '3 == hex.length');

", filename, _defaultInternalFunctions);
        }

        public static void ExitOnGlobalReturn(string filename)
        {
            SaveScriptTestFile(@"

var i = 0;
while (i < 5000) {
    i++;
    if (i > 1000)
        return;
}

unreachable();

", filename, new { unreachable = 0 });
        }

        public static void UnicodeString(string filename)
        {
            using (var stream = typeof(PintaTestFileGeneratorV1).Assembly.GetManifestResourceStream("Marius.Script.Test.TestFiles.unicode_string.js"))
            using (var reader = new StreamReader(stream))
            {
                SaveScriptTestFile(reader.ReadToEnd(), filename, new { outline = 0 });
            }
        }

        public static void UnicodeOutline(string filename)
        {
            SaveScriptTestFile(@"outline('Ąžuolų');", filename, new { outline = 0 });
        }

        public static void SubBufferOutline(string filename)
        {
            SaveScriptTestFile(@"
outline(hex('000102030405060708090A0B0C0D0E0F'));
expect(hex('000102030405060708090A0B0C0D0E0F'), 'No sub buffer');

// allow omit length
outline(hex('000102030405060708090A0B0C0D0E0F'), 5);
expect(hex('05060708090A0B0C0D0E0F'), 'Only offset 5');

outline(hex('000102030405060708090A0B0C0D0E0F'), 5, 0);
outline(hex('000102030405060708090A0B0C0D0E0F'), 6, 0);
outline(hex('000102030405060708090A0B0C0D0E0F'), 3, 0);
outline(hex('000102030405060708090A0B0C0D0E0F'), 4, 0);
outline(hex('000102030405060708090A0B0C0D0E0F'), 10, 0);
outline(hex('000102030405060708090A0B0C0D0E0F'), 5, 1);
expect(hex('05'), 'Offset 5, length 0 and length 1');

outline(hex('000102030405060708090A0B0C0D0E0F'), 15, 0);
outline(hex('000102030405060708090A0B0C0D0E0F'), 5, 2);
expect(hex('0506'), 'Offset 16, length 0 and offset 5, length 2');

", filename, new { outline = 0, expect = 1 });
        }

        public static void StringOperationsWithGlobalVariables(string filename)
        {
            SaveScriptTestFile(@"
assertEqual(3.5, a - b, 'Globals: \'12.9\' - \'9.4\' == 3.5');
", filename, _defaultInternalFunctions);
        }

        public static void SubtractTwoStrings(string filename)
        {
            SaveScriptTestFile(@"
var value = '9' - '0';
var result;
if (value > 9) {
    result = 0;
}
else {
    result = 1;
}
assertEqual(1, result, '\'9\' - \'0\' == 9');
", filename, _defaultInternalFunctions);
        }

        private static void SaveScriptTestFile(string source, string filename, object internalFunctions)
        {
            var compiler = new PintaCompiler(_emitBigEndian);
            var path = Path.Combine(@"..\..\..\Marius.Pinta.Test.Files\", filename);
            path = Path.GetFullPath(path);
            using (var output = new FileStream(path, FileMode.Create))
            {
                if (internalFunctions != null)
                    compiler.SetInternalFunctions(MakeDictionary(internalFunctions));

                compiler.CompileString(source, output);

                output.Flush();
            }
        }

        private static Dictionary<string, uint> MakeDictionary(object internalFunctions)
        {
            var type = internalFunctions.GetType();
            var properties = type.GetProperties(System.Reflection.BindingFlags.DeclaredOnly | System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Instance);
            var result = new Dictionary<string, uint>();

            foreach (var item in properties)
            {
                if (item.PropertyType == typeof(int))
                {
                    result[item.Name] = (uint)(int)item.GetValue(internalFunctions, null);
                }
                else if (item.PropertyType == typeof(uint))
                {
                    result[item.Name] = (uint)item.GetValue(internalFunctions, null);
                }
            }

            return result;
        }

        private static void SaveTestFile(PintaProgramBuilder program, string filename)
        {
            program.Save(Path.Combine(@"..\..\..\Marius.Pinta.Test.Files\", filename));
        }
    }
}
