ds=glove200d
dsbasepath=/media/xikafe/dataset
vfile=${dsbasepath}/${ds}/bin/${ds}s1000_base.fvecs
qfile=${dsbasepath}/${ds}/bin/${ds}s1000_query.fvecs
n=1192514
d=200
nq=1000
dGT=1000

./main -ds ${ds} -v ${vfile} -q ${qfile} -n $n -d $d -nq ${nq} -dGT ${dGT}
