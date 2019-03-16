#pragma once

#include "PintaUnmanaged.h"
#include "PintaEngineException.h"
#include "IPintaEnvironment.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace Marius
{
    namespace Pinta
    {
        namespace Managed
        {
            public ref class PintaEngine : public IDisposable
            {
            private:

                void *_nativeMemory;
                PintaManagedEngineState *_state;
                IPintaEnvironment ^_environment;

            public:

                property IPintaEnvironment ^Environment
                {
                    IPintaEnvironment ^get() { return _environment; }
                }

                PintaEngine(String ^scriptFilename);
                PintaEngine(String ^scriptFilename, size_t heapSize, size_t stackSize);
                PintaEngine(String ^scriptFilename, IPintaEnvironment ^environment);
                PintaEngine(String ^scriptFilename, size_t heapSize, size_t stackSize, IPintaEnvironment ^environment);

                ~PintaEngine();
                !PintaEngine();

                void SetGlobal(String ^name, String ^value);
                String ^GetGlobal(String ^name);
                array<Byte> ^Execute();

            private:

                void Init(String ^scriptFilename, size_t heapSize, size_t stackSize, IPintaEnvironment ^environment);
                void ThrowEngineException(PintaException exception);
            };
        }
    }
}
