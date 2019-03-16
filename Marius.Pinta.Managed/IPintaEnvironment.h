#pragma once
namespace Marius
{
    namespace Pinta
    {
        namespace Managed
        {
            public interface class IPintaEnvironment
            {
                Object ^OpenFile(String^ name);
                UInt32 GetFileSize(Object^ handle);
                UInt32 ReadFile(Object^ handle, array<Byte>^ buffer);
                void CloseFile(Object^ handle);
                UInt32 WriteFile(Object^ handle, array<Byte>^ buffer);
            };
        }
    }
}