all:
	python -m grpc_tools.protoc -I../proto --python_out=. --grpc_python_out=. ../proto/taish.proto

clean:
	$(RM) -r __pycache__ *pb2.py *pb2_grpc.py
