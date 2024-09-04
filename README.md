# metapro
Static recompilation of Nintendo 64 software.  Started in March of 2020 and made public in January of 2021, this project pioneered the technology of static recompilation for Nintendo 64 and IRIX software.

Static recompilation involves converting a binary into C code that may be compiled to run on a different platform.  This technique allows for optimization of the entire binary at compile time, removing the overhead of emulation and providing performance comparable to compiling the original source code.  The following describes the recompilation process in further detail:
* The source binary is analyzed to find structural information, such as C function entry points.
* Each machine instruction is decoded and converted to a line of C code.  For example, `addiu t0,a0,a1` becomes `t0 = a0 + a1;`.
* Additional pattern-matching techniques are used to translate complex instruction patterns (such as jump tables or function calls) into equivalent C code.
* System calls or library functions are reimplemented as equivalent system calls or functionality specific to the target platform.
* The generated C code is compiled with aggressive optimization flags and built into a new binary.

## Building
Install dependencies (Debian):
```
apt install make python3 gcc
```
For Win32 (cross compile): `apt install gcc-mingw-w64-i686`
For OSX: Install SDL2 and OpenGL frameworks
For GameCube/Wii: Install devkitPPC
For DSi/3DS: Install devkitARM

Build:
```
make native APP=UNSME00 DEBUG=0
```
Build targets: `native` `win32` `osx` `gcn` `wii` `nds` `3ds`
