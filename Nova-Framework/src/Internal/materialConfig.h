#pragma once

enum class BlendingConfig {
	AlphaBlending,
	AdditiveBlending,
	PureAdditiveBlending,
	PremultipliedAlpha,
	Disabled
};

enum class DepthTestingMethod {
	DepthTest,
	NoDepthWrite,
	NoDepthWriteTest
};

enum class CullingConfig {
	Enable,
	Disable
};