QMAKE_CXXFLAGS += -nostdinc++ -nodefaultlibs -isystem ${LIBSTDCPP_INCLUDE_PATH}

LIBS += -L${LIBSTDCPP_LIB_PATH} -lc -lc++ -lc++abi 
