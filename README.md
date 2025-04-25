baranium
========

baranium is an open-source programming language originally designed
for use with a personal game engine project, however later repurposed
and redesigned for use as an actual programming language.<br/>
It's design allows easy expansion of the available functions in the runtime
and easy embedding into other C(++) projects for various purposes.

## Discord
To see more indepth stuff and ask questions, join the [baranium discord server](https://discord.gg/8R5YyM3MEn).

## Language specifications
baranium was originally designed to be used with a game engine and
therefore it has features that other languages may not have such as
fields. Fields are and behave just like variables the only difference
being that fields can be changed by the user using the runtime.
However the current runtime executable may not support this at the
moment because of this feature being oriented towards integration of
the language in other projects.
More information can be found in the [syntax section of the wiki](../../wiki/Syntax).

## Examples
You can find example scripts under `examples` in the root of this repository.
Some of them may compile, some of them may not.

## Building
1. Open a command line at the root of the reporitory, create a `build` directory and change into that directory using
```bash
mkdir build
cd build
```
2. Run cmake and point towards the root of the repository containing the `CMakeLists.txt` file, optionally add `-D CMAKE_BUILD_TYPE=<Release/Debug>` at the end to specify whether to build a debug or release build
```bash
cmake ..
```

3. If on windows skip to 5, if linux or mac goto 4

4. Run `make` and enjoy!

5. Open the generated visual studio solution and build the entire solution

Note: The generated binaries are under the subfolders `Compiler`, `Runtime`
      and `RuntimeExecutable` and if using Windows under a folder with the
      same name as the build configuration

## Using the compiler
Compiler flags/options:<br/>
| Flag | Description |
|------|-------------|
| -o `path` | Specify output file |
| -h | Show the help message |
| -I `file` | Specify file containing all custom user include directories |
| -d | Print debug messages (only useful for debugging the compiler itself!) |
| -e | Export output as a library file |
| -l `name` | Link against a library named `name` (filename has to exactly match the library name, no extensions!) |

As for specifying source files, just list the paths to the files like every other compiler on this planet.

## Using the provided runtime executable
The provided runtime executable doesn't put a focus on advanced
functionality and library flexibility that you would see in languages like Python or Java.
Instead it has just a few standard libraries and console output
functions. This can and will probably change with either future updates
or if enough people want a feature to be added.
<br/>
To use the provided runtime, just provide a compiled script to the runtime executable and it
should run.

## Embedding the runtime
For further information please consider reading the [wiki](../../wiki).
