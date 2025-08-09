// Base C# Script

#pragma once

namespace ScriptingAPI {
	public ref class Script abstract
	{
	public:
		virtual void Init() {};
		virtual void update() {};
		virtual void Exit() {};
	};
}


