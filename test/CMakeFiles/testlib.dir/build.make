# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

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
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.18.4/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.18.4/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/mailund/Projects/cstr

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/mailund/Projects/cstr

# Include any dependencies generated for this target.
include test/CMakeFiles/testlib.dir/depend.make

# Include the progress variables for this target.
include test/CMakeFiles/testlib.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/testlib.dir/flags.make

test/CMakeFiles/testlib.dir/testlib.c.o: test/CMakeFiles/testlib.dir/flags.make
test/CMakeFiles/testlib.dir/testlib.c.o: test/testlib.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mailund/Projects/cstr/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object test/CMakeFiles/testlib.dir/testlib.c.o"
	cd /Users/mailund/Projects/cstr/test && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/testlib.dir/testlib.c.o -c /Users/mailund/Projects/cstr/test/testlib.c

test/CMakeFiles/testlib.dir/testlib.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/testlib.dir/testlib.c.i"
	cd /Users/mailund/Projects/cstr/test && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/mailund/Projects/cstr/test/testlib.c > CMakeFiles/testlib.dir/testlib.c.i

test/CMakeFiles/testlib.dir/testlib.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/testlib.dir/testlib.c.s"
	cd /Users/mailund/Projects/cstr/test && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/mailund/Projects/cstr/test/testlib.c -o CMakeFiles/testlib.dir/testlib.c.s

# Object files for target testlib
testlib_OBJECTS = \
"CMakeFiles/testlib.dir/testlib.c.o"

# External object files for target testlib
testlib_EXTERNAL_OBJECTS =

test/libtestlib.a: test/CMakeFiles/testlib.dir/testlib.c.o
test/libtestlib.a: test/CMakeFiles/testlib.dir/build.make
test/libtestlib.a: test/CMakeFiles/testlib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/mailund/Projects/cstr/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C static library libtestlib.a"
	cd /Users/mailund/Projects/cstr/test && $(CMAKE_COMMAND) -P CMakeFiles/testlib.dir/cmake_clean_target.cmake
	cd /Users/mailund/Projects/cstr/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/testlib.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/testlib.dir/build: test/libtestlib.a

.PHONY : test/CMakeFiles/testlib.dir/build

test/CMakeFiles/testlib.dir/clean:
	cd /Users/mailund/Projects/cstr/test && $(CMAKE_COMMAND) -P CMakeFiles/testlib.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/testlib.dir/clean

test/CMakeFiles/testlib.dir/depend:
	cd /Users/mailund/Projects/cstr && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/mailund/Projects/cstr /Users/mailund/Projects/cstr/test /Users/mailund/Projects/cstr /Users/mailund/Projects/cstr/test /Users/mailund/Projects/cstr/test/CMakeFiles/testlib.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/testlib.dir/depend

