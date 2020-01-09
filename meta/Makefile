.PHONY: test

all: taimetadata.c taimetadata.h taiserialize.c taimetadatautils.c
	gcc -g3 -Werror -Wall -shared -fPIC -I ../inc taiserialize.c taimetadatautils.c taimetadata.c cJSON.c -o libmetatai.so

taimetadata.c taimetadata.h: main.py $(TAI_META_CUSTOM_FILES) $(wildcard ../inc/*)
	python main.py ../inc/tai.h $(TAI_META_CUSTOM_FILES)

test:
	make -C sample
	make -C test
	LD_LIBRARY_PATH=. ./test/test

clean:
	rm libmetatai.so taimetadata.c taimetadata.h
