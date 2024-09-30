PREFIX = /home/public/vgp
SRC = ./src/vhlogger.cc 
FMT_OPTS = -I${FMT_HOME}/include -L${FMT_HOME}/lib64 -Wl,-Bstatic -lfmt -Wl,-Bdynamic 
SPDLOG_OPTS = -I${SPDLOG_HOME}/include -L${SPDLOG_HOME}/lib64 -Wl,-Bstatic -lspdlog -Wl,-Bdynamic -DSPDLOG_FMT_EXTERNAL -DSPDLOG_COMPILED_LIB
HEADER = ./include/vhlogger/vhlogger.h
OPTIONAL_HEADER = ./include/tl/optional.hpp
STRING_VIEW_HEADER = ./include/nonstd/string_view.hpp
OPTIONS = -g

default install:
	mkdir -p $(PREFIX)/lib64 && g++ $(OPTIONS) -shared -fPIC -o $(PREFIX)/lib64/libvhlogger.so ${SRC} -I./include/ ${FMT_OPTS} ${SPDLOG_OPTS}
	mkdir -p ${PREFIX}/include/vhlogger && cp ${HEADER} ${PREFIX}/include/vhlogger
	mkdir -p ${PREFIX}/include/tl && cp ${OPTIONAL_HEADER} ${PREFIX}/include/tl
	mkdir -p ${PREFIX}/include/nonstd && cp ${STRING_VIEW_HEADER} ${PREFIX}/include/nonstd
