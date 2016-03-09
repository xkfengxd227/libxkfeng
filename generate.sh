# clear 
sudo rm *.o
sudo rm *.a

# compile basic classes
sudo gcc -c clustering.c
sudo gcc -c common.c
sudo gcc -c cost.c
sudo gcc -c heap.c
sudo gcc -c toolstype.c
sudo gcc -c HB.c
sudo gcc -c HBPlus.c
sudo gcc -c vafile.c

# libxkfeng: clustering, common, cost, heap, toolstype
sudo ar -crs libxkfeng.a clustering.o common.o cost.o heap.o toolstype.o

# libxkfenghb includes classes that HB (or HBPlus) needs (include elements in libxkfeng)
sudo ar -crs libxkfenghb.a clustering.o common.o cost.o heap.o toolstype.o HB.o HBPlus.o

# libxkfengvafile: includes classes that VA-File class needs
sudo ar -crs libxkfengvafile.a heap.o cost.o common.o vafile.o
