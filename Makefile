PREFIX = /home/public/ssln
SRC = ./src/sslogger.cc 
#FMT_OPTS = -I${FMT_HOME}/include -L${FMT_HOME}/lib64 -Wl,-Bstatic -lfmt -Wl,-Bdynamic 
FMT_OPTS = -I${FMT_HOME}/include -L${FMT_HOME}/lib64 -lfmt
#SPDLOG_OPTS = -I${SPDLOG_HOME}/include -L${SPDLOG_HOME}/lib64 -Wl,-Bstatic -lspdlog -Wl,-Bdynamic -DSPDLOG_FMT_EXTERNAL -DSPDLOG_COMPILED_LIB
SPDLOG_OPTS = -I${SPDLOG_HOME}/include -L${SPDLOG_HOME}/lib64 -lspdlog -DSPDLOG_FMT_EXTERNAL -DSPDLOG_COMPILED_LIB
HEADER = ./include/ssln/sslogger.h
OPTIONAL_HEADER = ./include/tl/optional.hpp
STRING_VIEW_HEADER = ./include/nonstd/string_view.hpp
OPTIONS = -g

default install:
	mkdir -p $(PREFIX)/lib64 && g++ $(OPTIONS) -shared -fPIC -o $(PREFIX)/lib64/libsslogger.so ${SRC} -I./include/ ${FMT_OPTS} ${SPDLOG_OPTS}
	mkdir -p ${PREFIX}/include/ssln && cp ${HEADER} ${PREFIX}/include/ssln
	mkdir -p ${PREFIX}/include/tl && cp ${OPTIONAL_HEADER} ${PREFIX}/include/tl
	mkdir -p ${PREFIX}/include/nonstd && cp ${STRING_VIEW_HEADER} ${PREFIX}/include/nonstd
