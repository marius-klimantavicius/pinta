#include "stdafx.h"

#include "PintaEngine.h"

namespace Marius
{
    namespace Pinta
    {
        namespace Managed
        {
            PintaEngine::PintaEngine(String ^scriptFilename)
            {
                Init(scriptFilename, 32 * 1024, 2048, nullptr);
            }

            PintaEngine::PintaEngine(String ^scriptFilename, size_t heapSize, size_t stackSize)
            {
                Init(scriptFilename, heapSize, stackSize, nullptr);
            }

            PintaEngine::PintaEngine(String ^scriptFilename, IPintaEnvironment ^environment)
            {
                Init(scriptFilename, 32 * 1024, 2048, environment);
            }

            PintaEngine::PintaEngine(String ^scriptFilename, size_t heapSize, size_t stackSize, IPintaEnvironment ^environment)
            {
                Init(scriptFilename, heapSize, stackSize, environment);
            }

            PintaEngine::~PintaEngine()
            {
                this->!PintaEngine();
            }

            PintaEngine::!PintaEngine()
            {
                if (_nativeMemory != NULL)
                {
                    free(_nativeMemory);
                    _nativeMemory = NULL;
                }

                if (_state != NULL)
                {
                    if (_state->engine != NULL)
                    {
                        GCHandle handle = GCHandle::operator GCHandle(IntPtr(_state->engine));
                        handle.Free();

                        _state->engine = nullptr;
                    }

                    free(_state);
                    _state = NULL;
                }
            }

            void PintaEngine::SetGlobal(String ^name, String ^value)
            {
                PintaException exception = PINTA_OK;

                IntPtr namePtr = IntPtr::Zero;
                IntPtr valuePtr = IntPtr::Zero;

                if (name == nullptr)
                    throw gcnew ArgumentNullException("name");

                try
                {
                    if (value != nullptr)
                    {
                        namePtr = Marshal::StringToCoTaskMemUni(name);
                        valuePtr = Marshal::StringToCoTaskMemUni(value);

                        exception = pinta_managed_set_global(_state, namePtr.ToPointer(), name->Length, valuePtr.ToPointer(), value->Length);
                        ThrowEngineException(exception);
                    }
                    else
                    {
                        namePtr = Marshal::StringToCoTaskMemUni(name);

                        exception = pinta_managed_set_global_null(_state, namePtr.ToPointer(), name->Length);
                        ThrowEngineException(exception);
                    }
                }
                finally
                {
                    if (namePtr != IntPtr::Zero)
                        Marshal::FreeCoTaskMem(namePtr);

                    if (valuePtr != IntPtr::Zero)
                        Marshal::FreeCoTaskMem(valuePtr);
                }
            }

            String ^PintaEngine::GetGlobal(String ^name)
            {
                PintaException exception = PINTA_OK;

                IntPtr namePtr = IntPtr::Zero;
                void *data;
                i32 length;

                if (name == nullptr)
                    throw gcnew ArgumentNullException("name");

                try
                {
                    namePtr = Marshal::StringToCoTaskMemUni(name);

                    exception = pinta_managed_get_global(_state, namePtr.ToPointer(), name->Length, &data, &length);
                    ThrowEngineException(exception);

                    String ^result;

                    if (data != NULL)
                        result = Marshal::PtrToStringUni(IntPtr(data), (int)length);
                    else
                        result = nullptr;

                    return result;
                }
                finally
                {
                    if (namePtr != IntPtr::Zero)
                        Marshal::FreeCoTaskMem(namePtr);
                }

                return nullptr;
            }

            array<Byte>^ PintaEngine::Execute()
            {
                PintaException exception = PINTA_OK;
                uint32_t buffer_length;
                void *buffer_data;
                array<Byte> ^result;

                exception = pinta_managed_execute(_state, &buffer_length, &buffer_data);
                ThrowEngineException(exception);

                result = gcnew array<Byte>(buffer_length);
                Marshal::Copy(IntPtr(buffer_data), result, 0, result->Length);

                return result;
            }

