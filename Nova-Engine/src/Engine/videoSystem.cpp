#include "videoSystem.h"
#include "engine.h"
#include "ECS/ECS.h"
#define PL_MPEG_IMPLEMENTATION
#include "Imported/pl_mpeg.h"
#pragma warning(pop)
VideoSystem::VideoSystem(Engine& engine)
	: engine{engine}
	, registry{engine.ecs.registry}{}

void VideoSystem::update(float dt) {
	for (auto&& [entity, videoPlayer] : registry.view<VideoPlayer>().each()) {
		if (!engine.ecs.isComponentActive<VideoPlayer>(entity))
			continue;
		auto&& [video, result] = engine.resourceManager.getResource<Video>(videoPlayer.videoId);
		if (result != ResourceManager::QueryResult::Success) {
			videoPlayer.isPlaying = false;
			continue;
		}
		plm_set_loop(video->getPLM(), videoPlayer.loop);
		if (video->decodingFinished() && !videoPlayer.loop) {
			videoPlayer.isPlaying = false;
			continue;
		}
		videoPlayer.timeAccumulator += dt;
		
	}
}
void VideoSystem::Reload() {
	for (auto&& [entity, videoPlayer] : registry.view<VideoPlayer>().each()) {
		// Resets the video data used 
		auto&& [video, result] = engine.resourceManager.getResource<Video>(videoPlayer.videoId);
		if (result != ResourceManager::QueryResult::Success)
			continue;
		// Initialize the loop and start state
		plm_set_loop(video->getPLM(), videoPlayer.loop);
		if (!video->decodingFinished())
			plm_demux_rewind(video->getPLM()->demux);
		if (videoPlayer.playOnStart)
			videoPlayer.isPlaying = true;
	}
}
