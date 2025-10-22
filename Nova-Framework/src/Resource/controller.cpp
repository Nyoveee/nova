#include "controller.h"

Controller::Controller(ResourceID id, ResourceFilePath filePath, Data data) :
	Resource	{ id, std::move(filePath) },
	data		{ std::move(data) }
{}

std::unordered_map<ControllerNodeID, Controller::Node> const& Controller::getNodes() const {
	return data.nodes;
}
