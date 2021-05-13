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
include src/k2/partitionManager/CMakeFiles/partition_manager.dir/depend.make

# Include the progress variables for this target.
include src/k2/partitionManager/CMakeFiles/partition_manager.dir/progress.make

# Include the compile flags for this target's objects.
include src/k2/partitionManager/CMakeFiles/partition_manager.dir/flags.make

src/k2/partitionManager/CMakeFiles/partition_manager.dir/PartitionManager.cpp.o: src/k2/partitionManager/CMakeFiles/partition_manager.dir/flags.make
src/k2/partitionManager/CMakeFiles/partition_manager.dir/PartitionManager.cpp.o: ../src/k2/partitionManager/PartitionManager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/k2/partitionManager/CMakeFiles/partition_manager.dir/PartitionManager.cpp.o"
	cd /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/partitionManager && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/partition_manager.dir/PartitionManager.cpp.o -c /home/kvgroup/lcy/chogori-platform-indexer-tmp/src/k2/partitionManager/PartitionManager.cpp

src/k2/partitionManager/CMakeFiles/partition_manager.dir/PartitionManager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/partition_manager.dir/PartitionManager.cpp.i"
	cd /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/partitionManager && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kvgroup/lcy/chogori-platform-indexer-tmp/src/k2/partitionManager/PartitionManager.cpp > CMakeFiles/partition_manager.dir/PartitionManager.cpp.i

src/k2/partitionManager/CMakeFiles/partition_manager.dir/PartitionManager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/partition_manager.dir/PartitionManager.cpp.s"
	cd /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/partitionManager && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kvgroup/lcy/chogori-platform-indexer-tmp/src/k2/partitionManager/PartitionManager.cpp -o CMakeFiles/partition_manager.dir/PartitionManager.cpp.s

# Object files for target partition_manager
partition_manager_OBJECTS = \
"CMakeFiles/partition_manager.dir/PartitionManager.cpp.o"

# External object files for target partition_manager
partition_manager_EXTERNAL_OBJECTS =

src/k2/partitionManager/libpartition_manager.a: src/k2/partitionManager/CMakeFiles/partition_manager.dir/PartitionManager.cpp.o
src/k2/partitionManager/libpartition_manager.a: src/k2/partitionManager/CMakeFiles/partition_manager.dir/build.make
src/k2/partitionManager/libpartition_manager.a: src/k2/partitionManager/CMakeFiles/partition_manager.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libpartition_manager.a"
	cd /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/partitionManager && $(CMAKE_COMMAND) -P CMakeFiles/partition_manager.dir/cmake_clean_target.cmake
	cd /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/partitionManager && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/partition_manager.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/k2/partitionManager/CMakeFiles/partition_manager.dir/build: src/k2/partitionManager/libpartition_manager.a

.PHONY : src/k2/partitionManager/CMakeFiles/partition_manager.dir/build

src/k2/partitionManager/CMakeFiles/partition_manager.dir/clean:
	cd /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/partitionManager && $(CMAKE_COMMAND) -P CMakeFiles/partition_manager.dir/cmake_clean.cmake
.PHONY : src/k2/partitionManager/CMakeFiles/partition_manager.dir/clean

src/k2/partitionManager/CMakeFiles/partition_manager.dir/depend:
	cd /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kvgroup/lcy/chogori-platform-indexer-tmp /home/kvgroup/lcy/chogori-platform-indexer-tmp/src/k2/partitionManager /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/partitionManager /home/kvgroup/lcy/chogori-platform-indexer-tmp/cmake-build-debug/src/k2/partitionManager/CMakeFiles/partition_manager.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/k2/partitionManager/CMakeFiles/partition_manager.dir/depend

