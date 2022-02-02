# Makefile for deps

TARGET = deps

# Main routine
SUBDIRS_CUSTOM = saim

all: $(TARGET)

.PHONY: clean
clean:
	@$(foreach directory, $(SUBDIRS_CUSTOM), $(MAKE) -C $(directory) clean ;)

.PHONY: help
help:
	@echo available targets: all clean

$(TARGET): $(SUBDIRS_CUSTOM)

# Declare rules for custom subdirs
.PHONY: saim
saim:
	@$(MAKE) -C libsaim
