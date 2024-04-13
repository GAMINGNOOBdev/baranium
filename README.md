baranium
========

baranium is an open-source programming language originally designed
for use with a personal game engine project, however later repurposed
and redesigned for use as an actual programming language.<br/>
It's design allows easy expansion of it's runtime and easy embedding
into other C(++) projects for various purposes.

## Info
THIS PROJECT IS STILL INCOMPLETE AT THE MOMENT!!! <br/>
Expect somewhat regular updates but don't expect a fully finished
release in a short time. <br/>
To see more indepth stuff and ask questions, join the newly created discord server that
will be as inactive as every other server you may know. https://discord.gg/Bp9emHJ6

## Language specifications
baranium was originally designed to be used with a game engine and
therefore it has features that other languages may not have such as
fields. Fields are and behave just like variables the only difference
being that fields can be changed by the user using the runtime.
However the current runtime executable may not support this at the
moment because of this feature being oriented towards integration of
the language in other projects.

## Building
1. Open a command line at the root of the reporitory, create a `build` directory and change into that directory using
```bash
mkdir build
cd build
```
2. Run cmake and point towards the root of the repository containing the `CMakeLists.txt` file
```bash
cmake ..
```

3. If on windows skip to 5, if linux or mac goto 4

4. Run `make` and enjoy!

5. Open the generated visual studio solution and build the entire solution

Note: The generated binaries are under the subfolders `Compiler`, `Runtime`
      and `RuntimeExecutable` and under a folder with the same name as the
      build configuration

## Using the compiler
just let the compiler print out the help message, it'll tell you everything.

## Using the provided runtime executable
The provided runtime executable doesn't put a focus on advanced
functionality that you would expect from languages like Python or Java.
Instead it has just a few standard mathematical and console output
functions. This can and will probably change with either future updates
or if enough people want a feature to be added.
<br/>
To use the provided runtime, just provide a compiled script and it
should run, hopefully.

## Embedding the runtime
don't, it no work rn <br/>
Only thing rn is if you want to link against the shared object/dll you'll
have to define `BARANIUM_DYNAMIC` before including any files.