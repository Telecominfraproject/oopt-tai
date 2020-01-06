PROG := libtai.so

TAI_DIR := ../../../../
TAI_FRAMEWORK_DIR := $(TAI_DIR)/tools/framework
TAI_LIB_DIR := $(TAI_DIR)/tools/lib

SRCS := basic.cpp $(wildcard $(TAI_LIB_DIR)/*.cpp) $(wildcard $(TAI_FRAMEWORK_DIR)/*.cpp)
HEADERS := basic.hpp $(wildcard $(TAI_LIB_DIR)/*.hpp) $(wildcard $(TAI_FRAMEWORK_DIR)/*.hpp)

INCLUDE := -I $(TAI_DIR)/inc -I $(TAI_DIR)/meta -I $(TAI_LIB_DIR) -I $(TAI_FRAMEWORK_DIR) -include basic.hpp
# -fno-gnu-unique
# tai::Logger::get_instance() is an inline method of Logger and it returns a static Logger.
# when the library is used under tai-mux which uses dlopen(), we want to make the logger per library not globally unique
# hence we need -fno-gnu-unique option
# ref) https://stackoverflow.com/questions/38510621/destructor-of-a-global-static-variable-in-a-shared-library-is-not-called-on-dlcl
#
# Alternative solution is to make tai::Logger::get_instance() non-inline ( implement it in a cpp file )
CFLAGS := -std=c++17 -g3 -shared -fPIC -DTAI_EXPOSE_PLATFORM -fno-gnu-unique

LIBS := -L $(TAI_DIR)/meta -lmetatai -lpthread

${PROG}: ${SRCS} ${HEADERS} Makefile
	$(CXX) ${CFLAGS} -o $@ ${INCLUDE} ${SRCS} ${LIBS}

clean:
	$(RM) ${PROG}
