PREFIX = /home/public/vgp
SRC = ./src/vhlogger.cc 
HEADER = ./vhlogger/vhlogger.h

default install:
	g++  -o $(PREFIX)/lib64/libvhlogger.so ${SRC} -I./include/vhlogger
	mkdir -p ${PREFIX}/include/vgp && cp ${HEADER} ${PREFIX}/include/vgp
