# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/kvgroup/lcy/chogori-platform-indexer-tmp

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug

# Include any dependencies generated for this target.
include src/k2/common/CMakeFiles/common.dir/depend.make

# Include the progress variables for this target.
include src/k2/common/CMakeFiles/common.dir/progress.make

# Include the compile flags for this target's objects.
include src/k2/common/CMakeFiles/common.dir/flags.make

src/k2/common/CMakeFiles/common.dir/Chrono.cpp.o: src/k2/common/CMakeFiles/common.dir/flags.make
src/k2/common/CMakeFiles/common.dir/Chrono.cpp.o: ../src/k2/common/Chrono.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/k2/common/CMakeFiles/common.dir/Chrono.cpp.o"
	cd /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/common && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/common.dir/Chrono.cpp.o -c /home/kvgroup/lcy/chogori-platform-indexer-tmp/src/k2/common/Chrono.cpp

src/k2/common/CMakeFiles/common.dir/Chrono.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/common.dir/Chrono.cpp.i"
	cd /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/common && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lcy/chogori-platform-indexer-tmp/src/k2/common/Chrono.cpp > CMakeFiles/common.dir/Chrono.cpp.i

src/k2/common/CMakeFiles/common.dir/Chrono.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/common.dir/Chrono.cpp.s"
	cd /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/common && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lcy/chogori-platform-indexer-tmp/src/k2/common/Chrono.cpp -o CMakeFiles/common.dir/Chrono.cpp.s

src/k2/common/CMakeFiles/common.dir/Log.cpp.o: src/k2/common/CMakeFiles/common.dir/flags.make
src/k2/common/CMakeFiles/common.dir/Log.cpp.o: ../src/k2/common/Log.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/k2/common/CMakeFiles/common.dir/Log.cpp.o"
	cd /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/common && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/common.dir/Log.cpp.o -c /home/kvgroup/lcy/chogori-platform-indexer-tmp/src/k2/common/Log.cpp

src/k2/common/CMakeFiles/common.dir/Log.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/common.dir/Log.cpp.i"
	cd /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/common && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lcy/chogori-platform-indexer-tmp/src/k2/common/Log.cpp > CMakeFiles/common.dir/Log.cpp.i

src/k2/common/CMakeFiles/common.dir/Log.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/common.dir/Log.cpp.s"
	cd /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/common && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lcy/chogori-platform-indexer-tmp/src/k2/common/Log.cpp -o CMakeFiles/common.dir/Log.cpp.s

# Object files for target common
common_OBJECTS = \
"CMakeFiles/common.dir/Chrono.cpp.o" \
"CMakeFiles/common.dir/Log.cpp.o"

# External object files for target common
common_EXTERNAL_OBJECTS =

src/k2/common/libcommon.a: src/k2/common/CMakeFiles/common.dir/Chrono.cpp.o
src/k2/common/libcommon.a: src/k2/common/CMakeFiles/common.dir/Log.cpp.o
src/k2/common/libcommon.a: src/k2/common/CMakeFiles/common.dir/build.make
src/k2/common/libcommon.a: src/k2/common/CMakeFiles/common.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libcommon.a"
	cd /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/common && $(CMAKE_COMMAND) -P CMakeFiles/common.dir/cmake_clean_target.cmake
	cd /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/common && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/common.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/k2/common/CMakeFiles/common.dir/build: src/k2/common/libcommon.a

.PHONY : src/k2/common/CMakeFiles/common.dir/build

src/k2/common/CMakeFiles/common.dir/clean:
	cd /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/common && $(CMAKE_COMMAND) -P CMakeFiles/common.dir/cmake_clean.cmake
.PHONY : src/k2/common/CMakeFiles/common.dir/clean

src/k2/common/CMakeFiles/common.dir/depend:
	cd /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kvgroup/lcy/chogori-platform-indexer-tmp /home/kvgroup/lcy/chogori-platform-indexer-tmp/src/k2/common /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/common /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/common/CMakeFiles/common.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/k2/common/CMakeFiles/common.dir/depend

