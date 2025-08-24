// For Nova-Engine, This header must be used instead of the usual <tracyprofiler/tracy/tracy.hpp> that nova-editor uses
#pragma once
// Important when building through nova-game, Set this to 0 to disable profiling, as you don't want
// people to use the external tracy-profiler application to profile, that includes seeing the code exposed in the profiler
#if 1
	// Would be nice If there's a better way to define macro only when it's the editor running so don't have to manually set
	#ifndef TRACY_ENABLE
		#define TRACY_ENABLE // This is needed for the profiler to show cpu usage of a function etc, otherwise the profiler will do nothing
	#endif
#endif
#include <tracyprofiler/tracy/Tracy.hpp>