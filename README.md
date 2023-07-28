# mysh - Custom Shell

A shell like that of bash, zsh, etc.

mysh is a command-line application written in C. It has 2 operating
modes: interactive and batch. Supported features are:
- cd & pwd (built-in)
- path names and bare names
- wildcards (including directories)
- standard IO redirection
- multi-piping (|)
- logical AND & OR (&& ||)


## Composition
This shell uses the POSIX standard and utilizes a pre-built base-runtime library contained
in the '_import' folder. The source code from which the pre-built libraries were compiled is
included in that folder.

This project is divided in 3 main parts:
- 'src' - the main source code for mysh
- 'unit-tests' - testing of individual components for mysh
- 'qa' - testing mysh itself

'_export' is the folder where b2 copies the compilied executable to.

The 3 open-source projects incorporated in mysh are:
- [re2c](https://re2c.org/) as the lexer generator
- [coco/R](https://ssw.jku.at/Research/Projects/Coco/) as the parser generator
- [Boost build (b2)](https://www.bfgroup.xyz/b2/) as the compilation system.

re2c takes in a lexer.re2c file as the input and produces a lexout.c file for mysh.
The specific command for this is:
```console
$ re2c --no-generation-date --nested-ifs -s -W -o lexout.c lexer.re2c
```

coco/R works likewise as it converts an attributed grammar of a source language into defined code.
It was used to create an early base version of the parser which now is developed alone in the
parser.c file.

b2 is a complete build system tool that is advertized to build C++ projects, but can really be used
as a compilation system for most languages. It has many advantages, namely, portability for
cross platform development, variant builds, and full dependency tracking. More information is on
their site [here](https://www.bfgroup.xyz/b2/).

## Build
To build mysh, you need to have gcc, re2c if you plan to change the lexer, and b2. However, the project comes
with 2 executables of mysh, for linux-like systems and windows. b2 builds in the .bin folder above the root
of the project. The Jamroot in the root of the project defines mysh to compile under 2 variants: debug-static
and release-static. Both of these variants are the same with regard to compilation parameters except for debug information.

(All code examples are from b2 on linux, unless specified elsewise)

To build the entire project run the build command, b2, in the root of the project (use the '-a' flag if you want a full rebuild).
Here is an example compilation:
```console
$ b2
...found 55 targets...
...updating 16 targets...
gcc.compile.c ../.bin/mysh/src/gcc-11/release-static/address-model-64/architecture-x86/lexer-release-static-linux-x86-64-gcc-11.o
gcc.compile.c ../.bin/mysh/src/gcc-11/release-static/address-model-64/architecture-x86/translator-release-static-linux-x86-64-gcc-11.o
gcc.compile.c ../.bin/mysh/src/gcc-11/release-static/address-model-64/architecture-x86/token-release-static-linux-x86-64-gcc-11.o
gcc.compile.c ../.bin/mysh/src/gcc-11/release-static/address-model-64/architecture-x86/mysh-release-static-linux-x86-64-gcc-11.o
gcc.compile.c ../.bin/mysh/src/gcc-11/release-static/address-model-64/architecture-x86/glob-release-static-linux-x86-64-gcc-11.o
gcc.compile.c ../.bin/mysh/src/gcc-11/release-static/address-model-64/architecture-x86/parser-release-static-linux-x86-64-gcc-11.o
gcc.compile.c ../.bin/mysh/src/gcc-11/release-static/address-model-64/architecture-x86/command-release-static-linux-x86-64-gcc-11.o
gcc.link ../.bin/mysh/src/gcc-11/release-static/address-model-64/architecture-x86/mysh-release-static-linux-x86-64-gcc-11
common.copy _export/mysh-release-static-linux-x86-64-gcc-11
...updated 16 targets...
```

To build each component seperately run the build command in each subfolder. As an example:
```console
$ cd src
$ b2
```

In this project by default, b2 compiles and exports the release version of mysh. If you want to compile mysh
with debug information use the 2nd build variant in the src folder:
```console
$ cd src
$ b2 debug-static
```
This will compile the executable in the .bin folder.

Building the unit-tests folder will automatically compile and execute all the unit-tests. When a
specific unit-test does not pass, b2 will inform you. Here is an example of 2/3 tests passing:
```console
$ cd unit-tests
$ b2
...found 65 targets...
...updating 25 targets...
gcc.compile.c ../../.bin/mysh/src/gcc-11/debug-static/address-model-64/architecture-x86/translator-debug-static-linux-x86-64-gcc-11.o
gcc.compile.c ../../.bin/mysh/src/gcc-11/debug-static/address-model-64/architecture-x86/token-debug-static-linux-x86-64-gcc-11.o
gcc.compile.c ../../.bin/mysh/unit-tests/gcc-11/debug-static/address-model-64/architecture-x86/translator-test-debug-static-linux-x86-64-gcc-11.o
gcc.compile.c ../../.bin/mysh/src/gcc-11/debug-static/address-model-64/architecture-x86/lexer-debug-static-linux-x86-64-gcc-11.o
gcc.compile.c ../../.bin/mysh/unit-tests/gcc-11/debug-static/address-model-64/architecture-x86/lexer-test-debug-static-linux-x86-64-gcc-11.o
gcc.compile.c ../../.bin/mysh/unit-tests/gcc-11/debug-static/address-model-64/architecture-x86/glob-test-debug-static-linux-x86-64-gcc-11.o
gcc.compile.c ../../.bin/mysh/src/gcc-11/debug-static/address-model-64/architecture-x86/glob-debug-static-linux-x86-64-gcc-11.o
gcc.link ../../.bin/mysh/unit-tests/gcc-11/debug-static/address-model-64/architecture-x86/lexer-test-debug-static-linux-x86-64-gcc-11
gcc.link ../../.bin/mysh/unit-tests/gcc-11/debug-static/address-model-64/architecture-x86/translator-test-debug-static-linux-x86-64-gcc-11
testing.unit-test ../../.bin/mysh/unit-tests/gcc-11/debug-static/address-model-64/architecture-x86/translator-test-debug-static-linux-x86-64-gcc-11.passed
testing.unit-test ../../.bin/mysh/unit-tests/gcc-11/debug-static/address-model-64/architecture-x86/lexer-test-debug-static-linux-x86-64-gcc-11.passed
gcc.link ../../.bin/mysh/unit-tests/gcc-11/debug-static/address-model-64/architecture-x86/glob-test-debug-static-linux-x86-64-gcc-11
testing.unit-test ../../.bin/mysh/unit-tests/gcc-11/debug-static/address-model-64/architecture-x86/glob-test-debug-static-linux-x86-64-gcc-11.passed

    LD_LIBRARY_PATH="/usr/bin:/usr/lib:/usr/lib32:/usr/lib64:$LD_LIBRARY_PATH"
export LD_LIBRARY_PATH

     "../../.bin/mysh/unit-tests/gcc-11/debug-static/address-model-64/architecture-x86/glob-test-debug-static-linux-x86-64-gcc-11"  && touch  "../../.bin/mysh/unit-tests/gcc-11/debug-static/address-model-64/architecture-x86/glob-test-debug-static-linux-x86-64-gcc-11.passed"

...failed testing.unit-test ../../.bin/mysh/unit-tests/gcc-11/debug-static/address-model-64/architecture-x86/glob-test-debug-static-linux-x86-64-gcc-11.passed...
...failed updating 1 target...
...updated 24 targets...
```

## Run
mysh was tested on linux through WSL version 2 and Windows through [MSYS2](https://www.msys2.org/)
64bit which is based on a modified version of [Cygwin](https://www.cygwin.com/).
mysh could have natively ran on Windows and even compiled with msvc if it were not for the
POSIX function for creating child processes, fork(). For this reason either MSYS2 or Cygwin
is needed to run mysh on windows.

(In the following examples the executable is called ./mysh, however by default
b2 names the executables by their build properties e.x. ./mysh-release-static-linux-x86-64-gcc-11)

Running mysh itself is trivial.

To run mysh in interactive mode just run the executable:
```console
$ ./mysh
```
use the 'exit' keyword to exit mysh.

Running mysh in batch mode will read commands from the inputed file with the
delimiter for seperate commands being a newline. To do so run:
```console
$ ./mysh [path-to-file]
```
Batch mode will print out 'mysh>' followed by the command it is running in standard output
before running that command. Like in interactive mode, batch mode will also exit on keyword
'exit' or until the first failed command.

In the qa folder there are two types shell scrips. shell_commands contains a list of various different
commands which can we used to execute in the current shell as a baseline. qa_test runs mysh in batch mode
with the commands in shell_commands. 