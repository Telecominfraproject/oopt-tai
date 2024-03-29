.PHONY: proto

TAI_DIR := ../../
TAI_META_DIR := $(TAI_DIR)/meta
TAI_LIB_DIR := $(TAI_DIR)/tools/lib
TAI_FRAMEWORK_DIR := $(TAI_DIR)/tools/framework
TAI_BASIC_LIB_DIR := $(TAI_FRAMEWORK_DIR)/examples/basic/

CFLAGS ?= -std=c++17 -fPIC -Wall -Werror

ifdef DEBUG
    CFLAGS += -g3 -O0
else
    CFLAGS += -O2
endif

LDFLAGS ?= -static-libstdc++ `pkg-config --libs protobuf grpc++ grpc` -L. -ltai -L ../../meta/ -Wl,'--no-as-needed' -lmetatai -lgrpc++_reflection -lpthread
INCLUDE ?= -I $(TAI_META_DIR) -I $(TAI_DIR)/inc -I ./include -I ./lib -I $(TAI_LIB_DIR)

LIB_GRPC_SRCS := lib/taish.grpc.pb.cc lib/taish.pb.cc
LIB_OBJS = lib/server.o $(TAI_LIB_DIR)/attribute.o $(LIB_GRPC_SRCS:%.cc=%.o)
SERVER_SRCS := server/main.cpp
SERVER_OBJS := $(SERVER_SRCS:%.cpp=%.o)

OBJS = $(LIB_OBJS) $(SERVER_OBJS)

PROG := taish_server
LIB_PROG := libtaigrpc.so

$(PROG): proto $(TAI_META_DIR)/taimetadata.h $(TAI_META_DIR)/libmetatai.so $(OBJS) libtai.so
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $(OBJS) $(LDFLAGS)

$(TAI_META_DIR)/taimetadata.h $(TAI_META_DIR)/libmetatai.so:
	$(MAKE) -C $(TAI_META_DIR)

libtai.so: Makefile
	$(MAKE) -C $(TAI_BASIC_LIB_DIR) clean
	$(MAKE) -C $(TAI_BASIC_LIB_DIR)
	ln -sf $(TAI_BASIC_LIB_DIR)/$@ $@

lib: proto $(LIB_OBJS) Makefile
	$(CXX) -shared $(CFLAGS) $(INCLUDE) -o $(LIB_PROG) $(LIB_OBJS) $(LDFLAGS)

.cc.o: Makefile
	mkdir -p $(@D)
	$(CXX) $(INCLUDE) $(CFLAGS) -c -o $@ $<

.cpp.o: Makefile
	mkdir -p $(@D)
	$(CXX) $(CFLAGS) $(INCLUDE) -c -o $@ $<

proto: lib/taish.grpc.pb.cc lib/taish.grpc.pb.h lib/taish.pb.cc lib/taish.pb.h

lib/taish.pb.cc lib/taish.pb.h: proto/taish/taish.proto
	protoc --cpp_out=./lib -I proto/taish taish.proto

lib/taish.grpc.pb.cc lib/taish.grpc.pb.h: proto/taish/taish.proto
	protoc --grpc_out=./lib --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` -I proto/taish taish.proto

python:
	python -m pip install wheel grpcio-tools
	$(MAKE) -C client
	python setup.py bdist_wheel
	python -m pip wheel -r requirements.txt -w dist
	pip install dist/*.whl

clean:
	-rm -f ${OBJS} */*.pb\.* ${PROG} ${LIB_PROG}
