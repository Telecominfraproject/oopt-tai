.PHONY: proto

CFLAGS := -std=c++17 -g3 -fPIC
LIB := `pkg-config --libs protobuf grpc++ grpc` -L ../../meta/ -lmetatai -L . -ltai
INCLUDE := -I ../../meta -I ../../inc -I ./include -I ./lib

LIB_OBJS = lib/taish.grpc.pb.o lib/taish.pb.o lib/server.o
OBJS = server/main.o $(LIB_OBJS)

PROG := taish_server
LIB_PROG := libtaigrpc.so

${PROG}: proto $(OBJS)
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $(OBJS) $(LIB) -lgrpc++_reflection -lpthread

lib: proto $(LIB_OBJS)
	$(CXX) -shared $(CFLAGS) $(INCLUDE) -o $(LIB_PROG) $(LIB_OBJS) $(LIB) -lgrpc++_reflection -lpthread

.cc.o:
	$(CXX) $(CFLAGS) $(INCLUDE) -c -o $@ $<

.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDE) -c -o $@ $<

proto: lib/taish.grpc.pb.cc lib/taish.grpc.pb.h lib/taish.pb.cc lib/taish.pb.h

lib/taish.pb.cc lib/taish.pb.h: proto/taish.proto
	protoc --cpp_out=./lib -I proto taish.proto

lib/taish.grpc.pb.cc lib/taish.grpc.pb.h: proto/taish.proto
	protoc --grpc_out=./lib --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` -I proto taish.proto

clean:
	-rm -f ${OBJS} */*.pb\.* ${PROG} ${LIB_PROG}
