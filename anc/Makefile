CXX = g++

CXXFLAGS = -g -O3 -Wall -Wfatal-errors -Wshadow -I/usr/local/include
LDLIBS = -L/usr/lib -lstdc++ -lyael -lm
YEALFLAGS = -Wl,-rpath,/usr/local/lib

OBJ = main.o heap.o clustering.o anc.o dyarray.o common.o cost.o

all: main
main: $(OBJ)
	$(CXX) -o $@ $^ $(YEALFLAGS) $(LDLIBS)


#############################
# Dependencies

# for i in *.c ; do cpp -I.. -MM $i; done

main.o: main.cpp common.h anc.h cost.h clustering.h heap.h mainext.h
common.o: common.cpp cost.h
heap.o: heap.cpp common.h
clustering.o: clustering.cpp common.h dyarray.h
anc.o: anc.cpp common.h cost.h clustering.h heap.h
dyarray.o: common.h

clean:
	rm -f  *.o 
	rm -f  *.gch 


