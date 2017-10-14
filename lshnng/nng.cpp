/*
Implementations of some operations on NNG

@author   xikafe
@date     2017-1-18
*/

#include "nng.h"
#include "common.h"
#include "heap.h"
#include <iostream>
#include <algorithm>
#include <vector>
using namespace std;
extern "C"{
  #include <yael/vector.h>
}

void converge_to_knn( const float *v, int n, int d,
                      const float *q,
                      const int *cand, int nc,
                      const int *G, int g,
                      int k,
                      int *knn, float *knndis, int *status){
  /* validity check */
  ASSERTINFO(   v == NULL || q == NULL || cand == NULL || G == NULL
                || n <= 0 || d <= 0 || nc <= 0 || g <= 0 || k <= 0
                || knn == NULL || knndis == NULL || status == NULL,

                "IPP");
  ASSERTINFO(k > n, "k should be smaller than n");

  int i;

  /* init */
  Heap candHeap(nc);                    // cand-min-heap
  Heap knnHeap(k);                      // knn-max-heap

  for(i = 0; i < k; i++){
    printf("%d: %d-%lf\n", i, knnHeap.elem[i].id, knnHeap.elem[i].val);
  }

  for(i = 0; i < n; i++){
    status[i] = CleanNode;              // all points are [clean]
  }

  /* fulfill candHeap with candidates */
  for(i = 0; i < nc; i++){
    DoubleIndex di = {cand[i], odistance(q, v+cand[i]*d, d) };
    candHeap.min_insert(&di);

    status[cand[i]] = CandidateNode;

    printf("%d(%d): %lf\n", i, candHeap.length, candHeap.topValue());
  }

  /* begin the convergence on nng */
  int oid, gid;
  float oqdis, gqdis;
  float knnRadius;                    // knn radius: current range of knns
  int   _count = 0;
  while(!candHeap.isempty()){
    DoubleIndex tmpdi;
    candHeap.min_extract(&tmpdi);
    oid = tmpdi.id;
    oqdis = tmpdi.val;

    /* try to add it to knnHeap */
    if(!knnHeap.isfull() || f_bigger(knnHeap.topValue(), oqdis)){
      if(knnHeap.isfull() && f_bigger(knnHeap.topValue(), oqdis)){
        _count++;
        printf("lucky...%d\n", _count);
      } 

      knnHeap.max_insert(&tmpdi);

      printf("%d: knn radius: %lf, oqdis: %lf\n", _count, knnHeap.topValue(), oqdis);
    }


    knnRadius = knnHeap.elem[0].val;
    if(status[oid] == CandidateNode){
      // for a candidate: traverse all neighbors
      for(i = 0; i < g; i++){
        gid = G[oid * g + i];

        if(status[gid] != CleanNode){
          // a neighber must be a clean node
          continue;
        }

        gqdis = odistance(q, v+gid*d, d);
        DoubleIndex di = {gid, gqdis};

        if(f_bigger(oqdis, gqdis)){
          // [candidate]: nearer to query than o
          status[gid] = CandidateNode;
          candHeap.min_expand(&di);
        }
        if(f_bigger(gqdis, knnRadius)){
          // [cast]: farther to query than knnRadius
          status[gid] = CastNode;
        }
      }// endfor

      // mark o [expanded]
      status[oid] = ExpandedNode;
    }// endif
  }// endwhile

  /* extract knns */
  DI_MergeSort(knnHeap.elem, 0, k-1);
  for(i = 0; i < k; i++){
    knn[i] = knnHeap.elem[i].id;
    knndis[i] = knnHeap.elem[i].val;
  }
}

void converge_to_knn_one_by_one( const float *v, int n, int d,
                      const float *q,
                      const int *cand, int nc,
                      const int *G, int g,
                      int k,
                      int *knn, float *knndis, int *status){
  /* validity check */
  ASSERTINFO(   v == NULL || q == NULL || cand == NULL || G == NULL
                || n <= 0 || d <= 0 || nc <= 0 || g <= 0 || k <= 0
                || knn == NULL || knndis == NULL || status == NULL,

                "IPP");
  ASSERTINFO(k > n, "k should be smaller than n");

  int i;

  /*  main stuctures: 
   *  - a max-heap to record the current k-NNs,
   *  - an array to record the neighbor nearest to [q] for each node 
   */
  Heap  knnHeap(k);                     // knn-max-heap
  Heap  candHeap(nc);
  int   *bestNeighborReg = ivec_new_set(n, -1);
  for(i = 0; i < n; i++){
    status[i] = CleanNode;              // all points are [clean]
  }

  /* init knnHeap with candidates */
  for(i = 0; i < nc; i++){
    DoubleIndex di = {cand[i], odistance(q, v+cand[i]*d, d) };

    knnHeap.max_insert_wisely(&di);
    status[cand[i]] = CandidateNode;
  }

  int oid, gid;                       // current node id, neighbor id
  float oqdis, gqdis;                 // dist(q, o), dist(q, g)
  float knnRadius;                    // knn radius: current range of knns
  DoubleIndex tmpdi;
  int gi;

  /* for each candidate, converge to its best */
  for(i = 0; i < nc; i++){
    oid = cand[i];

    // printf("--- cand-%d: %d ---\n", i, oid);

    while(true){

      //printf("oid: %d\tbest: %d\n", oid, bestNeighborReg[oid]);

      if(bestNeighborReg[oid] == oid){
        /* finish checking this candidate when the prior's best is the prior itself */
        break;
      }else if(-1 == bestNeighborReg[oid]){
        /* if the current node has not been expanded, expand it */
        // check all neighbors, find the best neighbor (the one nearest to [q])
        vector<DoubleIndex> neighbor_dist;
        for(gi = 0; gi < g; gi++){
          gid = G[oid*g + gi];
          tmpdi.id = gid;
          tmpdi.val = odistance(q, v + gid*d, d);
          neighbor_dist.push_back(tmpdi);

          status[gid] = CandidateNode;
        }
        sort(neighbor_dist.begin(), neighbor_dist.end(), dicomp_asc);

        // try to update the knns with all neighbors
        gi = 0;
        while(f_bigger(knnHeap.topValue(), neighbor_dist[gi].val)){
          knnHeap.max_insert(&neighbor_dist[gi]);
          gi++;
        }

        // reg the best neighbor of [oid]: a better neighbor or itself
        oqdis = odistance(q, v + oid*d, d);
        if(f_bigger(oqdis, neighbor_dist[0].val)){
          bestNeighborReg[oid] = neighbor_dist[0].id;
        }else{
          bestNeighborReg[oid] = oid;
        }

/*
        printf("oid-quality: %d-%lf\n", oid, oqdis);
        for(int j = 0; j < g; j++){
          cout << neighbor_dist[j].id << "-" << neighbor_dist[j].val << " ";
        }
        cout << endl;
*/
      }else{  
        /* move to a newer node */
        oid = bestNeighborReg[oid];
      }
    }
  }// [end while]

  /* extract knns */
  DI_MergeSort(knnHeap.elem, 0, k-1);
  for(i = 0; i < k; i++){
    knn[i] = knnHeap.elem[i].id;
    knndis[i] = knnHeap.elem[i].val;
  }
}

