# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.21

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

# Produce verbose output by default.
VERBOSE = 1

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
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.21.3/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.21.3/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/mailund/Projects/cstr

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/mailund/Projects/cstr

# Include any dependencies generated for this target.
include src/CMakeFiles/cstr.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/CMakeFiles/cstr.dir/compiler_depend.make

# Include the progress variables for this target.
include src/CMakeFiles/cstr.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/cstr.dir/flags.make

src/CMakeFiles/cstr.dir/alphabet.c.o: src/CMakeFiles/cstr.dir/flags.make
src/CMakeFiles/cstr.dir/alphabet.c.o: src/alphabet.c
src/CMakeFiles/cstr.dir/alphabet.c.o: src/CMakeFiles/cstr.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mailund/Projects/cstr/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object src/CMakeFiles/cstr.dir/alphabet.c.o"
	cd /Users/mailund/Projects/cstr/src && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/CMakeFiles/cstr.dir/alphabet.c.o -MF CMakeFiles/cstr.dir/alphabet.c.o.d -o CMakeFiles/cstr.dir/alphabet.c.o -c /Users/mailund/Projects/cstr/src/alphabet.c

src/CMakeFiles/cstr.dir/alphabet.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cstr.dir/alphabet.c.i"
	cd /Users/mailund/Projects/cstr/src && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/mailund/Projects/cstr/src/alphabet.c > CMakeFiles/cstr.dir/alphabet.c.i

src/CMakeFiles/cstr.dir/alphabet.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cstr.dir/alphabet.c.s"
	cd /Users/mailund/Projects/cstr/src && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/mailund/Projects/cstr/src/alphabet.c -o CMakeFiles/cstr.dir/alphabet.c.s

src/CMakeFiles/cstr.dir/bwt.c.o: src/CMakeFiles/cstr.dir/flags.make
src/CMakeFiles/cstr.dir/bwt.c.o: src/bwt.c
src/CMakeFiles/cstr.dir/bwt.c.o: src/CMakeFiles/cstr.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mailund/Projects/cstr/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object src/CMakeFiles/cstr.dir/bwt.c.o"
	cd /Users/mailund/Projects/cstr/src && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/CMakeFiles/cstr.dir/bwt.c.o -MF CMakeFiles/cstr.dir/bwt.c.o.d -o CMakeFiles/cstr.dir/bwt.c.o -c /Users/mailund/Projects/cstr/src/bwt.c

src/CMakeFiles/cstr.dir/bwt.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cstr.dir/bwt.c.i"
	cd /Users/mailund/Projects/cstr/src && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/mailund/Projects/cstr/src/bwt.c > CMakeFiles/cstr.dir/bwt.c.i

src/CMakeFiles/cstr.dir/bwt.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cstr.dir/bwt.c.s"
	cd /Users/mailund/Projects/cstr/src && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/mailund/Projects/cstr/src/bwt.c -o CMakeFiles/cstr.dir/bwt.c.s

src/CMakeFiles/cstr.dir/cstr.c.o: src/CMakeFiles/cstr.dir/flags.make
src/CMakeFiles/cstr.dir/cstr.c.o: src/cstr.c
src/CMakeFiles/cstr.dir/cstr.c.o: src/CMakeFiles/cstr.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mailund/Projects/cstr/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object src/CMakeFiles/cstr.dir/cstr.c.o"
	cd /Users/mailund/Projects/cstr/src && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/CMakeFiles/cstr.dir/cstr.c.o -MF CMakeFiles/cstr.dir/cstr.c.o.d -o CMakeFiles/cstr.dir/cstr.c.o -c /Users/mailund/Projects/cstr/src/cstr.c

src/CMakeFiles/cstr.dir/cstr.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cstr.dir/cstr.c.i"
	cd /Users/mailund/Projects/cstr/src && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/mailund/Projects/cstr/src/cstr.c > CMakeFiles/cstr.dir/cstr.c.i

src/CMakeFiles/cstr.dir/cstr.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cstr.dir/cstr.c.s"
	cd /Users/mailund/Projects/cstr/src && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/mailund/Projects/cstr/src/cstr.c -o CMakeFiles/cstr.dir/cstr.c.s

