default: release

.PHONY: default release debug all clean

include make-utils/flags.mk
include make-utils/cpp-utils.mk

CXX_FLAGS += -Ilib/rapidjson/include -Ilib/cxxopts/src/

# Make sure warnings are not ignored
CXX_FLAGS += -Werror -pedantic

# Disable documentation warnings for dependencies
ifneq (,$(findstring clang,$(CXX)))
CXX_FLAGS += -Wno-documentation
endif

# Use the correct stdlib
ifneq (,$(findstring clang,$(CXX)))
CXX_FLAGS += -stdlib=libc++
endif

# Generate the default executable
$(eval $(call auto_folder_compile,src))
$(eval $(call auto_add_executable,cpm))

$(eval $(call folder_compile,examples))
$(eval $(call add_executable,simple,examples/simple.cpp))
$(eval $(call add_executable,full,examples/full.cpp))

PREFIX = $(DESTDIR)/usr/local
BINDIR = $(PREFIX)/bin
INCDIR = $(PREFIX)/include

release: release/bin/cpm
release_debug: release_debug/bin/cpm
debug: debug/bin/cpm

install: release
	install -D release/bin/cpm $(BINDIR)/cpm
	install -D -m 0644 include/cpm/*.hpp $(INCDIR)/cpm

install-strip: release
	install -D -s release/bin/cpm $(BINDIR)/cpm
	install -d -m 0644 include/cpm $(INCDIR)/cpm

examples: debug/bin/simple debug/bin/full

release_examples: release_debug/bin/simple release_debug/bin/full

all: release release_debug debug

clean: base_clean

include make-utils/cpp-utils-finalize.mk
