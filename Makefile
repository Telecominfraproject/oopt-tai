.PHONY: doc stub test docker meta

TAI_DOCKER_CMD ?= bash
TAI_DOCKER_IMAGE ?= tai
TAI_DOCKER_BUILDER_IMAGE ?= tai-builder
TAI_DOCKER_CMD_IMAGE ?= $(TAI_DOCKER_BUILDER_IMAGE)
TAI_DOCKER_MOUNT ?= "$(PWD):/data"
TAI_DOCKER_WORKDIR ?= "/data"
TAI_DOCKER_RUN_OPTION ?= -it --rm

all: meta stub

meta:
	$(MAKE) -C ./meta

doc:
	doxygen Doxyfile

stub:
	$(MAKE) -C ./stub

test:
	TAI_TEST_TARGET=$(abspath ./stub/libtai.so) $(MAKE) -C ./tests c
	$(MAKE) -C ./tests

cmd:
	docker run $(TAI_DOCKER_RUN_OPTION) -v $(TAI_DOCKER_MOUNT) -w $(TAI_DOCKER_WORKDIR) $(TAI_DOCKER_CMD_IMAGE) $(TAI_DOCKER_CMD)

image:
	DOCKER_BUILDKIT=1 docker build $(TAI_DOCKER_BUILD_OPTION) --build-arg TAI_DOCKER_BUILDER_IMAGE=$(TAI_DOCKER_BUILDER_IMAGE) \
								  -f docker/run.Dockerfile \
								  -t $(TAI_DOCKER_IMAGE) .

builder:
	DOCKER_BUILDKIT=1 docker build $(TAI_DOCKER_BUILD_OPTION) -f docker/builder.Dockerfile \
				       -t $(TAI_DOCKER_BUILDER_IMAGE) .

bash:
	$(MAKE) cmd

ci:
	TAI_DOCKER_RUN_OPTION='--rm' TAI_DOCKER_CMD="make test" $(MAKE) cmd

clean:
	$(MAKE) -C ./meta clean
	$(MAKE) -C ./stub clean
