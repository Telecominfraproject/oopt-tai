.PHONY: doc stub test docker meta

ifndef TAI_DOCKER_CMD
    TAI_DOCKER_CMD := bash
endif

ifndef TAI_DOCKER_IMAGE
    TAI_DOCKER_IMAGE := tai
endif

ifndef TAI_DOCKER_BUILDER_IMAGE
    TAI_DOCKER_BUILDER_IMAGE := tai-builder
endif

ifndef TAI_DOCKER_MOUNT
    TAI_DOCKER_MOUNT := "$(PWD):/data"
endif

ifndef TAI_DOCKER_WORKDIR
    TAI_DOCKER_WORKDIR := "/data"
endif

ifndef TAI_DOCKER_RUN_OPTION
    TAI_DOCKER_RUN_OPTION := -it --rm
endif

all: meta stub

meta:
	$(MAKE) -C ./meta

doc:
	doxygen Doxyfile

stub:
	$(MAKE) -C ./stub

test:
	$(MAKE) -C ./tests

cmd:
	docker run $(TAI_DOCKER_RUN_OPTION) -v $(TAI_DOCKER_MOUNT) -w $(TAI_DOCKER_WORKDIR) $(TAI_DOCKER_BUILDER_IMAGE) $(TAI_DOCKER_CMD)

image:
	DOCKER_BUILDKIT=1 docker build $(TAI_DOCKER_BUILD_OPTION) --build-arg TAI_DOCKER_BUILDER_IMAGE=$(TAI_DOCKER_BUILDER_IMAGE) \
								  -f docker/run.Dockerfile \
								  -t $(TAI_DOCKER_IMAGE) .

builder:
	DOCKER_BUILDKIT=1 docker build $(TAI_DOCKER_BUILD_OPTION) -f docker/builder.Dockerfile \
				       -t $(TAI_DOCKER_BUILDER_IMAGE) .

bash:
	$(MAKE) cmd

ci: builder
	TAI_DOCKER_RUN_OPTION='--rm' TAI_DOCKER_CMD="make test" $(MAKE) cmd

clean:
	$(MAKE) -C ./meta clean
	$(MAKE) -C ./stub clean
