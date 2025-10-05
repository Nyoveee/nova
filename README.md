Prerequisites:
- VS2022 17.12 or higher
- .NET Desktop Development (install through Visual Studio Installer)
- .NET 9 SDK (or download through Visual Studio Installer)
- Windows 10.

We use a VS2022 solution for our build system. No cmake.
(This solution builds fine on the school PC, DIT0828SG in E2-08-14)

We have 6 projects
1. Nova-Editor (.EXE, represents our Editor for our game)
2. Nova-Engine (.DLL, provides core systems)
3. Nova-Framework (.DLL, provides core libraries and utilities, both 3rd party and internal.)
4. Nova-Game (.EXE, represents our game)
5. Nova-ResourceCompiler (.EXE, compiles assets to resources)
6. Nova-ScriptingAPI (.DLL, C++/CLI for interop between C++ and C#)

Folder structure
- Assets (contains all intermediary assets)
- Descriptors (contains all descriptor files)
- DLL (contains 3rd party DLL for our applications to link with)
- ExternalApplication (contains executables that our editor relies on, also output directory for our compiler.)
- Include (contains 3rd party header files)
- Library (contains 3rd party LIB files for static linking)
- The 6 Nova projects
- Nova-Scripts (contains solution to compile our C# scripts for hot reloading.)
- Resources (contains cooked data, final resource files)
- System (contains system assets that is not intended to be part of the Assets folder)

Working directory: Solution directory.

To build the project, simply build the solution file. Set Nova-Editor as start up project to run the Editor and
set Nova-Game as start up project to run the Game. 

(The first time the editor is launch, it compiles all resources file, so it will take some time. Subsequently, because the resource file exist, starting up is much faster.)