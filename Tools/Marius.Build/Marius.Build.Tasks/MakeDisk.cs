/* 
 Copyright (c) 2010 Marius Klimantavičius

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.

 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Build.Utilities;
using Microsoft.Build.Framework;
using System.IO;

namespace Marius.Build.Tasks
{
    public class MakeDisk: Task
    {
        private int _size = 0;

        [Required]
        public ITaskItem[] InputFiles { get; set; }

        [Required]
        public string OutputFile { get; set; }

        [Required]
        public int DiskSize { get; set; }

        public override bool Execute()
        {
            using (var writer = new FileStream(OutputFile, FileMode.Create))
            {
                for (int i = 0; i < InputFiles.Length; i++)
                {
                    using (var input = new FileStream(InputFiles[i].ItemSpec, FileMode.Open))
                    {
                        Copy(input, writer);
                    }
                }

                if (_size > DiskSize)
                    Log.LogWarning("Specified DiskSize is smaller than created image");
                else if (_size < DiskSize)
                {
                    writer.SetLength(DiskSize);
                }
            }

            return true;
        }

        private void Copy(Stream source, Stream dest)
        {
            byte[] data = new byte[256];
            int read = 0;

            while ((read = source.Read(data, 0, data.Length)) > 0)
            {
                dest.Write(data, 0, read);

                _size += read;
            }
        }
    }
}
