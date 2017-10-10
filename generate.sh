clear
sudo rm *.o
sudo rm *.a

# compile basic classes
sudo g++ -c rand.cpp
sudo g++ -c clustering.cpp
sudo g++ -c common.cpp
sudo g++ -c cost.cpp
sudo g++ -c heap.cpp
sudo g++ -c dyarray.cpp
sudo g++ -c hb.cpp
sudo g++ -c hbplus.cpp
sudo g++ -c vafile.cpp
sudo g++ -c nng.cpp
sudo g++ -c lsh.cpp

# libxkfeng: clustering, common, cost, heap, toolstype
sudo ar -crs libxkfeng.a rand.o clustering.o common.o cost.o heap.o dyarray.o nng.o lsh.o

# libxkfenghb includes classes that HB (or HBPlus) needs (include elements in libxkfeng)
sudo ar -crs libxkfenghb.a clustering.o common.o cost.o heap.o dyarray.o hb.o hbplus.o

# libxkfengvafile: includes classes that VA-File class needs
sudo ar -crs libxkfengvafile.a heap.o cost.o common.o vafile.o

# libxkfeng
sudo ar -crs libxkfenglsh.a lsh.o common.o rand.o dyarray.o

#
mv *.a /usr/lib
cp *.h /usr/include/xkfeng
