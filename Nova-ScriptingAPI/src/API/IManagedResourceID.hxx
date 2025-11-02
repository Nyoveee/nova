#pragma once

public ref class IManagedResourceID abstract {
public:
	IManagedResourceID(System::UInt64 resourceID) : resourceID{ resourceID } {};

internal:
	System::UInt64 resourceID;
};