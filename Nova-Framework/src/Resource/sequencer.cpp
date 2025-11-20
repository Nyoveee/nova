#include "sequencer.h"

Sequencer::Sequencer(ResourceID id, ResourceFilePath filePath) :
	Resource(id, std::move(filePath))
{}
