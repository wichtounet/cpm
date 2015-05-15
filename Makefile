default: release

.PHONY: default release debug all clean

include make-utils/flags.mk
include make-utils/cpp-utils.mk

CXX_FLAGS += -Ilib/rapidjson/include

# Make sure warnings are not ignored
CXX_FLAGS += -Werror

# Use the correct stdlib
ifneq (,$(findstring clang,$(CXX)))
CXX_FLAGS += -stdlib=libc++
endif

$(eval $(call auto_folder_compile,src))
$(eval $(call add_src_executable,cpm,cpm.cpp))
$(eval $(call add_src_executable,sample,sample.cpp))

release: release/bin/cpm release/bin/sample
release_debug: release_debug/bin/cpm release_debug/bin/sample
debug: debug/bin/cpm debug/bin/sample

all: release release_debug debug

clean: base_clean

include make-utils/cpp-utils-finalize.mk
