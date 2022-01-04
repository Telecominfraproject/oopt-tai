TOP_DIR ?= ../../../

run: test
	LD_LIBRARY_PATH=$(TOP_DIR)/meta ./test

test: $(TOP_DIR)/meta/libmetatai.so $(wildcard ../*.cpp ../*.hpp) test.cpp
	g++ -g3 -Wall -std=c++17 -o test test.cpp ../attribute.cpp -L $(TOP_DIR)/meta -I $(TOP_DIR)/inc -I $(TOP_DIR)/meta -I .. -lmetatai

$(TOP_DIR)/meta/libmetatai.so:
	$(MAKE) -C $(TOP_DIR)/meta

FORCE:
