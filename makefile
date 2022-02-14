# Makefile for Earth tile server

# Platform-specific defines
STATIC_LIB_EXT = .a
AR = ar rcs
ifeq ($(OS),Windows_NT)
	#CCFLAGS += -D WIN32
	MAKE := mingw32-make.exe
	LDFLAGS = -s -shared
	LIBRARY_PATH := $(shell cd)\\lib
	BINARY_PATH := $(shell cd)\\bin
	CC := gcc
	CXX := g++
	SHARED_LIB_EXT = .so
else
	MAKE := make
	LDFLAGS = -shared -fPIC
	LIBRARY_PATH := $(shell pwd)/lib
	BINARY_PATH := $(shell pwd)/bin
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		#CCFLAGS += -D LINUX
		CC := gcc
		CXX := g++
		SHARED_LIB_EXT = .so
	endif
	ifeq ($(UNAME_S),Darwin)
		#CCFLAGS += -D OSX
		CC := clang
		CXX := clang++
		SHARED_LIB_EXT = .dylib
		# OSX has its own CURL with command line tools
		CURL_LIB := curl
	endif
endif

# Exports
export STATIC_LIB_EXT
export SHARED_LIB_EXT
export CC
export CXX
export AR
export LDFLAGS
export LIBRARY_PATH
export BINARY_PATH

HELP_FILE = help.html
LIBRARY_DIRS = deps
BINARY_DIRS = src
DIRS_ORDER = \
	create_libs_dir $(LIBRARY_DIRS) \
	create_bins_dir $(BINARY_DIRS) \
	copy_help_file

ifeq ($(OS),Windows_NT)
	CREATE_LIBS_DIR = if not exist $(LIBRARY_PATH) mkdir $(LIBRARY_PATH)
	CREATE_BINS_DIR = if not exist $(BINARY_PATH) mkdir $(BINARY_PATH)
	COPY_HELP_FILE = if exist $(HELP_FILE) cp /Y $(HELP_FILE) $(BINARY_PATH)
else
	CREATE_LIBS_DIR = test -d $(LIBRARY_PATH) || mkdir -p $(LIBRARY_PATH)
	CREATE_BINS_DIR = test -d $(BINARY_PATH) || mkdir -p $(BINARY_PATH)
	COPY_HELP_FILE = test -f $(HELP_FILE) && cp $(HELP_FILE) $(BINARY_PATH)
endif

all: $(DIRS_ORDER)

.PHONY: clean
clean:
	@$(foreach directory, $(LIBRARY_DIRS) $(BINARY_DIRS), $(MAKE) -C $(directory) clean ;)

.PHONY: help
help:
	@echo available targets: all clean

$(LIBRARY_DIRS):
	@$(MAKE) -C $@ $@

$(BINARY_DIRS):
	@$(MAKE) -C $@ $@

create_libs_dir:
	@$(CREATE_LIBS_DIR)

create_bins_dir:
	@$(CREATE_BINS_DIR)

copy_help_file:
	@$(COPY_HELP_FILE)

.PHONY: $(DIRS_ORDER)
