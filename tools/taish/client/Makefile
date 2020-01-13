TAISH_DIR := ..

all: taish/taish_pb2.py taish/taish_pb2_grpc.py

taish/taish_pb2.py taish/taish_pb2_grpc.py: $(TAISH_DIR)/proto/taish/taish.proto
	python -m grpc_tools.protoc -I$(TAISH_DIR)/proto --python_out=. --python_grpc_out=. $(TAISH_DIR)/proto/taish/taish.proto

clean:
	$(RM) -r __pycache__ *pb2.py *pb2_grpc.py