            void PintaEngine::Init(String ^scriptFilename, size_t heapSize, size_t stackSize, IPintaEnvironment ^environment)
            {
                PintaException exception;
                PintaEngine ^self = this;
                GCHandle engine = GCHandle::Alloc(this);

                _environment = environment;

                _state = (PintaManagedEngineState*)malloc(sizeof(PintaManagedEngineState));
                if (!_state)
                    throw gcnew OutOfMemoryException();

                _state->engine = ((GCHandle::operator IntPtr(engine)).ToPointer());

                _state->heap_length = heapSize;
                _state->stack_length = stackSize;

                _nativeMemory = malloc(heapSize + stackSize + 32 * 1024);
                _state->native_length = heapSize + stackSize + 32 * 1024;
                _state->native_memory = _nativeMemory;

                IntPtr filename = IntPtr::Zero;

                try
                {
                    filename = Marshal::StringToCoTaskMemUni(scriptFilename);
                    exception = pinta_managed_init(_state, (wchar*)filename.ToPointer());
                    ThrowEngineException(exception);
                }
                finally
                {
                    if (filename != IntPtr::Zero)
                        Marshal::FreeCoTaskMem(filename);
                }
            }

            void PintaEngine::ThrowEngineException(PintaException exception)
            {
                switch (exception)
                {
                case PINTA_OK:
                    return;
                case PINTA_EXCEPTION_STACK_OVERFLOW:
                    throw gcnew PintaEngineException("Stack overflow");
                case PINTA_EXCEPTION_STACK_UNDERFLOW:
                    throw gcnew PintaEngineException("Stack underflow");
                case PINTA_EXCEPTION_TYPE_MISMATCH:
                    throw gcnew PintaEngineException("Type mismatch");
                case PINTA_EXCEPTION_OUT_OF_MEMORY:
                    throw gcnew PintaEngineException("Out of memory");
                case PINTA_EXCEPTION_NULL_REFERENCE:
                    throw gcnew PintaEngineException("Null reference");
                case PINTA_EXCEPTION_BAD_FORMAT:
                    throw gcnew PintaEngineException("Bad format");
                case PINTA_EXCEPTION_OUT_OF_RANGE:
                    throw gcnew PintaEngineException("Out of range");
                case PINTA_EXCEPTION_INVALID_OPERATION:
                    throw gcnew PintaEngineException("Invalid operation");
                case PINTA_EXCEPTION_INVALID_OPCODE:
                    throw gcnew PintaEngineException("Invalid opcode");
                case PINTA_EXCEPTION_NOT_IMPLEMENTED:
                    throw gcnew PintaEngineException("Not implemented");
                case PINTA_EXCEPTION_ENGINE:
                    throw gcnew PintaEngineException("Engine error");
                case PINTA_EXCEPTION_INDEX_OUT_OF_BOUNDS:
                    throw gcnew PintaEngineException("Out of bounds");
                case PINTA_EXCEPTION_DIVISION_BY_ZERO:
                    throw gcnew PintaEngineException("Division by zero");
                case PINTA_EXCEPTION_INVALID_MODULE:
                    throw gcnew PintaEngineException("Invalid module");
                case PINTA_EXCEPTION_FILE_NOT_FOUND:
                    throw gcnew PintaEngineException("File not found");
                case PINTA_EXCEPTION_INVALID_ARGUMENTS:
                    throw gcnew PintaEngineException("Invalid arguments");
                case PINTA_EXCEPTION_PLATFORM:
                    throw gcnew PintaEngineException("Platform error");
                case PINTA_EXCEPTION_NOT_REACHABLE:
                    throw gcnew PintaEngineException("Not reachable");
                case PINTA_EXCEPTION_INVALID_SEQUENCE:
                    throw gcnew PintaEngineException("Invalid utf8 sequence");
                default:
                    throw gcnew PintaEngineException(String::Format(L"Unknown exception: {0}", (int)exception));
                }
            }
        }
    }
}