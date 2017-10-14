//
//  NeighborGraph.h
//  NeighborGraph class
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

    // 输出所有边：起点 终点 长度
    void nng_into_files(const char *filename);

};