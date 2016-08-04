sudoku_dlx -- A Sudoku puzzle solver in C++
===========================================

Copyright (C) 2008, 2016 Mark Deric

This work is offered under the the terms of the MIT License; see the
LICENSE file in the top directory of this source distribution or on
Github: https://github.com/jmderic/sudoku_dlx

Summary
-------

Uses Knuth's DLX "Dancing Links" concept for implementing Algorithm X
to solve the "exact cover" problem.  The DLX algorithm is installed as
a shared library; effort was made to separate the Sudoku semantics
from the exact cover library so the library can be re-used.

Also, it's pretty short:

```
 $ find . \( -name "*.cpp" -o -name "*.h" \) -type f | xargs wc -l
   95 ./src/sudoku_main.cpp
  176 ./src/sudoku_squares.h
   91 ./lib/jmdlx.cpp
  197 ./include/jmdlx.h
  559 total
```

Building
--------

This project builds using the GNU autotools suite.  It has been tested
on CentOS 7, Mac OSX, and ubuntu.  Building currently requires that
the autotools be installed to generate ./configure, the Makefile.in's,
and their supporting files.  Details are in the INSTALL file in this
directory.

Running
-------

If you did a `make install`, the program name is `sudoku_dlx`, probably
installed in `/usr/local/bin` and in your PATH.  If you want to run
the program in place where you built it, use the script in the src
directory by typing `./src/sudoku_dlx` as the program name.

After the program name, Enter each of the known squares as a 3 digit
argument representing number, row, and column.  For example, 312 says
a 3 is in row 1, column 2; 474, a 4 in row 7 column 4.

An example run:

```
$ ./src/sudoku_dlx 312 914 715 822 626 528 136 142 543 747 649 351 752 858 559 961 663 167 268 474 282 784 688 895 996 798
solution:
r1c1-5
r1c3-4
r1c6-8
r1c7-6
r1c8-1
r1c9-2
r2c1-1
r2c3-9
r2c4-3
r2c5-2
r2c7-4
r2c9-7
r3c1-2
r3c2-6
r3c3-7
r3c4-5
r3c5-4
r3c7-3
r3c8-9
r3c9-8
r4c1-8
r4c4-2
r4c5-9
r4c6-3
r4c8-4
r5c3-2
r5c4-6
r5c5-1
r5c6-4
r5c7-9
r6c2-4
r6c4-8
r6c5-5
r6c6-7
r6c9-3
r7c1-7
r7c2-9
r7c3-8
r7c5-6
r7c6-2
r7c7-5
r7c8-3
r7c9-1
r8c1-4
r8c3-1
r8c5-3
r8c6-5
r8c7-8
r8c9-9
r9c1-6
r9c2-5
r9c3-3
r9c4-1
r9c7-2
r9c9-4
Number of solutions: 1
$
```

Debugging and Misc
------------------

to debug in place w/o installing:

`libtool --mode=execute gdb sudoku_dlx`

convenient find exec grep:

`find . -name ".git" -prune -o -type f \( -name "*.cpp" -o -name "*.h" \) -exec grep -E -nH -e "to_find" {} +`

Feedback and Patches
--------------------

You are welcome to contact me with feedback and patches at the email
address in the source files.
