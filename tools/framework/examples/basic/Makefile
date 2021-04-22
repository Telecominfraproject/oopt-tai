PROG := libtai.so

TAI_DIR := ../../../../
TAI_FRAMEWORK_DIR := $(TAI_DIR)/tools/framework
TAI_LIB_DIR := $(TAI_DIR)/tools/lib

SRCS = basic.cpp $(wildcard $(TAI_LIB_DIR)/*.cpp) $(wildcard $(TAI_FRAMEWORK_DIR)/*.cpp) $(wildcard $(TAI_DIR)/meta/*.c)

BUILD_DIR ?= build

OBJS = $(addsuffix .o,$(basename $(SRCS)))

HEADERS := basic.hpp $(wildcard $(TAI_LIB_DIR)/*.hpp) $(wildcard $(TAI_FRAMEWORK_DIR)/*.hpp)

INCLUDE := -I $(TAI_DIR)/inc -I $(TAI_DIR)/meta -I $(TAI_LIB_DIR) -I $(TAI_FRAMEWORK_DIR)
# -fno-gnu-unique
# tai::Logger::get_instance() is an inline method of Logger and it returns a static Logger.
# when the library is used under tai-mux which uses dlopen(), we want to make the logger per library not globally unique
# hence we need -fno-gnu-unique option
# ref) https://stackoverflow.com/questions/38510621/destructor-of-a-global-static-variable-in-a-shared-library-is-not-called-on-dlcl
#
# Alternative solution is to make tai::Logger::get_instance() non-inline ( implement it in a cpp file )
LDFLAGS ?= -shared -fPIC -fno-gnu-unique

CFLAGS ?= -fPIC -fno-gnu-unique -DTAI_EXPOSE_PLATFORM $(INCLUDE) -Bsymbolic

CXXFLAGS ?= $(CFLAGS) -std=c++17 -include basic.hpp

LIBS := -lpthread


TAI_META_CUSTOM_FILES ?= $(abspath $(wildcard custom/* $(MUX_SRCDIR)/custom_attrs/*))

all: meta
	$(MAKE) $(PROG)

$(PROG): meta $(OBJS) $(HEADERS) Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

meta:
	TAI_META_CUSTOM_FILES="$(TAI_META_CUSTOM_FILES)" $(MAKE) -C $(TAI_DIR)/meta

clean:
	$(RM) $(PROG) $(OBJS)
