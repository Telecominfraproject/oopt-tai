CC := g++
CFLAGS := -g -std=c++11
INCLUDES := -I ../../inc
LDFLAGS :=  -ldl -pthread
SRCDIR := .
SRCEXT := cpp
SOURCES := $(shell ls $(SRCDIR)/*.cpp)

TARGET := taish

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(INCLUDES) $(SOURCES) $(LDFLAGS) 
clean:
	rm $(TARGET)
