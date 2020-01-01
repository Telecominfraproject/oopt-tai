TAISH_DIR := ..

all:
	python -m grpc_tools.protoc -I$(TAISH_DIR)/proto --python_out=. --grpc_python_out=. $(TAISH_DIR)/proto/taish/taish.proto

clean:
	$(RM) -r __pycache__ *pb2.py *pb2_grpc.py
