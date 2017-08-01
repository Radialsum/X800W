
X800W
=====

2048 clone for Windows console

Why just another 2048?
----------------------
Actually playing with LLVM/GCC/MSVC compilers and
collecting some possibly useful (my own) pieces of code
in one place.

Target
------
Known to run on Windows 10 (Home).
There may be font issues with older versions of Windows.
Try with Consolas font.

Supported compilers
-------------------
1. MSVC 2015 (Community)
2. GCC 6.3 (need G++ from MinGW64 posix-seh)
3. LLVM 3.9 with one of above compilers (MSVC 2015 or GCC 6.3)

Compilation - the easy way
--------------------------
1. cl -EHsc 2048.cpp
2. g++ 2048.cpp
3. clang++ g.cpp

Playing 2048 for Windows Console
--------------------------------
Use arrow keys
key - function
  t - Transpose board
  r - Rotate clockwise board
  v - Vertically flip board
  h - Horizontally flip board
 F5 - Redraw board
  i - Initialize board (restart)
  z - Undo (only once, and if immediately
  w or e - ?
  Escape and q - quits game

Remarks
-------
There are some known bugs and idiosyncrasies not fixed
since they presumably make game more interesting!


License
-------
TODO - Check Free Software Foundation, Inc. for possible
licenses.



