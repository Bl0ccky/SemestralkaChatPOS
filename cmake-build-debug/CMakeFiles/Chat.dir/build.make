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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.

CMAKE_SOURCE_DIR = /tmp/tmp.5TDpTqwwZt

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /tmp/tmp.5TDpTqwwZt/cmake-build-debug


# Include any dependencies generated for this target.
include CMakeFiles/Chat.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Chat.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Chat.dir/flags.make

CMakeFiles/Chat.dir/main.c.o: CMakeFiles/Chat.dir/flags.make
CMakeFiles/Chat.dir/main.c.o: ../main.c

	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/tmp/tmp.5TDpTqwwZt/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/Chat.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/Chat.dir/main.c.o -c /tmp/tmp.5TDpTqwwZt/main.c

CMakeFiles/Chat.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/Chat.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /tmp/tmp.5TDpTqwwZt/main.c > CMakeFiles/Chat.dir/main.c.i

CMakeFiles/Chat.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/Chat.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /tmp/tmp.5TDpTqwwZt/main.c -o CMakeFiles/Chat.dir/main.c.s

CMakeFiles/Chat.dir/client.c.o: CMakeFiles/Chat.dir/flags.make
CMakeFiles/Chat.dir/client.c.o: ../client.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/tmp/tmp.5TDpTqwwZt/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/Chat.dir/client.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/Chat.dir/client.c.o -c /tmp/tmp.5TDpTqwwZt/client.c

CMakeFiles/Chat.dir/client.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/Chat.dir/client.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /tmp/tmp.5TDpTqwwZt/client.c > CMakeFiles/Chat.dir/client.c.i

CMakeFiles/Chat.dir/client.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/Chat.dir/client.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /tmp/tmp.5TDpTqwwZt/client.c -o CMakeFiles/Chat.dir/client.c.s

CMakeFiles/Chat.dir/server.c.o: CMakeFiles/Chat.dir/flags.make
CMakeFiles/Chat.dir/server.c.o: ../server.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/tmp/tmp.5TDpTqwwZt/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/Chat.dir/server.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/Chat.dir/server.c.o -c /tmp/tmp.5TDpTqwwZt/server.c

CMakeFiles/Chat.dir/server.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/Chat.dir/server.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /tmp/tmp.5TDpTqwwZt/server.c > CMakeFiles/Chat.dir/server.c.i

CMakeFiles/Chat.dir/server.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/Chat.dir/server.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /tmp/tmp.5TDpTqwwZt/server.c -o CMakeFiles/Chat.dir/server.c.s

CMakeFiles/Chat.dir/arrayList.c.o: CMakeFiles/Chat.dir/flags.make
CMakeFiles/Chat.dir/arrayList.c.o: ../arrayList.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/tmp/tmp.5TDpTqwwZt/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/Chat.dir/arrayList.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/Chat.dir/arrayList.c.o -c /tmp/tmp.5TDpTqwwZt/arrayList.c

CMakeFiles/Chat.dir/arrayList.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/Chat.dir/arrayList.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /tmp/tmp.5TDpTqwwZt/arrayList.c > CMakeFiles/Chat.dir/arrayList.c.i

CMakeFiles/Chat.dir/arrayList.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/Chat.dir/arrayList.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /tmp/tmp.5TDpTqwwZt/arrayList.c -o CMakeFiles/Chat.dir/arrayList.c.s

# Object files for target Chat
Chat_OBJECTS = \
"CMakeFiles/Chat.dir/main.c.o" \
"CMakeFiles/Chat.dir/client.c.o" \
"CMakeFiles/Chat.dir/server.c.o" \
"CMakeFiles/Chat.dir/arrayList.c.o"

# External object files for target Chat
Chat_EXTERNAL_OBJECTS =

Chat: CMakeFiles/Chat.dir/main.c.o
Chat: CMakeFiles/Chat.dir/client.c.o
Chat: CMakeFiles/Chat.dir/server.c.o
Chat: CMakeFiles/Chat.dir/arrayList.c.o
Chat: CMakeFiles/Chat.dir/build.make
Chat: CMakeFiles/Chat.dir/link.txt

	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/tmp/tmp.5TDpTqwwZt/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking C executable Chat"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Chat.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Chat.dir/build: Chat

.PHONY : CMakeFiles/Chat.dir/build

CMakeFiles/Chat.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Chat.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Chat.dir/clean

CMakeFiles/Chat.dir/depend:

	cd /tmp/tmp.5TDpTqwwZt/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /tmp/tmp.5TDpTqwwZt /tmp/tmp.5TDpTqwwZt /tmp/tmp.5TDpTqwwZt/cmake-build-debug /tmp/tmp.5TDpTqwwZt/cmake-build-debug /tmp/tmp.5TDpTqwwZt/cmake-build-debug/CMakeFiles/Chat.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Chat.dir/depend

