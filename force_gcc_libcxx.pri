QMAKE_CXXFLAGS += -nostdinc++ -nodefaultlibs -isystem ${LIBSTDCPP_INCLUDE_PATH}

_LIBSTDCPP_LIB_PATH = $$system(echo $LIBSTDCPP_LIB_PATH)
if (!isEmpty(_LIBSTDCPP_LIB_PATH)) {
  message("Using libc++ from $$_LIBSTDCPP_LIB_PATH")
  LIBS += -L${LIBSTDCPP_LIB_PATH}
} else {
  message("Searching /lib:/lib64:/usr/lib:/usr/lib64 for libc++")
}
LIBS += -lc -lc++ -lc++abi 
