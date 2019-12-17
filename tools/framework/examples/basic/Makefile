PROG := libtai.so

TAI_DIR := ../../../../
TAI_LIB_DIR := $(TAI_DIR)/tools/lib

SRCS := basic.cpp $(TAI_LIB_DIR)/tai.cpp $(TAI_LIB_DIR)/attribute.cpp

INCLUDE := -I $(TAI_DIR)/inc -I $(TAI_DIR)/meta -I $(TAI_LIB_DIR) -include basic.hpp
CFLAGS := -std=c++11 -g3 -shared -fPIC -DTAI_EXPOSE_PLATFORM -fno-inline

LIBS := -L $(TAI_DIR)/meta -lmetatai -lpthread

${PROG}: ${SRCS} Makefile
	g++ ${CFLAGS} -o $@ ${INCLUDE} ${SRCS} ${LIBS}

clean:
	$(RM) ${PROG}
