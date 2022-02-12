.PHONY: test

INCLUDE ?= -I ../inc

CFLAGS ?= -fPIC $(INCLUDE) -Wall -Werror

ifdef DEBUG
    CFLAGS += -g3 -O0
else
    CFLAGS += -O2
endif

LDFLAGS ?= -shared

all: taimetadata.c taimetadata.h taiserialize.c taimetadatautils.c
	$(CC) $(LDFLAGS) $(CFLAGS) taiserialize.c taimetadatautils.c taimetadata.c cJSON.c -o libmetatai.so

taimetadata.c taimetadata.h: $(TAI_META_CUSTOM_FILES) $(wildcard ../inc/*)
	tai_meta_generator ../inc/tai.h $(TAI_META_CUSTOM_FILES)

test:
	make -C sample
	make -C test
	LD_LIBRARY_PATH=. ./test/test

clean:
	rm libmetatai.so taimetadata.c taimetadata.h
