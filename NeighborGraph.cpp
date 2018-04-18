//
//  NeighborGraph.cpp
//  FANNG类的实现
//
//  Created by 冯小康 on 2017/7/22.
//  Copyright © 2017年 cui1001. All rights reserved.
//
#include "NeighborGraph.h"
#include "common.h"
extern "C"{
	#include <yael/vector.h>
	#include <yael/nn.h>
	#include <yael/machinedeps.h>
}
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <algorithm>

bool DoubleIndex_Comp(DoubleIndex da, DoubleIndex db){
	return (da.val <= db.val);
}

void NeighborGraph::construct_neat_graph_incrementally(const float *v, int k) {
	/* tools variables */
	DoubleIndex difoo;
	vector<int> neighbor;                           // neighbor ID list
    vector<double> neighbordis;                     // neighbor distance list
	double *nndist = NULL;
	int *nn = NULL;

	/* instance the edge and distance */
	edge.erase(edge.begin(), edge.end());
    edgedis.erase(edgedis.begin(), edgedis.end());

	/* find neighbors for each node */
	for (int i = 0; i < n; i++) {	
		// clear the neighbors
		neighbor.erase(neighbor.begin(), neighbor.end());
        neighbordis.erase(neighbordis.begin(), neighbordis.end());
		
		// the range to seek kNNs
		int seek_range = i;                             // NN seek range is set at i in default
		if(i < k){
			// node (0,...,k-1)'s neighbor from the whole set (may contain itself)
			seek_range = n;
		}

		// find k-NNs from the seek_range
		nn = ivec_new(k);
		nndist = knn(1, seek_range, d, k, v, v + i*d, nn);

		for (int ki = 0; ki < k; ki++) {                // copy IDs and distances into graph
			neighbor.push_back(nn[ki]);
			neighbordis.push_back(nndist[ki]);
		}

		// add edge set of the node-i
		edge.insert(pair<int, vector<int> >(i, neighbor));
        edgedis.insert(pair<int, vector<double> >(i, neighbordis));

		// release 
		FREE(nn);
		FREE(nndist);
	}
}


/* void NeighborGraph::converge_to_best_neighbor(const float *query, float *v, int d, int start_id, DoubleIndex &nn){
  /// looply check out all neighbors of the current node, converge to the best (nearest to the query)
  ///  end when no neighbor is better
  

  // current distance
  float cur_dis = odistance_square(query, v+start_id*d, d);

  // prepare neighbors 
  int nNeighbor = edges[start_id].size();
  float *vNeighbors = fvec_new(d * nNeighbor);
  // get coordinates of all neighbors
  for(int i = 0; i < nNeighbor; i++){
    int _nid = edge[start_id][i];
    memcpy(vNeighbors+i*d, v+_nid*d, sizeof(float)*d);
  }

  // best neighbor's id, dis
  int best_nid = -1;
  float best_dis = nn(1, nNeighbor, d, vNeighbors, query, &best_neighbor);

  // judge of converge 
  if(f_bigger(best_cur, cur_dis)){

  }
}*/
// void NeighborGraph::converge_greedy(const float *query, float *v, int d, int start_id, DoubleIndex &nn);
/**
 *  @param  osh   the random vector of the orientation sensitive hashing functions
 *  @param  m     number of hashing functions
 *  @param  okeys points' orientation bits
 */
// void NeighborGraph::converge_based_on_orientation(const float *query, float *v, int d, int start_id, float *osh_a, int m, int *okeys, DoubleIndex &nn);



void NeighborGraph::save_knn_graph(const char *indexfolder, const char *dsname, int K){
    char filename[255];

    // text format
    sprintf(filename, "%s/%s/%s%dSW.kNNG.txt", indexfolder, dsname, dsname, K);
    FILE *fp = open_file(filename, "w");

    int ib, ie;
    for(ib = 0; ib < n; ib++){
        for(ie = 0; ie < edge[ib].size(); ie++){
            fprintf(fp, "%d %d %lf\n", ib, edge[ib][ie], edgedis[ib][ie]);
        }
    }
    fclose(fp);

    // binary file - id
    sprintf(filename, "%s/%s/%s_%dswknng.ivecs", indexfolder, dsname, dsname, K);
    fp = open_file(filename, "wb");
    for(ib = 0; ib < n; ib++){
    	fwrite(&K, sizeof(int), 1, fp);
        for(ie = 0; ie < edge[ib].size(); ie++){
        	fwrite(&edge[ib][ie], sizeof(int), 1, fp);
        }
    }
    fclose(fp);

    // binary file -distance
    sprintf(filename, "%s/%s/%s_%dswknngdis.fvecs", indexfolder, dsname, dsname, K);
    fp = open_file(filename, "wb");
    for(ib = 0; ib < n; ib++){
    	fwrite(&K, sizeof(int), 1, fp);
        for(ie = 0; ie < edgedis[ib].size(); ie++){
        	float val = (float)edgedis[ib][ie];
        	fwrite(&val, sizeof(float), 1, fp);
        }
    }
    fclose(fp);
}


void load_graph(const char *filename){
	// open file
	FILE *fp = open_file(filename, "r");


}
