ARCH ?= x86
CMAKE ?= cmake

BUILD_DIR := build/$(ARCH)
OUT_DIR := out/$(ARCH)

ifeq ($(filter $(ARCH),x86 arm),)
$(error ARCH must be x86 or arm)
endif

.PHONY: all configure build

all: build

configure:
	$(CMAKE) -S . -B $(BUILD_DIR) -DARCH=$(ARCH) -DCPP_SDK_OUTPUT_DIR=$(abspath $(OUT_DIR))

build: configure
	$(CMAKE) --build $(BUILD_DIR)

clean:
	rm -rf out build
