#include "sequencer.h"

Sequencer::Sequencer(ResourceID id, ResourceFilePath filePath, Data data) :
	Resource		{ id, std::move(filePath) },
	data			{ std::move(data) }
{}
