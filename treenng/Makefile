CC = gcc

CFLAGS = -g -O0 -Wall -Wfatal-errors -Wshadow -I/usr/local/include
LDLIBS = -L/usr/lib -lstdc++ -lyael -lm
YEALFLAGS = -Wl,-rpath,/usr/local/lib

all: main

main: main.o cost.o mainext.o common.o toolstype.o hkmsupport.o
	$(CC) -o $@ $^ $(YEALFLAGS) $(LDLIBS)

#################
# Dependences

main.o: mainext.h common.h toolstype.h cost.h hkmsupport.h
toolstype.o: common.h
mainext.o: common.h
common.o: cost.h common.c
hkmsupport.o: common.h

# 隐晦规则
.PHONY: clean

clean:
	@echo 正在删除
	-rm -f  *.o
	-rm -f  *.gch
