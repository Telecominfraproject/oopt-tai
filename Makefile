.PHONY: doc stub test docker meta

ifndef TAI_DOCKER_CMD
    TAI_DOCKER_CMD=bash
endif

ifndef TAI_DOCKER_IMAGE
    TAI_DOCKER_IMAGE=tai
endif

all: meta stub

meta:
	$(MAKE) -C ./meta

doc:
	doxygen Doxyfile

stub:
	$(MAKE) -C ./stub

test: stub
	$(MAKE) -C ./test
	LD_LIBRARY_PATH=./stub:./meta ./test/test

cmd:
	docker run --net=host -it --rm -v `pwd`:/data -w /data $(TAI_DOCKER_IMAGE) $(TAI_DOCKER_CMD)

docker:
	TAI_DOCKER_CMD='make' $(MAKE) cmd

docker-image:
	docker build -t $(TAI_DOCKER_IMAGE) .

bash:
	$(MAKE) cmd

clean:
	$(MAKE) -C ./meta clean
	$(MAKE) -C ./stub clean
