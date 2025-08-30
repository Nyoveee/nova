#pragma once
public ref class IManagedComponent abstract {
internal:
	virtual bool LoadDetailsFromEntity(System::UInt32 p_entityID) = 0;
protected:
	System::UInt32 entityID;
};