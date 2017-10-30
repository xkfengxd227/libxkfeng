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
}
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>

void construct_incremental(const float *v, int k) {
	/* tools variables */
	DoubleIndex difoo;
	vector<DoubleIndex> neighbors;

	/* instance the edges */
	edges.erase(edges.begin(), edges.end());

	/* find neighbors for each node */
	for (int i = 1; i < n; i++) {	
	// start from the 2nd node, since the 1st has no
	// neighbor according to the principle
		// clear the neighbors
		neighbors.erase(neighbors.begin(), neighbors.end());
		
		if (i <= k) {
		// node (1-k)'s neighbor is their former nodes
			for (int gi = 0; gi < i; gi++) {
				difoo.id = gi;
				difoo.value = odistance(v + i*d, v + gi*d, d);

				neighbors.push_back(difoo);
			}
		}
		else {
		// find nns in the former i nodes (i.e., [0,i-1])
			int *nn = ivec_new(k);
			float *nndist = knn(1, i, d, k, v + i*d, v, nn);

			for (int ki = 0; ki < k; ki++) {
				difoo.id = nn[ki];
				difoo.val = nndist[ki];

				neighbors.push_back(difoo);
			}
		}

		// add edge set of the node-i
		edges.insert(pair<int, vector<DoubleIndex>>(i, neighbors));
	}
}


converge_to_best_neighbor(const float *query, float *v, int d, int start_id){
  /** looply check out all neighbors of the current node, converge to the best (nearest to the query)
   *  end when no neighbor is better
   */

  // current distance
  float cur_dis = odistance_square(query, v+start_id*d, d);

  /* prepare neighbors */
  int nNeighbor = edges[start_id].size();
  float *vNeighbors = fvec_new(d * nNeighbor);
  // get coordinates of all neighbors
  for(int i = 0; i < nNeighbor; i++){
    int _nid = edges[start_id][i].id;
    memcpy(vNeighbors+i*d, v+_nid*d, sizeof(float)*d);
  }

  // best neighbor's id, dis
  int best_nid = -1;
  float best_dis = nn(1, nNeighbor, d, vNeighbors, query, &best_neighbor);

  /* judge of converge */
  if(f_bigger(best_cur, cur_dis)){

  }

  
}
converge_greedy(const float *query, float *v, int d, int start_id);
/**
 *  @param  osh   the random vector of the orientation sensitive hashing functions
 *  @param  m     number of hashing functions
 *  @param  okeys points' orientation bits
 */
converge_based_on_orientation(const float *query, float *v, int d, int start_id, float *osh_a, int m, int *okeys);



void NeighborGraph::nng_into_files(const char *filename){
    FILE *fp = fopen(filename, "w");
    if(fp == NULL){
        printf("failed to open file\n");
        return;
    }

    int ib, ie;
    for(ib = 0; ib < n; ib++){
        for(ie = 0; ie < edges[ib].size(); ie++){
            fprintf(fp, "%d %d %lf\n", ib, edges[ib][ie].id, edges[ib][ie].val);
        }
    }
    fclose(fp);
}
