sudoku_dlx -- C++ header-only Dancing Links exact cover library 
===============================================================

© 2008, 2016, 2021 Mark Deric

This work is offered under the the terms of the MIT License; see the
LICENSE file in the top directory of this source distribution or on
Github: https://github.com/jmderic/sudoku_dlx

Summary
-------

Implements Knuth's
["Dancing Links" Algorithm X](https://www.ocf.berkeley.edu/~jchu/publicportal/sudoku/0011047.pdf)
to solve the "exact cover" problem.  This DLX algorithm was originally
implemented here as a shared library; the shared library version is
tagged as `shared_lib`.  Now it is a C++ header-only library wholly
within `./include/jmdlx.h`.  A couple supporting files give the Sudoku
Solver use case which demonstrates the DLX API.  Unit tests are
implemented in the `test` directory.

The whole thing is pretty short. I like to write code a lot; but I
don't like to write a lot of code!

```
 $ find . \( -name "*.cpp" -o -name "*.h" \) -type f | xargs wc -l
  110 ./test/SudokuDlxTest.cpp
  175 ./src/sudoku_squares.h
   91 ./src/sudoku_main.cpp
  258 ./include/jmdlx.h
  634 total
```

These sizes are likely to change as I'm currently working to update
this code to the C++17 standard as well as to improve it with what
I've learned since the earlier work.

Building and Testing
--------------------

This project builds using `cmake`, recently changed from GNU
autotools.  For more information on building for production, debug,
and building with ASAN and LSAN see
[Building and Testing](BUILDnTEST.md).  This also includes information
on running the unit tests.

Running the Product
-------------------

To run the program in place where you built it, use the executable in
the src directory by typing `./src/sudoku_dlx` from this directory.

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
