# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.23

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
CMAKE_COMMAND = /home/duo/Downloads/clion-2022.2.1/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/duo/Downloads/clion-2022.2.1/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/duo/SimpleOS

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/duo/SimpleOS/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/SimpleOS.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/SimpleOS.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/SimpleOS.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/SimpleOS.dir/flags.make

CMakeFiles/SimpleOS.dir/main.c.o: CMakeFiles/SimpleOS.dir/flags.make
CMakeFiles/SimpleOS.dir/main.c.o: ../main.c
CMakeFiles/SimpleOS.dir/main.c.o: CMakeFiles/SimpleOS.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/duo/SimpleOS/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/SimpleOS.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/SimpleOS.dir/main.c.o -MF CMakeFiles/SimpleOS.dir/main.c.o.d -o CMakeFiles/SimpleOS.dir/main.c.o -c /home/duo/SimpleOS/main.c

CMakeFiles/SimpleOS.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/SimpleOS.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/duo/SimpleOS/main.c > CMakeFiles/SimpleOS.dir/main.c.i

CMakeFiles/SimpleOS.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/SimpleOS.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/duo/SimpleOS/main.c -o CMakeFiles/SimpleOS.dir/main.c.s

CMakeFiles/SimpleOS.dir/tools/floppy.c.o: CMakeFiles/SimpleOS.dir/flags.make
CMakeFiles/SimpleOS.dir/tools/floppy.c.o: ../tools/floppy.c
CMakeFiles/SimpleOS.dir/tools/floppy.c.o: CMakeFiles/SimpleOS.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/duo/SimpleOS/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/SimpleOS.dir/tools/floppy.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/SimpleOS.dir/tools/floppy.c.o -MF CMakeFiles/SimpleOS.dir/tools/floppy.c.o.d -o CMakeFiles/SimpleOS.dir/tools/floppy.c.o -c /home/duo/SimpleOS/tools/floppy.c

CMakeFiles/SimpleOS.dir/tools/floppy.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/SimpleOS.dir/tools/floppy.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/duo/SimpleOS/tools/floppy.c > CMakeFiles/SimpleOS.dir/tools/floppy.c.i

CMakeFiles/SimpleOS.dir/tools/floppy.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/SimpleOS.dir/tools/floppy.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/duo/SimpleOS/tools/floppy.c -o CMakeFiles/SimpleOS.dir/tools/floppy.c.s

CMakeFiles/SimpleOS.dir/tools/fileinfo.c.o: CMakeFiles/SimpleOS.dir/flags.make
CMakeFiles/SimpleOS.dir/tools/fileinfo.c.o: ../tools/fileinfo.c
CMakeFiles/SimpleOS.dir/tools/fileinfo.c.o: CMakeFiles/SimpleOS.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/duo/SimpleOS/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/SimpleOS.dir/tools/fileinfo.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/SimpleOS.dir/tools/fileinfo.c.o -MF CMakeFiles/SimpleOS.dir/tools/fileinfo.c.o.d -o CMakeFiles/SimpleOS.dir/tools/fileinfo.c.o -c /home/duo/SimpleOS/tools/fileinfo.c

CMakeFiles/SimpleOS.dir/tools/fileinfo.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/SimpleOS.dir/tools/fileinfo.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/duo/SimpleOS/tools/fileinfo.c > CMakeFiles/SimpleOS.dir/tools/fileinfo.c.i

CMakeFiles/SimpleOS.dir/tools/fileinfo.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/SimpleOS.dir/tools/fileinfo.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/duo/SimpleOS/tools/fileinfo.c -o CMakeFiles/SimpleOS.dir/tools/fileinfo.c.s

# Object files for target SimpleOS
SimpleOS_OBJECTS = \
"CMakeFiles/SimpleOS.dir/main.c.o" \
"CMakeFiles/SimpleOS.dir/tools/floppy.c.o" \
"CMakeFiles/SimpleOS.dir/tools/fileinfo.c.o"

# External object files for target SimpleOS
SimpleOS_EXTERNAL_OBJECTS =

SimpleOS: CMakeFiles/SimpleOS.dir/main.c.o
SimpleOS: CMakeFiles/SimpleOS.dir/tools/floppy.c.o
SimpleOS: CMakeFiles/SimpleOS.dir/tools/fileinfo.c.o
SimpleOS: CMakeFiles/SimpleOS.dir/build.make
SimpleOS: CMakeFiles/SimpleOS.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/duo/SimpleOS/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C executable SimpleOS"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/SimpleOS.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/SimpleOS.dir/build: SimpleOS
.PHONY : CMakeFiles/SimpleOS.dir/build

CMakeFiles/SimpleOS.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/SimpleOS.dir/cmake_clean.cmake
.PHONY : CMakeFiles/SimpleOS.dir/clean

CMakeFiles/SimpleOS.dir/depend:
	cd /home/duo/SimpleOS/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/duo/SimpleOS /home/duo/SimpleOS /home/duo/SimpleOS/cmake-build-debug /home/duo/SimpleOS/cmake-build-debug /home/duo/SimpleOS/cmake-build-debug/CMakeFiles/SimpleOS.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/SimpleOS.dir/depend

