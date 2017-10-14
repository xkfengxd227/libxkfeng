CXX = g++

CXXFLAGS = -g -O3 -Wall -Wfatal-errors -Wshadow -I/usr/local/include
LDLIBS = -L/usr/lib -lstdc++ -lyael -lm
YEALFLAGS = -Wl,-rpath,/usr/local/lib

OBJ = main.o common.o cost.o clustering.o heap.o dyarray.o

all: main
main: $(OBJ)
	$(CXX) -o $@ $^ $(LDLIBS)

#############################
# Dependencies

# for i in *.c ; do cpp -I.. -MM $i; done

main.o: main.cpp common.h cost.h clustering.h heap.h mainext.h
heap.o: heap.cpp common.h
clustering.o: clustering.cpp common.h dyarray.h
common.o: common.cpp cost.h
dyarray.o: dyarray.cpp common.h
# HB.o: common.h cost.h clustering.h heap.h

clean:
	rm -f  *.o 
	rm -f  *.gch 


