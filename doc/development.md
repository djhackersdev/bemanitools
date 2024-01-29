# Development

This document is intended for developers interested in contributing to Bemanitools. Please read this
document before you start developing.

## Goals

We want you to understand what this project is about and its goals. The following list serves as a
guidance for all developers to identify valuable contributions for this project. As the project
evolves, these gaols might do as well.

- Allow running Konami arcade rhythm games, i.e. games of the Bemani series, on arbitrary hardware.
  - Emulate required software and hardware features.
  - Provide means to cope with incompatibility issues resulting from using a different software
    platform (e.g. version of Windows).
- Provide an API for custom interfaces and configuring fundamental application features.

## Development environment

The following tooling is required in order to build this project.

### Tooling

#### Linux / MacOSX

- git
- make
- mingw-w64
- clang-format
- wine (optional, for running tests or some quick testing without requiring a VM)

On MacOSX, you can use homebrew or macports to install these packages.

#### Windows

TODO

### IDE

Ultimately, you are free to use whatever you feel comfortable with for development. The following is
our preferred development environment which we run on a Linux distribution of our choice:

- Visual Studio Code with the following extensions
  - C/C++
  - C++ Intellisense
  - Clang-Format

### Further tools for testing and debugging

- Debugger: Can be part of your reverse engineering IDE of your choice or stand-along like
  [OllyDbg](http://www.ollydbg.de/).
- [apitrace](https://apitrace.github.io/): Trace render calls to graphics APIs like D3D and OpenGL.
  This tool allows you to record and re-play render calls of an application with frame-by-frame
  debugging. Very useful to analyze the render pipeline or debug graphicial glitches.

## Building

Simply run make in the root folder:

```
make
```

All output is located in the *build* folder including the final *bemanitools.zip* package.

Note about using `-j n` option on make: This is currently considered broken/unreliable. Expect to
run into odd issues like randomly changing unresolved dependency errors. If you attempted this, run
a `make clean` before running `make` again.

### Release building

A release build is a clean build including code formatting and testing. This can be executed by
running the following command:

```
make release
```

## Building with docker

You can also build bemanitools using docker which avoids having to setup a full development
environment if you are just interested in building binaries for the latest changes. Naturally, this
requires you to have the docker daemon installed. Then, run the following command from the root
folder of the project:

```bash
make build-docker
```

Once completed successfully, the build output is located in the `build/docker` sub-folder.

## Creating releases

For developers to create official releases with major and minor versioning:

1. Ensure that all everything you want to have for this release is merged into master.
1. Create a [changelog](CHANGELOG.md) based on aggregating and summerizing commit messages of
   whatever got added starting from the tag of the current version to the current head of master.
   Commit that changelog and push to master.
1. Create a tag with the next version number, e.g. for 5.28:

```bash
git tag v5.28
```

1. Push the tag upstream:

```bash
git push origin v5.28
```

1. Wait for the CI pipeline to finish building the release and check if everything's ok.
1. On GitLab, go to "Tags" in the repository, click on the commit ID below the tag in the list you
   just created.
1. Click on the "Pipelines" tab and click on the download button on the right and on "Download
   release artifacts".
1. Rename the downloaded zip to "bemanitools-v5.28.zip" and the replace v5.28 with the version you
   want to release.
1. Upload the release.
1. For publishing the release, create a post with the following contents:

```
v5.28
<insert link to uploaded zip here>

<additional comments or things to point out for this release>

Changelog copy-paste:
<insert copy-pasted changelog you created previously here>
```

## Code formatting

To apply our code style using clang-format, simply run the following command:

```
make code-format
```

Please also refer to the [section about](###Additional-code-style-guidelines) which cannot be
covered using clang-format.

## Testing

This still needs to be improved/implemented properly to run the unit-tests easily. Currently, you
have to be on either a Linux/MacOSX system and run

```
make run-tests
```

This executes all currently available unit-tests and reports to the terminal any errors. This
requires wine to be installed.

## Project structure

Now that your setup is ready to go, here is brief big picture of what you find in this project.

- build: This folder will be generated once you have run the build process.
- dist: Distribution related files such as (default) configuration files, shell scripts, etc.
- doc: Documentation for the tools and hooks as well as some development related docs.
- src: The source code
  - imports: Provides headers and import definitions for AVS libs and a few other dependencies.
  - main: The main source code with game specific hook libraries, hardware emulation and application
    fixes.
  - test: Unit tests for modules that are not required for hooking or the presence of a piece of
    hardware.
- .clang-format: Code style for clang-format.
- GNUmakefile: Our makefile to build this project.
- Module.mk: Defines the various libraries and exe files to build for the distribution packages.

## Code style and guidelines

Please follow these guidelines to keep a consistent style for the code base. Furthermore, we provide
some best practices that have shown to be helpful. Please read them and reach out to us if you have
any concerns or valuable additions/changes.

### Clang-format

The style we agreed on is provided as a clang-format file. Therefore, we use clang-format for
autoformatting our code.

You can use clang-format from your terminal but when using Visual Studio Code, just install the
extension. Apply formatting manually at the end or enable the "reformat on save" feature.

However, clang-format cannot provide guidance to cover all our style rules. Therefore, we ask you to
stick to the "additional" guidelines in the following sections.

### Additional code style guidelines

#### No trailing comments

```
// NOPE
int var = 1; // this is a variable

// OK
// this is a variable
int var = 1;
```

#### Comment style

- Use either // or /\* ... \*/ for single line.
- Use /\* ... \*/ for multiline comments.
- Use /\* ... \*/ for documentation.

Examples:

```
// single line comment
int var = 1;

/* another single line comment */
int var2 = 2;

/* multi
   line
   comment */

/**
 * This is a function.
 */
int func(int a, int b);
```

#### Include guards

Provide include guards for every header file. The naming follows the namespacing of the module and
the module name.

Example for bsthook/acio.h file:

```
#ifndef BSTHOOK_ACIO_H
#define BSTHOOK_ACIO_H

// ...

#endif
```

#### Empty line before and after control blocks

Control blocks include: if, if-else, for, while, do-while, switch

Makes the code more readible with control blocks being easily visible.

Example

```
int var = 1;

if (var == 2) {
    // ...
}

printf("%d\n", var);
```

#### Empty line at the start of blocks

There are situations when you want to have an empty line at the start of the block to enhance
readability, for example:

```
if (a && 
    b && 
    c) {
    d = 5;
}
```

Here the assignment aligns with the conditions of the if-black making it hard to read. Better:

```
if (a && 
    b && 
    c) {

    d = 5;
}
```

#### Includes

- Always keep all includes at the top of a header and source file. Exception: Documentation before
  include guards on header file.
- Use *\< >* for system-based includes, e.g. \<stdio.h>.
- Use *" "* for project-based includes, e.g. "util/log.h".
- For project-based includes, always use the full path relative to the root folder (src/main,
  src/test), e.g. "util/log.h" and not "log.h" when being in another module in the "util" namespace.
- Sorting
  - System-based includes before project-based includes
  - Block group them by different namespaces
  - Lex sort block groups
  - Because windows header files are a mess, the sorting on system-based includes is not always
    applicable. Please add a comment when applicable and apply the necessary order.

Example for sorting

```
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "iidxhook-util/acio.h"
#include "iidxhook-util/d3d9.h"

#include "util/log.h"
#include "util/mem.h"
```

### Documentation

In general, add comments where required to explain a certain piece of code. If is not
self-explanatory about:

- Why is it implemented like this
- Very important details to understand how and why it is working that only you know
- A complex algorithm/logic

Make sure to add some comments or even an extended document. Avoid comments that explain trivial and
obvious things like "enable feature X" before an if-block with a feature switch.

Especially if it comes to reverse-engineering efforts, comments or even a separate document is
crucial to allow others to understand the depths you dived into.

Any extended notes about documentation some hardware, protocol, reverse-engineering a feature etc.
can be stored in the *doc/dev* folder and stay with the repository. Make good use of that!

#### Header files

Document any enum, struct or function exposed by a header file. Documentation of static functions or
variables in source modules is not required. Also provide documentation for the module.

Example for my-namespace/my-module.h

```
/**
 * Some example module to show you where documentation is expected.
 */
#ifndef MY_NAMESPACE_MY_MODULE_H
#define MY_NAMESPACE_MY_MODULE_H

/**
 * Very useful enum for things.
 */
enum my_enum {
    MY_NAMESPACE_MY_MODULE_MY_ENUM_VAL_1 = 1,
    MY_NAMESPACE_MY_MODULE_MY_ENUM_VAL_2 = 2,
}

/**
 * Some cool data structure.
 */
struct my_struct {
    int a;
    float b;
};

/**
 * This is my awesome function doing great things.
 *
 * Here are some details about it:
 * - Detail 1
 * - Detail 2
 *
 * @param a If > 0, something happens.
 * @param b Only positive values valid, makes sure fancy things happen.
 * @return Result of the computation X which is always > 0. -1 on error.
 */
int my_namespace_my_module_func(int a, int b);

#endif
```

### Naming conventions

In general, try to keep names short but don't overdo it by using abbrevations of things you created.
Sometimes this is not possible and we accept exceptions if there are no proper alternatives.

#### Namespacing

The folder names use lower-case names with dashes *-* as seperators, e.g. *my-namespace*.

#### Modules

Header and source files of modules use lower-case names with dashes *-* as seperators, e.g.
*my-module.c*, \*my-module.h.

The include guards contain the name of the namespace and module, see \[here\](#### Include guards).

Variables, functions, structs, enums and macros are namespace accordingly.

#### Variables

Snake-case with proper namespacing to namespace and module. Namespacing applies to static and global
variables. Local variables are not namespaced.

```
// For namespace "ezusb", module "device", static variable in module
static HANDLE ezusb_device_handle;

// Local variable in some function
int buffer_size = 256;
```

#### Functions

Snake-case with proper namespacing to namespace and module for all functions that are not hook
functions.

```
// For namespace "ezusb", module "device", static variable in module, init function
void ezusb_device_init(...);

// CreateFileA hook function inside module
HANDLE my_CreateFileA(...)
{
    // ...
}
```

#### Structs

Snake-case with proper namespacing to namespace and module.

```
// For namespace "ezusb", module "device" ctx struct
struct ezusb_device_ctx {
    // ...
}
```

#### Enums

Snake-case with proper namespacing to namespace and module. Upper-case for enum entries

```
// For namespace "ezusb", module "device" state enum
struct EZUSB_DEVICE_STATE {
    EZUSB_DEVICE_STATE_INIT = 0,
    EZUSB_DEVICE_STATE_RUNNING = 1,
    // ...
}
```

#### Macros

Upper-case with underscore as spacing, proper namespacing to namespace and module.

```
// For namespace "ezusb", module "device" vid
#define EZUSB_DEVICE_VID 0xFFFF
```

#### Multiple variable declarations

Only one declaration per line which also avoids various errors, for example:

```
// Nope
char a, b, b;
```

```
// Ok
char a;
char b;
char c;
```

### Testing

We advice you to write unit tests for all modules that allow proper unit testing. This applies to
modules that are not part of the actual hooking process and do not rely on external devices to be
available. Add these tests to the *src/test* sub-folder.

This does not only speed up your own development but hardens the code base and avoids having to test
these things by running the real applications, hence saving a lot of time and trouble.

### Further best practices

- Avoid external dependencies like additional libraries. Bemanitools is extremely self-contained
  which enables high portability and control which is important for implementing various "hacks" to
  just make things work.
- If you see some module/function lacking documentation that you have to use/understand, add
  documentation once you figured out what the function/module is doing. This does not only help your
  future you but others reading the code.
- Keep documentation and readme files up-to-date. When introducing changes/adding new features,
  review existing documentation and apply necessary changes.

## Misc

The core API interception code was ripped out, cleaned up a tiny bit and released on GitHub. BT5
will eventually be ported to use this external library in order to avoid maintaining the same code
in two places at once.

https://github.com/decafcode/capnhook

This too is a little rudimentary; it doesn't come with any examples or even a README yet.
