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

  // 构造函数，传递基本参数，初始化边存储空间
  NeighborGraph(int _n, int _d): n(_n), d(_d){};
  ~NeighborGraph(){};

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
