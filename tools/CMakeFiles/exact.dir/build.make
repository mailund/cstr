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
include tools/CMakeFiles/exact.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tools/CMakeFiles/exact.dir/compiler_depend.make

# Include the progress variables for this target.
include tools/CMakeFiles/exact.dir/progress.make

# Include the compile flags for this target's objects.
include tools/CMakeFiles/exact.dir/flags.make

tools/CMakeFiles/exact.dir/exact.c.o: tools/CMakeFiles/exact.dir/flags.make
tools/CMakeFiles/exact.dir/exact.c.o: tools/exact.c
tools/CMakeFiles/exact.dir/exact.c.o: tools/CMakeFiles/exact.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mailund/Projects/cstr/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object tools/CMakeFiles/exact.dir/exact.c.o"
	cd /Users/mailund/Projects/cstr/tools && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT tools/CMakeFiles/exact.dir/exact.c.o -MF CMakeFiles/exact.dir/exact.c.o.d -o CMakeFiles/exact.dir/exact.c.o -c /Users/mailund/Projects/cstr/tools/exact.c

tools/CMakeFiles/exact.dir/exact.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/exact.dir/exact.c.i"
	cd /Users/mailund/Projects/cstr/tools && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/mailund/Projects/cstr/tools/exact.c > CMakeFiles/exact.dir/exact.c.i

tools/CMakeFiles/exact.dir/exact.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/exact.dir/exact.c.s"
	cd /Users/mailund/Projects/cstr/tools && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/mailund/Projects/cstr/tools/exact.c -o CMakeFiles/exact.dir/exact.c.s

tools/CMakeFiles/exact.dir/fasta.c.o: tools/CMakeFiles/exact.dir/flags.make
tools/CMakeFiles/exact.dir/fasta.c.o: tools/fasta.c
tools/CMakeFiles/exact.dir/fasta.c.o: tools/CMakeFiles/exact.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mailund/Projects/cstr/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object tools/CMakeFiles/exact.dir/fasta.c.o"
	cd /Users/mailund/Projects/cstr/tools && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT tools/CMakeFiles/exact.dir/fasta.c.o -MF CMakeFiles/exact.dir/fasta.c.o.d -o CMakeFiles/exact.dir/fasta.c.o -c /Users/mailund/Projects/cstr/tools/fasta.c

tools/CMakeFiles/exact.dir/fasta.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/exact.dir/fasta.c.i"
	cd /Users/mailund/Projects/cstr/tools && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/mailund/Projects/cstr/tools/fasta.c > CMakeFiles/exact.dir/fasta.c.i

tools/CMakeFiles/exact.dir/fasta.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/exact.dir/fasta.c.s"
	cd /Users/mailund/Projects/cstr/tools && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/mailund/Projects/cstr/tools/fasta.c -o CMakeFiles/exact.dir/fasta.c.s

tools/CMakeFiles/exact.dir/fastq.c.o: tools/CMakeFiles/exact.dir/flags.make
tools/CMakeFiles/exact.dir/fastq.c.o: tools/fastq.c
tools/CMakeFiles/exact.dir/fastq.c.o: tools/CMakeFiles/exact.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mailund/Projects/cstr/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object tools/CMakeFiles/exact.dir/fastq.c.o"
	cd /Users/mailund/Projects/cstr/tools && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT tools/CMakeFiles/exact.dir/fastq.c.o -MF CMakeFiles/exact.dir/fastq.c.o.d -o CMakeFiles/exact.dir/fastq.c.o -c /Users/mailund/Projects/cstr/tools/fastq.c

tools/CMakeFiles/exact.dir/fastq.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/exact.dir/fastq.c.i"
	cd /Users/mailund/Projects/cstr/tools && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/mailund/Projects/cstr/tools/fastq.c > CMakeFiles/exact.dir/fastq.c.i

tools/CMakeFiles/exact.dir/fastq.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/exact.dir/fastq.c.s"
	cd /Users/mailund/Projects/cstr/tools && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/mailund/Projects/cstr/tools/fastq.c -o CMakeFiles/exact.dir/fastq.c.s

tools/CMakeFiles/exact.dir/sam.c.o: tools/CMakeFiles/exact.dir/flags.make
tools/CMakeFiles/exact.dir/sam.c.o: tools/sam.c
tools/CMakeFiles/exact.dir/sam.c.o: tools/CMakeFiles/exact.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mailund/Projects/cstr/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object tools/CMakeFiles/exact.dir/sam.c.o"
	cd /Users/mailund/Projects/cstr/tools && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT tools/CMakeFiles/exact.dir/sam.c.o -MF CMakeFiles/exact.dir/sam.c.o.d -o CMakeFiles/exact.dir/sam.c.o -c /Users/mailund/Projects/cstr/tools/sam.c

tools/CMakeFiles/exact.dir/sam.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/exact.dir/sam.c.i"
	cd /Users/mailund/Projects/cstr/tools && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/mailund/Projects/cstr/tools/sam.c > CMakeFiles/exact.dir/sam.c.i

tools/CMakeFiles/exact.dir/sam.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/exact.dir/sam.c.s"
	cd /Users/mailund/Projects/cstr/tools && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/mailund/Projects/cstr/tools/sam.c -o CMakeFiles/exact.dir/sam.c.s

# Object files for target exact
exact_OBJECTS = \
"CMakeFiles/exact.dir/exact.c.o" \
"CMakeFiles/exact.dir/fasta.c.o" \
"CMakeFiles/exact.dir/fastq.c.o" \
"CMakeFiles/exact.dir/sam.c.o"

# External object files for target exact
exact_EXTERNAL_OBJECTS =

tools/exact: tools/CMakeFiles/exact.dir/exact.c.o
tools/exact: tools/CMakeFiles/exact.dir/fasta.c.o
tools/exact: tools/CMakeFiles/exact.dir/fastq.c.o
tools/exact: tools/CMakeFiles/exact.dir/sam.c.o
tools/exact: tools/CMakeFiles/exact.dir/build.make
tools/exact: src/libcstr.a
tools/exact: test/libtestlib.a
tools/exact: src/libcstr.a
tools/exact: test/libtestlib.a
tools/exact: tools/CMakeFiles/exact.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/mailund/Projects/cstr/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking C executable exact"
	cd /Users/mailund/Projects/cstr/tools && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/exact.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tools/CMakeFiles/exact.dir/build: tools/exact
.PHONY : tools/CMakeFiles/exact.dir/build

tools/CMakeFiles/exact.dir/clean:
	cd /Users/mailund/Projects/cstr/tools && $(CMAKE_COMMAND) -P CMakeFiles/exact.dir/cmake_clean.cmake
.PHONY : tools/CMakeFiles/exact.dir/clean

tools/CMakeFiles/exact.dir/depend:
	cd /Users/mailund/Projects/cstr && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/mailund/Projects/cstr /Users/mailund/Projects/cstr/tools /Users/mailund/Projects/cstr /Users/mailund/Projects/cstr/tools /Users/mailund/Projects/cstr/tools/CMakeFiles/exact.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tools/CMakeFiles/exact.dir/depend

