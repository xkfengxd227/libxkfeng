dsname=sift1M
h=1
nq=200
k=100

for K in 20 40 60 80 100 200 400 600 800 1000 2000 4000 6000 8000 10000
do
	./main -ds ${dsname} -alg 12 -K ${K} -h ${h} -nq ${nq} -k ${k}

done