using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection
{
    public class PintaCodeBinaryComparer : IComparer<byte[]>, IEqualityComparer<byte[]>
    {
        private static readonly byte[] PearsonTable = new byte[] 
        {
            211, 124, 185, 102, 88, 255, 64, 226, 33, 250, 252, 157, 144, 246, 243, 90,
            146, 91, 221, 249, 254, 214, 193, 154, 15, 196, 87, 190, 201, 192, 229, 24,
            84, 230, 40, 11, 109, 75, 70, 83, 220, 162, 232, 37, 132, 25, 92, 18,
            65, 82, 171, 134, 156, 39, 127, 55, 205, 4, 79, 175, 203, 161, 85, 136,
            49, 202, 135, 191, 242, 142, 10, 138, 3, 67, 99, 152, 14, 181, 104, 179,
            172, 52, 73, 186, 106, 166, 21, 100, 59, 93, 223, 119, 103, 183, 50, 16,
            167, 178, 107, 208, 155, 188, 241, 74, 131, 71, 69, 235, 36, 13, 120, 66,
            113, 27, 123, 81, 240, 189, 43, 60, 251, 159, 140, 68, 28, 8, 164, 56,
            149, 233, 253, 89, 45, 101, 160, 110, 199, 248, 187, 245, 96, 76, 128, 46,
            111, 215, 163, 204, 200, 168, 195, 247, 58, 198, 228, 61, 153, 35, 32, 150,
            137, 47, 7, 180, 12, 31, 224, 216, 44, 97, 98, 126, 95, 148, 147, 80,
            22, 34, 94, 38, 30, 238, 6, 5, 212, 117, 197, 29, 236, 115, 225, 112,
            139, 121, 78, 231, 207, 51, 234, 122, 194, 2, 184, 222, 182, 57, 108, 118,
            0, 239, 19, 9, 86, 219, 244, 26, 133, 48, 53, 169, 237, 116, 143, 54,
            114, 42, 165, 23, 177, 210, 158, 209, 20, 151, 173, 170, 129, 213, 218, 174,
            141, 217, 62, 145, 63, 125, 17, 130, 206, 176, 77, 72, 105, 1, 227, 41
        };

        public static readonly PintaCodeBinaryComparer Instance = new PintaCodeBinaryComparer();

        public bool Equals(byte[] x, byte[] y)
        {
            if (x == null && y == null)
                return true;
            
            if (object.ReferenceEquals(x, y))
                return true;

            if (x.Length != y.Length)
                return false;

            for (var i = 0; i < x.Length; i++)
            {
                if (x[i] != y[i])
                    return false;
            }

            return true;
        }

        public int GetHashCode(byte[] obj)
        {
            byte r0, r1, r2, r3;

            if (obj.Length == 0)
                return 0;

            r0 = PearsonTable[obj[0] + 0];
            r1 = PearsonTable[obj[0] + 1];
            r2 = PearsonTable[obj[0] + 2];
            r3 = PearsonTable[obj[0] + 3];

            for (var index = 0; index < obj.Length; index++)
            {
                r0 = PearsonTable[r0 ^ obj[index]];
                r1 = PearsonTable[r1 ^ obj[index]];
                r2 = PearsonTable[r2 ^ obj[index]];
                r3 = PearsonTable[r3 ^ obj[index]];
            }

            return (r0 << 24) | (r1 << 16) | (r2 << 8) | r3;
        }

        public int Compare(byte[] x, byte[] y)
        {
            if (x == null && y == null)
                return 0;

            if (object.ReferenceEquals(x, y))
                return 0;

            if (x.Length < y.Length)
            {
                for (var i = 0; i < x.Length; i++)
                {
                    if (x[i] < y[i])
                        return -1;
                    else if (x[i] > y[i])
                        return 1;
                }

                return -1;
            }

            for (var i = 0; i < y.Length; i++)
            {
                if (x[i] < y[i])
                    return -1;
                else if (x[i] > y[i])
                    return 1;
            }

            if (x.Length > y.Length)
                return 1;
            return 0;
        }
    }
}
