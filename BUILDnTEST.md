Building and Testing
====================

To build outside of the source tree, the preferred method, the
directory structure looks like this:

```
 sudoku_dlx_project
 \
  + sudoku_dlx <- git checkout, source directory
  |
  + sudoku_dlx_build_20210312_155145 <- build directory
```

The build directory created above uses the bash commands described
below.

Cmake as shown below creates the makefiles that perform the build.

Basic Build
-----------

Run the first build directory creation step from the source directory:


```
 [sudoku_dlx]$ BUILD_DIR=sudoku_dlx_build_`date '+%Y%m%d_%H%M%S'`; cd ..; mkdir $BUILD_DIR; cd $BUILD_DIR; cmake ../sudoku_dlx
```

This creates the new cmake build directory, cd's into it, and runs cmake on the source directory.  This prepares makefiles for building the product and the unit tests:

```
 [sudoku_dlx_build_20210312_155145]$ make -k VERBOSE=1
```

You can run the `make` command without arguments for simpler output.  The above `-k` tells make to "keep going" if it can on error.  The `VERBOSE=1` showe the details of the commands it is executing, e.g., the compile and link commands.

Build Variations
----------------

### Debug build

Adds `-DCMAKE_BUILD_TYPE=Debug` to the cmake line.

```
 [sudoku_dlx]$ BUILD_DIR=sudoku_dlx_build_`date '+%Y%m%d_%H%M%S'`; cd ..; mkdir $BUILD_DIR; cd $BUILD_DIR; cmake -DCMAKE_BUILD_TYPE=Debug ../sudoku_dlx
```
### Sanitizers

All invoke with `-DCMAKE_BUILD_TYPE=Debug`:

#### AddressSanitizer

```
 [sudoku_dlx]$ BUILD_DIR=sudoku_dlx_build_`date '+%Y%m%d_%H%M%S'`; cd ..; mkdir $BUILD_DIR; cd $BUILD_DIR; cmake -DASAN_ENABLED=ON ../sudoku_dlx
```

#### LeakSanitizer

```
 [sudoku_dlx]$ BUILD_DIR=sudoku_dlx_build_`date '+%Y%m%d_%H%M%S'`; cd ..; mkdir $BUILD_DIR; cd $BUILD_DIR; cmake -DLSAN_ENABLED=ON ../sudoku_dlx
```

#### Both

```
 [sudoku_dlx]$ BUILD_DIR=sudoku_dlx_build_`date '+%Y%m%d_%H%M%S'`; cd ..; mkdir $BUILD_DIR; cd $BUILD_DIR; cmake -DASAN_ENABLED=ON -DLSAN_ENABLED=ON ../sudoku_dlx
```

### Note on invocation directory

Any of the above commands can be invoked from the prior build directory; there is no reason to return to the source directory.  For example, if you modify some cmake files in the source directory, you can recreate the project in a new build directory **from the old build directory**:

```
 [sudoku_dlx_build_20210312_155145]$ BUILD_DIR=sudoku_dlx_build_`date '+%Y%m%d_%H%M%S'`; cd ..; mkdir $BUILD_DIR; cd $BUILD_DIR; cmake ../sudoku_dlx
 [sudoku_dlx_build_20210312_163723]$ 
```

Notice that you are now in a new build directory, created using your cmake changes.

Running Product Tests
---------------------

Running product tests and running the product itself are both done from the build directory.  The following is an example of running the tests:

    [sudoku_dlx_build_20210312_155145]$ ./test/SudokuDlxTest --gtest_output=json:test.json
    Running main() from /home/mark/development/sudoku_dlx_project/sudoku_dlx_build_20210312_155145/test/googletest-src/googletest/src/gtest_main.cc
    [==========] Running 3 tests from 1 test suite.
    [----------] Global test environment set-up.
    [----------] 3 tests from SudokuDlxTest
    [ RUN      ] SudokuDlxTest.Headline
    [       OK ] SudokuDlxTest.Headline (4 ms)
    [ RUN      ] SudokuDlxTest.DupInput
    [       OK ] SudokuDlxTest.DupInput (0 ms)
    [ RUN      ] SudokuDlxTest.Hardest
    [       OK ] SudokuDlxTest.Hardest (7 ms)
    [----------] 3 tests from SudokuDlxTest (12 ms total)
    
    [----------] Global test environment tear-down
    [==========] 3 tests from 1 test suite ran. (12 ms total)
    [  PASSED  ] 3 tests.
    [sudoku_dlx_build_20210312_155145]$ 
