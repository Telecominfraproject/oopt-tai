ifndef TAI_TEST_TARGET
    TAI_TEST_TARGET :=  ../tools/framework/examples/basic/libtai.so
endif

ifndef TAI_TAISH_SERVER
    TAI_TAISH_SERVER := ../tools/taish/taish_server
endif

ifndef LD_LIBRARY_PATH
    LD_LIBRARY_PATH := ../meta:$(dir $(TAI_TEST_TARGET))
endif

.PHONY: all c python

all: c python

c: $(TAI_TEST_TARGET)
	gcc -I ../inc -I ../sai/inc -o test test.c -L $(dir $(TAI_TEST_TARGET)) -ltai -L ../meta -lmetatai
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) ./test

python: taish $(TAI_TAISH_SERVER) $(TAI_TEST_TARGET)
	PATH=$(dir $(TAI_TAISH_SERVER)):${PATH} LD_LIBRARY_PATH=../meta:$(dir $(TAI_TEST_TARGET)) python -m unittest test_base.py

taish:
	cd $(dir $(TAI_TAISH_SERVER)) && python setup.py install

$(TAI_TEST_TARGET):
	$(MAKE) -C $(@D)

$(TAI_TAISH_SERVER):
	$(MAKE) -C $(@D)

clean:
	rm test
