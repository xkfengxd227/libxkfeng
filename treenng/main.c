/**
 *  hkm+nng:    use hkm to fastly locate at the nn's area
 *              use nng to guide the search process
 *
 *  author:     xkfeng
 *  time:       2016-4-8 10:56:02
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "/media/xikafe/dataset/dsbconfig.h"		// config of the data sets
#include "common.h"									
#include "cost.h"									// cost of the algorithm
#include "toolstype.h"
#include "mainext.h"
#include "hkmsupport.h"								// hierarchical clustering
#include <yael/hkm.h>
#include <yael/vector.h>
#define     G   1000            // out degree of the nodes in nng

/**
 *	\brief	point register
 */
typedef struct PointReg{
	int		id;			// point id
	int		status;		// point status: -1-unloaded, 0:loaded, 1:calculated, 2:opened
} PointReg;
/**
 *	\brief	cluster register
 */
typedef	struct ClusterReg{
	int 	id;
	int		status;		// cluster status: -1-unloaded, 0-loaded, 1-traversed
	DyArray	member;		// pointid that locates in this cluster
} ClusterReg;

int main(int argc, char *argv[]){
    /* basic parameters */
    DSBConfig   dscfg;
    int         bf,		// branch factor
    			 h,		// depth of hierarchical kmeans
    			 g,		// number of neighbors in the g-NNG
    			 n, 	// number of vectors in the base set
    			 d, 	// vector dimension
    			 nq, 
    			 dGT, 
    			 k, 
    			 pk;	/* 	prior k nns enjoys the privileges of 
    			 			traversing all points that share the same cluster with them */
    fDataSet    ds, qds;
    iDataSet    nng;
    hkm_t       *hkm = NULL;
	int			*assign = NULL;
	int			verbose;
	char		folder[255], filename[255];
	int			nclu = -1;
	int			i, ii;
	ClusterReg	*clureg = NULL;

    /* input parameters */
    _input_parameters(argc, argv, dscfg.dsname, &bf, &h, &g, &k, &pk);
    config_dataset(dscfg.dsname, &dscfg);
    n = dscfg.n;
    d = dscfg.d;
    nq = dscfg.nq;
    dGT = dscfg.dGT;

	/* check parameters */
	printf("treenng: %s -n %d -d %d -bf %d -h %d -G %d -g %d -k %d -pk %d\n",
		dscfg.dsname, n, d, bf, h, G, g, k, pk);

	/* load data set */
	fDataSet_init(&ds, n, d);
	fvecs_read(dscfg.basefile, d, n, ds.data);
	
    /* load/create the hkm clusters */
	verbose = 1;
	sprintf(folder, "%s/%s_bf%d_h%d", IndexFolder, dscfg.dsname, bf, h);
	makesure_folder(folder);
	if(!hkm_from_file(&hkm, &assign, folder, n, IndexData_Index | IndexData_Data)){
		/* index and/or data not exists, create */
		// learn
		hkm = hkm_learn(n, d, h, bf, ds.data, CLUSTERING_NITER, CLUSTERING_NTHREAD, verbose, &assign);
		// store the index and the data
		sprintf(filename, "%s/index.hkm", folder);
		hkm_write(filename, hkm);
		sprintf(filename, "%s/base_assign.ivecs", folder);
		ivecs_write(filename, 1, n, assign);
    }

	/* statistic the members in each cluster */
	// prepare the clu-reg list
	nclu = (int)pow(bf, h);
	clureg = (ClusterReg*)malloc(sizeof(ClusterReg)*nclu);
	for(i = 0; i < nclu; i++){
		clureg[i].id = i;
		DyArray_init(&clureg[i].member, sizeof(int), 2 * n / nclu);
	}
	// statistic
	for(i = 0; i < n; i++){
		DyArray_add(&clureg[assign[i]].member, (void*)&i, 1);
	}

    /* load nng data */
    iDataSet_init(&nng, n, g);
    iDataSet_load_partd(dscfg.nngfile, G, g, n, nng.data);

    /* search */
	PointReg 		*ptsreg = (PointReg*)malloc(sizeof(PointReg)*n);
	DoubleIndex		*knn = NULL, **knns = NULL, *gt = NULL;
	DoubleIndex		*dislist = NULL;
	int				*qidx = ivec_new(nq);
	int				qi;
	float			*qv = fvec_new(d);
	int				c0;
	int				pid;
	int				count, npoint, npts_load, _min, nknn, iknn, iclu, ipoint, total, ifresh, idng;
	int				*pnng = NULL;
	bool			startover = false;
	Cost			cost, icost;
	struct timeval			tbegin, tend, itb, ite;
	int				npk;

	// prepare the query
	fDataSet_init(&qds, nq, d);
	fvecs_read(dscfg.queryfile, d, nq, qds.data);
	
	// begin query
	CostInit(&cost);
	gettimeofday(&tbegin, NULL);

	hkm_quantize(hkm, nq, qds.data, qidx);
	knns = (DoubleIndex**)malloc(sizeof(DoubleIndex*)*nq);
    for(qi = 0; qi < nq; qi++){
		/* initialize: npoint, npts_load, nknn, qv, ptsreg, clureg, knn, cost recorder */
		CostInit(&icost);
		npoint = 0;
		npts_load = 0;
		nknn = 0;
        memcpy(qv, qds.data+qi*d, sizeof(float)*d);
		for(i = 0; i < n; i++){
			ptsreg[i].id = i;
			ptsreg[i].status = -1;
		}
		for(i = 0; i < nclu; i++){
			clureg[i].status = -1;
		}
		knn = dilist_new(k);

		/* firstly, check out c0's member to form the first patch of knn seeds */
		c0 = qidx[qi];
		// load all points in cluster-c0, calculate the distances
		count = clureg[c0].member.count;
		dislist = dilist_new(count);
		for(i = 0; i < count; i++){
			pid = *(int*)DyArray_get(&clureg[c0].member, i, 1);
			dislist[i].id = pid;
			dislist[i].val = odistance(qv, ds.data+pid*d, d);
			ptsreg[pid].status = 1;		/**< !!! update the i-th member point's status: calculated */
		}
		clureg[c0].status = 1;			/**< !!! update the c0-th cluster's status: traversed */
		icost.page = 1;
		npoint += count;
		npts_load += count;

		// form the original knn list (maybe unfull)
		DI_MergeSort(dislist, 0, count-1);
		nknn = i_min(k, count);
		memcpy(knn, dislist, sizeof(DoubleIndex)*nknn);
		dilist_unset(dislist);

		/*--------------- begin the loop of nn seeking directed by nng --------------*/
		while(true){
			/* initialize the nn pointer */
			iknn = 0;

			/* test the prior nns */
			// the realastic pk
			npk = i_min(pk, nknn);
			while(iknn < npk){
				iclu = assign[knn[iknn].id];
				/* a privilege cluster that is not traversed (impossible not loaded, 
				 * since the i-th nn belongs to the i-th cluster */
				if(clureg[iclu].status != 1){
					total = clureg[iclu].member.count;		// count of members in this cluster
					dislist = dilist_new(total + nknn);		// prepare dilist for nknn and new-coming total points
					memcpy(dislist, knn, sizeof(DoubleIndex)*nknn);
														// put the already knn at the head of dilist
					// pick up unloaded or uncalculated points to calculate
					ifresh = 0;
					for(i = 0; i < total; i++){
						pid = *(int*)DyArray_get(&clureg[iclu].member, i, 1);
						if(ptsreg[pid].status <= 0){
							// the point has not been loaded or been calculated	
							dislist[nknn+ifresh].id = pid;
							dislist[nknn+ifresh].val = odistance(qv, ds.data+pid*d, d);
							ptsreg[pid].status = 1;		/**< !!! update the i-th member point's status: calculated */
							ifresh++;
						}
					}
					// update the status of the cluster
					clureg[iclu].status = 1;			/**< !!! update the iclu-th cluster's status: traversed */

					if(ifresh > 0){
						// there do exists new-coming points
						DI_MergeSort(dislist, 0, (nknn+ifresh-1));
						nknn = i_min(k, nknn+ifresh);
						memcpy(knn, dislist, sizeof(DoubleIndex)*nknn);

						dilist_unset(dislist);			/* release */
						npoint += ifresh;				/* record traversed points */

						// the knn list has been updated, goto the begining of knn list
						npk = i_min(pk, nknn);
						iknn = 0;
					}else{
						// not fresh point, means this cluster has been opened, move to next one
						iknn++;
					}
				}else{
					/* move to next privilege cluster */
					iknn++;
				}
			}/* [end of privilege clusters] */

			/* the nng period: start from the first one, for each nn, expand them along the nng, to update knn */ 
			iknn = 0;
			dislist = dilist_new(k+g);		// the temp list cannot exceeds k+g
			startover = false;
			while(iknn < nknn){
				// if the i-th nn is not opened, open it
				ipoint = knn[iknn].id;
				if(ptsreg[ipoint].status != 2){
					// the i-th nn is not opened, extract its neighbors
					memcpy(dislist, knn, sizeof(DoubleIndex)*nknn);
					ifresh = 0;
					pnng = nng.data + ipoint * g;
					for(i = 0; i < g; i++){
						idng = pnng[i];
						if(ptsreg[idng].status >= 1){
							// a neighbor has been calculated or is opened, pass
							continue;
						}else if(ptsreg[idng].status == -1){
							// a neighbor has not been loaded, load the cluster
							iclu = assign[idng];
							total = clureg[iclu].member.count;
							for(ii = 0; ii < total; ii++){
								pid = *(int*)DyArray_get(&clureg[iclu].member, ii, 1);
								ptsreg[pid].status = 0;
							}
							clureg[iclu].status = 0;			/**< !!! update the iclu-th cluster's status: loaded */
							icost.page++;						/* record a new loaded cluster */
							npts_load += total;
						}
							
						// confirm the neighbor is a fresh one
						dislist[nknn+ifresh].id = idng;
						dislist[nknn+ifresh].val = odistance(qv, ds.data+idng*d, d);
						ptsreg[idng].status = 1;				/**< !!! update the idng-th point's status: calculated */
						ifresh++;						
					}
					ptsreg[ipoint].status = 2;					/**< !!! update the ipoint-th point's status: opened */

					// a least one fresh point, update the knn, and goto the startover
					if(ifresh > 0){
						DI_MergeSort(dislist, 0, nknn+ifresh-1);
						nknn = i_min(k, nknn+ifresh);
						memcpy(knn, dislist, sizeof(DoubleIndex)*nknn);

						// goto the begining to verify the knn[0]
						npoint += ifresh;

						startover = true;
						break;
					}
				}

				// update the nn pointer
				iknn++;
			}/* [end of the traverse of knn loop]  */
			dilist_unset(dislist);

			// check to startover or end of loop
			if(startover){
				continue;
			}else{
				break;
			}
		}/* [end of while] */

		/* checkout the number of nn */
		ASSERTINFO(nknn < k, "insufficient nn found\n");
		
		knns[qi] = knn;
		knn = NULL;

		icost.counter[0] = npts_load;		
		icost.point = npoint;
		CostCombine(&cost, &icost);
    }/* [end the loop of query */
	gettimeofday(&tend, NULL);
	cost.search = timediff(tbegin, tend);
	CostMultiply(&cost, 1 / (float)nq);


	/* accuracy of knn */
	gt = dilist_new(nq * dscfg.dGT);
	load_groundtruth(dscfg.gtfile, dscfg.gtdisfile, dscfg.dGT, nq, gt);

	/* --------------------- report the performance ------------------------ */
	char	envstr[255];
	sprintf(envstr, "%s -bf %d -h %d -g %d -k %d -pk %d\n", dscfg.dsname, bf, h, g, k, pk);

	int		nklist[11] = {1, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
	for(i = 0; i < 11; i++){
		if(nklist[i] <= k){
			printf("rank %d:\t%g\n", nklist[i], knn_ratio(knns, gt, nq, nklist[i], dGT, 0));
		}
	}
	performance_report(knns, nq, k, gt, dGT, cost, envstr);

    /* release */
	fDataSet_unset(&ds);
	fDataSet_unset(&qds);
	iDataSet_unset(&nng);
	hkm_delete(hkm);
	if(assign != NULL){
		free(assign);	assign = NULL;
	}
	if(clureg != NULL){	free(clureg);	clureg = NULL;	}
	if(ptsreg != NULL){	free(ptsreg);	ptsreg = NULL;	}
	knn = NULL;
	if(knns != NULL){
		for(i = 0; i < nq; i++){
			if(knns[i] != NULL){	free(knns[i]);	knns[i] = NULL;	}
		}
		free(knns);	knns = NULL;
	}
	if(gt != NULL){		free(gt);		gt = NULL;		}
	if(qidx != NULL){	free(qidx);		qidx = NULL;	}
	pnng = NULL;
    return 0;
}
