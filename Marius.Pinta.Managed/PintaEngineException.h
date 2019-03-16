#pragma once

using namespace System;

namespace Marius
{
    namespace Pinta
    {
        namespace Managed
        {
            [Serializable]
            public ref class PintaEngineException : public Exception
            {
            public:
                PintaEngineException();
                PintaEngineException(String^ message);
                PintaEngineException(String^ message, Exception^ inner);
            protected:
                PintaEngineException(System::Runtime::Serialization::SerializationInfo^ info, System::Runtime::Serialization::StreamingContext context);
            };
        }
    }
}