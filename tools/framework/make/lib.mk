TAI_PROG ?= libtai.so

ifndef TAI_DIR
$(error $TAI_DIR not defined)
endif

ifndef TAI_FRAMEWORK_PLATFORM_HEADER
$(error $TAI_FRAMEWORK_PLATFORM_HEADER not defined)
endif

TAI_FRAMEWORK_DIR ?= $(TAI_DIR)/tools/framework
TAI_LIB_DIR ?= $(TAI_DIR)/tools/lib

TAI_META_OUT_DIR ?= $(abspath .)

SRCS ?= $(wildcard *.cpp $(TAI_LIB_DIR)/*.cpp $(TAI_FRAMEWORK_DIR)/*.cpp $(TAI_META_OUT_DIR)/taimetadata.c)
HEADERS ?= $(wildcard *.hpp $(TAI_LIB_DIR)/*.hpp $(TAI_FRAMEWORK_DIR)/*.hpp $(TAI_META_OUT_DIR)/*.h) $(TAI_META_CUSTOM_FILES)
OBJS = $(addsuffix .o,$(basename $(SRCS)))

INCLUDE ?= $(VENDOR_INCLUDE) -I $(TAI_META_OUT_DIR) -I $(TAI_DIR)/inc -I $(TAI_DIR)/meta -I $(TAI_LIB_DIR) -I $(TAI_FRAMEWORK_DIR)

CFLAGS ?= $(VENDOR_CFLAGS) -DTAI_EXPOSE_PLATFORM -fPIC $(INCLUDE) -Wall -Werror
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

CXXFLAGS ?= $(VENDOR_CXXFLAGS) $(CFLAGS) -std=c++17 -include $(TAI_FRAMEWORK_PLATFORM_HEADER)

LDFLAGS ?= $(VENDOR_LDFLAGS) -shared -L $(TAI_META_OUT_DIR) -lmetatai -lpthread

$(TAI_PROG): meta $(OBJS) $(HEADERS) Makefile
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

meta:
	$(MAKE) -C $(TAI_DIR)/meta TAI_META_CUSTOM_FILES="$(TAI_META_CUSTOM_FILES)" TAI_META_OUT_DIR=$(TAI_META_OUT_DIR)

clean:
	$(RM) $(TAI_PROG) $(OBJS)
	$(MAKE) -C $(TAI_DIR)/meta clean TAI_META_OUT_DIR=$(TAI_META_OUT_DIR)
