#pragma once

#ifdef NOVA_ENGINE_DLL_EXPORT
	#define ENGINE_DLL_API __declspec(dllexport)
#else
	#define ENGINE_DLL_API __declspec(dllimport)
#endif

#ifdef NOVA_FRAMEWORK_DLL_EXPORT
#define FRAMEWORK_DLL_API __declspec(dllexport)
#else
#define FRAMEWORK_DLL_API __declspec(dllimport)
#endif