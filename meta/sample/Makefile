TAI_META_OUT_DIR ?= $(abspath .)/

all:
	TAI_META_CUSTOM_FILES=$(abspath ./custom.h) TAI_META_OUT_DIR=$(TAI_META_OUT_DIR) $(MAKE) -C ..

clean:
	TAI_META_OUT_DIR=$(TAI_META_OUT_DIR) $(MAKE) -C .. $@
