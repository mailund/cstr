# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.22.2/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.22.2/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/mailund/Projects/cstr

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/mailund/Projects/cstr

# Include any dependencies generated for this target.
include test/CMakeFiles/sa_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/sa_test.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/sa_test.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/sa_test.dir/flags.make

test/CMakeFiles/sa_test.dir/sa_test.c.o: test/CMakeFiles/sa_test.dir/flags.make
test/CMakeFiles/sa_test.dir/sa_test.c.o: test/sa_test.c
test/CMakeFiles/sa_test.dir/sa_test.c.o: test/CMakeFiles/sa_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mailund/Projects/cstr/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object test/CMakeFiles/sa_test.dir/sa_test.c.o"
	cd /Users/mailund/Projects/cstr/test && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/sa_test.dir/sa_test.c.o -MF CMakeFiles/sa_test.dir/sa_test.c.o.d -o CMakeFiles/sa_test.dir/sa_test.c.o -c /Users/mailund/Projects/cstr/test/sa_test.c

test/CMakeFiles/sa_test.dir/sa_test.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/sa_test.dir/sa_test.c.i"
	cd /Users/mailund/Projects/cstr/test && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/mailund/Projects/cstr/test/sa_test.c > CMakeFiles/sa_test.dir/sa_test.c.i

test/CMakeFiles/sa_test.dir/sa_test.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/sa_test.dir/sa_test.c.s"
	cd /Users/mailund/Projects/cstr/test && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/mailund/Projects/cstr/test/sa_test.c -o CMakeFiles/sa_test.dir/sa_test.c.s

# Object files for target sa_test
sa_test_OBJECTS = \
"CMakeFiles/sa_test.dir/sa_test.c.o"

# External object files for target sa_test
sa_test_EXTERNAL_OBJECTS =

test/sa_test: test/CMakeFiles/sa_test.dir/sa_test.c.o
test/sa_test: test/CMakeFiles/sa_test.dir/build.make
test/sa_test: src/libcstr.a
test/sa_test: test/libtestlib.a
test/sa_test: src/libcstr.a
test/sa_test: test/libtestlib.a
test/sa_test: test/CMakeFiles/sa_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/mailund/Projects/cstr/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable sa_test"
	cd /Users/mailund/Projects/cstr/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/sa_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/sa_test.dir/build: test/sa_test
.PHONY : test/CMakeFiles/sa_test.dir/build

test/CMakeFiles/sa_test.dir/clean:
	cd /Users/mailund/Projects/cstr/test && $(CMAKE_COMMAND) -P CMakeFiles/sa_test.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/sa_test.dir/clean

test/CMakeFiles/sa_test.dir/depend:
	cd /Users/mailund/Projects/cstr && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/mailund/Projects/cstr /Users/mailund/Projects/cstr/test /Users/mailund/Projects/cstr /Users/mailund/Projects/cstr/test /Users/mailund/Projects/cstr/test/CMakeFiles/sa_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/sa_test.dir/depend

