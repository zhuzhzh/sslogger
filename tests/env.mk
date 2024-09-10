EXEC = exec

OBJ = env_tester
LEVEL = 5

LOGGER_HEADER = ../include/vhlogger/vhlogger.h
#LOGGER_SRCS = ../src/vhlogger.cc
LOGGER_SRCS = 
SRCS = test_env.cc 

ZMQ_OPTS = -I${ZEROMQ_HOME}/include -L${ZEROMQ_HOME}/lib64 -lzmq 
TOML11_OPTS = -I${TOML11_HOME}/include -L${TOML11_HOME}/lib64 -ltoml11 -DTOML11_COMPILE_SOURCES
#TOML11_OPTS = -I/home/harriszh/install/toml11/single_include
FMT_OPTS = -I${FMT_HOME}/include -L${FMT_HOME}/lib64 -lfmt
FMTLOG_OPTS = -I${FMTLOG_HOME}/include -L${FMTLOG_HOME}/lib64 -lfmtlog-shared
SPDLOG_OPTS = -I${SPDLOG_HOME}/include -L${SPDLOG_HOME}/lib64 -lspdlog -DSPDLOG_FMT_EXTERNAL -DSPDLOG_COMPILED_LIB
OPTIONS = -pthread -I../include -std=c++14 -DVHLOGGER_COMPILE_LEVEL=${LEVEL}

export SSLN_LOG_LEVEL=0
export SSLN_LOG_VERBOSE=3

build b : $(OBJ)


$(OBJ): ${SRCS} ${LOGGER_HEADER}
	g++ -g ${SRCS} \
		${LOGGER_SRCS} \
		${FMT_OPTS} \
		${SPDLOG_OPTS} \
		${OPTIONS} \
		-o $@

test:
	echo $@

run r: ${OBJ}
	./${OBJ}

run1 r1: ${OBJ}
	export SSLN_LOG_FILE=sock.log
	./${OBJ}
	@if [ -f sock.log ];then echo "Pass"; fi

clean:
	rm -rf $(VE) ${HE}


.PHONY: build run r clean test

