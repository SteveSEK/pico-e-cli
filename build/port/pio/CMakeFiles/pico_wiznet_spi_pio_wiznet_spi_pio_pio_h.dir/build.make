# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

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
CMAKE_COMMAND = /home/ssekim/.local/lib/python3.10/site-packages/cmake/data/bin/cmake

# The command to remove a file.
RM = /home/ssekim/.local/lib/python3.10/site-packages/cmake/data/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ssekim/work-pico/pico-e/pico-e-cli

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ssekim/work-pico/pico-e/pico-e-cli/build

# Utility rule file for pico_wiznet_spi_pio_wiznet_spi_pio_pio_h.

# Include any custom commands dependencies for this target.
include port/pio/CMakeFiles/pico_wiznet_spi_pio_wiznet_spi_pio_pio_h.dir/compiler_depend.make

# Include the progress variables for this target.
include port/pio/CMakeFiles/pico_wiznet_spi_pio_wiznet_spi_pio_pio_h.dir/progress.make

port/pio/CMakeFiles/pico_wiznet_spi_pio_wiznet_spi_pio_pio_h: port/pio/wiznet_spi_pio.pio.h

port/pio/wiznet_spi_pio.pio.h: /home/ssekim/work-pico/pico-e/pico-e-cli/port/pio/wiznet_spi_pio.pio
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/ssekim/work-pico/pico-e/pico-e-cli/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating wiznet_spi_pio.pio.h"
	cd /home/ssekim/work-pico/pico-e/pico-e-cli/build/port/pio && ../../pioasm/pioasm -o c-sdk /home/ssekim/work-pico/pico-e/pico-e-cli/port/pio/wiznet_spi_pio.pio /home/ssekim/work-pico/pico-e/pico-e-cli/build/port/pio/wiznet_spi_pio.pio.h

pico_wiznet_spi_pio_wiznet_spi_pio_pio_h: port/pio/CMakeFiles/pico_wiznet_spi_pio_wiznet_spi_pio_pio_h
pico_wiznet_spi_pio_wiznet_spi_pio_pio_h: port/pio/wiznet_spi_pio.pio.h
pico_wiznet_spi_pio_wiznet_spi_pio_pio_h: port/pio/CMakeFiles/pico_wiznet_spi_pio_wiznet_spi_pio_pio_h.dir/build.make
.PHONY : pico_wiznet_spi_pio_wiznet_spi_pio_pio_h

# Rule to build all files generated by this target.
port/pio/CMakeFiles/pico_wiznet_spi_pio_wiznet_spi_pio_pio_h.dir/build: pico_wiznet_spi_pio_wiznet_spi_pio_pio_h
.PHONY : port/pio/CMakeFiles/pico_wiznet_spi_pio_wiznet_spi_pio_pio_h.dir/build

port/pio/CMakeFiles/pico_wiznet_spi_pio_wiznet_spi_pio_pio_h.dir/clean:
	cd /home/ssekim/work-pico/pico-e/pico-e-cli/build/port/pio && $(CMAKE_COMMAND) -P CMakeFiles/pico_wiznet_spi_pio_wiznet_spi_pio_pio_h.dir/cmake_clean.cmake
.PHONY : port/pio/CMakeFiles/pico_wiznet_spi_pio_wiznet_spi_pio_pio_h.dir/clean

port/pio/CMakeFiles/pico_wiznet_spi_pio_wiznet_spi_pio_pio_h.dir/depend:
	cd /home/ssekim/work-pico/pico-e/pico-e-cli/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ssekim/work-pico/pico-e/pico-e-cli /home/ssekim/work-pico/pico-e/pico-e-cli/port/pio /home/ssekim/work-pico/pico-e/pico-e-cli/build /home/ssekim/work-pico/pico-e/pico-e-cli/build/port/pio /home/ssekim/work-pico/pico-e/pico-e-cli/build/port/pio/CMakeFiles/pico_wiznet_spi_pio_wiznet_spi_pio_pio_h.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : port/pio/CMakeFiles/pico_wiznet_spi_pio_wiznet_spi_pio_pio_h.dir/depend

