.PHONY: doc stub test docker meta

ifndef TAI_DOCKER_CMD
    TAI_DOCKER_CMD := bash
endif

ifndef TAI_DOCKER_IMAGE
    TAI_DOCKER_IMAGE := tai
endif

ifndef TAI_DOCKER_MOUNT
    TAI_DOCKER_MOUNT := "$(PWD):/data"
endif

ifndef TAI_DOCKER_WORKDIR
    TAI_DOCKER_WORKDIR := "/data"
endif

ifndef TAI_DOCKER_RUN_OPTION
    TAI_DOCKER_RUN_OPTION := --net=host -it --rm --privileged
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
	docker run $(TAI_DOCKER_RUN_OPTION) -v $(TAI_DOCKER_MOUNT) -w $(TAI_DOCKER_WORKDIR) $(TAI_DOCKER_IMAGE) $(TAI_DOCKER_CMD)

docker:
	TAI_DOCKER_CMD='make' $(MAKE) cmd

docker-image:
	docker build -t $(TAI_DOCKER_IMAGE) .

bash:
	$(MAKE) cmd

clean:
	$(MAKE) -C ./meta clean
	$(MAKE) -C ./stub clean
