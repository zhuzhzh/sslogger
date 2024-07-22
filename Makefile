PREFIX = /home/public/vgp
SRC = ./src/vhlogger.cc 
FMT_OPTS = -I${FMT_HOME}/include -L${FMT_HOME}/lib64 -lfmt
HEADER = ./include/vhlogger/vhlogger.h
DEP_HEADER = ./include/tl/optional.hpp
OPTIOns = -g

default install:
	mkdir -p $(PREFIX)/lib64 && g++ $(OPTIONS) -shared -fPIC -o $(PREFIX)/lib64/libvhlogger.so ${SRC} -I./include/ ${FMT_OPTS}
	mkdir -p ${PREFIX}/include/vgp && cp ${HEADER} ${PREFIX}/include/vgp
	mkdir -p ${PREFIX}/include/tl && cp ${DEP_HEADER} ${PREFIX}/include/tl
