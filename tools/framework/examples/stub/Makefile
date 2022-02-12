PROG := libtai.so

TAI_DIR := ../../../../
TAI_FRAMEWORK_DIR := $(TAI_DIR)/tools/framework
TAI_LIB_DIR := $(TAI_DIR)/tools/lib

SRCS := stub.cpp $(wildcard $(TAI_LIB_DIR)/*.cpp $(TAI_FRAMEWORK_DIR)/*.cpp)
HEADERS := stub.hpp $(wildcard $(TAI_LIB_DIR)/*.hpp $(TAI_FRAMEWORK_DIR)/*.hpp)
OBJS = $(addsuffix .o,$(basename $(SRCS)))

INCLUDE := -I $(TAI_DIR)/inc -I $(TAI_DIR)/meta -I $(TAI_LIB_DIR) -I $(TAI_FRAMEWORK_DIR)

CFLAGS ?= -DTAI_EXPOSE_PLATFORM -fPIC $(INCLUDE) -Wall -Werror
# -fno-gnu-unique
# tai::Logger::get_instance() is an inline method of Logger and it returns a static Logger.
# when the library is used under tai-mux which uses dlopen(), we want to make the logger per library not globally unique
# hence we need -fno-gnu-unique option
# ref) https://stackoverflow.com/questions/38510621/destructor-of-a-global-static-variable-in-a-shared-library-is-not-called-on-dlcl
#
# Alternative solution is to make tai::Logger::get_instance() non-inline ( implement it in a cpp file )
CFLAGS += -fno-gnu-unique

ifdef DEBUG
    CFLAGS += -g3 -O0
else
    CFLAGS += -O2
endif

CXXFLAGS ?= $(CFLAGS) -std=c++17 -include stub.hpp

LDFLAGS ?= -shared -L $(TAI_DIR)/meta -lmetatai -lpthread -static-libstdc++

$(PROG): $(OBJS) $(HEADERS) Makefile
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

clean:
	$(RM) ${PROG}
