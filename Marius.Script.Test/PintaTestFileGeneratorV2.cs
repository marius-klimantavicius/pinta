using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using Marius.Pinta.Script;
using Marius.Pinta.Script.Code;
using Marius.Pinta.Script.Parser;
using Marius.Pinta.Script.Reflection;

namespace Marius.Script.Test
{
    public static class PintaTestFileGeneratorV2
    {
        private const bool _emitBigEndian = false;

        private static readonly object _defaultInternalFunctions;

        static PintaTestFileGeneratorV2()
        {
            _defaultInternalFunctions = new { assertEqual = 0 };
        }

        public static void ArraySum(string filename)
        {
            var program = new PintaCodeModule();

            var main = program.DefineFunction("Main");
            program.StartFunction = main;

            var glo = program.GetGlobal("glo");
            var gg = program.GetGlobal("gg");

            var arr = main.DeclareLocal("arr");
            var index = main.DeclareLocal("index");
            var sum = main.DeclareLocal("sum");

            var code = main.GetCodeGenerator();

            code.Emit(PintaCode.LoadInteger, 5);
            code.Emit(PintaCode.NewArray);
            code.Emit(PintaCode.StoreLocal, arr);

            code.Emit(PintaCode.LoadIntegerZero);
            code.Emit(PintaCode.StoreLocal, index);

            code.Emit(PintaCode.LoadDecimalZero);
            code.Emit(PintaCode.StoreLocal, sum);

            for (int i = 0; i < 5; i++)
            {
                code.Emit(PintaCode.LoadInteger, (i + 1) * 5);
                code.Emit(PintaCode.ConvertDecimal);
                code.Emit(PintaCode.LoadInteger, i);
                code.Emit(PintaCode.LoadLocal, arr);
                code.Emit(PintaCode.StoreItem);
            }

            var breakLabel = code.DefineLabel();
            var continueLabel = code.DefineLabel();
            var startLabel = code.DefineLabel();

            code.Emit(PintaCode.Jump, continueLabel);

            code.MarkLabel(startLabel);

            code.Emit(PintaCode.LoadLocal, index);
            code.Emit(PintaCode.LoadLocal, arr);
            code.Emit(PintaCode.LoadItem);

            code.Emit(PintaCode.LoadLocal, sum);
            code.Emit(PintaCode.Add);
            code.Emit(PintaCode.StoreLocal, sum);

            code.Emit(PintaCode.LoadIntegerOne);
            code.Emit(PintaCode.LoadLocal, index);
            code.Emit(PintaCode.Add);
            code.Emit(PintaCode.StoreLocal, index);

            code.MarkLabel(continueLabel);

            code.Emit(PintaCode.LoadLocal, index);
            code.Emit(PintaCode.LoadLocal, arr);
            code.Emit(PintaCode.GetLength);

            code.Emit(PintaCode.CompareLessThan);
            code.Emit(PintaCode.JumpNotZero, startLabel);

            code.MarkLabel(breakLabel);

            code.Emit(PintaCode.LoadLocal, sum);
            code.Emit(PintaCode.StoreGlobal, glo);

            code.Emit(PintaCode.LoadLocal, index);
            code.Emit(PintaCode.StoreGlobal, gg);

            code.Emit(PintaCode.Exit);

            SaveTestFile(program, filename);
        }

        public static void LoadConstants(string filename)
        {
            var program = new PintaCodeModule();

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

            var code = main.GetCodeGenerator();
            code.Emit(PintaCode.LoadNull);
            code.Emit(PintaCode.StoreGlobal, gnull);

            code.Emit(PintaCode.LoadIntegerOne);
            code.Emit(PintaCode.StoreGlobal, gione);

            code.Emit(PintaCode.LoadIntegerZero);
            code.Emit(PintaCode.StoreGlobal, gizero);

            code.Emit(PintaCode.LoadInteger, -31415);
            code.Emit(PintaCode.StoreGlobal, gimpi);

            code.Emit(PintaCode.LoadDecimalOne);
            code.Emit(PintaCode.StoreGlobal, gdone);

            code.Emit(PintaCode.LoadDecimalZero);
            code.Emit(PintaCode.StoreGlobal, gdzero);

            code.Emit(PintaCode.LoadString, "1");
            code.Emit(PintaCode.StoreGlobal, gstring1);

            code.Emit(PintaCode.LoadString, "A very very very long string, that is extremely long. Well I am lying a bit. Well... A lot. This is not that long, but still good enough. I guess... I might need like a kilobyte string or even longer.\nMaybe I should add special function to read file? What I am talking (writing) about? There are no special functions yet ;))");
            code.Emit(PintaCode.StoreGlobal, gstringLong);

            code.Emit(PintaCode.Exit);

            SaveTestFile(program, filename);
        }

