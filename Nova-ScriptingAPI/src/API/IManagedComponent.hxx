#pragma once
public ref class IManagedComponent abstract {
internal:
	virtual bool LoadDetailsFromEntity(System::UInt32 p_entityID) = 0;
	virtual bool NativeReferenceLost() = 0;
internal:
	System::UInt32 entityID;
};