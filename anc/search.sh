dsname=sift1M
h=1
nq=200
k=100
g=2

for K in 20 40 60 80 100 200 400 600 800 1000 2000 4000 6000 8000 10000
do
	./main -ds ${dsname} -alg 1 -K ${K} -h ${h} -g ${g} -nq ${nq} -k ${k}

done