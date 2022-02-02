# Makefile for src

# Platform-specific defines
ifeq ($(OS),Windows_NT)
	# Windows
	LDFLAGS = -s -mwindows
	TARGET_EXT = .exe
else
	LDFLAGS =
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		# Linux
		TARGET_EXT = .app
	endif
	ifeq ($(UNAME_S),Darwin)
		# Mac OS X
		TARGET_EXT = .app
	endif
endif

TARGET = src
TARGET_NAME = earth-webserver
TARGET_FILE = $(BINARY_PATH)/$(TARGET_NAME)$(TARGET_EXT)

SRC_DIRS = .
SRC_FILES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))

INCLUDE = -I../deps/libsaim/include -I../deps/libsaim/deps -I../deps/libsaim/src/rasterizer
DEFINES =

CFLAGS := -std=c99
CFLAGS += -Wall -O3
CFLAGS += $(INCLUDE)
CFLAGS += $(DEFINES)

LDLIBS = -lmicrohttpd -lsaim -lcurl -ljpeg -lpng -lz

all: $(TARGET)

$(TARGET):
	@$(CC) $(SRC_FILES) -o $(TARGET_FILE) $(CFLAGS) $(LDFLAGS) $(LDLIBS)