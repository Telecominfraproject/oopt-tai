.PHONY: test

TAI_META_OUT_DIR ?= $(abspath .)

INCLUDE ?= -I $(TAI_META_OUT_DIR) -I ../inc

CFLAGS ?= -fPIC $(INCLUDE) -Wall -Werror

ifdef DEBUG
    CFLAGS += -ggdb -g3 -O0
else
    CFLAGS += -O2
endif

LDFLAGS ?= -shared

SRCS ?= $(TAI_META_OUT_DIR)/taimetadata.c $(filter-out taimetadata.c,$(wildcard *.c))
HEADERS ?= $(TAI_META_OUT_DIR)/taimetadata.h $(filter-out taimetadata.h,$(wildcard *.h))
OBJS = $(addsuffix .o,$(basename $(SRCS)))

$(TAI_META_OUT_DIR)/libmetatai.so: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(TAI_META_OUT_DIR)/taimetadata.c $(TAI_META_OUT_DIR)/taimetadata.h: $(TAI_META_CUSTOM_FILES) $(wildcard ../inc/*)
	tai_meta_generator --out-dir $(TAI_META_OUT_DIR) ../inc/tai.h $(TAI_META_CUSTOM_FILES)

test:
	make -C sample
	make -C test TAI_META_OUT_DIR=$(abspath sample)
	LD_LIBRARY_PATH=$(abspath sample) ./test/test

clean:
	$(RM) $(addprefix $(TAI_META_OUT_DIR)/,libmetatai.so taimetadata.c taimetadata.h) *.o
