.PHONY: proto

CC := g++
CFLAGS := -std=c++11 -g3 -fPIC
LIB := `pkg-config --libs protobuf grpc++ grpc` -L ../../meta/ -lmetatai -L . -ltai
INCLUDE := -I ../../meta -I ../../inc -I ./include -I ./lib

LIB_OBJS = lib/tai.grpc.pb.o lib/tai.pb.o lib/server.o
OBJS = server/main.o $(LIB_OBJS)

PROG := taish_server
LIB_PROG := libtaigrpc.so

${PROG}: proto $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $(OBJS) $(LIB) -lgrpc++_reflection -lpthread

lib: proto $(LIB_OBJS)
	$(CC) -shared $(CFLAGS) $(INCLUDE) -o $(LIB_PROG) $(LIB_OBJS) $(LIB) -lgrpc++_reflection -lpthread

.cc.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

proto: proto/tai.proto
	protoc --cpp_out=./lib -I proto tai.proto
	protoc --grpc_out=./lib --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` -I proto tai.proto

docker:
	# realpath doesn't work on travis use cd and pwd instead
	cd ../../ && docker run -u `id -u`:`id -g` -w /data/tools/taish --rm -v `pwd`:/data -it libtai-grpc-builder make

docker-image:
	docker build -t libtai-grpc-builder .

bash:
	# realpath doesn't work on travis use cd and pwd instead
	cd ../../ && docker run --net=host -it -v `pwd`:/data -w /data/tools/taish libtai-grpc-builder bash

clean:
	-rm -f ${OBJS} */*.pb\.* ${PROG} ${LIB_PROG}
