CC = g++
CXXFLAGS = -g -O3 -Wall -Wfatal-errors -Wshadow -I/usr/local/include
LDLIBS = -L/usr/lib -lyael -lm

OBJ = main.o common.o nng.o lsh.o heap.o rand.o

all: main
main: $(OBJ)
	$(CC) -o $@ $^ $(LDLIBS)

#############################
# Dependencies

# for i in *.cpp ; do cpp -I.. -MM $i; done

main.o: main.cpp common.h lsh.h mainext.h nng.h
nng.o: nng.cpp heap.h common.h
lsh.o: lsh.cpp rand.h common.h

.PHONY: clean
clean:
	-rm -f $(OBJ)