        public static void Substring(string filename)
        {
            var program = new PintaCodeModule();
            var main = program.DefineFunction("main");

            program.StartFunction = main;

            var empty = program.GetGlobal("empty");
            var whole = program.GetGlobal("whole");
            var fr = program.GetGlobal("fr");

            var code = main.GetCodeGenerator();
            code.Emit(PintaCode.LoadInteger, 1);
            code.Emit(PintaCode.LoadIntegerZero);
            code.Emit(PintaCode.LoadString, "qwertyuiop");
            code.Emit(PintaCode.Substring);
            code.Emit(PintaCode.StoreGlobal, empty);

            code.Emit(PintaCode.LoadIntegerZero);
            code.Emit(PintaCode.LoadInteger, 10);
            code.Emit(PintaCode.LoadString, "qwertyuiop");
            code.Emit(PintaCode.Substring);
            code.Emit(PintaCode.StoreGlobal, whole);

            code.Emit(PintaCode.LoadInteger, 3);
            code.Emit(PintaCode.LoadInteger, 3);
            code.Emit(PintaCode.LoadString, "qwertyuiop");
            code.Emit(PintaCode.Substring);
            code.Emit(PintaCode.StoreGlobal, fr);

            code.Emit(PintaCode.Exit);

            SaveTestFile(program, filename);
        }

        public static void SimpleSpecialFunction(string filename)
        {
            var program = new PintaCodeModule();
            var main = program.DefineFunction("main");

            program.StartFunction = main;

            var code = main.GetCodeGenerator();
            code.Emit(PintaCode.LoadInteger, 6);
            code.Emit(PintaCode.CallInternal, 0, 1);

            code.Emit(PintaCode.LoadString, "Well, hello");
            code.Emit(PintaCode.LoadString, "Hey there");
            code.Emit(PintaCode.CallInternal, 1, 2);

            code.Emit(PintaCode.LoadString, "314.15");
            code.Emit(PintaCode.ConvertDecimal);
            code.Emit(PintaCode.CallInternal, 2, 1);

            code.Emit(PintaCode.Exit);

            SaveTestFile(program, filename);
        }

