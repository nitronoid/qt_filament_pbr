LIBSTDCPP_INCLUDE_PATH = /usr/include/c++/v1
LIBSTDCPP_LIB_PATH = /usr/include/c++/v1

QMAKE_CXXFLAGS += -nostdinc++ -nodefaultlibs -isystem $${LIBSTDCPP_INCLUDE_PATH}

LIBS += -lc -lc++ -lc++abi 
