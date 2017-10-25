//
//  NeighborGraph.h
//  NeighborGraph class
//    desp: the number of neighbors is not constant
//
//  Created by 冯小康 on 2017/7/22.
//  Copyright © 2017年 cui1001. All rights reserved.
//
class NeighborGraph{
public:
  int     n;                  // 数据集规模
  int     d;                  // 数据维度
  map<int, vector<DoubleIndex> > edges;
                              // 图中所有有向边倒排列表：id为记录，以id为起点的边为列表
	bool	NorF;				// Near-or-Far图的性质：true-近邻图；false-远邻图

  // 构造函数，传递基本参数，初始化边存储空间
  NeighborGraph(int _n, int _d, bool nf): n(_n), d(_d), NorF(nf){};
  ~NeighborGraph(){};
  
  /**
   * construction method - incrementally
   *	means the neighbor of the i-th node can only be found 
   * 	from the former i nodes 
   * @params 	v 	base vector
   * @params 	k 	number of nns to search
   */
   void construct_incremental(const float *v, int k);
   
  /**
   * normal construct with knn search
   *	means seach knns for each node as their neighbors
   */
   void construct_knn(const float *v, int k);

  /**
   * some converge style:
   *  best neighbor: check out all neighbors, and converge to the best
   *  greedy: converge once find a positive neighbors
   *  orientation: evaluate the quality of neighbors to converge
   *
   *  @param  start_id  id of the start point
   */
  converge_to_best_neighbor(const float *query, float *v, int d, int start_id, DoubleIndex &nn);
  converge_greedy(const float *query, float *v, int d, int start_id, DoubleIndex &nn);
  /**
   *  @param  osh   the random vector of the orientation sensitive hashing functions
   *  @param  m     number of hashing functions
   *  @param  okeys points' orientation bits
   */
  converge_based_on_orientation(const float *query, float *v, int d, int start_id, float *osh_a, int m, int *okeys, DoubleIndex &nn);

  // 输出所有边：起点 终点 长度
  void nng_into_files(const char *filename);

};
