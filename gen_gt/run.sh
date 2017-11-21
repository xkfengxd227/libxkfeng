ds=color
dsbasepath=/media/xikafe/dataset
vfile=${dsbasepath}/${ds}/bin/${ds}_base.fvecs
qfile=${dsbasepath}/${ds}/bin/${ds}_base.fvecs
n=67040
d=32
nq=67040
dGT=1000
type=nng # gt: groundtruth, nng: nng

./main -type ${type} -ds ${ds} -v ${vfile} -q ${qfile} -n $n -d $d -nq ${nq} -dGT ${dGT}
