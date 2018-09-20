CC := g++
CFLAGS := -g -std=c++11
CFLAGS_API := -shared -DTAISH_API_MODE -fPIC
INCLUDES := -I ../../inc
LDFLAGS :=  -ldl -pthread
SRCDIR := .
SRCEXT := cpp
SOURCES := $(shell ls $(SRCDIR)/*.cpp)
HEADERS := $(shell ls $(SRCDIR)/*.hpp) $(shell ls $(SRCDIR)/*.h)

TARGET := taish
TARGET_API := libtaishapi.so

all: $(TARGET) $(TARGET_API)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(INCLUDES) $(SOURCES) $(LDFLAGS) 

$(TARGET_API): $(SOURCES)
	$(CC) $(CFLAGS) $(CFLAGS_API) -o $(TARGET_API) $(INCLUDES) $(SOURCES) $(LDFLAGS) 
clean:
	rm $(TARGET) $(TARGET_API)
