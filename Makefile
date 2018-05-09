stub:
	$(MAKE) -C ./stub

test: stub
	$(MAKE) -C ./test
	LD_LIBRARY_PATH=./stub ./test/test

.PHONY: stub test