        public static void SimpleScript(string filename)
        {
            SaveScriptTestFile(@"
var greeting = 'Hello, ' + name + '!';
assertEqual('Hello, World!', greeting, '\'Hello, World!\' == greeting');
", filename, _defaultInternalFunctions);
        }

        public static void ScriptClosure(string filename)
        {
            SaveScriptTestFile(@"
var greeting = 'Hello, ' + name;

(function() {
    greeting = greeting + withname();


})();

function withname() {
    var num = 4;
    
    function withinner() {
        var outer = 1;
        return function() {
            num = num + outer;
        };
    }

    if (num > 4)
        return '??';

    var infn = withinner();
    infn();
    if (num > 4)
        return '!';
    return ':(';
}

assertEqual('Hello, World!', greeting, '\'Hello, World!\' == greeting');
", filename, _defaultInternalFunctions);
        }

        public static void GlobalFunctionObjects(string filename)
        {
            SaveScriptTestFile(@"
var a = glob;
var b = a(3, 4);

", filename, new object());
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
(function() {
var i = 0;
while (i < 5000) {
    i++;
    if (i > 1000)
        return;
}

unreachable();
})();
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

        public static void NewFunction(string filename)
        {
            SaveScriptTestFile(@"
var i = 0;
i++;
assertEqual(1, i, 'i++ == 1');

function Model() {
    var self = this;
    
    self.index = 5;
}

Model.prototype.fromproto = 6.28;

var model = new Model();
assertEqual(5, model.index, 'model.index == 5');
assertEqual(5, model['index'], 'model.index == 5');
assertEqual(6.28, model.fromproto, 'model.fromproto == 6.28');

model.fromproto = 2.78;
assertEqual(2.78, model.fromproto, 'model.fromproto == 2.78');
assertEqual(6.28, Model.prototype.fromproto, 'Model.prototype.fromproto == 6.28');

", filename, _defaultInternalFunctions);
        }

        public static void GlobalsAccessibleViaProperties(string filename)
        {
            SaveScriptTestFile(@"
g1 = 5;
g2 = 6;

assertEqual(5, this.g1, 'this.g1 == 5');
assertEqual(5, this[""g1""], 'this.g1 == 5');
assertEqual(6, this.g2, 'this.g2 == 5');
assertEqual(6, this[""g2""], 'this.g2 == 5');

this.g2 = 628;
assertEqual(628, g2, 'g2 == 628');

a1  = 1 ;
a2  = 2 ;
a3  = 3 ;
a4  = 4 ;
a5  = 5 ;
a6  = 6 ;
a7  = 7 ;
a8  = 8 ;
a9  = 9 ;
a10 = 10;
a11 = 11;
a12 = 12;
a13 = 13;
a14 = 14;
a15 = 15;
a16 = 16;
a17 = 17;
a18 = 18;
a19 = 19;
a20 = 20;
a21 = 21;
a22 = 22;
a23 = 23;
a24 = 24;
a25 = 25;
a26 = 26;
a27 = 27;
a28 = 28;
a29 = 29;
a30 = 30;
a31 = 31;
a32 = 32;
a33 = 33;
a34 = 34;
a35 = 35;
a36 = 36;
a37 = 37;
a38 = 38;
a39 = 39;
a40 = 40;
a41 = 41;
a42 = 42;
a43 = 43;
a44 = 44;
a45 = 45;
a46 = 46;
a47 = 47;
a48 = 48;

for (var i = 1; i < 49; i++)
    assertEqual(i, this['a' + i], 'this[a' + i + '] == ' + i);

", filename, _defaultInternalFunctions);
        }

        public static void Require(string filename)
        {
            SaveScriptTestFile(@"
var ex = require('..\\Marius.Pinta.Test.Files\\require-sub-v2.pint');
var sub = ex.sub;
assertEqual(42, sub.answer, 'Answer from sub');
assertEqual(-42, sub.getAnswer(), 'Answer 2 from sub');
", filename, _defaultInternalFunctions);

            SaveScriptTestFile(@"
var answerino = -42;
function Object() {
    this.answer = 42;
    this.getAnswer = function() {
        return answerino;
    };
}

exports.sub = new Object();

assertEqual(42, exports.sub.answer, 'Answer from exports.sub');
assertEqual(-42, exports.sub.getAnswer(), 'Answer 2 from exports.sub');

", "require-sub-v2.pint", _defaultInternalFunctions);
        }

        public static void RequireReturnNewExports(string filename)
        {
            SaveScriptTestFile(@"
var sub = require('..\\Marius.Pinta.Test.Files\\require-exports-sub-v2.pint');
assertEqual(42, sub.answer, 'Answer from sub');
assertEqual(-42, sub.getAnswer(), 'Answer 2 from sub');
", filename, _defaultInternalFunctions);

            SaveScriptTestFile(@"
var answerino = -42;
function Object() {
    this.answer = 42;
    this.getAnswer = function() {
        return answerino;
    };
}

exports = new Object();

assertEqual(42, exports.answer, 'Answer from exports');
assertEqual(-42, exports.getAnswer(), 'Answer 2 from exports');

", "require-exports-sub-v2.pint", _defaultInternalFunctions);
        }

        public static void ConstructTest(string filename)
        {
            SaveScriptTestFile(@"
function ctor() {
    this.theValue = 42;
    this.getValue = getValue;

    function getValue() {
        return this.theValue;
    }
}

var inst = new ctor();
assertEqual(42, inst.getValue(), '42 == inst.getValue()');

", filename, _defaultInternalFunctions);
        }

        public static void NativeFunctionCallingManagedFunction(string filename)
        {
            SaveScriptTestFile(@"

var wasCalled = false;
function callMe() {
    wasCalled = true;
}

nativeCaller(callMe);

assertEqual(true, wasCalled, 'callMe was not called');

", filename, _defaultInternalFunctions);
        }

        public static void CreateObjectUsingObjectLiteral(string filename)
        {
            SaveScriptTestFile(@"
var inst = {
    theValue: 42,
    getValue: function() {
        return inst.theValue;
    }
};
assertEqual(42, inst.getValue(), '42 == inst.getValue()');

", filename, _defaultInternalFunctions);
        }

        public static void PatternSimple(string filename)
        {
            SaveScriptTestFile(@"
var pan = '123456789';

var nok = 0;

function checkTrue(value) {
    if (!value)
        nok++;
}

function checkFalse(value) {
    if (value)
        nok++;
}

checkTrue (pan.length >= 2 && pan[1] == '2');
checkTrue ($match_simple(pan, '2', 1));
checkFalse (pan.length >= 2 && pan[1] == '3');
checkFalse ($match_simple(pan, '3', 1));
checkTrue (pan.length >= 6 && pan.substring(0, 6) == '123456');
checkTrue ($match_simple(pan, '123456'));
checkTrue ($match_simple(pan, '123456', 0));
checkTrue ($match_simple(pan, '123456', 0, 6));
checkFalse (pan.length >= 6 && pan.substring(0, 6) == '123455');
checkFalse ($match_simple(pan, '123455'));
checkFalse ($match_simple(pan, '123455', 0));
checkFalse ($match_simple(pan, '123556', 0, 6));

checkTrue($match_simple(pan, '789', -3));
checkTrue($match_simple(pan, '456', 3, 2));

checkFalse($match_simple(pan, '456', 3, 4));
checkFalse($match_simple(pan, '4567890', 3, 7));

pan = '123';

checkFalse ($match_simple(pan, '123456'));
checkFalse ($match_simple(pan, '9', 8));

result = nok == 0;

", filename, _defaultInternalFunctions);
        }

        public static void PatternPan(string filename)
        {
            SaveScriptTestFile(@"
var pan = '123456789';

var nok = 0;

function checkTrue(value) {
    if (!value)
        nok++;
}

function checkFalse(value) {
    if (value)
        nok++;
}

checkTrue ('120000000' <= pan && pan <= '129999999');
checkTrue ($match_pan(pan, '12', 9));
checkFalse ('130000000' <= pan && pan <= '139999999');
checkFalse ($match_pan(pan, '13', 9));
checkFalse ('1200' <= pan && pan <= '1299');
checkFalse ($match_pan(pan, '12', 4));
checkFalse ('1200000000' <= pan && pan <= '1299999999');
checkFalse ($match_pan(pan, '12', 10));

result = nok == 0;

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

        public static void TailCallSimple(string filename)
        {
            SaveScriptTestFile(@"

function count(n) {
    return count2(n, 0);
}

function count2(n, ps) {
    if (n == 0) return ps;
    return count2(n - 1, ps + 1);
}

var value = count(10000);
assertEqual(10000, value, 'Tail call');

", filename, _defaultInternalFunctions);
        }

        public static void TailCallInvoke(string filename)
        {
            SaveScriptTestFile(@"

function counter() {
    this.count = function(n) {
        return this.count2(n, 0);
    };

    this.count2 = function(n, ps) {
        if (n == 0) return ps;
        return this.count2(n - 1, ps + 1);
    };
}

var cnt = new counter();
var value = cnt.count(10000);
assertEqual(10000, value, 'Tail call');

", filename, _defaultInternalFunctions);
        }

        private static void SaveScriptTestFile(string source, string filename, object internalFunctions)
        {
            var compiler = new PintaCompiler();
            var path = Path.Combine(@"..\..\..\Marius.Pinta.Test.Files\", filename);
            path = Path.GetFullPath(path);
            using (var output = new FileStream(path, FileMode.Create))
            {
                if (internalFunctions != null)
                    compiler.SetInternalFunctions(MakeDictionary(internalFunctions));

                compiler.CompileString(source, output, _emitBigEndian);

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

        private static void SaveTestFile(PintaCodeModule module, string filename)
        {
            using (var output = File.OpenWrite(Path.Combine(@"..\..\..\Marius.Pinta.Test.Files\", filename)))
            {
                module.Save(output, _emitBigEndian);
            }
        }
    }
}
