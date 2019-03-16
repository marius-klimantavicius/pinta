#include "stdafx.h"
#include "PintaEngineException.h"

namespace Marius
{
    namespace Pinta
    {
        namespace Managed
        {
            PintaEngineException::PintaEngineException() : Exception() {}
            PintaEngineException::PintaEngineException(String^ message) : Exception(message) {}
            PintaEngineException::PintaEngineException(String^ message, Exception^ inner) : Exception(message, inner) {}
            PintaEngineException::PintaEngineException(System::Runtime::Serialization::SerializationInfo^ info, System::Runtime::Serialization::StreamingContext context) : Exception(info, context) {}
        }
    }
}