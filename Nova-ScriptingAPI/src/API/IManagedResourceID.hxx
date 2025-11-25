#pragma once

#include "type_alias.h"

public ref class IManagedResourceID abstract {
public:
	IManagedResourceID()							: resourceID	{ INVALID_RESOURCE_ID } {};
	IManagedResourceID(System::UInt64 resourceID)	: resourceID	{ resourceID } {};

internal:
	System::UInt64 resourceID;
};