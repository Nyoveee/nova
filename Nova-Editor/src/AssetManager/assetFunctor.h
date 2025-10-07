#pragma once

// contains functors to serialise each type of assets

class AssetManager;

struct SerialiseDescriptor {
public:
	virtual ~SerialiseDescriptor() = 0 {};
	virtual void operator()(ResourceID id, AssetManager& assetManager) const = 0;
};

template <ValidResource T>
struct SerialiseDescriptorFunctor : public SerialiseDescriptor {
	void operator()(ResourceID id, AssetManager& assetManager) const final;
};