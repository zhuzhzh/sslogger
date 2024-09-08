EXEC = exec

OBJ = test_bm
LEVEL = 5

LOGGER_HEADER = ../include/vhlogger/vhlogger.h
LOGGER_SRCS = ../src/vhlogger.cc
SRCS = benchmarks.cc ${LOGGER_SRCS}

ZMQ_OPTS = -I${ZEROMQ_HOME}/include -L${ZEROMQ_HOME}/lib64 -lzmq 
TOML11_OPTS = -I${TOML11_HOME}/include -L${TOML11_HOME}/lib64 -ltoml11 -DTOML11_COMPILE_SOURCES
#TOML11_OPTS = -I/home/harriszh/install/toml11/single_include
FMT_OPTS = -I${FMT_HOME}/include -L${FMT_HOME}/lib64 -lfmt
FMTLOG_OPTS = -I${FMTLOG_HOME}/include -L${FMTLOG_HOME}/lib64 -lfmtlog-shared
SPDLOG_OPTS = -I${SPDLOG_HOME}/include -L${SPDLOG_HOME}/lib64 -lspdlog
OPTIONS = -pthread -I../include -std=c++14 -DVHLOGGER_COMPILE_LEVEL=${LEVEL}

build b : $(OBJ)


$(OBJ): ${SRCS} ${LOGGER_HEADER}
	g++ -g ${SRCS} \
		${FMT_OPTS} \
		${OPTIONS} \
		-o $@

test:
	echo $@

run r: ${OBJ}
	./${OBJ}

clean:
	rm -rf $(VE) ${HE}


.PHONY: build run r clean test

