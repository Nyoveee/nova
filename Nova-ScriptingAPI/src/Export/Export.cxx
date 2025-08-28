// For .hxx files without .cxx 
// These need to be compiled into the dll for c# scripts to show
// Because these are header files without .cxx it won't compile unless we use this file
// Some of this contains definitions so might better to include it here only to avoid linker errors from multiple definitions
// If it's not being included in other parts of ScriptingAPI
// creating .cxx is highly recommended though, except in certain cases like the header files below
#include "ScriptLibrary/ManagedTypes.hxx"
#include "ScriptLibrary/Time.hxx"
