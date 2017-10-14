CC = g++
CXXFLAGS = -g -O3 -Wall -Wfatal-errors -Wshadow -I/usr/local/include
LDLIBS = -L/usr/lib -lyael -lm

OBJ = main.o bitvector.o common.o

all: main
main: $(OBJ)
	$(CC) -o $@ $^ $(LDLIBS)

#############################
# Dependencies

# for i in *.cpp ; do cpp -I.. -MM $i; done

main.o: main.cpp common.h cost.h
bitvector.o: bitvector.cpp bitvector.h common.h

.PHONY: clean
clean:
	-rm -f $(OBJ)
