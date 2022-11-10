# Makefile for src

# Platform-specific defines
LINUX_LIBS =
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
		LINUX_LIBS = -lpthread -lm -ldl
	endif
	ifeq ($(UNAME_S),Darwin)
		# Mac OS X
		TARGET_EXT = .app
	endif
endif

TARGET = src
TARGET_NAME = earth-tileserver
TARGET_FILE = $(BINARY_PATH)/$(TARGET_NAME)$(TARGET_EXT)

SRC_DIRS = .
SRC_FILES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))

INCLUDE = -I../deps/libsaim/include -I../deps/libsaim/deps -I../deps/libsaim/src/rasterizer
DEFINES =

CFLAGS := -std=c99
CFLAGS += -Wall -O3
CFLAGS += $(INCLUDE)
CFLAGS += $(DEFINES)

LDLIBS = -lmicrohttpd -lsaim -lcurl -ljpeg -lpng -lz $(LINUX_LIBS)

ifeq ($(OS),Windows_NT)
	CLEAN = rmdir /Q /S $(BINARY_PATH)
else
	CLEAN = rm -r $(BINARY_PATH)
endif

all: $(TARGET)

$(TARGET):
	@$(CC) $(SRC_FILES) -o $(TARGET_FILE) $(CFLAGS) $(LDFLAGS) $(LDLIBS)

.PHONY: clean
clean:
	@$(CLEAN)
