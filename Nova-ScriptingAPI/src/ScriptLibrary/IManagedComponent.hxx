#pragma once
public ref class IManagedComponent abstract {
internal:
	void setEntityID(System::UInt32 p_entityID) { entityID = p_entityID; };
	virtual bool exist(System::UInt32 p_entityID) = 0;
protected:
	System::UInt32 entityID;
};