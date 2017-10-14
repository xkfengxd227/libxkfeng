dsname=aerial
h=1
nq=200
k=100

for K in 100	200	300	400	500	600	700	800	900	1000 2000 3000 4000 5000
do
	./main -ds ${dsname} -alg 11 -K ${K} -h ${h} -nq ${nq} -k ${k}

done