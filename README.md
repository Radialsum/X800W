
X800W
=====

2048 clone for Windows console.

Why just another 2048 clone?
----------------------------
Actually playing with some Win32 console API using LLVM/GCC/MSVC
compilers and collecting some pieces of code in one place.

These pieces are variations of old codes used when writing code
for quick testing purposes in odd circumstances with Windows OSes.

Target
------
Known to run on Windows 10 (Home).
There may be font issues with older versions of Windows.

Try Consolas font.

Supported compilers
-------------------
* MSVC 2015 (Community)
* GCC 6.3 (G++ from MinGW64 posix-seh)
* LLVM 3.9 with one of above compilers (MSVC 2015 or GCC 6.3)

Compilation - the easy way
--------------------------
* cl -EHsc 2048.cpp
* g++ 2048.cpp
* clang++ 2048.cpp

Playing 2048 for Windows Console
--------------------------------
Clear the screen before starting the game and use arrow keys.


| key | function |
|-----|----------|
| t | Transpose board |
| r | Rotate board clockwise |
| v | Vertically flip board |
| h | Horizontally flip board |
| i | Initialize board (restart) |
| z | Undo (only once, and if pressed immediately) |
| e | ? |
| w | ? |
| F5 | Redraw board |
| q | quit game |
| Escape | quit game |
| Arrow keys | move tiles |

Remarks
-------
There are some known bugs and idiosyncrasies not fixed
since they presumably make game more interesting!

Mouse clicks are supported to some extent. With older Windows or
with _`Use legacy console`_ option in Windows 10, mouse wheel
can be used --- can try mouse wheel with shift key too.

> **Caution**: a mouse operation with another modifier key does
> something weird.


License
-------
_TODO - check Free Software Foundation, Inc. for possible
licenses._

If stealing fragments of the code, assume zlib license or public
domain and feel free to modify and use.

Acknowledgements
----------------
MSDN web pages and GCC and LLVM manuals deserve credits.
Also, some posts found at `stackoverflow.com` have helped to
isolate/remove compiler compatibility issues or tricky issues
with Win32 API. 

