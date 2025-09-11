// Converting to native type: https://learn.microsoft.com/en-us/cpp/dotnet/overview-of-marshaling-in-cpp?view=msvc-170
#include "Debug.h"
#include "Export/Logger.h"
#include <vcclr.h>
#include "API/IManagedComponent.hxx"
#include <msclr/marshal_cppstd.h>
void Debug::Print(System::Object^ object)
{
	if (object->GetType()->IsSubclassOf(IManagedComponent::typeid))
		Logger::info(msclr::interop::marshal_as<std::string>(object->ToString()));
	else
		Logger::info(msclr::interop::marshal_as<std::string>(object->GetType()->Name + object->ToString()));
}
