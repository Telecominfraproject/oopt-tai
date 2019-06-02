.PHONY: test

all: taimetadata.c taimetadata.h taiserialize.c taimetadatautils.c
	gcc -shared -fPIC -I ../inc taiserialize.c taimetadatautils.c taimetadata.c cJSON.c -o libmetatai.so

taimetadata.c taimetadata.h: main.py
	python3 main.py ../inc/tai.h

docker-image:
	docker build --build-arg http_proxy=${http_proxy} --build-arg https_proxy=${https_proxy} -t taimeta-builder .

docker:
	cd ../; docker run -v `pwd`:/data -w /data -u `id -u`:`id -g` taimeta-builder make -C meta

test:
	make -C test
	LD_LIBRARY_PATH=. ./test/test

clean:
	rm libmetatai.so taimetadata.c taimetadata.h
