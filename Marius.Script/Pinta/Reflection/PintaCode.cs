﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public enum PintaCode
    {
        Nop                     = 0x00,
        Add                     = 0x01,
        Subtract                = 0x02,
        Multiply                = 0x03,
        Divide                  = 0x04,
        Remainder               = 0x05,
        BitwiseAnd              = 0x06,
        BitwiseOr               = 0x07,
        ExclusiveOr             = 0x08,
        BitwiseExclusiveOr      = 0x09,
        Not                     = 0x0A,
        BitwiseNot              = 0x0B,
        Negate                  = 0x0C,
        CompareEqual            = 0x0D,
        CompareLessThan         = 0x0E,
        CompareMoreThan         = 0x0F,
        CompareNull             = 0x10,
        ConvertInteger          = 0x11,
        ConvertDecimal          = 0x12,
        ConvertString           = 0x13,
        NewArray                = 0x14,
        Concat                  = 0x15,
        Substring               = 0x16,
        Jump                    = 0x17,
        JumpZero                = 0x18,
        JumpNotZero             = 0x19,
        Call                    = 0x1A,
        CallInternal            = 0x1B,
        Return                  = 0x1C,
        LoadNull                = 0x1D,
        LoadIntegerZero         = 0x1E,
        LoadDecimalZero         = 0x1F,
        LoadIntegerOne          = 0x20,
        LoadDecimalOne          = 0x21,
        LoadInteger             = 0x22,
        LoadString              = 0x23,
        StoreLocal              = 0x24,
        StoreGlobal             = 0x25,
        StoreArgument           = 0x26,
        StoreItem               = 0x27,
        LoadLocal               = 0x28,
        LoadGlobal              = 0x29,
        LoadArgument            = 0x2A,
        LoadItem                = 0x2B,
        Duplicate               = 0x2C,
        Pop                     = 0x2D,
        Exit                    = 0x2E,
        GetLength               = 0x2F,
        Error                   = 0x30,
        LoadBlob                = 0x31,
        Label                   = -1,
    }
}
