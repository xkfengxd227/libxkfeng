CC = g++
CXXFLAGS = -g -O3 -Wall -Wfatal-errors -Wshadow -I/usr/local/include
LDLIBS = -L/usr/lib -lyael -lm

OBJ = main.o common.o NeighborGraph.o

all: main
main: $(OBJ)
	$(CC) -o $@ $^ $(LDLIBS)

#############################
# Dependencies

# for i in *.cpp ; do cpp -I.. -MM $i; done

main.o: main.cpp common.h mainext.h NeighborGraph.h
NeighborGraph.o: NeighborGraph.h common.h

.PHONY: clean
clean:
	-rm -f $(OBJ)
