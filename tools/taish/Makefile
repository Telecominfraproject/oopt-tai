CC := g++
CFLAGS := -g -std=c++11
INCLUDES := -I ../../inc -I ../../meta
LDFLAGS :=  -ldl -pthread -L ../../meta -lmetatai
SRCDIR := .
SRCEXT := cpp
SOURCES := $(shell ls $(SRCDIR)/*.cpp)

TARGET := taish

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(INCLUDES) $(SOURCES) $(LDFLAGS) 
clean:
	rm $(TARGET)
