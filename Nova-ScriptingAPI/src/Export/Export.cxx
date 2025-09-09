// For .hxx files without .cxx 
// These need to be compiled into the dll for c# scripts to show
// Because these are header files without .cxx it won't compile unless we use this file
// If it's not being included in other parts of ScriptingAPI
// creating .cxx is highly recommended though
#include "ScriptLibrary/Time.hxx"