src/CMakeFiles/cstr.dir/exact.c.o: src/CMakeFiles/cstr.dir/flags.make
src/CMakeFiles/cstr.dir/exact.c.o: src/exact.c
src/CMakeFiles/cstr.dir/exact.c.o: src/CMakeFiles/cstr.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mailund/Projects/cstr/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object src/CMakeFiles/cstr.dir/exact.c.o"
	cd /Users/mailund/Projects/cstr/src && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/CMakeFiles/cstr.dir/exact.c.o -MF CMakeFiles/cstr.dir/exact.c.o.d -o CMakeFiles/cstr.dir/exact.c.o -c /Users/mailund/Projects/cstr/src/exact.c

src/CMakeFiles/cstr.dir/exact.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cstr.dir/exact.c.i"
	cd /Users/mailund/Projects/cstr/src && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/mailund/Projects/cstr/src/exact.c > CMakeFiles/cstr.dir/exact.c.i

src/CMakeFiles/cstr.dir/exact.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cstr.dir/exact.c.s"
	cd /Users/mailund/Projects/cstr/src && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/mailund/Projects/cstr/src/exact.c -o CMakeFiles/cstr.dir/exact.c.s

src/CMakeFiles/cstr.dir/skew.c.o: src/CMakeFiles/cstr.dir/flags.make
src/CMakeFiles/cstr.dir/skew.c.o: src/skew.c
src/CMakeFiles/cstr.dir/skew.c.o: src/CMakeFiles/cstr.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mailund/Projects/cstr/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object src/CMakeFiles/cstr.dir/skew.c.o"
	cd /Users/mailund/Projects/cstr/src && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/CMakeFiles/cstr.dir/skew.c.o -MF CMakeFiles/cstr.dir/skew.c.o.d -o CMakeFiles/cstr.dir/skew.c.o -c /Users/mailund/Projects/cstr/src/skew.c

src/CMakeFiles/cstr.dir/skew.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cstr.dir/skew.c.i"
	cd /Users/mailund/Projects/cstr/src && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/mailund/Projects/cstr/src/skew.c > CMakeFiles/cstr.dir/skew.c.i

src/CMakeFiles/cstr.dir/skew.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cstr.dir/skew.c.s"
	cd /Users/mailund/Projects/cstr/src && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/mailund/Projects/cstr/src/skew.c -o CMakeFiles/cstr.dir/skew.c.s

# Object files for target cstr
cstr_OBJECTS = \
"CMakeFiles/cstr.dir/alphabet.c.o" \
"CMakeFiles/cstr.dir/bwt.c.o" \
"CMakeFiles/cstr.dir/cstr.c.o" \
"CMakeFiles/cstr.dir/exact.c.o" \
"CMakeFiles/cstr.dir/skew.c.o"

# External object files for target cstr
cstr_EXTERNAL_OBJECTS =

src/libcstr.a: src/CMakeFiles/cstr.dir/alphabet.c.o
src/libcstr.a: src/CMakeFiles/cstr.dir/bwt.c.o
src/libcstr.a: src/CMakeFiles/cstr.dir/cstr.c.o
src/libcstr.a: src/CMakeFiles/cstr.dir/exact.c.o
src/libcstr.a: src/CMakeFiles/cstr.dir/skew.c.o
src/libcstr.a: src/CMakeFiles/cstr.dir/build.make
src/libcstr.a: src/CMakeFiles/cstr.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/mailund/Projects/cstr/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking C static library libcstr.a"
	cd /Users/mailund/Projects/cstr/src && $(CMAKE_COMMAND) -P CMakeFiles/cstr.dir/cmake_clean_target.cmake
	cd /Users/mailund/Projects/cstr/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cstr.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/cstr.dir/build: src/libcstr.a
.PHONY : src/CMakeFiles/cstr.dir/build

src/CMakeFiles/cstr.dir/clean:
	cd /Users/mailund/Projects/cstr/src && $(CMAKE_COMMAND) -P CMakeFiles/cstr.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/cstr.dir/clean

src/CMakeFiles/cstr.dir/depend:
	cd /Users/mailund/Projects/cstr && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/mailund/Projects/cstr /Users/mailund/Projects/cstr/src /Users/mailund/Projects/cstr /Users/mailund/Projects/cstr/src /Users/mailund/Projects/cstr/src/CMakeFiles/cstr.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/cstr.dir/depend
