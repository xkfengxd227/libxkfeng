CXX = g++

CXXFLAGS = -g -O3 -Wall -Wfatal-errors -Wshadow -I/usr/local/include
LDLIBS = -L/usr/lib -lstdc++ -lyael -lm
YEALFLAGS = -Wl,-rpath,/usr/local/lib

OBJ = main.o heap.o clustering.o hb.o dyarray.o common.o

all: main
main: $(OBJ)
	$(CXX) -o $@ $^ $(YEALFLAGS) $(LDLIBS)


#############################
# Dependencies

# for i in *.c ; do cpp -I.. -MM $i; done

main.o: main.cpp common.h hb.h clustering.h heap.h mainext.h
common.o: common.cpp common.h
heap.o: heap.cpp common.h
clustering.o: clustering.cpp common.h dyarray.h
hb.o: hb.cpp common.h clustering.h heap.h
dyarray.o: common.h

clean:
	rm -f  *.o 
	rm -f  *.gch 


