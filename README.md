
X800W
=====

A 2048 puzzle clone for Windows console.

Play [original 2048 here](https://gabrielecirulli.github.io/2048/)
that is created by Gabriele Cirulli.

Below is a screenshot of 2048 on Windows 10 cmd.exe with Consolas font.

![2048 clone for Windows console](won.png)


Playing 2048 on Windows Console
-------------------------------

Run 2048.exe from a command prompt, and use the arrow keys.

| key | function |
|-----|----------|
| *Arrow keys* | move tiles |
| `t` | Transpose board |
| `r` | Rotate board clockwise |
| `v` | Vertically flip board |
| `h` | Horizontally flip board |
| `i` | Initialize board (new game starts) |
| `z` | Undo (only once, and if pressed immediately) |
| `e` | ? |
| `w` | ? |
| `F5` | Redraw board |
| `q` | quit game (unconditionally, no confirmation) |
| `Escape` | quit game (unconditionally, no confirmation) |


### Command line options

The `2048.exe` executable supports following options:

| short option | long option | description |
| ------------ | ----------- | ----------- |
| -c *VALUE* | --color=*VALUE* | color scheme: `0`, `1`, `2` or `3` (default: 0) |
| -g *VALUE* | --grid=*VALUE* | draw with `ascii` or `unicode` characters (default: unicode) |
| -w | --wipe | wipes the display when exiting (default: do not wipe) |
| | --test | with '--color' shows color scheme and exit |
| | --tile-set=*VALUE* | previews grid/tiles, choices `1`, `2` or `3` (default: 1) |
| | --version | displays version and other info |
| | --help | this help info (except help and version) |


### Quirks

Mouse clicks are supported to some extent. With older Windows or
with _`Use legacy console`_ option in Windows 10, mouse wheel
can be used --- can try mouse wheel with shift key too.

> **Caution**: a mouse operation with a modifier key does
> something weird!

> **Note**: *There are some known bugs and idiosyncrasies not fixed
> since they presumably make game more interesting!*

>
> **Hint**: Keep bottom row filled then game can be fast paced.
> For example `2` will not appear once `256` or higher block obtained,
> and so on...
>


#### Font

Known to run on Windows 10 (Home).
There can be font issues with older versions of Windows.

Try `Consolas` font. With other fonts there can be gaps or missing characters.


#### Other consoles

The 2048 console game is tested with cmd.exe of Windows 10.

Of course, 2048 runs on Console2, ConsoleZ or Cmder, but possibly without mouse
support and there are other limitations, for example color related issues.


Why just another 2048 clone?
----------------------------

Actually *playing* with some Win32 console API using LLVM/GCC/MSVC
compilers and collecting some pieces of code in one place.

These pieces are variations of old codes used when writing code
for quick testing purposes in odd circumstances with Windows OSes.


Compilation
-----------

### Supported compilers

Known to compile with below compilers. (There can be warnings.)

* MSVC 2015 (Community) or MSVC 2017 (Build Tools)
* GNU/GCC 6.3 or 7.2 (G++ from MinGW-w64 posix-seh)
* LLVM 3.9, 4.0 or 5.0 with one of above compilers (MSVC or GCC 6.3)

Minimal compilation command can be:

* cl -EHsc 2048.cpp
* g++ 2048.cpp
* clang++ 2048.cpp


### Using cc.bat

A single file compilation helper batch file `cc.bat` can also be used.
`cc.bat` sets few macros and automates collecting few trickier or more useful
info on tool chains used.

Create `vce.bat` that calls *VS2015 x64 Native Tools Command Prompt* and place
it in the `PATH` or in the current directory. Or create `g72.bat` that setups
*Mingw-w64 GNU/GCC 7.2* `x86_64-posix-seh` from `SourceForge.net` or
*MinGW Distro* from `nuwen.net` (compiled by *Stephan T. Lavavej*).

Examples to compile with cc.bat in a command prompt:
*(below are valid for current directory where 2048.cpp is found)*

* cc e 2048.cpp
* cc 7 2048.cpp

or to see more warnings:

* cc e 2048.cpp -Wall
* cc 7 2048.cpp -Weffc++ -pedantic -Wunused

and also try `2048 --version` to see the details.


License
-------

> **TODO**: *check Free Software Foundation, Inc. for possible licenses.*

If stealing fragments of the code, assume zlib license or public
domain and feel free to modify and use.


Acknowledgements
----------------

MSDN web pages and GCC and LLVM manuals deserve credits.
Also, some posts found at `stackoverflow.com` have helped to
isolate/remove compiler compatibility issues or tricky issues
with Win32 API.

