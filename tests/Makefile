ifndef TAI_TEST_TARGET
    TAI_TEST_TARGET :=  ../tools/framework/examples/basic/libtai.so
endif

ifndef TAI_TEST_CASE
    TAI_TEST_CASE := test_base.py
endif

ifndef TAI_TAISH_SERVER
    TAI_TAISH_SERVER := ../tools/taish/taish_server
endif

ifndef LD_LIBRARY_PATH
    LD_LIBRARY_PATH := ../meta:$(dir $(TAI_TEST_TARGET))
endif

.PHONY: all c python

all: c python

../meta/libmetatai.so:
	$(MAKE) -C $(@D)

c: $(TAI_TEST_TARGET) ../meta/libmetatai.so
	gcc -I ../inc -I ../sai/inc -o test test.c -L $(dir $(TAI_TEST_TARGET)) -ltai -L ../meta -lmetatai
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) ./test

python: taish $(TAI_TAISH_SERVER) $(TAI_TEST_TARGET)
	TAI_TEST_MODULE_LOCATION=$(TAI_TEST_MODULE_LOCATION) PATH=$(dir $(TAI_TAISH_SERVER)):${PATH} LD_LIBRARY_PATH=../meta:$(dir $(TAI_TEST_TARGET)) python -m unittest $(TAI_TEST_CASE) -f -v

taish:
	which taish 2>&1 > /dev/null || (cd $(dir $(TAI_TAISH_SERVER)) && python setup.py install)

$(TAI_TEST_TARGET):
	$(MAKE) -C $(@D)

$(TAI_TAISH_SERVER):
	$(MAKE) -C $(@D)

clean:
	rm test
