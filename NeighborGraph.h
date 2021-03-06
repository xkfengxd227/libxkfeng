//
//  NeighborGraph.h
//  NeighborGraph class
//    desp: the number of neighbors is not constant
//
//  Created by 冯小康 on 2017/7/22.
//  Copyright ? 2018年 cui1001. All rights reserved.
//
#include "common.h"
#include <map>

#define     NonDGraphFlag       "NonD"  // 无向图的标志

class NeighborGraph{
public:
  int     n;                            // 数据集规模
  int     d;                            // 数据维度
  map<int, vector<int> > edge;          // 图中的边：一个节点ID-邻居ID列表
  map<int, vector<double> > edgedis;	// 图中的边的距离：同上
	
  bool	NorF;                           // Near-or-Far图的性质：true-近邻图；false-远邻图

  // 构造函数，传递基本参数，初始化边存储空间
  NeighborGraph(int _n, int _d, bool nf): n(_n), d(_d), NorF(nf){};
  ~NeighborGraph(){};
  
  /**
   * construction method - incrementally
   *    - means the i-th node can only find neighbor from the former i nodes 
   *    - [*] to make sure a neat kNN Graph, we seek K-NNs for the first K nodes from the whole database
          e.g., K=10, the first 10 nodes can get 9 neighbors at most, which make the graph not neat
   * @params 	v 	base vector
   * @params 	k 	number of nns to search
   */
   void construct_neat_graph_incrementally(const float *v, int k);
   
  /**
   * normal construct with knn search
   *	means seach knns for each node as their neighbors
   */
   // void construct_knn(const float *v, int k);

  /**
   * some converge style:
   *  best neighbor: check out all neighbors, and converge to the best
   *  greedy: converge once find a positive neighbors
   *  orientation: evaluate the quality of neighbors to converge
   *
   *  @param  start_id  id of the start point
   */
  // void converge_to_best_neighbor(const float *query, float *v, int d, int start_id, DoubleIndex &nn);
  // void converge_greedy(const float *query, float *v, int d, int start_id, DoubleIndex &nn);
  /**
   *  @param  osh   the random vector of the orientation sensitive hashing functions
   *  @param  m     number of hashing functions
   *  @param  okeys points' orientation bits
   */
  // void converge_based_on_orientation(const float *query, float *v, int d, int start_id, float *osh_a, int m, int *okeys, DoubleIndex &nn);

  
 
  /**
   *    普通的存储近邻图的方法
   *    说明：
   *        - 只能是txt格式的文件，文件名color1000SWNonDNNG.txt，意为Non Direct NNG
   *        - 一个存放ID，一个存放距离
   *        - 每一行一个数据点的邻居数据，拿ID为例，【count id1 id2 id3...】
   *        - 即先存一个邻居数量，再依次存放邻居ID
   *        - 默认行号即为该节点ID
   */
  void save_graph();
  
  /**
   *	kNN Graph的存储，用户判断图是kNN Graph，其存储方式略有不同
   *    说明：
   *        - 因为是kNN Graph，因此比较整齐（即每个数据点的邻居数目相同）
   *        - 因此除了按照txt格式存储，还可以额外存储ivecs和fvecs的二进制格式
   *        - 文本文件后缀为color1000SWNNG.txt, color_1000swnng.ivecs, color_1000swnngdis.fvecs
   */
  void save_knn_graph(const char *, const char*, int);

};
