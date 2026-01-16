#include "video.h"
#include <filesystem>

Video::Video(ResourceID id, ResourceFilePath resourceFilePath) :
	Resource{ id, std::move(resourceFilePath) }
{}

Video::~Video() {}
