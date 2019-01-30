LIB := -L ../meta/ -lmetatai
INCLUDE := -I ../meta -I ../inc

all:
	gcc -shared -fPIC $(INCLUDE) stub_tai.c -o libtai.so $(LIB)

clean:
	rm libtai.so
